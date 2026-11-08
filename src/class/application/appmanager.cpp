/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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
#include "startupfeedback.h"
#include "startuptrace.h"

#include <QFont>
#include <QScopeGuard>
#include <QSettings>

#include "QConsoleWidget/QConsoleWidget.h"
#include "QConsoleWidget/commandhistorymanager.h"
#include "class/settings/languagemanager.h"
#include "class/settings/settingmanager.h"
#include "class/settings/thememanager.h"
#include "control/toast.h"
#include "crashhandler.h"
#include "dbghelper.h"
#include "define.h"
#include "dialog/common/wingmessagebox.h"
#include "dialog/mainwindow.h"
#include "dialog/splashdialog.h"
#include "inspectqtloghelper.h"
#include "logger.h"
#include "luau/runtime/scriptmanager.h"
#include "settings/settings.h"
#include "utilities.h"

#include <KSyntaxHighlighting/Repository>

AppManager *AppManager::_instance = nullptr;

AppManager::AppManager(int &argc, char *argv[])
    : QtSingleApplication(argc, argv) {
    StartupTrace::mark("application.qt_constructed");
    StartupTrace::PhaseTimer startupPhases;
    ASSERT_SINGLETON;

    _instance = this;

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

    startupPhases.checkpoint("application.language_and_services");
    auto &set = SettingManager::instance();
    Logger::instance();
    QFont font(set.appFontFamily(), set.appfontSize());
    setFont(font);

    auto &theme = ThemeManager::instance();
    theme.setStyleSheetDeferred(true);
    auto finishStyle =
        qScopeGuard([&theme] { theme.setStyleSheetDeferred(false); });
    theme.setTheme(ThemeManager::Theme(set.themeID()));

    startupPhases.checkpoint("application.settings_and_theme");

    // add luau highlight support
    auto &repo = WingCodeEdit::syntaxRepo();
    repo.addCustomSearchPath(QStringLiteral(":/WingScript/Luau"));

    auto dontSplash = set.dontUseSplash();
    QString SPLASH_HISTORY_KEY;
    QVariantList splashHistory;

    if (!dontSplash) {
        // Separate histories for materially different startup configurations.
        SPLASH_HISTORY_KEY = QStringLiteral("startup.splash.%1.%2.%3.%4.%5")
                                 .arg(applicationVersion())
                                 .arg(set.themeID())
                                 .arg(set.scriptEnabled())
                                 .arg(set.enablePlugin())
                                 .arg(args.size() > 1);
        HANDLE_CONFIG;
        splashHistory = READ_CONFIG(SPLASH_HISTORY_KEY, {}).toList();
        splash = new SplashDialog(SplashDialog::DisplayMode::Automatic,
                                  StartupFeedback::estimate(splashHistory));
    }

    const auto cmdlist = CommandHistoryManager::load();
    auto &his = QConsoleWidget::history();
    for (const auto &cmd : cmdlist) {
        his.add(cmd);
    }

    _timer.start();

    startupPhases.checkpoint("application.syntax_history_and_splash");
    _w = new MainWindow(splash);
    startupPhases.checkpoint("application.mainwindow");
    // Applying QSS before restoring docks repeatedly restyles the same widget
    // subtrees as they are reparented. Polish the complete tree once instead.
    if (splash) {
        splash->prepareForBlockingPhase();
    }
    theme.setStyleSheetDeferred(false);
    finishStyle.dismiss();
    startupPhases.checkpoint("application.stylesheet");

    setActivationWindow(_w);

    connect(this, &QtSingleApplication::messageReceived, this,
            [this](QByteArray message) {
                Q_ASSERT(_w);
                if (!_w->isEnabled()) {
                    return;
                }

                QStringList failedFile;
                QDataStream out(&message, QIODevice::ReadOnly);
                while (!out.atEnd()) {
                    QString param;
                    out >> param;
                    bool isWs;
                    if (openFile(param, true, false, &isWs) !=
                        WingHex::Success) {
                        failedFile.append(param);
                    }
                }
                _w->show();

                if (!failedFile.isEmpty()) {
                    WingMessageBox::critical(
                        _w, tr("Error"),
                        tr("Could not open the following file. Check that it "
                           "exists and that you have permission to read it:") +
                            QStringLiteral("\n") + failedFile.join('\n'));
                }
            });

    if (splash) {
        splash->setInfoText(tr("Opening files"));
    }

    if (args.size() > 1) {
        QStringList failedFile;
        for (auto var = args.begin() + 1; var != args.end(); ++var) {
            auto file = *var;
            bool isWs;
            if (openFile(file, true, false, &isWs) != WingHex::Success) {
                failedFile.append(file);
            }
        }

        if (!failedFile.isEmpty()) {
            WingMessageBox::critical(
                _w, tr("Error"),
                tr("Could not open the following file. Check that it exists "
                   "and that you have permission to read it:") +
                    QStringLiteral("\n") + failedFile.join('\n'));
        }
    }

    startupPhases.checkpoint("application.open_files");
    connect(_w, &MainWindow::closed, this,
            []() { AppManager::instance()->exit(); });

    if (splash) {
        HANDLE_CONFIG;
        WRITE_CONFIG(
            SPLASH_HISTORY_KEY,
            StartupFeedback::remember(splashHistory, splash->elapsedTime()));
        splash->finish();
        splash = nullptr;
    }
}

