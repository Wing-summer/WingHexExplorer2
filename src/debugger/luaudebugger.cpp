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

LuauDebugger::LuauDebugger() {}

void LuauDebugger::attach(lua_State *L) {
    if (L == nullptr) {
        return;
    }

    lua_Callbacks *cb = lua_callbacks(L);
}

void LuauDebugger::detach() {}

bool LuauDebugger::isDebugBreak() {}

void LuauDebugger::stepOver() {
    if (!isDebugBreak())
        return;

    // TODO

    resumeInternal();
}

void LuauDebugger::stepIn() {}

void LuauDebugger::stepOut() {}

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
    ctx.source_ = QString::fromUtf8(ar.source);
    ctx.line_ = ar.currentline;
    ctx.depth_ = getStackDepth(L);
    ctx.L_ = L;
    return ctx;
}

void LuauDebugger::enableDebugStep(bool enable) {}

void LuauDebugger::resumeInternal() {}

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
