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

#include "class/luauscheduler.h"
#include "luau/luauutil.h"

LuauDebugger::LuauDebugger() : QObject() {}

void LuauDebugger::attach(lua_State *L) {
    if (L == nullptr) {
        return;
    }

    if (attached_) {
        detach();
    }

    L_ = lua_mainthread(L);
    thread_stack_.clear();
    thread_stack_.push(L_);
    callbacks_ = lua_callbacks(L_);
    callbacks_->debugbreak = &LuauDebugger::debugBreak;
    callbacks_->debugstep = &LuauDebugger::debugStep;
    attached_ = true;
    lua_singlestep(L_, true);
}

void LuauDebugger::detach() {
    if (!attached_) {
        return;
    }

    // If we are currently parked in the break loop, make sure we release the
    // VM so it can finish running before we tear the hooks down.
    if (breakLoop_ != nullptr) {
        processSingleStep(nullptr);
        resumeInternal();
    }

    if (callbacks_ != nullptr) {
        callbacks_->debugbreak = nullptr;
        callbacks_->debugstep = nullptr;
    }
    lua_singlestep(L_, false);

    for (auto &file : files_) {
        file->removeRef(L_);
    }
    files_.clear();
    variable_registry_.clear();
    thread_stack_.clear();
    stackFrames_.clear();
    frameStates_.clear();
    frameLevels_.clear();
    frameDepths_.clear();
    callbacks_ = nullptr;
    L_ = nullptr;
    breakVm_ = nullptr;
    attached_ = false;
    resume_ = true;
    pauseRequested_ = false;
    terminateRequested_ = false;
    singleStepProcessor_ = nullptr;
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
    if (debugger->terminateRequested_) {
        LuauUtil::throwError(L, "Thread execution is terminated");
        return;
    }
    if (debugger->singleStepProcessor_ == nullptr) {
        return;
    }
    if (debugger->singleStepProcessor_(L, ar)) {
        debugger->onDebugBreak(L, ar, BreakReason::Step);
    }
}

void LuauDebugger::onLuaFileLoaded(lua_State *L, const QString &path,
                                   const QByteArray &source) {
    if (L == nullptr) {
        return;
    }

    lua_singlestep(L, true);
    auto f = files_.value(path);
    if (f) {
        if (!source.isEmpty()) {
            f->setSource(QString::fromUtf8(source));
        }
        const auto adjustedLines = f->addRef(LuaFileRef(L));
        for (const auto &[oldLine, newLine] : adjustedLines) {
            Q_EMIT onAdjustBreakPointLine(path, oldLine, newLine);
        }
    } else {
        auto file = LuauFileContext::create(path);
        file->setSource(QString::fromUtf8(source));
        const auto adjustedLines = file->addRef(LuaFileRef(L));
        files_.insert(path, file);
        for (const auto &[oldLine, newLine] : adjustedLines) {
            Q_EMIT onAdjustBreakPointLine(path, oldLine, newLine);
        }
    }
}

void LuauDebugger::resume() {
    processSingleStep(nullptr);
    resumeInternal();
}

void LuauDebugger::pause() {
    if (!attached_ || isDebugBreak()) {
        return;
    }
    pauseRequested_ = true;
    processSingleStep(
        [this](lua_State *, lua_Debug *) { return pauseRequested_; });
}

void LuauDebugger::terminate() {
    if (!attached_) {
        return;
    }

    terminateRequested_ = true;
    if (isDebugBreak()) {
        processSingleStep([](lua_State *, lua_Debug *) { return false; });
        resume_ = true;
        if (breakLoop_ != nullptr) {
            breakLoop_->quit();
        }
        Q_EMIT onDebugActionExec();
    } else {
        processSingleStep([](lua_State *, lua_Debug *) { return false; });
    }
}

void LuauDebugger::setBreakPoints(
    const QString &path,
    const std::unordered_map<int, BreakPoint> &breakpoints) {
    auto file = files_.value(path);
    if (!file) {
        file = LuauFileContext::create(path);
        files_.insert(path, file);
    }
    file->setBreakPoints(breakpoints);
}

void LuauDebugger::clearBreakPoints() {
    for (auto &file : files_) {
        file->clearBreakPoints();
    }
}

void LuauDebugger::onDebugBreak(lua_State *L, lua_Debug *, BreakReason reason) {
    if (L == nullptr || !attached_) {
        return;
    }

    if (reason == BreakReason::BreakPoint && !hitBreakPoint(L)) {
        return;
    }

    breakVm_ = L;
    breakContext_ = getBreakContext(L);
    resume_ = false;
    pauseRequested_ = false;
    terminateRequested_ = false;
    processSingleStep(nullptr);
    const auto frameState = L;
    frameStates_.clear();
    frameLevels_.clear();
    frameDepths_.clear();
    stackFrames_ = updateStackFrames(L);
    variable_registry_.update(getThreadAncestors(frameState));

    Q_EMIT onRunCurrentLine(breakContext_.source_, breakContext_.line_);
    Q_EMIT onPullVariables();
    Q_EMIT onPullCallStack();

    waitForResume();
}

bool LuauDebugger::isDebugBreak() { return attached_ && !resume_; }

