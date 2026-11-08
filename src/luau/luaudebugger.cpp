/*==============================================================================
 ** Copyright (C) 2026-2029 WingSummer
 **
 ** This program is free software: you can redistribute it and/or modify it
 ** under the terms of the GNU Affero General Public License as published by the
 ** Free Software Foundation, version 3.
 **
 ** This program is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 ** for more details.
 **
 ** You should have received a copy of the GNU Affero General Public License
 ** along with this program. If not, see <https://www.gnu.org/licenses/>.
 ** =============================================================================
 */

#include "luaudebugger.h"

#include "luau/luauutil.h"
#include "luau/runtime/luauscheduler.h"

LuauDebugger::LuauDebugger() : QObject() {}

void LuauDebugger::attach(lua_State *L) {
    if (L == nullptr) {
        return;
    }

    if (m_attached) {
        detach();
    }

    m_state = lua_mainthread(L);
    m_threadStack.clear();
    m_threadStack.push(m_state);
    m_callbacks = lua_callbacks(m_state);
    m_callbacks->debugbreak = &LuauDebugger::debugBreak;
    m_callbacks->debugstep = &LuauDebugger::debugStep;
    m_attached = true;
    lua_singlestep(m_state, true);
}

void LuauDebugger::detach() {
    if (!m_attached) {
        return;
    }

    // If we are currently parked in the break loop, make sure we release the
    // VM so it can finish running before we tear the hooks down.
    if (m_breakLoop != nullptr) {
        processSingleStep(nullptr);
        resumeInternal();
    }

    if (m_callbacks != nullptr) {
        m_callbacks->debugbreak = nullptr;
        m_callbacks->debugstep = nullptr;
    }
    lua_singlestep(m_state, false);

    for (auto &file : m_files) {
        file->removeRef(m_state);
    }
    m_files.clear();
    m_variableRegistry.clear();
    m_threadStack.clear();
    m_stackFrames.clear();
    m_frameStates.clear();
    m_frameLevels.clear();
    m_frameDepths.clear();
    m_callbacks = nullptr;
    m_state = nullptr;
    m_breakVm = nullptr;
    m_attached = false;
    m_resume = true;
    m_pauseRequested = false;
    m_terminateRequested = false;
    m_singleStepProcessor = nullptr;
}

void LuauDebugger::debugBreak(lua_State *L, lua_Debug *ar) {
    auto pd = reinterpret_cast<LuauThreadData *>(lua_getthreaddata(L));
    if (pd == nullptr) {
        return;
    }
    auto *debugger = pd->debugger;
    if (debugger != nullptr) {
        debugger->onDebugBreak(L, ar, BreakReason::BreakPoint);
    }
}

void LuauDebugger::debugStep(lua_State *L, lua_Debug *ar) {
    auto pd = reinterpret_cast<LuauThreadData *>(lua_getthreaddata(L));
    if (pd == nullptr) {
        return;
    }
    auto *debugger = pd->debugger;
    if (debugger == nullptr) {
        return;
    }
    if (debugger->m_terminateRequested) {
        LuauUtil::throwError(L, "Thread execution is terminated");
        return;
    }
    if (debugger->m_singleStepProcessor == nullptr) {
        return;
    }
    if (debugger->m_singleStepProcessor(L, ar)) {
        debugger->onDebugBreak(L, ar, BreakReason::Step);
    }
}

void LuauDebugger::onLuaFileLoaded(lua_State *L, const QString &path,
                                   const QByteArray &source) {
    if (L == nullptr) {
        return;
    }

    lua_singlestep(L, true);
    auto f = m_files.value(path);
    if (f) {
        if (!source.isEmpty()) {
            f->setSource(QString::fromUtf8(source));
        }
        const auto adjustedLines = f->addRef(LuaFileRef(L));
        for (const auto &[oldLine, newLine] : adjustedLines) {
            Q_EMIT sigOnAdjustBreakPointLine(path, oldLine, newLine);
        }
    } else {
        auto file = LuauFileContext::create(path);
        file->setSource(QString::fromUtf8(source));
        const auto adjustedLines = file->addRef(LuaFileRef(L));
        m_files.insert(path, file);
        for (const auto &[oldLine, newLine] : adjustedLines) {
            Q_EMIT sigOnAdjustBreakPointLine(path, oldLine, newLine);
        }
    }
}

void LuauDebugger::resume() {
    processSingleStep(nullptr);
    resumeInternal();
}

void LuauDebugger::pause() {
    if (!m_attached || isDebugBreak()) {
        return;
    }
    m_pauseRequested = true;
    processSingleStep(
        [this](lua_State *, lua_Debug *) { return m_pauseRequested; });
}

