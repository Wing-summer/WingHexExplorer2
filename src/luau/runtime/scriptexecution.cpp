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

#include "scriptexecution.h"

#include <utility>

#include <QTimer>

#include "lualib.h"

#include "class/application/applicationclock.h"
#include "class/plugin/pluginsystem.h"
#include "class/settings/settingmanager.h"
#include "luau/luauutil.h"
#include "scriptmachine.h"

ScriptExecution::ScriptExecution(ScriptMachine &machine, const QString &path)
    : QObject(&machine), m_machine(machine), m_path(path),
      m_scheduler(machine, &m_thread) {
    static quint64 nextContext = 0;
    m_data.execution = this;
    m_data.mode = ScriptMachine::Background;
    m_cacheNamespace = QStringLiteral("task:%1:").arg(++nextContext);
    m_data.timeOutTime =
        quint64(SettingManager::instance().scriptTimeout()) * 60000;
    m_thread.data = &m_data;
    m_data.parent = &m_thread;
    connect(&m_scheduler, &LuauScheduler::sigOnFinished, this,
            &ScriptExecution::finishJob);
}

ScriptExecution::~ScriptExecution() { dispose(); }

void ScriptExecution::start() {
    if (m_state != State::Pending) {
        return;
    }
    if (m_machine.isExecuting()) {
        QTimer::singleShot(10, this, &ScriptExecution::start);
        return;
    }
    m_state = State::Entry;
    try {
        auto *main = m_machine.m_main;
        m_thread.state = lua_newthread(main);
        m_thread.refID = lua_ref(main, -1);
        lua_pop(main, 1);
        lua_setthreaddata(m_thread.state, &m_data);
        luaL_sandboxthread(m_thread.state);
        const auto loaded = m_machine.loadScript(
            m_thread.state, m_path, ScriptMachine::Background, false, false);
        if (loaded.result != ScriptMachine::ExecutionResult::Success) {
            fail(loaded.message,
                 loaded.result == ScriptMachine::ExecutionResult::FileError
                     ? ScriptFailure::FileError
                     : ScriptFailure::CompilationError);
            return;
        }
        startJob();
    } catch (const std::exception &error) {
        fail(QString::fromUtf8(error.what()));
    }
}

void ScriptExecution::startJob(int arguments) {
    m_executionTime = 0;
    m_timedOut = false;
    m_data.startTime = ApplicationClock::elapsed();
    m_data.lastInteruptTime = m_data.startTime;
    m_scheduler.start(arguments);
}

void ScriptExecution::finishJob(int status) {
    const auto error = m_scheduler.executeError();
    if (m_state == State::Stopping) {
        finishStop();
        return;
    }
    if (status != LUA_OK || m_timedOut) {
        fail(error.isEmpty() ? QStringLiteral("Script execution failed")
                             : error,
             m_timedOut                   ? ScriptFailure::Timeout
             : m_state == State::Resident ? ScriptFailure::CallbackError
                                          : ScriptFailure::ExecutionError);
        return;
    }
    // Reset only the Lua stack. The task's globals, callbacks and owned Qt
    // resources remain alive while entry and callback invocations share it.
    lua_resetthread(m_thread.state);
    if (m_state == State::Entry) {
        m_state = State::Resident;
        Q_EMIT sigOnEntryCompleted();
    }
    QTimer::singleShot(0, this, &ScriptExecution::startNextCallback);
}

ScriptExecution *ScriptExecution::fromThread(lua_State *thread) {
    return static_cast<LuauThreadData *>(lua_getthreaddata(thread))->execution;
}

int ScriptExecution::resumeCoroutine(lua_State *thread, int originalFunction) {
    const int arguments = lua_gettop(thread);
    lua_pushvalue(thread, originalFunction);
    lua_pushcclosurek(thread, resumeChild, "resume", 1,
                      resumeChildContinuation);
    lua_insert(thread, 1);
    return lua_callyieldable(thread, arguments, LUA_MULTRET);
}

