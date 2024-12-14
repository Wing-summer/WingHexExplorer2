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

#include "appmanager.h"

#include <QFont>

#include "angelscript.h"
#include "clangformatmanager.h"
#include "dbghelper.h"
#include "define.h"
#include "dialog/mainwindow.h"
#include "dialog/splashdialog.h"
#include "languagemanager.h"
#include "logger.h"
#include "settingmanager.h"
#include "skinmanager.h"
#include "utilities.h"
#include "wingmessagebox.h"

AppManager *AppManager::_instance = nullptr;

AppManager::AppManager(int &argc, char *argv[])
    : SingleApplication(argc, argv, true) {
    ASSERT_SINGLETON;

    auto args = arguments();
    if (isSecondary()) {
        QByteArray buffer;
        QDataStream out(&buffer, QIODevice::WriteOnly);
        if (args.size() > 1) {
            for (auto var = args.begin() + 1; var != args.end(); ++var) {
                out << *var;
            }
        }
        sendMessage(buffer);
        throw CrashCode::AlreadyStart;
    }

#ifndef ANGELSCRIPT_H
#error "You should include angelscript.h header to check the PORTABILITY"
#else
#ifdef AS_MAX_PORTABILITY
#error "Generic call is NOT FULLY SUPPORTED in WingHexExplorer2 library!"
#endif
#endif

    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY")) {
        WingMessageBox::critical(nullptr, qAppName(),
                                 tr("GenericCallNotFullySupported"));
        throw CrashCode::GenericCallNotSupported;
    }

    Logger::instance();

    auto &set = SettingManager::instance();
    QFont font(set.appFontFamily(), set.appfontSize());
    setFont(font);

    SkinManager::instance();
    LanguageManager::instance();

    auto dontSplash = set.dontUseSplash();

    SplashDialog *splash = nullptr;
    if (!dontSplash) {
        splash = new SplashDialog;
        splash->setInfoText(tr("SetupClang"));
        ClangFormatManager::instance();
    }

    _w = new MainWindow(splash);

    connect(this, &SingleApplication::instanceStarted, this, [this] {
        Q_ASSERT(_w);
        if (!_w->isEnabled()) {
            return;
        }
        _w->show();
        _w->activateWindow();
        _w->raise();
    });

    connect(this, &SingleApplication::receivedMessage, this,
            [this](quint32 instanceId, QByteArray message) {
                Q_UNUSED(instanceId);

                Q_ASSERT(_w);
                if (!_w->isEnabled()) {
                    return;
                }

                QDataStream out(&message, QIODevice::ReadOnly);
                while (!out.atEnd()) {
                    QString param;
                    out >> param;
                    openFile(param);
                }
                _w->show();
                _w->activateWindow();
                _w->raise();
            });

    if (splash)
        splash->setInfoText(tr("OpeningFiles"));

    if (args.size() > 1) {
        for (auto var = args.begin() + 1; var != args.end(); ++var) {
            openFile(*var);
        }
    }

    _instance = this;

    if (splash)
        splash->close();
}

AppManager::~AppManager() {
    ClangFormatManager::instance().save();

    _w->deleteLater();
    _w = nullptr;
}

AppManager *AppManager::instance() { return _instance; }

MainWindow *AppManager::mainWindow() const { return _w; }

void AppManager::openFile(const QString &file, bool autoDetect, qsizetype start,
                          qsizetype stop) {
    EditorView *editor = nullptr;
    Q_ASSERT(_w);
    if (Utilities::isStorageDevice(file)) {
        openDriver(file);
    } else {
        ErrFile ret = ErrFile::Error;
        if (autoDetect) {
            ret = _w->openWorkSpace(file, &editor);
        }
        if (ret == ErrFile::Error) {
            if (start >= 0 && stop > 0) {
                ret = _w->openRegionFile(file, &editor, start, stop);
            } else {
                ret = _w->openFile(file, &editor);
            }
            if (ret == ErrFile::AlreadyOpened) {
                Q_ASSERT(editor);
                editor->raise();
                editor->setFocus();
            }
        } else if (ret == ErrFile::AlreadyOpened) {
            Q_ASSERT(editor);
            editor->raise();
            editor->setFocus();
        }
    }
}

void AppManager::openRawFile(const QString &file) {
    EditorView *editor = nullptr;
    Q_ASSERT(_w);
    auto ret = _w->openFile(file, &editor);
    if (ret == ErrFile::AlreadyOpened) {
        Q_ASSERT(editor);
        editor->raise();
        editor->setFocus();
    }
}

void AppManager::openDriver(const QString &driver) {
    EditorView *editor = nullptr;
    if (Utilities::isStorageDevice(driver)) {
        auto ret = _w->openDriver(driver, &editor);
        if (ret == ErrFile::AlreadyOpened) {
            Q_ASSERT(editor);
            editor->raise();
            editor->setFocus();
        }
    }
}

void AppManager::openRegionFile(const QString &region, qsizetype start,
                                qsizetype length) {
    EditorView *editor = nullptr;
    Q_ASSERT(_w);
    auto ret = _w->openRegionFile(region, &editor, start, length);
    if (ret == ErrFile::AlreadyOpened) {
        Q_ASSERT(editor);
        editor->raise();
        editor->setFocus();
    }
}

void AppManager::openWorkSpace(const QString &ws) {
    EditorView *editor = nullptr;
    Q_ASSERT(_w);
    auto ret = _w->openWorkSpace(ws, &editor);
    if (ret == ErrFile::AlreadyOpened) {
        Q_ASSERT(editor);
        editor->raise();
        editor->setFocus();
    }
}
