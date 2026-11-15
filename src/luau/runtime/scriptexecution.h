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

#include <QHash>
#include <QQueue>
#include <QSet>
#include <QVector>

#include "luauscheduler.h"
#include "scriptchunk.h"

class ScriptMachine;

enum class ScriptFailure {
    FileError,
    CompilationError,
    ExecutionError,
    CallbackError
};

// One independently owned entry and its serial asynchronous invocations.
class ScriptExecution final : public QObject {
    Q_OBJECT
public:
    enum class Lifetime { OneShot, Resident };
    enum class State {
        Pending,
        Running,
        WaitingInput,
        Resident,
        Stopping,
        Success,
        Failed,
        Aborted
    };

private:
    friend class ScriptMachine;
    explicit ScriptExecution(ScriptMachine &machine, const QString &path,
                             quint64 taskId, Lifetime lifetime);

public:
    inline quint64 taskId() const { return m_taskId; }

    inline QString path() const { return m_path; }

    inline State state() const { return m_state; }

    inline bool active() const {
        return m_state != State::Success && m_state != State::Failed &&
               m_state != State::Aborted;
    }

    void setWaitingInput(bool waiting);
    void startChunk(const QByteArray &bytes, ScriptChunkFormat format);
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
    void sigOnExecutionStateChanged(ScriptExecution::State state);
    void sigOnExecutionCompleted();
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
    void finishCallback(int status);
    void finishStop();
    void fail(const QString &error,
              ScriptFailure failure = ScriptFailure::ExecutionError);
    void dispose();
    void startJob(int arguments = 0);

private:
    void setState(State state);
    void output(const QString &message, bool error = false);

    struct PendingModule {
        lua_State *caller;
        lua_State *thread;
        int reference;
    };

private:
    QHash<lua_State *, QVector<PendingModule>> m_modules;
    lua_State *m_activeRoot = nullptr;
    QSet<lua_State *> m_moduleResults;
    ScriptMachine &m_machine;
    QString m_path;
    QString m_cacheNamespace;
    QSet<QByteArray> m_moduleCacheKeys;
    QSet<lua_State *> m_hostYields;
    quint64 m_sliceStart = 0;
    LuauThreadData m_data;
    LuauThread m_thread;
    LuauScheduler m_scheduler;
    LuauThread m_callbackThread;
    LuauScheduler m_callbackScheduler;
    QQueue<qtluau::LuaCallbackInvocation> m_pending;
    const quint64 m_taskId;
    const Lifetime m_lifetime;
    QByteArray m_chunk;
    ScriptChunkFormat m_format = ScriptChunkFormat::Source;
    bool m_hasChunk = false;
    bool m_entryCompleted = false;
    State m_state = State::Pending;
};
#endif // SCRIPTEXECUTION_H