int ScriptExecution::resumeChild(lua_State *thread) {
    luaL_checktype(thread, 1, LUA_TTHREAD);
    const int arguments = lua_gettop(thread);
    lua_pushboolean(thread, true);
    lua_insert(thread, 2);
    lua_pushvalue(thread, lua_upvalueindex(1));
    lua_pushvalue(thread, 1);
    for (int index = 3; index <= arguments + 1; ++index) {
        lua_pushvalue(thread, index);
    }
    for (int index = 1; index < arguments; ++index) {
        lua_remove(thread, 3);
    }
    return lua_callyieldable(thread, arguments, LUA_MULTRET);
}

void ScriptExecution::takeModuleResult(lua_State *thread, lua_State *caller) {
    if (m_moduleResults.remove(thread)) {
        lua_xmove(thread, caller, 1);
    }
}

int ScriptExecution::resumeChildContinuation(lua_State *thread, int status) {
    // A yielded multret call restores only its result base as the C frame top.
    // Reserve API-accessible slots before inspecting the resumed results.
    luaL_checkstack(thread, LUA_MINSTACK,
                    "Coroutine continuation stack overflow");
    if (status != LUA_OK) {
        lua_error(thread);
    }
    auto *execution = fromThread(thread);
    auto *child = lua_tothread(thread, 1);
    if (!lua_toboolean(thread, 2)) {
        lua_settop(thread, 1);
        execution->takeModuleResult(child, thread);
        return resumeChild(thread);
    }
    if (execution->m_hostYields.remove(child)) {
        // Native resume may itself use a deferred VM continuation. Inspect its
        // results only after it finishes, keeping host yields invisible to Lua.
        lua_settop(thread, 1);
        lua_pushboolean(thread, false);
        execution->m_hostYields.insert(thread);
        return lua_yield(thread, 0);
    }
    return lua_gettop(thread) - 2;
}

int ScriptExecution::wrapCoroutine(lua_State *thread) {
    luaL_checktype(thread, 1, LUA_TFUNCTION);
    auto *child = lua_newthread(thread);
    lua_pushvalue(thread, 1);
    lua_xmove(thread, child, 1);
    // The VM's sandboxed root owns the actual resume adapter. Do not consult
    // a script's shadowed coroutine global or framework-private registry keys.
    auto *main = m_machine.m_main;
    const luabridge::StackRestore restore(main);
    lua_getglobal(main, "coroutine");
    lua_getfield(main, -1, "resume");
    lua_xmove(main, thread, 1);
    lua_pop(main, 1);
    lua_pushcclosurek(thread, resumeWrapped, "wrap", 2,
                      resumeWrappedContinuation);
    return 1;
}

int ScriptExecution::resumeWrapped(lua_State *thread) {
    const int arguments = lua_gettop(thread);
    lua_pushvalue(thread, lua_upvalueindex(1));
    lua_insert(thread, 1);
    lua_pushvalue(thread, lua_upvalueindex(2));
    lua_insert(thread, 1);
    return lua_callyieldable(thread, arguments + 1, LUA_MULTRET);
}

int ScriptExecution::resumeWrappedContinuation(lua_State *thread, int status) {
    luaL_checkstack(thread, LUA_MINSTACK,
                    "Coroutine continuation stack overflow");
    if (status != LUA_OK) {
        lua_error(thread);
    }
    const bool ok = lua_toboolean(thread, 1);
    lua_remove(thread, 1);
    if (!ok) {
        lua_error(thread);
    }
    return lua_gettop(thread);
}

void ScriptExecution::awaitModule(lua_State *caller, lua_State *module) {
    lua_pushthread(module);
    const auto reference = lua_ref(module, -1);
    lua_pop(module, 1);
    m_modules.append({caller, module, reference});
    m_hostYields.insert(caller);
}

int ScriptExecution::resumeThread(lua_State *root, int arguments) {
    m_sliceStart = ApplicationClock::elapsed();
    const auto status = resumeInvocation(root, arguments);
    m_executionTime += ApplicationClock::elapsed() - m_sliceStart;
    return status;
}

void ScriptExecution::interrupt(lua_State *thread) {
    const auto elapsed = ApplicationClock::elapsed() - m_sliceStart;
    if ((m_machine.m_interruptionCheck && m_machine.m_interruptionCheck()) ||
        m_state == State::Stopping || m_thread.requestStop) {
        m_state = State::Stopping;
        LuauUtil::throwError(thread, "Script execution was aborted");
    } else if (m_data.timeOutTime &&
               m_executionTime + elapsed > m_data.timeOutTime) {
        m_timedOut = true;
        LuauUtil::throwError(thread, "Thread execution is timed-out");
    } else if (elapsed >= 10 && lua_isyieldable(thread)) {
        m_hostYields.insert(thread);
        lua_yield(thread, 0);
    }
}