void LuauDebugger::stepOver() {
    if (!isDebugBreak())
        return;

    const auto oldContext = breakContext_;
    processSingleStep([this, oldContext](lua_State *L, lua_Debug *) {
        if (oldContext.L_ != nullptr && oldContext.L_->status == LUA_YIELD) {
            return false;
        }
        const auto context = getBreakContext(L);
        if (L != oldContext.L_ && getParent(L) != oldContext.L_) {
            return false;
        }
        return (context.depth_ == oldContext.depth_ &&
                context.line_ != oldContext.line_) ||
               context.depth_ < oldContext.depth_;
    });
    resumeInternal();
}

void LuauDebugger::stepIn() {
    if (!isDebugBreak()) {
        return;
    }
    const auto oldContext = breakContext_;
    processSingleStep([this, oldContext](lua_State *L, lua_Debug *) {
        const auto context = getBreakContext(L);
        return context.L_ != oldContext.L_ ||
               context.line_ != oldContext.line_ ||
               context.depth_ != oldContext.depth_;
    });
    resumeInternal();
}

void LuauDebugger::stepOut() {
    if (!isDebugBreak()) {
        return;
    }
    const auto oldContext = breakContext_;
    processSingleStep([this, oldContext](lua_State *L, lua_Debug *) {
        return getBreakContext(L).depth_ < oldContext.depth_;
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
            frameStates_.append(sourceState);
            frameLevels_.append(level);
            frameDepths_.append(depth);
            ++depth;
        }
        L = getParent(L);
    }

    return frames;
}

LuauFileContext LuauDebugger::findLoadedLuauFile(const QString &path) {
    return files_.value(path);
}

const QVector<LuauStackFrame> &LuauDebugger::stackFrames() const {
    return stackFrames_;
}

LuauVariableRegistry *LuauDebugger::variableRegistry() {
    return &variable_registry_;
}

LuauVariable::Ptr LuauDebugger::evaluateExpression(const QString &expression,
                                                   int frameId) {
    if (!isDebugBreak() || frameId < 0 || frameId >= frameStates_.size()) {
        return {};
    }

    auto exp = expression.trimmed();
    if (exp.isEmpty()) {
        return {};
    }

    auto *L = frameStates_.at(frameId);
    const auto level = frameLevels_.at(frameId);
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
    return variable_registry_.createVariable(L, exp, level);
}

LuauScope LuauDebugger::localScope(int frameId) const {
    if (frameId < 0 || frameId >= frameStates_.size()) {
        return {};
    }
    return LuauVariableRegistry::getLocalScope(frameStates_.at(frameId),
                                               frameDepths_.at(frameId));
}

LuauScope LuauDebugger::upvalueScope(int frameId) const {
    if (frameId < 0 || frameId >= frameStates_.size()) {
        return {};
    }
    return LuauVariableRegistry::getUpvalueScope(frameStates_.at(frameId),
                                                 frameDepths_.at(frameId));
}

LuauScope LuauDebugger::globalScope() const {
    return LuauVariableRegistry::getGlobalScope(breakVm_);
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
    ctx.source_ = getDebugSource(ar);
    ctx.line_ = ar.currentline;
    ctx.depth_ = getStackDepth(L);
    ctx.L_ = L;
    return ctx;
}

void LuauDebugger::processSingleStep(SingleStepProcessor processor) {
    singleStepProcessor_ = std::move(processor);
    enableDebugStep(singleStepProcessor_ != nullptr);
}

void LuauDebugger::enableDebugStep(bool enable) {
    if (L_ == nullptr || callbacks_ == nullptr) {
        return;
    }
    callbacks_->debugstep = enable ? &LuauDebugger::debugStep : nullptr;
    lua_singlestep(L_, enable);
    if (breakVm_ != nullptr && breakVm_ != L_) {
        lua_singlestep(breakVm_, enable);
    }
}

void LuauDebugger::resumeInternal() {
    if (!isDebugBreak()) {
        return;
    }
    resume_ = true;
    pauseRequested_ = false;
    variable_registry_.clear();
    stackFrames_.clear();
    frameStates_.clear();
    frameLevels_.clear();
    frameDepths_.clear();
    if (breakLoop_ != nullptr) {
        breakLoop_->quit();
    }
    Q_EMIT onDebugActionExec();
}

void LuauDebugger::waitForResume() {
    QEventLoop loop;
    breakLoop_ = &loop;
    loop.exec();
    breakLoop_ = nullptr;
    breakVm_ = nullptr;
    variable_registry_.clear();
    stackFrames_.clear();
    frameStates_.clear();
    frameLevels_.clear();
    frameDepths_.clear();
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
    auto file = files_.value(getDebugSource(ar));
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
    thread_stack_.push(state);
}

void LuauDebugger::popThreadStack() { thread_stack_.pop(); }

lua_State *LuauDebugger::getParent(lua_State *L) const {
    auto it = std::find(thread_stack_.begin(), thread_stack_.end(), L);
    if (it != thread_stack_.end()) {
        return it == thread_stack_.begin() ? nullptr : *(it - 1);
    }
    return thread_stack_.isEmpty() ? nullptr : thread_stack_.back();
}
