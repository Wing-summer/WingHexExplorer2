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

#ifndef LUAUDEBUGGER_H
#define LUAUDEBUGGER_H

#include <QStack>

#include "debugger/luauvariableregistry.h"

#include "lua.h"

class LuauDebugger {
public:
    LuauDebugger();

public:
    void attach(lua_State *L);
    void detach();

public:
    QVector<lua_State *> getThreadAncestors(lua_State *L);

    void pushThreadStack(lua_State *state);
    void popThreadStack();

private:
    lua_State *getParent(lua_State *L) const;

private:
    lua_State *L_ = nullptr;

    QStack<lua_State *> thread_stack_;
    LuauVariableRegistry variable_registry_;
};

#endif // LUAUDEBUGGER_H
