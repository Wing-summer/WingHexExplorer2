/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include "luauvariable.h"

#include <lualib.h>

#include "debugger/luautypes.h"
#include "debugger/luauutil.h"
#include "debugger/luauvariableregistry.h"

LuauScope LuauVariable::getScope() const { return scope_; }

bool LuauVariable::isTable() const { return type_ == LUA_TTABLE; }

bool LuauVariable::isUserData() const { return type_ == LUA_TUSERDATA; }

bool LuauVariable::hasFields() const { return isTable() || isUserData(); }

QString LuauVariable::getName() const { return name_; }

QString LuauVariable::getValue() const { return value_; }

QString LuauVariable::getType() const { return LuauTypes::getTypeName(type_); }

QString LuauVariable::setValue(LuauScope scope, const QString &value) {
    if (!LuauUtil::pushBreakEnv(L_, level_))
        throw std::runtime_error("Failed to push break environment");

    auto result = LuauUtil::eval(L_, preprocess(value), -1);
    if (!result.has_value()) {
        auto error = LuauTypes::toString(L_, -1);
        lua_pop(L_, 2);
        throw std::runtime_error(error.toStdString());
    }

    auto ret_count = result.value();
    if (ret_count == 0) {
        lua_pop(L_, 1);
        return value_;
    }

    if (ret_count > 1)
        lua_pop(L_, ret_count - 1);

    auto name = name_.toUtf8();
    auto new_value = LuauTypes::toDbgString(L_, -1);
    if (scope.isTable() || scope.isUserData()) {
        if (scope.pushRef()) {
            // -1: key
            // -2: table | userdata
            // -3: value
            // -4: env
            lua_checkstack(L_, 2);
            if (index_.has_value()) {
                lua_pushinteger(L_, index_.value());
            } else {
                lua_pushstring(L_, name_.toUtf8());
            }
            lua_pushvalue(L_, -3);
            lua_settable(L_, -3);
            lua_pop(L_, 3);
        }
    } else if (scope.isLocal()) {
        if (!LuauUtil::setLocal(L_, level_, name, -1)) {
            lua_pop(L_, 2);
            throw std::runtime_error("Failed to set local variable");
        }
    } else if (scope.isUpvalue()) {
        if (!LuauUtil::setUpvalue(L_, level_, name, -1)) {
            lua_pop(L_, 2);
            throw std::runtime_error("Failed to set upvalue");
        }
    } else {
        lua_pop(L_, 2);
        throw std::runtime_error("Invalid scope");
    }

    lua_pop(L_, 2);
    return new_value;
}

void LuauVariable::loadFields(LuauVariableRegistry *registry,
                              const LuauScope &scope) {
    auto *L = scope.getLuaState();
    if (L == nullptr) {
        return;
    }
    LuauUtil::StackGuard guard(L);
    if (!scope.pushRef()) {
        return;
    }

    int value_idx = lua_absindex(L, -1);

    // https://github.com/luau-lang/rfcs/blob/master/docs/generalized-iteration.md
    if (luaL_getmetafield(L, value_idx, "__iter")) {
        addIterFields(registry, L, scope, value_idx);
    } else if (hasGetters(L, value_idx)) {
        addCustomFields(registry, L, scope, value_idx);
    } else if (scope.isTable()) {
        addRawFields(registry, L, scope, value_idx);
    }
}

LuauVariable::LuauVariable(LuauVariableRegistry *registry, lua_State *L,
                           const QString &name, int level)
    : L_(L), level_(level), name_(name) {
    type_ = lua_type(L, -1);
    value_ = LuauTypes::toDbgString(L, -1);
    addScope(registry, L);
}

void LuauVariable::addScope(LuauVariableRegistry *registry, lua_State *L) {
    if (!hasFields())
        return;

    if (isTable())
        scope_ = LuauScope::createTable(L);
    else if (isUserData())
        scope_ = LuauScope::createUserData(L);

    scope_.setName(name_);
    scope_.setLevel(level_);

    if (!registry->isRegistered(scope_)) {
        registry->registerVariables(scope_, LuauVariableList::create());
    }
}

