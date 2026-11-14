// Copyright (c) 2026, wingsummer
// SPDX-License-Identifier: BSD-3-Clause

#include <iostream>
#include <memory>

#include <QFile>

#include <lua.h>
#include <lualib.h>

#include "Luau/Compiler.h"

#include "qtluau/integer64.hpp"

namespace {
void require(bool result, const char *message) {
    if (!result) {
        throw std::runtime_error(message);
    }
}

void checkRuntime(const std::string &bytecode) {
    const std::unique_ptr<lua_State, decltype(&lua_close)> state(
        luaL_newstate(), lua_close);
    auto *L = state.get();
    luaL_openlibs(L);
    qtluau::registerInteger64(L);
    lua_getglobal(L, "int64");
    const auto *constructor = lua_topointer(L, -1);
    lua_pop(L, 1);
    qtluau::pushInteger64(L, std::numeric_limits<qint64>::min());
    lua_getmetatable(L, -1);
    const auto *metatable = lua_topointer(L, -1);
    lua_pop(L, 2);
    for (int iteration = 0; iteration < 3; ++iteration) {
        qtluau::registerInteger64(L);
        lua_getglobal(L, "int64");
        require(lua_topointer(L, -1) == constructor,
                "Repeated registration replaced the constructor");
        lua_pop(L, 1);
        qtluau::pushInteger64(L, 0);
        lua_getmetatable(L, -1);
        require(lua_topointer(L, -1) == metatable,
                "Repeated registration replaced the metatable");
        lua_pop(L, 2);
    }
    qtluau::pushInteger64(L, std::numeric_limits<qint64>::min());
    lua_setglobal(L, "existing_signed");
    qtluau::pushUnsignedInteger64(L, std::numeric_limits<quint64>::max());
    lua_setglobal(L, "existing_unsigned");
    lua_getglobal(L, "uint64");
    const auto *unsignedConstructor = lua_topointer(L, -1);
    lua_pop(L, 1);
    qtluau::registerInteger64(L);
    lua_getglobal(L, "uint64");
    require(lua_topointer(L, -1) == unsignedConstructor,
            "Repeated registration replaced the unsigned constructor");
    lua_pop(L, 1);
    luaL_sandbox(L);
    luaL_sandboxthread(L);
    qtluau::registerInteger64(L);
    require(lua_gettop(L) == 0, "Registration leaked stack values");
    const auto loaded =
        luau_load(L, "numbers.luau", bytecode.data(), bytecode.size(), 0);
    require(loaded == LUA_OK, "Could not load integer regression script");
    const auto status = lua_pcall(L, 0, 0, 0);
    if (status != LUA_OK) {
        throw std::runtime_error(lua_tostring(L, -1));
    }
    require(lua_gettop(L) == 0, "Arithmetic leaked stack values");
}
} // namespace

int main() {
    try {
        QFile script(QStringLiteral(WING_NUMBER_SCRIPT));
        require(script.open(QIODevice::ReadOnly),
                "Could not read integer script");
        const auto source = script.readAll();
        const auto bytecode = Luau::compile(source.toStdString());
        for (int iteration = 0; iteration < 3; ++iteration) {
            checkRuntime(bytecode);
        }
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
