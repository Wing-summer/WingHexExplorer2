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

#include "QConsoleWidget/QConsoleWidget.h"
#include "QConsoleWidget/commandhistorymanager.h"
#include "angelscript.h"
#include "clangformatmanager.h"
#include "control/toast.h"
#include "crashhandler.h"
#include "dbghelper.h"
#include "define.h"
#include "dialog/mainwindow.h"
#include "dialog/splashdialog.h"
#include "inspectqtloghelper.h"
#include "languagemanager.h"
#include "logger.h"
#include "settingmanager.h"
#include "skinmanager.h"
#include "utilities.h"
#include "wingmessagebox.h"

#include <KSyntaxHighlighting/Repository>

AppManager *AppManager::_instance = nullptr;

AppManager::AppManager(int &argc, char *argv[])
    : QtSingleApplication(argc, argv) {
    ASSERT_SINGLETON;

    LanguageManager::instance();
    InspectQtLogHelper::instance().init();
    CrashHandler::instance().init();

    auto args = arguments();
    if (this->isRunning()) {
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

    auto &set = SettingManager::instance();
    Logger::instance();
    QFont font(set.appFontFamily(), set.appfontSize());
    setFont(font);

    SkinManager::instance();

    // add angelscript highlight support
    auto &repo = WingCodeEdit::syntaxRepo();
    repo.addCustomSearchPath(QStringLiteral(":/WingScript/Angelscript"));

    auto dontSplash = set.dontUseSplash();

    SplashDialog *splash = nullptr;
    if (!dontSplash) {
        splash = new SplashDialog;
        splash->setInfoText(tr("SetupClang"));
        ClangFormatManager::instance();
    }

    auto cmdlist = CommandHistoryManager::load();
    auto &his = QConsoleWidget::history();
    for (auto &cmd : cmdlist) {
        his.add(cmd);
    }

    _w = new MainWindow(splash);

    setActivationWindow(_w);

    connect(this, &QtSingleApplication::messageReceived, this,
            [this](QByteArray message) {
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
            });

    if (splash)
        splash->setInfoText(tr("OpeningFiles"));

    if (args.size() > 1) {
        for (auto var = args.begin() + 1; var != args.end(); ++var) {
            openFile(*var);
        }
    }

    connect(_w, &MainWindow::closed, this,
            []() { AppManager::instance()->exit(); });
    _instance = this;

    if (splash)
        splash->close();
}

AppManager::~AppManager() {
    InspectQtLogHelper::instance().destory();
    ClangFormatManager::instance().save();
    // CommandHistoryManager::save(QConsoleWidget::history().strings_);

    _w->deleteLater();
    _w = nullptr;
}

AppManager *AppManager::instance() { return _instance; }

MainWindow *AppManager::mainWindow() const { return _w; }

void AppManager::openFile(const QString &file, bool autoDetect) {
    EditorView *editor = nullptr;
    Q_ASSERT(_w);

    ErrFile ret = ErrFile::Error;
    if (autoDetect) {
        ret = _w->openWorkSpace(file, &editor);
    }
    if (ret == ErrFile::Error) {
        ret = _w->openFile(file, &editor);

        if (ret == ErrFile::AlreadyOpened) {
            Q_ASSERT(editor);
            if (_w->currentEditor() == editor) {
                Toast::toast(_w, NAMEICONRES("openapp"), tr("AlreadyOpened"));
            } else {
                editor->raise();
            }

            editor->setFocus();
        }
    } else if (ret == ErrFile::AlreadyOpened) {
        Q_ASSERT(editor);
        if (_w->currentEditor() == editor) {
            Toast::toast(_w, NAMEICONRES("openapp"), tr("AlreadyOpened"));
        } else {
            editor->raise();
        }

        editor->setFocus();
    }
}

void AppManager::openRawFile(const QString &file) {
    EditorView *editor = nullptr;
    Q_ASSERT(_w);
    auto ret = _w->openFile(file, &editor);
    if (ret == ErrFile::AlreadyOpened) {
        Q_ASSERT(editor);
        if (_w->currentEditor() == editor) {
            Toast::toast(_w, NAMEICONRES("openapp"), tr("AlreadyOpened"));
        } else {
            editor->raise();
        }

        editor->setFocus();
    }
}

void AppManager::openWorkSpace(const QString &ws) {
    EditorView *editor = nullptr;
    Q_ASSERT(_w);
    auto ret = _w->openWorkSpace(ws, &editor);
    if (ret == ErrFile::AlreadyOpened) {
        Q_ASSERT(editor);
        if (_w->currentEditor() == editor) {
            Toast::toast(_w, NAMEICONRES("openapp"), tr("AlreadyOpened"));
        } else {
            editor->raise();
        }

        editor->setFocus();
    }
}
