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

#include "contextmgr.h"
#include "define.h"
#include <QDebug>

BEGIN_AS_NAMESPACE

static void ScriptSleep(asUINT milliSeconds) {
    asIScriptContext *ctx = asGetActiveContext();
    if (!ctx)
        return;
    CContextMgr *mgr = reinterpret_cast<CContextMgr *>(
        ctx->GetUserData(AsUserDataType::UserData_ContextMgr));
    if (!mgr)
        return;
    ctx->Suspend();
    mgr->SetSleeping(ctx, milliSeconds);
}

static void ScriptYield() {
    asIScriptContext *ctx = asGetActiveContext();
    if (!ctx)
        return;
    CContextMgr *mgr = reinterpret_cast<CContextMgr *>(
        ctx->GetUserData(AsUserDataType::UserData_ContextMgr));
    if (!mgr)
        return;
    mgr->NextCoRoutine();
    ctx->Suspend();
}

static void ScriptCreateCoRoutine(asIScriptFunction *func,
                                  CScriptDictionary *arg) {
    if (!func)
        return;
    asIScriptContext *ctx = asGetActiveContext();
    if (!ctx)
        return;
    CContextMgr *mgr = reinterpret_cast<CContextMgr *>(
        ctx->GetUserData(AsUserDataType::UserData_ContextMgr));
    if (!mgr)
        return;
    asIScriptContext *co = mgr->AddCoRoutine(func, arg);
    if (co && arg)
        co->SetArgObject(0, arg);
}

// Constructor: no parameters. Use SetMainFunction to prepare a main context.
CContextMgr::CContextMgr() {
    m_getTimeFunc = nullptr;
    m_sleepUntil = 0;
    m_currentCoRoutine = 0;
    m_mainCtx = nullptr;
    m_mainKeptByCaller = true;
    m_executeDepth = 0;
    m_numExecutions = 0;
    m_numGCObjectsCreated = 0;
    m_numGCObjectsDestroyed = 0;
}

// Destructor: ensure pending returns and contexts are released to avoid leaks.
CContextMgr::~CContextMgr() {
    // Force-return any pending returns
    for (int i = (int)m_pendingReturns.size() - 1; i >= 0; --i) {
        asIScriptContext *ctx = m_pendingReturns[i];
        if (ctx)
            ctx->GetEngine()->ReturnContext(ctx);
    }
    m_pendingReturns.clear();

    // Return any remaining contexts. If main was kept by caller, return here to
    // avoid leaks.
    for (asUINT i = 0; i < m_coRoutines.size(); ++i) {
        asIScriptContext *ctx = m_coRoutines[i];
        if (ctx)
            ctx->GetEngine()->ReturnContext(ctx);
    }
    m_coRoutines.clear();

    // Clear main pointer for safety
    m_mainCtx = nullptr;
}

// Set time callback used by sleep()
void CContextMgr::SetGetTimeCallback(TIMEFUNC_t func) { m_getTimeFunc = func; }

// Register sleep(uint) helper. Requires time callback to be set.
void CContextMgr::RegisterThreadSupport(asIScriptEngine *engine) {
    Q_ASSERT(m_getTimeFunc != nullptr);
    int r = engine->RegisterGlobalFunction(
        "void sleep(uint)", asFUNCTION(ScriptSleep), asCALL_CDECL);
    Q_ASSERT(r >= 0);
}

