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

#include "showinshell.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

#ifdef Q_OS_LINUX
#include <QDBusInterface>
#include <QUrl>
#endif

#include "wingmessagebox.h"

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <QAxObject>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QTextBrowser>
#include <QUuid>
#include <ShObjIdl.h>
#include <ShlDisp.h>
#include <Windows.h>

bool ShowInShell::showInWindowsShell(const QString &filePath, bool deselect) {
    QFileInfo appFI(filePath);
    auto matchPath = appFI.dir().path().toLower();
    auto matchName = appFI.fileName().toLower();

    QAxObject shellApp("Shell.Application");

    QAxObject *windows = shellApp.querySubObject("Windows()");
    auto count = windows->dynamicCall("Count()").toInt();
    for (int i = 0; i < count; ++i) {
        QAxObject *win = windows->querySubObject("Item(QVariant)", {i});
        auto program = win->dynamicCall("FullName()").toString();
        QFileInfo programFI(program);
        if (programFI.baseName().toLower() != "explorer")
            continue;
        auto url = win->dynamicCall("LocationURL()").toUrl();
        if (!url.isLocalFile())
            continue;
        auto path = url.path().mid(1).toLower();
        if (path != matchPath)
            continue;

        QAxObject *doc = win->querySubObject("Document()");
        QAxObject *folder = doc->querySubObject("Folder()");
        QAxObject *folderItems = folder->querySubObject("Items()");

        QAxObject *ourEntry = {};
        int count = folderItems->dynamicCall("Count()").toInt();
        for (int j = 0; j < count; j++) {
            QAxObject *entry = folderItems->querySubObject("Item(QVariant)", j);
            auto name = entry->dynamicCall("Name()").toString().toLower();
            if (name == matchName)
                ourEntry = entry;
        }
        if (ourEntry) {
            doc->dynamicCall("SelectItem(QVariant, int)", ourEntry->asVariant(),
                             SVSI_SELECT |
                                 (deselect ? SVSI_DESELECTOTHERS : 0));
            auto hwnd = win->dynamicCall("HWND()").toLongLong();
            BringWindowToTop(HWND(hwnd));
            return true;
        }
    }
    return false;
}

#else
bool ShowInShell::showInWindowsShell(const QString &filePath, bool deselect) {
    Q_UNUSED(filePath);
    Q_UNUSED(deselect);
    return false;
}
#endif

#ifdef Q_OS_LINUX
bool openFreedesktopFileManager(const QString &path) {
    if (!QDBusConnection::sessionBus().isConnected()) {
        return false;
    }
    const QLatin1String serviceName =
        QLatin1String("org.freedesktop.FileManager1");
    const QLatin1String objectPath =
        QLatin1String("/org/freedesktop/FileManager1");
    const QLatin1String interfaceName =
        QLatin1String("org.freedesktop.FileManager1");
    QStringList uriList = {QUrl::fromLocalFile(path).toString()};
    QString startupId;
    QDBusInterface iface(serviceName, objectPath, interfaceName,
                         QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        return false;
    }
    QDBusMessage reply =
        iface.call(QLatin1String("ShowItems"), uriList, startupId);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qDebug() << "Received DBus error:" << reply.errorName()
                 << reply.errorMessage();
        return false;
    }
    return true;
}
#else
bool openFreedesktopFileManager(const QString &path) {
    Q_UNUSED(path);
    return false;
}
#endif

bool ShowInShell::showInGraphicalShell(QWidget *parent, const QString &pathIn,
                                       bool deselect) {
    const QFileInfo fileInfo(pathIn);
    // Mac, Windows support folder or file.
    if constexpr (HostOsInfo::isWindowsHost()) {
        if (showInWindowsShell(pathIn, deselect))
            return true;
        const auto explorer =
            QStandardPaths::findExecutable(QLatin1String("explorer.exe"));
        if (explorer.isEmpty()) {
            WingMessageBox::warning(
                parent,
                QApplication::translate("ShowInShell",
                                        "Launching Windows Explorer Failed"),
                QApplication::translate(
                    "ShowInShell",
                    "Could not find explorer.exe in path to launch "
                    "Windows Explorer."));
            return false;
        }
        QStringList param;
        if (!fileInfo.isDir())
            param += QLatin1String("/select,");
        param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
        return QProcess::startDetached(explorer, param);
    } else if constexpr (HostOsInfo::isMacHost()) {
        QStringList openArgs;
        openArgs << QLatin1String("-R") << fileInfo.canonicalFilePath();
        int rc = QProcess::execute(QLatin1String("/usr/bin/open"), openArgs);
        return rc != -2 && rc != 1;
    } else {
        if (!openFreedesktopFileManager(pathIn)) {
            // ok, fall back to xdg-open
            const QString folder = fileInfo.isDir()
                                       ? fileInfo.absoluteFilePath()
                                       : fileInfo.absolutePath();
            const QString app = QLatin1String("xdg-open");
            QProcess browserProc;
            bool success = browserProc.startDetached(app, {folder});
            const QString error =
                QString::fromLocal8Bit(browserProc.readAllStandardError());
            success = success && error.isEmpty();
            return success;
        }
        return true;
    }
}