void LuauVariable::addRawFields(LuauVariableRegistry *registry, lua_State *L,
                                const LuauScope &scope, int value_idx) {
    auto variables = registry->getVariables(scope, false);
    if (!variables) {
        return;
    }

    variables->clear();

    lua_pushnil(L);
    while (lua_next(L, value_idx)) {
        variables->emplace_back(addField(L, registry, scope));
        lua_pop(L, 1);
    }

    lua_pushliteral(L, "__metatable");
    if (lua_getmetatable(L, value_idx)) {
        variables->emplace_back(addField(L, registry, scope));
    }
}

void LuauVariable::addIterFields(LuauVariableRegistry *registry, lua_State *L,
                                 const LuauScope &scope, int value_idx) {
    auto variables = registry->getVariables(scope, false);
    if (!variables)
        return;

    variables->clear();

    lua_pushvalue(L, value_idx);
    int call_result = lua_pcall(L, 1, 3, 0);
    if (call_result != LUA_OK) {
        qCritical("[Variable::registryFields] Failed to call __iter "
                  "for %s, error: %s",
                  qUtf8Printable(scope.getName()), lua_tostring(L, -1));
        return;
    }

    auto next = lua_absindex(L, -3);
    auto state = lua_absindex(L, -2);
    auto init = lua_absindex(L, -1);

    lua_pushvalue(L, next);
    lua_pushvalue(L, state);
    lua_pushvalue(L, init);
    while (true) {
        if (LUA_OK != lua_pcall(L, 2, 2, 0)) {
            qCritical("[Variable::registryFields] Failed to call "
                      "__iter for %s, error: %s",
                      qUtf8Printable(scope.getName()), lua_tostring(L, -1));
            return;
        }
        if (lua_isnil(L, -2))
            return;

        variables->append(addField(L, registry, scope));

        // pop value
        lua_pop(L, 1);

        // prepare for next iteration
        lua_pushvalue(L, next);
        lua_pushvalue(L, state);
        lua_pushvalue(L, -3);
        lua_remove(L, -4);
    }
}

void LuauVariable::addCustomFields(LuauVariableRegistry *registry, lua_State *L,
                                   const LuauScope &scope, int value_idx) {
    auto variables = registry->getVariables(scope, false);
    if (!variables)
        return;

    variables->clear();

    lua_pushnil(L);
    while (lua_next(L, -2)) {
        lua_pushvalue(L, value_idx);

        // call getter to retrieve the value
        lua_pcall(L, 1, 1, 0);
        variables->emplace_back(addField(L, registry, scope));
        lua_pop(L, 1);
    }
}

LuauVariable::Ptr LuauVariable::addField(lua_State *L,
                                         LuauVariableRegistry *registry,
                                         const LuauScope &scope) {
    int key_type = lua_type(L, -2);
    auto field_name = LuauTypes::toString(L, -2);
    if (key_type == LUA_TNUMBER) {
        field_name = QStringLiteral("[%1]").arg(lua_tointeger(L, -2));
    } else if (key_type == LUA_TINTEGER) {
        field_name =
            QStringLiteral("[%1]").arg(lua_tointeger64(L, -2, nullptr));
    }
    auto variable = registry->createVariable(L, field_name, scope.getLevel());
    if (key_type == LUA_TNUMBER) {
        variable->index_ = lua_tointeger(L, -2);
    } else if (key_type == LUA_TINTEGER) {
        variable->index_ = static_cast<int>(lua_tointeger64(L, -2, nullptr));
    }
    return variable;
}

bool LuauVariable::hasGetters(lua_State *L, int value_idx) {
    if (luaL_getmetafield(L, value_idx, "__getters") != 1) {
        return false;
    }

    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return false;
    }

    return true;
}

QString LuauVariable::preprocess(const QString &input_value) {
    if (type_ == LUA_TSTRING) {
        return QStringLiteral("[[") + input_value + ("]]");
    } else if (type_ == LUA_TVECTOR) {
        return QStringLiteral("vector.create") + input_value;
    }

    return input_value;
}