// Register yield() and coroutine helper functions. Requires dictionary add-on.
void CContextMgr::RegisterCoRoutineSupport(asIScriptEngine *engine) {
    Q_ASSERT(engine->GetTypeInfoByDecl("dictionary"));
    int r = engine->RegisterGlobalFunction(
        "void yield()", asFUNCTION(ScriptYield), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterFuncdef("void coroutine(dictionary@)");
    Q_ASSERT(r >= 0);
    r = engine->RegisterGlobalFunction(
        "void createCoRoutine(coroutine @+, dictionary @+)",
        asFUNCTION(ScriptCreateCoRoutine), asCALL_CDECL);
    Q_ASSERT(r >= 0);
}

// Prepare and set the main function. Returns the prepared context or nullptr on
// error. If keptByCaller==true the caller must call DoneWithContext() when
// finished.
asIScriptContext *CContextMgr::SetMainFunction(asIScriptEngine *engine,
                                               asIScriptFunction *func,
                                               bool keptByCaller /*= true*/) {
    if (!engine || !func)
        return nullptr;

    asIScriptContext *ctx = engine->RequestContext();
    if (!ctx)
        return nullptr;

    int r = ctx->Prepare(func);
    if (r < 0) {
        engine->ReturnContext(ctx);
        return nullptr;
    }

    // set this manager as user data so script helpers can find it
    ctx->SetUserData(this, AsUserDataType::UserData_ContextMgr);

    // other properties
    ctx->SetUserData(const_cast<QString *>(&m_scriptName),
                     AsUserDataType::UserData_Section_StringPtr);
    ctx->SetUserData(reinterpret_cast<void *>(1),
                     AsUserDataType::UserData_NeedYeild);

    // store main context and ownership flag
    m_mainCtx = ctx;
    m_mainKeptByCaller = keptByCaller;

    // reset coroutine list so main is the first entry
    m_coRoutines.clear();
    m_coRoutines.push_back(ctx);
    m_currentCoRoutine = 0;

    return ctx;
}

// Add a coroutine that will be scheduled in the same group as the main context.
// Copies user-data attributes from the main context.
asIScriptContext *CContextMgr::AddCoRoutine(asIScriptFunction *func,
                                            CScriptDictionary *arg) {
    if (!func || !m_mainCtx)
        return nullptr;

    asIScriptEngine *engine = m_mainCtx->GetEngine();
    asIScriptContext *coctx = engine->RequestContext();
    if (!coctx)
        return nullptr;

    int r = coctx->Prepare(func);
    if (r < 0) {
        engine->ReturnContext(coctx);
        return nullptr;
    }

    coctx->SetUserData(this, AsUserDataType::UserData_ContextMgr);

    // copy user data attributes if present on main ctx
    for (unsigned int i = AsUserDataType::UserData_CopyAttr_Begin;
         i < AsUserDataType::UserData_CopyAttr_End; ++i) {
        coctx->SetUserData(m_mainCtx->GetUserData(i), i);
    }

    m_coRoutines.push_back(coctx);
    return coctx;
}

// Execute one scheduling step. Returns true if there are active contexts
// remaining.
bool CContextMgr::ExecuteScripts() {
    ++m_executeDepth;

    asUINT time = m_getTimeFunc ? m_getTimeFunc() : asUINT(-1);

    // No contexts to run
    if (m_coRoutines.isEmpty()) {
        --m_executeDepth;
        ProcessPendingReturns();
        return false;
    }

    // Sleep handling
    if (m_sleepUntil >= time) {
        --m_executeDepth;
        ProcessPendingReturns();
        return !m_coRoutines.isEmpty();
    }

    // Defensive clamp of current index
    if ((asUINT)m_currentCoRoutine >= m_coRoutines.size())
        m_currentCoRoutine = 0;

    asIScriptContext *ctx = m_coRoutines[m_currentCoRoutine];
    if (!ctx) {
        --m_executeDepth;
        ProcessPendingReturns();
        return !m_coRoutines.isEmpty();
    }

    asIScriptEngine *engine = ctx->GetEngine();
    asUINT gcSize1 = 0, gcSize2 = 0, gcSize3 = 0;
    engine->GetGCStatistics(&gcSize1);

    // Keep ctx pointer since Execute may re-enter and modify m_coRoutines
    int r = ctx->Execute();

    engine->GetGCStatistics(&gcSize2);
    m_numGCObjectsCreated += (gcSize2 > gcSize1) ? (gcSize2 - gcSize1) : 0;
    ++m_numExecutions;

    if (r != asEXECUTION_SUSPENDED) {
        // find ctx in possibly modified vector
        int foundIndex = -1;
        for (int i = 0; i < (int)m_coRoutines.size(); ++i) {
            if (m_coRoutines[i] == ctx) {
                foundIndex = i;
                break;
            }
        }

        const bool isMainKept = (m_mainCtx == ctx && m_mainKeptByCaller);

        if (!isMainKept) {
            if (foundIndex != -1) {
                int state = ctx->GetState();
                if (state != asEXECUTION_ACTIVE) {
                    engine->ReturnContext(ctx);
                } else {
                    ctx->Abort();
                    m_pendingReturns.push_back(ctx);
                }

                // remove the entry and adjust current index
                m_coRoutines.remove(foundIndex);
                if (m_currentCoRoutine > (asUINT)foundIndex)
                    --m_currentCoRoutine;
                if (m_coRoutines.empty())
                    m_currentCoRoutine = 0;
                else if (m_currentCoRoutine >= m_coRoutines.size())
                    m_currentCoRoutine = 0;
            } else {
                // already removed by nested calls, nothing to do
            }
        } else {
            // main kept by caller: remove from list, but do not ReturnContext
            if (foundIndex != -1) {
                m_coRoutines.remove(foundIndex);
                if (m_currentCoRoutine > (asUINT)foundIndex)
                    --m_currentCoRoutine;
                if (m_coRoutines.empty())
                    m_currentCoRoutine = 0;
                else if (m_currentCoRoutine >= m_coRoutines.size())
                    m_currentCoRoutine = 0;
            }
        }
    }

    if (gcSize2 > gcSize1) {
        engine->GarbageCollect(asGC_FULL_CYCLE | asGC_DESTROY_GARBAGE);
        engine->GetGCStatistics(&gcSize3);
        m_numGCObjectsDestroyed +=
            (gcSize3 > gcSize2) ? (gcSize3 - gcSize2) : 0;
    }
    engine->GarbageCollect(asGC_ONE_STEP | asGC_DETECT_GARBAGE);

    ProcessPendingReturns();

    --m_executeDepth;

    return !m_coRoutines.isEmpty() || (m_mainCtx && m_mainKeptByCaller);
}

// Mark group sleeping until now + milliSeconds
void CContextMgr::SetSleeping(asIScriptContext * /*ctx*/, asUINT milliSeconds) {
    Q_ASSERT(m_getTimeFunc != nullptr);
    m_sleepUntil = (m_getTimeFunc ? m_getTimeFunc() : 0) + milliSeconds;
}

// Advance to next coroutine in the group. Finds the currently active context
// and advances index.
void CContextMgr::NextCoRoutine() {
    if (m_coRoutines.isEmpty())
        return;
    asIScriptContext *active = asGetActiveContext();
    if (!active)
        return;
    for (asUINT i = 0; i < m_coRoutines.size(); ++i) {
        if (m_coRoutines[i] == active) {
            if (m_coRoutines.size() == 0)
                return;
            m_currentCoRoutine = (i + 1) % m_coRoutines.size();
            return;
        }
    }
}

// Abort all contexts. The main context will only be returned by manager if
// it is manager-owned (keptByCaller==false) and forceReturnMain==true.
void CContextMgr::AbortAll() {
    for (asUINT i = 0; i < m_coRoutines.size(); ++i) {
        asIScriptContext *ctx = m_coRoutines[i];
        if (!ctx)
            continue;
        if (ctx == m_mainCtx && m_mainKeptByCaller) {
            ctx->Abort();
            // caller will DoneWithContext later
        }
    }

    // Remove main from list if kept; otherwise clear everything.
    if (m_mainKeptByCaller && m_mainCtx) {
        for (int i = (int)m_coRoutines.size() - 1; i >= 0; --i)
            if (m_coRoutines[i] == m_mainCtx)
                m_coRoutines.remove(i);
    } else {
        m_coRoutines.clear();
        m_currentCoRoutine = 0;
    }

    ProcessPendingReturns();
}

// Caller returns a kept context to engine. If ctx is ACTIVE, defer the return.
void CContextMgr::DoneWithContext(asIScriptContext *ctx) {
    if (!ctx)
        return;

    int state = ctx->GetState();
    Q_ASSERT(state != asEXECUTION_ACTIVE);
    if (state == asEXECUTION_ACTIVE) {
        return;
    }
    if (state == asEXECUTION_SUSPENDED) {
        ctx->Abort();
    }

    ctx->GetEngine()->ReturnContext(ctx);
}

asIScriptContext *CContextMgr::MainContext() const { return m_mainCtx; }

// Try to Return contexts that are no longer ACTIVE.
void CContextMgr::ProcessPendingReturns() {
    if (m_pendingReturns.isEmpty())
        return;
    for (int i = (int)m_pendingReturns.size() - 1; i >= 0; --i) {
        asIScriptContext *ctx = m_pendingReturns[i];
        if (!ctx) {
            m_pendingReturns.remove(i);
            continue;
        }
        int state = ctx->GetState();
        if (state != asEXECUTION_ACTIVE) {
            ctx->GetEngine()->ReturnContext(ctx);
            m_pendingReturns.remove(i);
        }
    }
}

// Whether there are still active contexts (including main if kept by caller).
bool CContextMgr::HasActive() const {
    if (!m_coRoutines.isEmpty())
        return true;
    if (m_mainCtx && m_mainKeptByCaller)
        return true;
    return false;
}

QString CContextMgr::scriptName() const { return m_scriptName; }

void CContextMgr::setScriptName(const QString &newScriptName) {
    m_scriptName = newScriptName;
}

END_AS_NAMESPACE
