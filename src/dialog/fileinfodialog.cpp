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
#include "utilities.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QMimeDatabase>
#include <QPicture>
#include <QTextBrowser>
#include <QVBoxLayout>

FileInfoDialog::FileInfoDialog(EditorView *editor, QWidget *parent)
    : FramelessDialogBase(parent) {
    Q_ASSERT(editor);
    static const QString dfmt("yyyy/MM/dd hh:mm:ss ddd");

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    constexpr auto ICON_SIZE = 64;

    auto l = new QLabel(this);
    l->setFixedHeight(ICON_SIZE);
    l->setAlignment(Qt::AlignCenter);

    QIcon icon;
    auto b = new QTextBrowser(this);
    b->setOpenLinks(false);
    connect(b, &QTextBrowser::anchorClicked, this, [this](const QUrl &url) {
        ShowInShell::showInGraphicalShell(this, url.toLocalFile(), false);
    });

    auto resetBrowserCursor = [](QTextBrowser *b) {
        auto cursor = b->textCursor();
        cursor.setCharFormat({});
        b->setTextCursor(cursor);
    };

    auto url = editor->fileNameUrl();
    static auto sep = QStringLiteral(" : ");
    static auto link = QStringLiteral("<a href=\"%1\" title=\"%3\">%2</a>");
    auto tt = tr("ShowInShell");
    if (url.isLocalFile()) {
        if (EditorView::isNewFileUrl(url)) {
            icon = this->style()->standardIcon(QStyle::SP_FileIcon);
            b->append(tr("FileNew"));
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

            b->append(tr("FileName") + sep + finfo.fileName());
            b->append(tr("FilePath") + sep);
            b->insertHtml(
                link.arg(url.toString(QUrl::FullyDecoded), fileName, tt));
            resetBrowserCursor(b);
            if (editor->isWorkSpace()) {
                auto ws = editor->workSpaceName();
                QFileInfo winfo(ws);
                b->append(tr("Workspace") + sep + winfo.fileName());
                b->append(tr("WorkspacePath") + sep);
                b->insertHtml(link.arg(Utilities::getUrlString(ws), ws, tt));
                resetBrowserCursor(b);
            }
            b->append(tr("FileSize") + sep +
                      Utilities::processBytesCount(finfo.size()));
            b->append(tr("Mime") + sep + t.name());
            b->append(
                tr("FileBirthTime") + sep +
                finfo.fileTime(QFile::FileTime::FileBirthTime).toString(dfmt));
            b->append(
                tr("FileAccessTime") + sep +
                finfo.fileTime(QFile::FileTime::FileAccessTime).toString(dfmt));
            b->append(tr("FileModificationTime") + sep +
                      finfo.fileTime(QFile::FileTime::FileModificationTime)
                          .toString(dfmt));
            b->append(tr("LastRead") + sep + finfo.lastRead().toString(dfmt));
            b->append(tr("LastMod") + sep +
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

        b->append(tr("FilePath") + sep + url.authority() + url.path());
        b->append(
            tr("FileSize") + sep +
            Utilities::processBytesCount(editor->hexEditor()->documentBytes()));
        if (editor->isWorkSpace()) {
            auto ws = editor->workSpaceName();
            QFileInfo winfo(ws);
            b->append(tr("Workspace") + sep + winfo.fileName());
            b->append(tr("WorkspacePath") + sep);
            b->insertHtml(link.arg(Utilities::getUrlString(ws), ws, tt));
            resetBrowserCursor(b);
        }

        if (dev) {
            auto info = plgsys.getPluginInfo(dev);
            b->append(tr("Name") + sep + dev->pluginName());
            b->append(tr("ID") + sep + info.id);
            b->append(tr("License") + sep + info.license);
            b->append(tr("Author") + sep + info.author);
            b->append(tr("Vendor") + sep + info.vendor);
            b->append(tr("Version") + sep + info.version.toString());
        }
    }

    QPixmap ico;
    if (editor->isWorkSpace()) {
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

    l->setPixmap(ico);

    layout->addWidget(l, Qt::AlignHCenter);
    layout->addSpacing(10);
    layout->addWidget(b);

    buildUpContent(widget);

    setWindowTitle(tr("FileInfo"));
    setWindowIcon(ICONRES("info"));
    this->resize(500, 450);
}

FileInfoDialog::~FileInfoDialog() {}
