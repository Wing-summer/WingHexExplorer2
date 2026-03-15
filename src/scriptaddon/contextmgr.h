/*==============================================================================
** Copyright (C) 2024-2029 WingSummer
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

#ifndef CONTEXTMGR_H
#define CONTEXTMGR_H

#ifndef ANGELSCRIPT_H
#include <angelscript.h>
#endif

#include <QVector>

BEGIN_AS_NAMESPACE

class CScriptDictionary;

/// Single-script context manager. Each instance manages one "main" context
/// and any coroutines created by that main context.
///
/// Usage summary:
///  - Call SetGetTimeCallback() if you want to use sleep() in scripts.
///  - Call RegisterThreadSupport() / RegisterCoRoutineSupport(engine) to
///    register the helper functions into the engine (once).
///  - Call SetMainFunction(engine, func, keptByCaller) to prepare the main
///    context. The function returns the prepared asIScriptContext*.
///  - After executing, when caller is done with a kept main context, call
///    DoneWithContext(mainCtx) to return it to the engine.
class CContextMgr {
public:
    CContextMgr();
    ~CContextMgr();

    // Type for millisecond time callback
    typedef asUINT (*TIMEFUNC_t)();

    // Set the callback used by sleep()
    static void SetGetTimeCallback(TIMEFUNC_t func);

    // Register global functions: sleep(uint), yield(), coroutine support
    // Must call RegisterThreadSupport() after SetGetTimeCallback if sleep() is
    // used.
    static void RegisterThreadSupport(asIScriptEngine *engine);
    static void RegisterCoRoutineSupport(asIScriptEngine *engine);

    // Prepare and set the main function. Returns the prepared context or
    // nullptr on error. If keptByCaller == true, the caller must call
    // DoneWithContext(ctx) when finished. If keptByCaller == false, manager
    // will return the main ctx automatically when finished.
    asIScriptContext *SetMainFunction(asIScriptEngine *engine,
                                      asIScriptFunction *func,
                                      bool keptByCaller = true);

    // Create a coroutine for the current main context. Returns the coroutine
    // context.
    asIScriptContext *AddCoRoutine(asIScriptFunction *func,
                                   CScriptDictionary *arg = nullptr);

    // Execute a single scheduling step. Returns true if there are still active
    // contexts.
    bool ExecuteScripts();

    // Mark a context to sleep for milliSeconds (called by script sleep())
    void SetSleeping(asIScriptContext *ctx, asUINT milliSeconds);

    // Switch to the next coroutine in the group (called by script yield()).
    void NextCoRoutine();

    // Abort all running contexts. If forceReturnMain==true and main is
    // manager-owned, manager will also schedule main ctx for return.
    void AbortAll();

    // Caller returns a kept context when finished reading return values.
    void DoneWithContext(asIScriptContext *ctx);

    // Access main context pointer (if needed).
    asIScriptContext *MainContext() const;

    // Whether this manager currently has any active contexts (main kept by
    // caller counts).
    bool HasActive() const;

    QString scriptName() const;
    void setScriptName(const QString &newScriptName);

protected:
    // Sleep deadline for the group (ms)
    asUINT m_sleepUntil;

    // Vector of contexts: usually contains main ctx and any coroutines created.
    QVector<asIScriptContext *> m_coRoutines;

    // Index of the next coroutine to run
    asUINT m_currentCoRoutine;

    // Main context pointer and ownership flag
    asIScriptContext *m_mainCtx;
    bool m_mainKeptByCaller;

    // Time callback used by sleep()
    inline static TIMEFUNC_t m_getTimeFunc;

    // Contexts waiting for safe ReturnContext (deferred returns)
    QVector<asIScriptContext *> m_pendingReturns;

    // Reentrancy counter for nested ExecuteScripts calls
    int m_executeDepth;

    // Optional GC statistics counters
    asUINT m_numExecutions;
    asUINT m_numGCObjectsCreated;
    asUINT m_numGCObjectsDestroyed;

    QString m_scriptName;

    // Helper to attempt safe returns of pending contexts
    void ProcessPendingReturns();
};

END_AS_NAMESPACE

#endif // CONTEXTMGR_H
