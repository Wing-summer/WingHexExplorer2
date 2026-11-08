// Copyright (C) 2026 WingSummer
// SPDX-License-Identifier: AGPL-3.0-or-later
#include "luau/luauutil.h"
#include "luau/wingluaurequire.h"
#include <Luau/Compiler.h>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <iostream>

static bool run(lua_State *L, const QByteArray &source, const char *name) {
    const auto bytecode = Luau::compile(source.toStdString());
    if (luau_load(L, name, bytecode.data(), bytecode.size(), 0) != 0 ||
        lua_pcall(L, 0, 0, 0) != 0) {
        std::cerr << lua_tostring(L, -1) << '\n';
        return false;
    }
    return true;
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    QTemporaryDir directory;
    if (!directory.isValid()) {
        return 1;
    }
    for (const auto &entry :
         {qMakePair(QStringLiteral(WING_SOURCE "/src/luau/modules/hexdoc.luau"),
                    QStringLiteral("hexdoc.luau")),
          qMakePair(QStringLiteral(HEX_BINDINGS "/generated/hexdoc_api.luau"),
                    QStringLiteral("hexdoc_api.luau"))}) {
        if (!QFile::copy(entry.first, directory.filePath(entry.second))) {
            return 1;
        }
    }
    auto *L = luaL_newstate();
    luaL_openlibs(L);
    WingLuauRequire require;
    WingLuauRequire::Options options;
    options.systemRoot = directory.path();
    if (!require.initRequire(L, options)) {
        lua_close(L);
        return 1;
    }
    require.installRequire();
    require.installProxyRequire();
    QFile setup(QStringLiteral(WING_SOURCE "/tests/hexdoc/runtime.luau"));
    if (!setup.open(QIODevice::ReadOnly) ||
        !run(L, setup.readAll(), "=hexdoc-tests")) {
        lua_close(L);
        return 1;
    }
    lua_getglobal(L, "ctl");
    lua_pushcfunction(L, &LuauUtil::callSync, "ctl._call_sync");
    lua_setfield(L, -2, "_call_sync");
    lua_pop(L, 1);
    const bool ok = run(L, "run_hexdoc_tests()", "=hexdoc-run");
    lua_close(L);
    return ok ? 0 : 1;
}