void LuauDebugger::terminate() {
    if (!m_attached) {
        return;
    }

    m_terminateRequested = true;
    if (isDebugBreak()) {
        processSingleStep([](lua_State *, lua_Debug *) { return false; });
        m_resume = true;
        if (m_breakLoop != nullptr) {
            m_breakLoop->quit();
        }
        Q_EMIT sigOnDebugActionExec();
    } else {
        processSingleStep([](lua_State *, lua_Debug *) { return false; });
    }
}

void LuauDebugger::setBreakPoints(
    const QString &path,
    const std::unordered_map<int, BreakPoint> &breakpoints) {
    auto file = m_files.value(path);
    if (!file) {
        file = LuauFileContext::create(path);
        m_files.insert(path, file);
    }
    file->setBreakPoints(breakpoints);
}

void LuauDebugger::clearBreakPoints() {
    for (auto &file : m_files) {
        file->clearBreakPoints();
    }
}

void LuauDebugger::onDebugBreak(lua_State *L, lua_Debug *, BreakReason reason) {
    if (L == nullptr || !m_attached) {
        return;
    }

    if (reason == BreakReason::BreakPoint && !hitBreakPoint(L)) {
        return;
    }

    m_breakVm = L;
    m_breakContext = getBreakContext(L);
    m_resume = false;
    m_pauseRequested = false;
    m_terminateRequested = false;
    processSingleStep(nullptr);
    const auto frameState = L;
    m_frameStates.clear();
    m_frameLevels.clear();
    m_frameDepths.clear();
    m_stackFrames = updateStackFrames(L);
    m_variableRegistry.update(getThreadAncestors(frameState));

    Q_EMIT sigOnRunCurrentLine(m_breakContext.source, m_breakContext.line);
    Q_EMIT sigOnPullVariables();
    Q_EMIT sigOnPullCallStack();

    waitForResume();
}

bool LuauDebugger::isDebugBreak() { return m_attached && !m_resume; }

void LuauDebugger::stepOver() {
    if (!isDebugBreak()) {
        return;
    }

    const auto oldContext = m_breakContext;
    processSingleStep([this, oldContext](lua_State *L, lua_Debug *) {
        if (oldContext.state != nullptr &&
            oldContext.state->status == LUA_YIELD) {
            return false;
        }
        const auto context = getBreakContext(L);
        if (L != oldContext.state && getParent(L) != oldContext.state) {
            return false;
        }
        return (context.depth == oldContext.depth &&
                context.line != oldContext.line) ||
               context.depth < oldContext.depth;
    });
    resumeInternal();
}

void LuauDebugger::stepIn() {
    if (!isDebugBreak()) {
        return;
    }
    const auto oldContext = m_breakContext;
    processSingleStep([this, oldContext](lua_State *L, lua_Debug *) {
        const auto context = getBreakContext(L);
        return context.state != oldContext.state ||
               context.line != oldContext.line ||
               context.depth != oldContext.depth;
    });
    resumeInternal();
}

void LuauDebugger::stepOut() {
    if (!isDebugBreak()) {
        return;
    }
    const auto oldContext = m_breakContext;
    processSingleStep([this, oldContext](lua_State *L, lua_Debug *) {
        return getBreakContext(L).depth < oldContext.depth;
    });
    resumeInternal();
}

QVector<LuauStackFrame> LuauDebugger::updateStackFrames(lua_State *L) {
    QVector<LuauStackFrame> frames;
    lua_Debug ar;
    int depth = 0;
    const auto sourceState = L;
    while (L != nullptr) {
        for (int level = 0; lua_getinfo(L, level, "sln", &ar); ++level) {
            if (ar.what[0] == 'C') {
                continue;
            }
            LuauStackFrame frame;
            frame.name = ar.name ? QString::fromUtf8(ar.name)
                                 : QStringLiteral("<anonymous>");
            frame.source = getDebugSource(ar);
            frame.line = ar.currentline;
            frame.id = frames.size();
            frames.append(std::move(frame));
            m_frameStates.append(sourceState);
            m_frameLevels.append(level);
            m_frameDepths.append(depth);
            ++depth;
        }
        L = getParent(L);
    }

    return frames;
}

LuauFileContext LuauDebugger::findLoadedLuauFile(const QString &path) {
    return m_files.value(path);
}

const QVector<LuauStackFrame> &LuauDebugger::stackFrames() const {
    return m_stackFrames;
}

LuauVariableRegistry *LuauDebugger::variableRegistry() {
    return &m_variableRegistry;
}

LuauVariable::Ptr LuauDebugger::evaluateExpression(const QString &expression,
                                                   int frameId) {
    if (!isDebugBreak() || frameId < 0 || frameId >= m_frameStates.size()) {
        return {};
    }

    auto exp = expression.trimmed();
    if (exp.isEmpty()) {
        return {};
    }

    auto *L = m_frameStates.at(frameId);
    const auto level = m_frameLevels.at(frameId);
    LuauUtil::StackGuard guard(L);
    if (!LuauUtil::pushBreakEnv(L, level)) {
        return {};
    }

    const auto result = LuauUtil::eval(L, exp, -1);
    if (!result.has_value() || result.value() <= 0) {
        return {};
    }
    while (result.value() > 1) {
        lua_remove(L, -2);
        break;
    }
    return m_variableRegistry.createVariable(L, exp, level);
}