QString ScriptExecution::moduleCacheKey(const QString &key) {
    const QString scoped = m_cacheNamespace + key;
    m_moduleCacheKeys.insert(scoped.toUtf8());
    return scoped;
}

int ScriptExecution::resumeInvocation(lua_State *root, int arguments) {
    if (m_modules.isEmpty()) {
        m_hostYields.remove(root);
        return lua_resume(root, nullptr,
                          m_moduleResults.remove(root) ? 1 : arguments);
    }
    const auto pending = m_modules.last();
    auto *thread = pending.thread;
    const auto status =
        lua_resume(thread, nullptr, m_moduleResults.remove(thread) ? 1 : 0);
    if (status == LUA_YIELD) {
        return status;
    }
    if (status != LUA_OK || lua_gettop(thread) != 1) {
        const auto error =
            status == LUA_OK
                ? QStringLiteral("Module must return a single value")
                : QString::fromUtf8(lua_tostring(thread, -1));
        const auto bytes = error.toUtf8();
        lua_pushlstring(root, bytes.constData(), bytes.size());
        return LUA_ERRRUN;
    }
    lua_xmove(thread, pending.caller, 1);
    m_moduleResults.insert(pending.caller);
    lua_unref(m_machine.m_main, pending.reference);
    m_modules.removeLast();
    return LUA_YIELD;
}

void ScriptExecution::enqueueCallback(
    qtluau::LuaCallbackInvocation invocation) {
    if (m_state != State::Stopping && m_state != State::Finished) {
        m_pending.enqueue(std::move(invocation));
        QTimer::singleShot(0, this, &ScriptExecution::startNextCallback);
    }
}

void ScriptExecution::startNextCallback() {
    if (m_state != State::Resident || m_thread.isRunning ||
        m_pending.isEmpty()) {
        return;
    }
    if (m_machine.isExecuting()) {
        QTimer::singleShot(10, this, &ScriptExecution::startNextCallback);
        return;
    }
    auto invocation = m_pending.dequeue();
    if (!invocation.active()) {
        QTimer::singleShot(0, this, &ScriptExecution::startNextCallback);
        return;
    }
    try {
        auto *thread = m_thread.state;
        const auto args = invocation.arguments(thread);
        invocation.callback.push(thread);
        for (const auto &arg : args) {
            arg.push(thread);
        }
        startJob(int(args.size()));
    } catch (const std::exception &error) {
        fail(QString::fromUtf8(error.what()), ScriptFailure::CallbackError);
    }
}

void ScriptExecution::stop() {
    if (m_state == State::Stopping || m_state == State::Finished) {
        return;
    }
    m_state = State::Stopping;
    m_thread.requestStop = true;
    QTimer::singleShot(0, this, &ScriptExecution::finishStop);
}

void ScriptExecution::finishStop() {
    if (m_state == State::Finished) {
        return;
    }
    if (m_machine.isExecuting()) {
        QTimer::singleShot(10, this, &ScriptExecution::finishStop);
        return;
    }
    dispose();
    Q_EMIT sigOnExecutionStopped();
}

void ScriptExecution::fail(const QString &error, ScriptFailure failure) {
    dispose();
    Q_EMIT sigOnExecutionFailed(failure, error);
}

void ScriptExecution::dispose() {
    m_state = State::Finished;
    m_scheduler.stop();
    m_pending.clear();
    if (!m_thread.state) {
        return;
    }
    qtluau::clearLuaContext(m_thread.state);
    PluginSystem::instance().cleanScriptHandles(m_thread.state, {});
    for (const auto &module : std::as_const(m_modules)) {
        lua_resetthread(module.thread);
        lua_unref(m_machine.m_main, module.reference);
    }
    m_modules.clear();
    m_moduleResults.clear();
    m_hostYields.clear();
    for (const auto &key : std::as_const(m_moduleCacheKeys)) {
        m_machine.m_luaReq.clearRequireCacheEntry(key.constData());
    }
    m_moduleCacheKeys.clear();
    m_thread.destory();
}