AppManager::~AppManager() {
    ScriptMachine::instance().destoryMachine();
    InspectQtLogHelper::instance().destory();
    CommandHistoryManager::save(QConsoleWidget::history().strings_);

    delete _w;
    _w = nullptr;
}

AppManager *AppManager::instance() { return _instance; }

MainWindow *AppManager::mainWindow() const { return _w; }

quint64 AppManager::currentMSecsSinceEpoch() { return _timer.elapsed(); }

ErrFile AppManager::openFile(const QString &file, bool autoDetect,
                             bool skipScripting, bool *isWorkSpace) {
    EditorView *editor = nullptr;
    Q_ASSERT(_w);

    bool isWS = false;

    ErrFile ret = ErrFile::Error;
    if (autoDetect) {
        QFileInfo finfo(file);
        if (Utilities::isTextFile(finfo)) {
            auto suffix = finfo.suffix();
            if (suffix.compare(QLatin1String("wingpro")) == 0) {
                ret = _w->openWorkSpace(file, &editor);
                if (ret == ErrFile::Success) {
                    isWS = true;
                }
            } else if (!skipScripting &&
                       ScriptManager::isScriptFileSuffix(suffix)) {
                _w->openScriptFile(file, splash);
                ret = ErrFile::Success;
            }
        }
    }

    if (ret == ErrFile::Error) {
        ret = _w->openFile(file, &editor);

        if (ret == ErrFile::AlreadyOpened) {
            Q_ASSERT(editor);
            if (_w->currentEditor() == editor) {
                Toast::toast(_w,
                             ThemeManager::instance().standardPixmap(
                                 ThemeManager::StandardIcon::OpenFile),
                             tr("This file is already open"));
            } else {
                editor->raise();
            }

            editor->setFocus();
            _w->addRecentFile(nullptr, file);
        } else if (ret == ErrFile::Success) {
            _w->addRecentFile(editor, file);
        }
    } else if (ret == ErrFile::AlreadyOpened) {
        Q_ASSERT(editor);
        if (_w->currentEditor() == editor) {
            Toast::toast(_w,
                         ThemeManager::instance().standardPixmap(
                             ThemeManager::StandardIcon::OpenFile),
                         tr("This file is already open"));
        } else {
            editor->raise();
        }

        editor->setFocus();
        _w->addRecentFile(nullptr, file, isWS);
    }

    if (isWorkSpace) {
        *isWorkSpace = isWS;
    }

    return ret;
}
