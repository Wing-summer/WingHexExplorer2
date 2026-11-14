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

#ifndef SCRIPTEXECUTION_H
#define SCRIPTEXECUTION_H

#include <QQueue>
#include <QSet>

#include "luauscheduler.h"

class ScriptMachine;

enum class ScriptFailure {
    FileError,
    CompilationError,
    ExecutionError,
    CallbackError,
    Timeout
};

// One independently owned entry and its serial asynchronous invocations.
class ScriptExecution final : public QObject {
    Q_OBJECT
public:
    explicit ScriptExecution(ScriptMachine &machine, const QString &path);
    virtual ~ScriptExecution() override;

public:
    inline const void *contextKey() const { return &m_data; }

    void start();
    void stop();
    int resumeCoroutine(lua_State *thread, int originalFunction);
    int wrapCoroutine(lua_State *thread);
    void awaitModule(lua_State *caller, lua_State *module);
    int resumeThread(lua_State *root, int arguments);
    void interrupt(lua_State *thread);
    QString moduleCacheKey(const QString &key);
    void enqueueCallback(qtluau::LuaCallbackInvocation invocation);

signals:
    void sigOnEntryCompleted();
    void sigOnExecutionFailed(ScriptFailure failure, const QString &message);
    void sigOnExecutionStopped();

private:
    static ScriptExecution *fromThread(lua_State *thread);
    static int resumeChild(lua_State *thread);
    static int resumeChildContinuation(lua_State *thread, int status);
    static int resumeWrapped(lua_State *thread);
    static int resumeWrappedContinuation(lua_State *thread, int status);
    void takeModuleResult(lua_State *thread, lua_State *caller);
    int resumeInvocation(lua_State *root, int arguments);
    void startNextCallback();
    void finishJob(int status);
    void finishStop();
    void fail(const QString &error,
              ScriptFailure failure = ScriptFailure::ExecutionError);
    void dispose();
    void startJob(int arguments = 0);

private:
    enum class State { Pending, Entry, Resident, Stopping, Finished };

    struct PendingModule {
        lua_State *caller;
        lua_State *thread;
        int reference;
    };

private:
    QVector<PendingModule> m_modules;
    QSet<lua_State *> m_moduleResults;
    ScriptMachine &m_machine;
    QString m_path;
    QString m_cacheNamespace;
    QSet<QByteArray> m_moduleCacheKeys;
    QSet<lua_State *> m_hostYields;
    quint64 m_sliceStart = 0;
    quint64 m_executionTime = 0;
    LuauThreadData m_data;
    LuauThread m_thread;
    LuauScheduler m_scheduler;
    QQueue<qtluau::LuaCallbackInvocation> m_pending;
    State m_state = State::Pending;
    bool m_timedOut = false;
};
#endif // SCRIPTEXECUTION_H
