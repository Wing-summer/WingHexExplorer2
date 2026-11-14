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

#ifndef HEADLESSRUNNER_H
#define HEADLESSRUNNER_H

#include <memory>

#include <QObject>

#include "luau/runtime/scriptmachine.h"
#include "scriptenvironment.h"

class StartupOptions;
class ScriptService;

class HeadlessRunner final : public QObject {
    Q_OBJECT
public:
    explicit HeadlessRunner(QObject *parent = nullptr);
    virtual ~HeadlessRunner() override;
    int run(const StartupOptions &options);

private:
    void checkSignal();
    void scriptFinished(ScriptMachine::ExecutionResult result);
    void serviceStopped();
    void callbackFailed(const QString &message);
    void requestExit(int code);
    static bool hasPendingSignal();
    static void writeLog(const QString &message);
    static void print(const ScriptMachine::MessageInfo &message);

private:
    ScriptEnvironment m_environment;
    std::unique_ptr<ScriptService> m_service;
    int m_exitCode = 0;
    bool m_stopping = false;
};
#endif // HEADLESSRUNNER_H
