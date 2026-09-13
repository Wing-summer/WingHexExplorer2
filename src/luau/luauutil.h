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

#ifndef LUAUUTIL_H
#define LUAUUTIL_H

#include "luau/VM/src/lobject.h"

#include <QString>

#include <lua.h>
#include <optional>

namespace LuauUtil {

class StackGuard {
public:
    StackGuard(lua_State *L, int capacity = 5);
    ~StackGuard();

private:
    lua_State *L_;
    int top_;
};

class ReadOnlyGuard {
public:
    ReadOnlyGuard(lua_State *L, int index);
    ~ReadOnlyGuard();

private:
    lua_State *L_;
    int index_;
    bool enable_;
};

class DisableDebugStep {
public:
    DisableDebugStep(lua_State *L);
    ~DisableDebugStep();

private:
    lua_State *main_vm_;
    lua_Callbacks *callbacks_;
    using DebugStep = void (*)(lua_State *, lua_Debug *);
    DebugStep old_debug_step_;
};

int callMetaProtected(lua_State *L, int obj, const char *event);

bool pushBreakEnv(lua_State *L, int level);

std::optional<int> eval(lua_State *L, const QString &code, int env);

bool setLocal(lua_State *L, int level, const QByteArray &name, int index);

bool setUpvalue(lua_State *L, int level, const QByteArray &name, int index);

Closure *getLuaFunction(lua_State *L, int index);

Closure *getCFunction(lua_State *L, int index);

QString normalizeLuauRequirePath(const QString &path);

void lua_errorL(lua_State *L, const char *msg);

} // namespace LuauUtil

#endif // LUAUUTIL_H
