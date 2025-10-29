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
#include "class/pluginsystem.h"
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

    auto l = new QLabel(this);
    l->setFixedHeight(50);
    l->setAlignment(Qt::AlignCenter);

    QIcon icon;
    auto b = new QTextBrowser(this);

    auto url = editor->fileNameUrl();
    static auto sep = QStringLiteral(" : ");
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
            b->append(tr("FilePath") + sep + finfo.filePath());
            b->append(tr("FileSize") + sep +
                      Utilities::processBytesCount(finfo.size()));
            b->append(tr("Mime") + sep + t.name());
            b->append(tr("Md5") + sep +
                      Utilities::getMd5(fileName).toHex().toUpper());
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
        auto devs = plgsys.devices();
        auto r =
            std::find_if(devs.begin(), devs.end(), [plgID](IWingDevice *dev) {
                return plgID.compare(PluginSystem::getPUID(dev),
                                     Qt::CaseInsensitive) == 0;
            });

        IWingDevice *dev = nullptr;
        if (r != devs.end()) {
            dev = *r;
            icon = dev->supportedFileIcon();
        } else {
            icon = ICONRES(QStringLiteral("devext"));
        }

        b->append(tr("FilePath") + sep + url.authority() + url.path());
        b->append(
            tr("FileSize") + sep +
            Utilities::processBytesCount(editor->hexEditor()->documentBytes()));

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

    auto availSizes = icon.availableSizes();
    QList<int> availLens;
    for (auto &size : availSizes) {
        if (size.width() != size.height()) {
            continue;
        }
        availLens << size.width();
    }
    std::sort(availLens.begin(), availLens.end(), std::greater<int>());
    auto ico = icon.pixmap(availLens.front(), availLens.front())
                   .toImage()
                   .scaled(50, 50, Qt::AspectRatioMode::KeepAspectRatio,
                           Qt::TransformationMode::SmoothTransformation);
    l->setPixmap(QPixmap::fromImage(ico));

    layout->addWidget(l, Qt::AlignHCenter);
    layout->addSpacing(10);
    layout->addWidget(b);

    buildUpContent(widget);

    setWindowTitle(tr("FileInfo"));
    this->resize(500, 450);
}

FileInfoDialog::~FileInfoDialog() {}
