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

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QPicture>
#include <QVBoxLayout>

#include "class/file/showinshell.h"
#include "class/plugin/pluginsystem.h"
#include "class/settings/thememanager.h"
#include "control/common/compositeiconengine.h"
#include "control/toast.h"
#include "utilities.h"

constexpr auto ICON_SIZE = 64;

inline static QString processBytesCount(qint64 bytescount) {
    static QStringList B{"B", "KB", "MB", "GB", "TB"};
    auto av = bytescount;
    auto r = av;

    for (int i = 0; i < 5; i++) {
        auto lld = lldiv(r, 1024);
        r = lld.quot;
        av = lld.rem;
        if (r == 0) {
            return QStringLiteral("%1 %2").arg(av).arg(B.at(i));
        }
    }

    return QStringLiteral("%1 TB").arg(av);
}

FileInfoDialog::FileInfoDialog(EditorView *editor, QWidget *parent)
    : FramelessDialogBase(parent), m_editor(editor) {
    Q_ASSERT(editor);

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    m_lblicon = new QLabel(this);
    m_lblicon->setFixedHeight(ICON_SIZE);
    m_lblicon->setAlignment(Qt::AlignCenter);

    m_tb = new QTextBrowser(this);
    m_tb->setOpenLinks(false);
    connect(m_tb, &QTextBrowser::anchorClicked, this, [this](const QUrl &url) {
        ShowInShell::showInGraphicalShell(this, url.toLocalFile(), false);
    });

    m_clnbtn = new QPushButton(tr("Workspace plugin data cleanup"), this);
    m_clnbtn->setVisible(editor->isWorkSpace());
    connect(m_clnbtn, &QPushButton::clicked, this, [this]() {
        m_editor->cleanUpPluginData();
        reloadData();
        Toast::toast(this,
                     ThemeManager::instance().standardPixmap(
                         ThemeManager::StandardIcon::Workspace),
                     tr("Workspace plugin data cleanup complete"));
    });

    layout->addWidget(m_lblicon, Qt::AlignHCenter);
    layout->addSpacing(10);
    layout->addWidget(m_tb);
    layout->addWidget(m_clnbtn);

    buildUpContent(widget);

    reloadData();

    setWindowTitle(tr("File information"));
    setWindowIcon(ThemeManager::instance().standardIcon(
        ThemeManager::StandardIcon::Information));
    this->resize(500, 450);
}

FileInfoDialog::~FileInfoDialog() {}

void FileInfoDialog::reloadData() {
    m_tb->clear();

    static const QString dfmt("yyyy/MM/dd hh:mm:ss ddd");

    static constexpr auto resetBrowserCursor = [](QTextBrowser *b) {
        auto cursor = b->textCursor();
        cursor.setCharFormat({});
        b->setTextCursor(cursor);
    };

    QIcon icon;
    auto url = m_editor->fileNameUrl();
    static auto sep = QStringLiteral(" : ");
    static auto link = QStringLiteral("<a href=\"%1\" title=\"%3\">%2</a>");
    auto tt = tr("Show in file manager");
    if (url.isLocalFile()) {
        if (EditorView::isNewFileUrl(url)) {
            icon = this->style()->standardIcon(QStyle::SP_FileIcon);
            m_tb->append(tr("This file has not been saved yet."));
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

            m_tb->append(tr("File name") + sep + finfo.fileName());
            m_tb->append(tr("File path") + sep);
            m_tb->insertHtml(
                link.arg(url.toString(QUrl::FullyDecoded), fileName, tt));
            resetBrowserCursor(m_tb);
            if (m_editor->isWorkSpace()) {
                auto ws = m_editor->workSpaceName();
                QFileInfo winfo(ws);
                m_tb->append(tr("Workspace") + sep + winfo.fileName());
                m_tb->append(tr("Workspace path") + sep);
                m_tb->insertHtml(link.arg(Utilities::getUrlString(ws), ws, tt));
                resetBrowserCursor(m_tb);
            }
            m_tb->append(tr("File size") + sep +
                         processBytesCount(finfo.size()));
            m_tb->append(tr("MIME type") + sep + t.name());
            m_tb->append(
                tr("Created") + sep +
                finfo.fileTime(QFile::FileTime::FileBirthTime).toString(dfmt));
            m_tb->append(
                tr("Last accessed") + sep +
                finfo.fileTime(QFile::FileTime::FileAccessTime).toString(dfmt));
            m_tb->append(tr("Last modified") + sep +
                         finfo.fileTime(QFile::FileTime::FileModificationTime)
                             .toString(dfmt));
            m_tb->append(tr("Last read") + sep +
                         finfo.lastRead().toString(dfmt));
            m_tb->append(tr("Last modified") + sep +
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
            icon = ThemeManager::instance().standardIcon(
                ThemeManager::StandardIcon::Device);
        }

        m_tb->append(tr("File path") + sep + url.authority() + url.path());
        m_tb->append(tr("File size") + sep +
                     processBytesCount(m_editor->hexEditor()->documentBytes()));
        if (m_editor->isWorkSpace()) {
            auto ws = m_editor->workSpaceName();
            QFileInfo winfo(ws);
            m_tb->append(tr("Workspace") + sep + winfo.fileName());
            m_tb->append(tr("Workspace path") + sep);
            m_tb->insertHtml(link.arg(Utilities::getUrlString(ws), ws, tt));
            resetBrowserCursor(m_tb);
        }

        if (dev) {
            auto info = plgsys.getPluginInfo(dev);
            m_tb->append(tr("Name") + sep + dev->pluginName());
            m_tb->append(tr("ID") + sep + info.id);
            m_tb->append(tr("License") + sep + info.license);
            m_tb->append(tr("Author") + sep + info.author);
            m_tb->append(tr("Vendor") + sep + info.vendor);
            m_tb->append(tr("Version") + sep + info.version.toString());
        }
    }

    QPixmap ico;
    if (m_editor->isWorkSpace()) {
        auto tc = m_tb->textCursor();
        tc.movePosition(QTextCursor::End);
        tc.insertBlock();

        const auto plgdata = m_editor->pluginDataNames();
        if (!plgdata.isEmpty()) {
            m_tb->append(tr("Workspace plugin data") + sep);
            for (const auto &d : plgdata) {
                m_tb->append(QStringLiteral(" - ") + d);
            }
        }

        CompositeIconEngine engine(ThemeManager::instance().standardIcon(
                                       ThemeManager::StandardIcon::Project),
                                   icon);
        ico = engine.pixmap({ICON_SIZE, ICON_SIZE}, QIcon::Normal,
                            QIcon::State::Off);
    } else {
        QPixmap pixmap(ICON_SIZE, ICON_SIZE);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        icon.paint(&painter, QRect(0, 0, ICON_SIZE, ICON_SIZE));
        ico = pixmap;
    }

    m_lblicon->setPixmap(ico);
    m_clnbtn->setEnabled(m_editor->needCleanUpPluginData());
}
