// Copyright (c) 2026, wingsummer
// SPDX-License-Identifier: BSD-3-Clause

#include <iostream>
#include <limits>

#include <QCoreApplication>

#include <lua.h>
#include <lualib.h>

#include "WingPlugin/binding/builder.h"
#include "qtluau/value_conversion.hpp"

#include "generated_bindings.hpp"
#include "luau/luauformatter.h"
#include "luau/scriptbindingengine.h"

namespace {
void require(bool result, const char *message) {
    if (!result) {
        throw std::runtime_error(message);
    }
}

qint64 signedNumber(qint64 value) { return value; }

quint64 unsignedNumber(quint64 value) { return value; }

void checkPluginNumber(lua_State *state, const char *name, double value,
                       bool accepted) {
    lua_getglobal(state, name);
    lua_pushnumber(state, value);
    const auto status = lua_pcall(state, 1, 1, 0);
    require((status == LUA_OK) == accepted, "Plugin integer range mismatch");
    if (accepted) {
        require(lua_type(state, -1) == LUA_TNUMBER &&
                    lua_tonumber(state, -1) == value,
                "Plugin integer lost precision");
    }
    lua_pop(state, 1);
}

void checkNumbers(lua_State *state) {
    const auto formatted = LuauFormat::format(QStringLiteral(
        "local values={2147483648,9007199254740991,9007199254740993,"
        "0xffffffffffffffff,1e100}\n"));
    require(formatted.has_value(), "Numeric literals failed to format");
    for (const auto *literal :
         {"2147483648", "9007199254740991", "9007199254740993",
          "0xffffffffffffffff", "1e100"}) {
        require(formatted->formatted.contains(QString::fromLatin1(literal)),
                "Formatting changed a numeric literal's spelling");
    }
    bindings::registerPublicModule(state);
    const QList<qlonglong> signedValues = {
        std::numeric_limits<qlonglong>::min(),
        -9007199254740993LL,
        -9007199254740991LL,
        -2147483649LL,
        2147483648LL,
        9007199254740991LL,
        9007199254740993LL,
        std::numeric_limits<qlonglong>::max()};
    const QList<qulonglong> unsignedValues = {
        0, 4294967295ULL, 9007199254740991ULL, 9007199254740993ULL,
        std::numeric_limits<qulonglong>::max()};
    require(qtluau::qtFromLua<QList<qlonglong>>(
                qtluau::qtToLua(state, signedValues)) == signedValues,
            "Signed Qt integer lost precision");
    require(qtluau::qtFromLua<QList<qulonglong>>(
                qtluau::qtToLua(state, unsignedValues)) == unsignedValues,
            "Unsigned Qt integer lost precision");
    const auto boxed =
        qtluau::qtToLua(state, std::numeric_limits<qulonglong>::max());
    bool rejected = false;
    try {
        qtluau::qtFromLua<qlonglong>(boxed);
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    require(rejected, "Unsigned box overflowed signed integer");
    WingScriptBind::ScriptBindingEngine engine(state);
    engine.global()
        .addFunction(u"signed_number", &signedNumber)
        .addFunction(u"unsigned_number", &unsignedNumber);
    for (const double number : {-9007199254740991.0, -2147483649.0,
                                2147483648.0, 9007199254740991.0}) {
        checkPluginNumber(state, "signed_number", number, true);
    }
    checkPluginNumber(state, "signed_number", 9007199254740992.0, false);
    checkPluginNumber(state, "signed_number", -9007199254740992.0, false);
    checkPluginNumber(state, "unsigned_number", 4294967295.0, true);
    checkPluginNumber(state, "unsigned_number", 9007199254740992.0, false);
    checkPluginNumber(state, "unsigned_number", -1.0, false);
}
} // namespace

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    auto *state = luaL_newstate();
    luaL_openlibs(state);
    int status = 0;
    try {
        checkNumbers(state);
        require(lua_gettop(state) == 0,
                "Numeric conversion leaked stack values");
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        status = 1;
    }
    qtluau::clearLuaCallbacks(state);
    lua_close(state);
    return status;
}