LuauScope LuauDebugger::localScope(int frameId) const {
    if (frameId < 0 || frameId >= m_frameStates.size()) {
        return {};
    }
    return LuauVariableRegistry::getLocalScope(m_frameStates.at(frameId),
                                               m_frameDepths.at(frameId));
}

LuauScope LuauDebugger::upvalueScope(int frameId) const {
    if (frameId < 0 || frameId >= m_frameStates.size()) {
        return {};
    }
    return LuauVariableRegistry::getUpvalueScope(m_frameStates.at(frameId),
                                                 m_frameDepths.at(frameId));
}

LuauScope LuauDebugger::globalScope() const {
    return LuauVariableRegistry::getGlobalScope(m_breakVm);
}

LuauStackFrame LuauDebugger::stackFrame(int frameId) const {
    if (frameId < 0 || frameId >= m_stackFrames.size()) {
        return {};
    }
    return m_stackFrames.at(frameId);
}

int LuauDebugger::getStackDepth(lua_State *L) const {
    int depth = lua_stackdepth(L);
    auto *parent = getParent(L);
    while (parent != nullptr) {
        depth += lua_stackdepth(parent);
        parent = getParent(parent);
    }
    return depth;
}

BreakContext LuauDebugger::getBreakContext(lua_State *L) const {
    lua_Debug ar;
    lua_getinfo(L, 0, "sl", &ar);
    BreakContext ctx;
    ctx.source = getDebugSource(ar);
    ctx.line = ar.currentline;
    ctx.depth = getStackDepth(L);
    ctx.state = L;
    return ctx;
}

void LuauDebugger::processSingleStep(SingleStepProcessor processor) {
    m_singleStepProcessor = std::move(processor);
    enableDebugStep(m_singleStepProcessor != nullptr);
}

void LuauDebugger::enableDebugStep(bool enable) {
    if (m_state == nullptr || m_callbacks == nullptr) {
        return;
    }
    m_callbacks->debugstep = enable ? &LuauDebugger::debugStep : nullptr;
    lua_singlestep(m_state, enable);
    if (m_breakVm != nullptr && m_breakVm != m_state) {
        lua_singlestep(m_breakVm, enable);
    }
}

void LuauDebugger::resumeInternal() {
    if (!isDebugBreak()) {
        return;
    }
    m_resume = true;
    m_pauseRequested = false;
    m_variableRegistry.clear();
    m_stackFrames.clear();
    m_frameStates.clear();
    m_frameLevels.clear();
    m_frameDepths.clear();
    if (m_breakLoop != nullptr) {
        m_breakLoop->quit();
    }
    Q_EMIT sigOnDebugActionExec();
}

void LuauDebugger::waitForResume() {
    QEventLoop loop;
    m_breakLoop = &loop;
    loop.exec();
    m_breakLoop = nullptr;
    m_breakVm = nullptr;
    m_variableRegistry.clear();
    m_stackFrames.clear();
    m_frameStates.clear();
    m_frameLevels.clear();
    m_frameDepths.clear();
}

QString LuauDebugger::getDebugSource(lua_Debug ar) {
    auto src = ar.source;
    if (src == nullptr) {
        return QStringLiteral("<unknown>");
    }
    if (*src == '@') {
        return QString::fromUtf8(src + 1);
    } else {
        return QString::fromUtf8(src);
    }
}

bool LuauDebugger::hitBreakPoint(lua_State *L) const {
    return findBreakPoint(L) != nullptr;
}

BreakPoint *LuauDebugger::findBreakPoint(lua_State *L) const {
    lua_Debug ar;
    if (L == nullptr || !lua_getinfo(L, 0, "sl", &ar) || ar.source == nullptr) {
        return nullptr;
    }
    auto file = m_files.value(getDebugSource(ar));
    return file ? file->findBreakPoint(ar.currentline) : nullptr;
}

QVector<lua_State *> LuauDebugger::getThreadAncestors(lua_State *L) {
    QVector<lua_State *> ancestors;
    while (L != nullptr) {
        ancestors.append(L);
        L = getParent(L);
    }
    return ancestors;
}

void LuauDebugger::pushThreadStack(lua_State *state) {
    m_threadStack.push(state);
}

void LuauDebugger::popThreadStack() { m_threadStack.pop(); }

lua_State *LuauDebugger::getParent(lua_State *L) const {
    auto it = std::find(m_threadStack.begin(), m_threadStack.end(), L);
    if (it != m_threadStack.end()) {
        return it == m_threadStack.begin() ? nullptr : *(it - 1);
    }
    return m_threadStack.isEmpty() ? nullptr : m_threadStack.back();
}
