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

#include "headlessrunner.h"

#include <QApplication>
#include <QFileInfo>
#include <QScopeGuard>
#include <QTimer>

#include "applicationmode.h"
#include "class/plugin/pluginsystem.h"
#include "class/settings/languagemanager.h"
#include "class/settings/settingmanager.h"
#include "define.h"
#include "inspectqtloghelper.h"
#include "logger.h"
#include "scriptservice.h"
#include "scriptservicetray.h"
#include "terminalservices.h"

HeadlessRunner::HeadlessRunner(QObject *parent) : QObject(parent) {}

HeadlessRunner::~HeadlessRunner() {
    m_service.reset();
    m_environment.shutdown();
}

int HeadlessRunner::run(const StartupOptions &options) {
    TerminalServices terminal;
    const bool live = options.live();
    const QString entry = live ? QString() : options.files().first();
    auto &settings = SettingManager::instance();
    if (!settings.scriptEnabled()) {
        writeLog(QStringLiteral("Scripting is disabled in settings"));
        return int(CrashCode::ScriptDisabled);
    }
    if (!live && !ScriptService::validScript(entry)) {
        writeLog(QStringLiteral("Invalid or unreadable script: ") + entry);
        return int(CrashCode::ScriptFileError);
    }
    auto &nativeLog = InspectQtLogHelper::instance();
    nativeLog.initConsole();
    auto restoreNativeLog = qScopeGuard([&nativeLog] { nativeLog.destory(); });
    auto cleanup = qScopeGuard([this] {
        m_service.reset();
        m_environment.shutdown();
    });
    LanguageManager::instance();
    auto &logger = Logger::instance();
    logger.setLogLevel(Logger::Level(settings.logLevel()));
    connect(&logger, &Logger::sigOnLogMessage, this, &HeadlessRunner::writeLog);
    settings.checkWriteableAndWarn();
    m_environment.loadPlugins();
    auto &machine = ScriptMachine::instance();
    if (!ScriptEnvironment::initializeMachine({false, false, false, live}) ||
        !ScriptEnvironment::finishRegistration()) {
        writeLog(QStringLiteral("Script engine or plugin registration failed"));
        return int(CrashCode::ScriptInitFailed);
    }
    ScriptMachine::RegCallBacks callbacks;
    callbacks.printMsgFn = &HeadlessRunner::print;
    machine.registerCallBack(ScriptMachine::Background, callbacks);
    if (!live) {
        machine.setCallbackFailureHandler(
            [this](const QString &message) { callbackFailed(message); });
    }
    machine.setInterruptionCheck(&HeadlessRunner::hasPendingSignal);
    auto resetInterrupt = qScopeGuard([&machine] {
        machine.setInterruptionCheck({});
        machine.setCallbackFailureHandler({});
    });
    QTimer signalTimer;
    connect(&signalTimer, &QTimer::timeout, this, &HeadlessRunner::checkSignal);
    signalTimer.start(30);
    qApp->setQuitOnLastWindowClosed(false);
    std::unique_ptr<ScriptServiceTray> tray;
    PluginSystem::instance().dispatchAppReadyEvent();
    if (live) {
        m_service = std::make_unique<ScriptService>(options);
        connect(m_service.get(), &ScriptService::sigOnServiceStopped, this,
                &HeadlessRunner::serviceStopped);
        const auto result = m_service->start();
        if (result.error != ScriptService::StartError::None) {
            writeLog(result.message);
            return int(CrashCode::InitializationFailed);
        }
        tray = std::make_unique<ScriptServiceTray>(*m_service);
        writeLog(QStringLiteral("Script service listening on %1")
                     .arg(m_service->endpoint()));
    } else {
        const auto path = QFileInfo(entry).absoluteFilePath();
        QTimer::singleShot(0, this, [this, &machine, path] {
            if (m_stopping) {
                return;
            }
            machine.executeScriptWithResult(
                ScriptMachine::Background, path, false,
                [this](ScriptMachine::ExecutionResult result) {
                    scriptFinished(result);
                });
        });
    }
    qApp->exec();
    return m_exitCode;
}

bool HeadlessRunner::hasPendingSignal() {
    return TerminalServices::pendingSignal() != 0;
}

void HeadlessRunner::checkSignal() {
    const auto signal = TerminalServices::pendingSignal();
    if (signal) {
        requestExit(128 + signal);
    }
}

void HeadlessRunner::writeLog(const QString &message) {
    TerminalServices::write(true, message + QLatin1Char('\n'));
}

void HeadlessRunner::print(const ScriptMachine::MessageInfo &message) {
    using Type = ScriptMachine::MessageType;
    if (message.type == Type::ExecInfo) {
        return;
    }
    const bool error =
        message.type == Type::Error || message.type == Type::Warn;
    TerminalServices::write(error, message.message);
    if (message.type != Type::Print &&
        !message.message.endsWith(QLatin1Char('\n'))) {
        TerminalServices::write(error, QStringLiteral("\n"));
    }
}

void HeadlessRunner::scriptFinished(ScriptMachine::ExecutionResult result) {
    if (hasPendingSignal()) {
        checkSignal();
        return;
    }
    requestExit(result == ScriptMachine::ExecutionResult::Success ? 0
                : result == ScriptMachine::ExecutionResult::FileError
                    ? int(CrashCode::ScriptFileError)
                    : int(CrashCode::ScriptExecutionFailed));
}

void HeadlessRunner::requestExit(int code) {
    if (m_stopping) {
        return;
    }
    m_stopping = true;
    m_exitCode = code;
    if (m_service) {
        m_service->shutdown();
    } else {
        ScriptMachine::instance().abortScript(ScriptMachine::Background);
        QTimer::singleShot(0, qApp, &QCoreApplication::quit);
    }
}

void HeadlessRunner::serviceStopped() { qApp->quit(); }

void HeadlessRunner::callbackFailed(const QString &message) {
    Q_UNUSED(message);
    if (hasPendingSignal()) {
        checkSignal();
    } else {
        requestExit(int(CrashCode::ScriptExecutionFailed));
    }
}
