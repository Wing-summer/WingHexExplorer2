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

#include <cstdio>
#include <memory>

#include <QCommandLineParser>
#include <QDebug>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSettings>

#include "class/application/applicationclock.h"
#include "class/application/applicationmode.h"
#include "class/application/appmanager.h"
#include "class/application/headlessrunner.h"
#include "class/application/standalonemodes.h"
#include "class/application/startuptrace.h"
#include "class/application/terminalservices.h"
#include "class/settings/languagemanager.h"
#include "class/settings/settingmanager.h"
#include "define.h"
#include "dialog/common/wingmessagebox.h"
#ifndef QT_DEBUG
#include "class/file/fileaccesscheck.h"
#endif

namespace {
bool evaluateEnvironmentCondition(const QProcessEnvironment &env,
                                  const QString &statement) {
    // Parse and evaluate statements:
    // $NAME               -> check existence
    // $NAME=VALUE         -> ignore-case full match
    // $NAME==VALUE        -> case-sensitive full match
    // $NAME:=VALUE        -> ignore-case contains
    // $NAME::=VALUE       -> case-sensitive contains
    // VALUE: unless pure digits, must be enclosed in "" or ''

    static const QRegularExpression re(
        R"(^\$([A-Za-z_][A-Za-z0-9_]*)(?:\s*(==|::=|:=|=)\s*(\d+|"[^"]*"|'[^']*'))?$)");
    auto match = re.match(statement);
    if (!match.hasMatch()) {
        qWarning("[main::loadEnvConfig] Invalid syntax: %s",
                 qUtf8Printable(statement));
        return false;
    }

    auto name = match.captured(1);
    auto op = match.captured(2);
    auto value = match.captured(3);

    // Existence check: no operator provided
    if (op.isEmpty()) {
        return env.contains(name);
    }

    if (!value.isEmpty() &&
        ((value.startsWith('"') && value.endsWith('"')) ||
         (value.startsWith('\'') && value.endsWith('\'')))) {
        value.removeFirst().removeLast();
    }

    auto var = env.value(name);

    // Evaluate based on operator
    if (op == QStringLiteral(":=") || op == QStringLiteral("::=")) {
        const QStringList items = var.split(QDir::listSeparator());
        for (const QString &item : items) {
            if (op == QStringLiteral(":=")) {
                if (item.contains(value, Qt::CaseInsensitive)) {
                    return true;
                }
            } else {
                if (item.contains(value, Qt::CaseSensitive)) {
                    return true;
                }
            }
        }
        return false;
    }

    if (op == "=") {
        return QString::compare(var, value, Qt::CaseInsensitive) == 0;
    } else if (op == "==") {
        return var == value;
    } else {
        qWarning("[main::loadEnvConfig] Unknown operator: %s",
                 qUtf8Printable(op));
    }

    return false;
}
} // namespace

void loadEnvConfig(int argc, char *argv[]) {
    // anti-env-inject
    qputenv("WINGHEX_DISALLOW_ROOT_PLUGIN", QByteArrayLiteral("1"));
    qputenv("WINGHEX_CERT_SKIP", QByteArrayLiteral("0"));
    qputenv("WINGHEX_CERT_FORCE_ALL", QByteArrayLiteral("0"));

    QFileInfo info(QString::fromUtf8(argv[0]));
    QDir appDir(info.absoluteDir());

    if (!appDir.exists(QStringLiteral("config.ini"))) {
        return;
    }

    auto path = appDir.absoluteFilePath(QStringLiteral("config.ini"));

#ifndef QT_DEBUG
    if (FileAccessCheck::canStandardUserWriteFile(path) !=
        FileAccessCheck::Status::Deny) {
        return;
    }
#endif

    QSettings set(path, QSettings::IniFormat);

    // General
    const auto keys = set.childKeys();
    for (const auto &kv : keys) {
        qputenv(qPrintable(kv), set.value(kv).toByteArray());
    }

    const auto groups = set.childGroups();

    auto env = QProcessEnvironment::systemEnvironment();

    constexpr auto syslen = std::char_traits<char>::length(WING_SYSTEM_NAME);
    if (syslen) {
        set.beginGroup(WING_SYSTEM_NAME);
        const auto keys = set.childKeys();
        for (const auto &kv : keys) {
            qputenv(qPrintable(kv), set.value(kv).toByteArray());
        }
        set.endGroup();
    }

    for (const auto &g : groups) {
        if (evaluateEnvironmentCondition(env, g)) {
            set.beginGroup(g);
            const auto keys = set.childKeys();
            for (const auto &kv : keys) {
                qputenv(qPrintable(kv), set.value(kv).toByteArray());
            }
            set.endGroup();
        }
    }
}

int main(int argc, char *argv[]) {
    loadEnvConfig(argc, argv);

    ApplicationClock::start();

    StartupTrace::start();
    StartupTrace::PhaseTimer startupPhases;
    QCommandLineParser bootstrapParser;
    StartupOptions::configureBootstrapParser(bootstrapParser);
    QStringList arguments;
    for (int index = 0; index < argc; ++index) {
        arguments.append(QString::fromUtf8(argv[index]));
    }
    const auto bootstrap = StartupOptions::parse(bootstrapParser, arguments);
    const auto bootstrapMode = bootstrap.mode();
    const bool liveService =
        bootstrapMode == StartupMode::Headless && bootstrap.live() &&
        !bootstrap.informationRequested() && bootstrap.error().isEmpty();
    if (bootstrapMode == StartupMode::Headless || bootstrap.live() ||
        bootstrap.informationRequested() || !bootstrap.error().isEmpty()) {
        TerminalServices::prepareStreams();
        qputenv("QT_FORCE_STDERR_LOGGING", "1");
        if (!liveService || (!TerminalServices::desktopConfigured() &&
                             !qEnvironmentVariableIsSet("QT_QPA_PLATFORM"))) {
            qputenv("QT_QPA_PLATFORM", "offscreen");
        }
    }

    startupPhases.checkpoint("process.environment");
    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QApplication::setApplicationName(QStringLiteral(APP_NAME));
    QApplication::setOrganizationName(QStringLiteral(APP_ORG));
    QApplication::setApplicationVersion(QStringLiteral(WINGHEX_VERSION));

    static_assert(CHAR_BIT == 8, APP_NAME " is only supported with 8-bit char");

    std::unique_ptr<QApplication> application;
    try {
        if (bootstrapMode == StartupMode::Full &&
            !bootstrap.informationRequested()) {
            application = std::make_unique<AppManager>(argc, argv);
        } else {
            application = std::make_unique<QApplication>(argc, argv);
        }
#ifdef QT_DEBUG
        QStandardPaths::setTestModeEnabled(true);
#endif
        LanguageManager::instance();
        QCommandLineParser parser;
        StartupOptions::configureParser(parser);
        const auto options =
            StartupOptions::parse(parser, application->arguments());
        ApplicationMode::initialize(options);
        if (!options.error().isEmpty()) {
            TerminalServices::prepareStreams();
            TerminalServices::write(true, options.error() + QLatin1Char('\n'));
            return int(CrashCode::InvalidArguments);
        }
        if (options.informationRequested()) {
            parser.process(*application);
            return 0;
        }
#ifdef QT_DEBUG
        qputenv("WINGHEX_CERT_SKIP", "1");
#endif
        const auto mode = options.mode();
        if (mode == StartupMode::Headless) {
            HeadlessRunner runner;
            return runner.run(options);
        }
        if (mode != StartupMode::Full) {
            return runStandaloneMode(options);
        }
        auto &a = *static_cast<AppManager *>(application.get());
        a.initialize(options.files());
        startupPhases.checkpoint("process.application");
        auto w = a.mainWindow();
        StartupTrace::watchWindow(w);

        auto &set = SettingManager::instance();
        switch (set.defaultWinState()) {
        case Qt::WindowNoState:
            w->show();
            Utilities::moveToCenter(w);
            break;
        case Qt::WindowMinimized:
            w->showMinimized();
            break;
        case Qt::WindowActive:
        case Qt::WindowMaximized:
            w->showMaximized();
            break;
        case Qt::WindowFullScreen:
            w->showFullScreen();
            break;
        }

        startupPhases.checkpoint("process.show");
        return a.exec();
    } catch (CrashCode errCode) {
        if (bootstrapMode == StartupMode::Headless || !application) {
            std::fprintf(stderr, "Startup failed (exit code %d)\n",
                         int(errCode));
        }
        return int(errCode);
    } catch (const std::bad_alloc &) {
        if (bootstrapMode == StartupMode::Headless || !application) {
            std::fputs("Out of memory\n", stderr);
            return int(CrashCode::OutofMemory);
        }
        auto &lang = LanguageManager::instance();
        auto df = lang.defaultLocale();

        // this exception can only occur when your memory are too limit or
        // you are writing more than 2GB with QByteArray on 32-bit operating
        // system or with Qt5.
        if (QLocale::China == df.territory()) {
            WingMessageBox::critical(
                nullptr, QStringLiteral(APP_NAME),
                QStringLiteral("崩溃啦！发生内存溢出异常！"));
        } else {
            WingMessageBox::critical(
                nullptr, QStringLiteral(APP_NAME),
                QStringLiteral("WingHexExplorer2 is out of memory. Crashed!"));
        }
        return int(CrashCode::OutofMemory);
    } catch (const std::exception &error) {
        if (bootstrapMode != StartupMode::Headless && application) {
            if (ApplicationMode::hasMainWindow()) {
                throw;
            }
            WingMessageBox::critical(
                nullptr, qAppName(),
                QApplication::translate("main", "Initialization failed: %1")
                    .arg(QString::fromUtf8(error.what())));
            return int(CrashCode::InitializationFailed);
        }
        std::fprintf(stderr, "Initialization failed: %s\n", error.what());
        return int(CrashCode::InitializationFailed);
    } catch (...) {
        if (bootstrapMode != StartupMode::Headless && application) {
            if (ApplicationMode::hasMainWindow()) {
                throw;
            }
            WingMessageBox::critical(
                nullptr, qAppName(),
                QApplication::translate("main", "Initialization failed."));
            return int(CrashCode::InitializationFailed);
        }
        std::fputs("Initialization failed\n", stderr);
        return int(CrashCode::InitializationFailed);
    }
}
