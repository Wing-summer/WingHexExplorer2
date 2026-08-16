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

#include "luauutil.h"
#include "Luau/BytecodeBuilder.h"
#include "Luau/Compiler.h"
#include "luau/VM/src/lapi.h"
#include "luau/VM/src/lstate.h"

#include <lualib.h>

#include <QtDebug>

namespace LuauUtil {

StackGuard::StackGuard(lua_State *L, int capacity)
    : L_(L), top_(lua_gettop(L)) {
    lua_checkstack(L, capacity);
}

StackGuard::~StackGuard() { lua_settop(L_, top_); }

ReadOnlyGuard::ReadOnlyGuard(lua_State *L, int index) : L_(L), index_(index) {
    enable_ = lua_getreadonly(L, index);
    lua_setreadonly(L, index, false);
}

ReadOnlyGuard::~ReadOnlyGuard() { lua_setreadonly(L_, index_, enable_); }

DisableDebugStep::DisableDebugStep(lua_State *L) {
    main_vm_ = lua_mainthread(L);
    callbacks_ = lua_callbacks(main_vm_);
    old_debug_step_ = callbacks_->debugstep;
    callbacks_->debugstep = nullptr;
}

LuauUtil::DisableDebugStep::~DisableDebugStep() {
    callbacks_->debugstep = old_debug_step_;
}

bool pushBreakEnv(lua_State *L, int level) {
    lua_Debug ar;
    lua_checkstack(L, 10);

    // Create new table for break environment
    lua_newtable(L);

    // Push function at level
    if (!lua_getinfo(L, level, "f", &ar)) {
        qCritical("[pushBreakEnv] Failed to get function info at level %d",
                  level);
        lua_pop(L, 1);
        return false;
    }

    // Get function env
    lua_getfenv(L, -1);

    // -1: function env table
    // -2: function
    // -3: break env table
    int break_env = lua_absindex(L, -3);
    int fenv = lua_absindex(L, -1);

    // set fenv as metatable
    lua_newtable(L);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, fenv);
    lua_rawset(L, -3);
    lua_setmetatable(L, break_env);
    lua_pop(L, 1);

    // -1: function
    // -2: env table

    int index = 1;
    while (auto *name = lua_getlocal(L, level, index++)) {
        lua_pushstring(L, name);
        lua_insert(L, -2);

        // -1: value
        // -2: key
        // -3: function
        // -4: table
        lua_rawset(L, -4);
    }

    index = 1;
    while (auto *name = lua_getupvalue(L, -1, index++)) {
        lua_pushstring(L, name);
        lua_insert(L, -2);
        lua_rawset(L, -4);
    }

    // Pop function
    lua_pop(L, 1);

    return true;
}

std::optional<int> eval(lua_State *L, const QString &code, int env) {
    DisableDebugStep _(L);

    lua_setsafeenv(L, LUA_ENVIRONINDEX, false);
    auto env_idx = lua_absindex(L, env);
    int top = lua_gettop(L);

    auto c = code.toStdString();
    Luau::BytecodeBuilder bcb;
    try {
        Luau::compileOrThrow(bcb, std::string("return ") + c);
    } catch (const std::exception &) {
        try {
            Luau::compileOrThrow(bcb, c);
        } catch (const std::exception &e) {
            qCritical("Error compiling code: %s", e.what());
            lua_pushstring(L, e.what());
            return std::nullopt;
        }
    }

    auto bytecode = bcb.getBytecode();
    int result = luau_load(L, c.c_str(), bytecode.data(), bytecode.size(), 0);
    if (result != 0)
        return std::nullopt;

    lua_pushvalue(L, env_idx);
    lua_setfenv(L, -2);

    int call_result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (call_result == LUA_OK)
        return lua_gettop(L) - top;
    else {
        qCritical("Error running code: %s", lua_tostring(L, -1));
        return std::nullopt;
    }
}

bool setLocal(lua_State *L, int level, const QByteArray &name, int index) {
    auto value_idx = lua_absindex(L, index);
    int n = 1;
    while (const char *local_name = lua_getlocal(L, level, n)) {
        if (name == local_name) {
            lua_pushvalue(L, value_idx);
            auto *result = lua_setlocal(L, level, n);
            lua_pop(L, 1);
            return result != nullptr;
        }
        lua_pop(L, 1);
        ++n;
    }
    return false;
}

bool setUpvalue(lua_State *L, int level, const QByteArray &name, int index) {
    auto value_idx = lua_absindex(L, index);
    lua_Debug ar;
    lua_checkstack(L, 1);
    if (!lua_getinfo(L, level, "f", &ar)) {
        qCritical("[setUpvalue] Failed to get function info at level %d",
                  level);
        return false;
    }
    auto function_index = lua_absindex(L, -1);

    int n = 1;
    while (const char *local_name = lua_getupvalue(L, function_index, n)) {
        if (name == local_name) {
            lua_pushvalue(L, value_idx);
            auto *result = lua_setupvalue(L, function_index, n);
            Q_ASSERT(result != nullptr);
            lua_pop(L, 2);
            return result != nullptr;
        }
        lua_pop(L, 1);
        ++n;
    }
    lua_pop(L, 1);
    return false;
}

Closure *getLuaFunction(lua_State *L, int index) {
    auto o = luaA_toobject(L, index);
    return isLfunction(o) ? clvalue(o) : nullptr;
}

Closure *getCFunction(lua_State *L, int index) {
    auto o = luaA_toobject(L, index);
    return iscfunction(o) ? clvalue(o) : nullptr;
}

} // namespace LuauUtil
