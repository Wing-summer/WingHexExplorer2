/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#include "fileinfodialog.h"
#include "class/compositeiconengine.h"
#include "class/pluginsystem.h"
#include "class/showinshell.h"
#include "control/toast.h"
#include "utilities.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QPicture>
#include <QVBoxLayout>

constexpr auto ICON_SIZE = 64;

FileInfoDialog::FileInfoDialog(EditorView *editor, QWidget *parent)
    : FramelessDialogBase(parent), _editor(editor) {
    Q_ASSERT(editor);

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    _lblicon = new QLabel(this);
    _lblicon->setFixedHeight(ICON_SIZE);
    _lblicon->setAlignment(Qt::AlignCenter);

    _tb = new QTextBrowser(this);
    _tb->setOpenLinks(false);
    connect(_tb, &QTextBrowser::anchorClicked, this, [this](const QUrl &url) {
        ShowInShell::showInGraphicalShell(this, url.toLocalFile(), false);
    });

    _clnbtn = new QPushButton(tr("CleanUpWSData"), this);
    _clnbtn->setVisible(editor->isWorkSpace());
    connect(_clnbtn, &QPushButton::clicked, this, [this]() {
        _editor->cleanUpPluginData();
        reloadData();
        Toast::toast(this, NAMEICONRES(QStringLiteral("workspace")),
                     tr("CleanUpWSDataOK"));
    });

    layout->addWidget(_lblicon, Qt::AlignHCenter);
    layout->addSpacing(10);
    layout->addWidget(_tb);
    layout->addWidget(_clnbtn);

    buildUpContent(widget);

    reloadData();

    setWindowTitle(tr("FileInfo"));
    setWindowIcon(ICONRES("info"));
    this->resize(500, 450);
}

FileInfoDialog::~FileInfoDialog() {}

void FileInfoDialog::reloadData() {
    _tb->clear();

    static const QString dfmt("yyyy/MM/dd hh:mm:ss ddd");

    static constexpr auto resetBrowserCursor = [](QTextBrowser *b) {
        auto cursor = b->textCursor();
        cursor.setCharFormat({});
        b->setTextCursor(cursor);
    };

    QIcon icon;
    auto url = _editor->fileNameUrl();
    static auto sep = QStringLiteral(" : ");
    static auto link = QStringLiteral("<a href=\"%1\" title=\"%3\">%2</a>");
    auto tt = tr("ShowInShell");
    if (url.isLocalFile()) {
        if (EditorView::isNewFileUrl(url)) {
            icon = this->style()->standardIcon(QStyle::SP_FileIcon);
            _tb->append(tr("FileNew"));
        } else {
            auto fileName = url.toLocalFile();
            QMimeDatabase db;
            auto t = db.mimeTypeForFile(fileName);
            auto ico = t.iconName();
            icon = QIcon::fromTheme(ico, QIcon(ico));
            if (icon.isNull()) {
                icon = this->style()->standardIcon(QStyle::SP_FileIcon);
            }

            QFileInfo finfo(fileName);

            _tb->append(tr("FileName") + sep + finfo.fileName());
            _tb->append(tr("FilePath") + sep);
            _tb->insertHtml(
                link.arg(url.toString(QUrl::FullyDecoded), fileName, tt));
            resetBrowserCursor(_tb);
            if (_editor->isWorkSpace()) {
                auto ws = _editor->workSpaceName();
                QFileInfo winfo(ws);
                _tb->append(tr("Workspace") + sep + winfo.fileName());
                _tb->append(tr("WorkspacePath") + sep);
                _tb->insertHtml(link.arg(Utilities::getUrlString(ws), ws, tt));
                resetBrowserCursor(_tb);
            }
            _tb->append(tr("FileSize") + sep +
                        Utilities::processBytesCount(finfo.size()));
            _tb->append(tr("Mime") + sep + t.name());
            _tb->append(
                tr("FileBirthTime") + sep +
                finfo.fileTime(QFile::FileTime::FileBirthTime).toString(dfmt));
            _tb->append(
                tr("FileAccessTime") + sep +
                finfo.fileTime(QFile::FileTime::FileAccessTime).toString(dfmt));
            _tb->append(tr("FileModificationTime") + sep +
                        finfo.fileTime(QFile::FileTime::FileModificationTime)
                            .toString(dfmt));
            _tb->append(tr("LastRead") + sep + finfo.lastRead().toString(dfmt));
            _tb->append(tr("LastMod") + sep +
                        finfo.lastModified().toString(dfmt));
        }
    } else {
        auto plgID = url.authority();
        auto file = url.path();
        if (file.front() == '/') {
            file.removeFirst();
        }

        auto &plgsys = PluginSystem::instance();

        IWingDevice *dev = plgsys.ext2Device(plgID);
        if (dev) {
            icon = dev->supportedFileIcon();
        } else {
            icon = ICONRES(QStringLiteral("devext"));
        }

        _tb->append(tr("FilePath") + sep + url.authority() + url.path());
        _tb->append(tr("FileSize") + sep +
                    Utilities::processBytesCount(
                        _editor->hexEditor()->documentBytes()));
        if (_editor->isWorkSpace()) {
            auto ws = _editor->workSpaceName();
            QFileInfo winfo(ws);
            _tb->append(tr("Workspace") + sep + winfo.fileName());
            _tb->append(tr("WorkspacePath") + sep);
            _tb->insertHtml(link.arg(Utilities::getUrlString(ws), ws, tt));
            resetBrowserCursor(_tb);
        }

        if (dev) {
            auto info = plgsys.getPluginInfo(dev);
            _tb->append(tr("Name") + sep + dev->pluginName());
            _tb->append(tr("ID") + sep + info.id);
            _tb->append(tr("License") + sep + info.license);
            _tb->append(tr("Author") + sep + info.author);
            _tb->append(tr("Vendor") + sep + info.vendor);
            _tb->append(tr("Version") + sep + info.version.toString());
        }
    }

    QPixmap ico;
    if (_editor->isWorkSpace()) {
        auto tc = _tb->textCursor();
        tc.movePosition(QTextCursor::End);
        tc.insertBlock();

        const auto plgdata = _editor->pluginDataNames();
        if (!plgdata.isEmpty()) {
            _tb->append(tr("WorkspacePluginData") + sep);
            for (const auto &d : plgdata) {
                _tb->append(QStringLiteral(" - ") + d);
            }
        }

        CompositeIconEngine engine(ICONRES("pro"), icon);
        ico = engine.pixmap({ICON_SIZE, ICON_SIZE}, QIcon::Normal,
                            QIcon::State::Off);
    } else {
        QPixmap pixmap(ICON_SIZE, ICON_SIZE);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        icon.paint(&painter, QRect(0, 0, ICON_SIZE, ICON_SIZE));
        ico = pixmap;
    }

    _lblicon->setPixmap(ico);
    _clnbtn->setEnabled(_editor->needCleanUpPluginData());
}
