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

#include "luau/luautypes.h"
#include "luau/luauutil.h"
#include "luau/luauvariableregistry.h"

LuauScope LuauVariable::getScope() const { return m_scope; }

bool LuauVariable::isTable() const { return m_type == LUA_TTABLE; }

bool LuauVariable::isUserData() const { return m_type == LUA_TUSERDATA; }

bool LuauVariable::hasFields() const { return isTable() || isUserData(); }

bool LuauVariable::hasContent() const {
    if (!hasFields()) {
        return false;
    }

    auto *L = m_scope.getLuaState();
    if (L == nullptr) {
        return false;
    }

    LuauUtil::StackGuard guard(L);

    if (!m_scope.pushRef()) {
        return false;
    }

    const int valueIdx = lua_absindex(L, -1);

    if (luaL_getmetafield(L, valueIdx, "__iter")) {
        // stack:
        //   value
        //   __iter

        lua_pushvalue(L, valueIdx);

        // __iter(value) -> next, state, initial
        if (lua_pcall(L, 1, 3, 0) != LUA_OK) {
            return false;
        }

        const int nextIdx = lua_absindex(L, -3);
        const int stateIdx = lua_absindex(L, -2);
        const int initIdx = lua_absindex(L, -1);

        lua_pushvalue(L, nextIdx);
        lua_pushvalue(L, stateIdx);
        lua_pushvalue(L, initIdx);

        if (lua_pcall(L, 2, 2, 0) != LUA_OK) {
            return false;
        }

        // next(...) -> key, value
        return !lua_isnil(L, -2);
    }

    if (luaL_getmetafield(L, valueIdx, "__getters")) {
        if (lua_istable(L, -1)) {
            lua_pushnil(L);
            return lua_next(L, -2) != 0;
        }

        lua_pop(L, 1);
    }

    // table
    if (!m_scope.isTable()) {
        return false;
    }

    lua_pushnil(L);
    if (lua_next(L, valueIdx) != 0) {
        return true;
    }

    // dectect metatable
    if (lua_getmetatable(L, valueIdx)) {
        return true;
    }

    return false;
}

QString LuauVariable::getName() const { return m_name; }

QString LuauVariable::getValue() const { return m_value; }

QString LuauVariable::getType() const { return LuauTypes::getTypeName(m_type); }

QString LuauVariable::setValue(LuauScope scope, const QString &value) {
    if (!LuauUtil::pushBreakEnv(m_state, m_level)) {
        throw std::runtime_error("Failed to push break environment");
    }

    auto result = LuauUtil::eval(m_state, preprocess(value), -1);
    if (!result.has_value()) {
        auto error = LuauTypes::toString(m_state, -1);
        lua_pop(m_state, 2);
        throw std::runtime_error(error.toStdString());
    }

    auto retCount = result.value();
    if (retCount == 0) {
        lua_pop(m_state, 1);
        return m_value;
    }

    if (retCount > 1) {
        lua_pop(m_state, retCount - 1);
    }

    auto name = m_name.toUtf8();
    auto newValue = LuauTypes::toDbgString(m_state, -1);
    if (scope.isTable() || scope.isUserData()) {
        if (scope.pushRef()) {
            // -1: key
            // -2: table | userdata
            // -3: value
            // -4: env
            lua_checkstack(m_state, 2);
            if (m_index.has_value()) {
                lua_pushinteger(m_state, m_index.value());
            } else {
                lua_pushstring(m_state, m_name.toUtf8());
            }
            lua_pushvalue(m_state, -3);
            lua_settable(m_state, -3);
            lua_pop(m_state, 3);
        }
    } else if (scope.isLocal()) {
        if (!LuauUtil::setLocal(m_state, m_level, name, -1)) {
            lua_pop(m_state, 2);
            throw std::runtime_error("Failed to set local variable");
        }
    } else if (scope.isUpvalue()) {
        if (!LuauUtil::setUpvalue(m_state, m_level, name, -1)) {
            lua_pop(m_state, 2);
            throw std::runtime_error("Failed to set upvalue");
        }
    } else {
        lua_pop(m_state, 2);
        throw std::runtime_error("Invalid scope");
    }

    lua_pop(m_state, 2);
    return newValue;
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

    int valueIdx = lua_absindex(L, -1);

    // https://github.com/luau-lang/rfcs/blob/master/docs/generalized-iteration.md
    if (luaL_getmetafield(L, valueIdx, "__iter")) {
        addIterFields(registry, L, scope, valueIdx);
    } else if (hasGetters(L, valueIdx)) {
        addCustomFields(registry, L, scope, valueIdx);
    } else if (scope.isTable()) {
        addRawFields(registry, L, scope, valueIdx);
    }
}

LuauVariable::LuauVariable(LuauVariableRegistry *registry, lua_State *L,
                           const QString &name, int level)
    : m_state(L), m_level(level), m_name(name) {
    m_type = lua_type(L, -1);
    m_value = LuauTypes::toDbgString(L, -1);
    addScope(registry, L);
}

void LuauVariable::addScope(LuauVariableRegistry *registry, lua_State *L) {
    if (!hasFields()) {
        return;
    }

    if (isTable()) {
        m_scope = LuauScope::createTable(L);
    } else if (isUserData()) {
        m_scope = LuauScope::createUserData(L);
    }

    m_scope.setName(m_name);
    m_scope.setLevel(m_level);

    if (!registry->isRegistered(m_scope)) {
        registry->registerVariables(m_scope, LuauVariableList::create());
    }
}

void LuauVariable::addRawFields(LuauVariableRegistry *registry, lua_State *L,
                                const LuauScope &scope, int valueIdx) {
    auto variables = registry->getVariables(scope, false);
    if (!variables) {
        return;
    }

    variables->clear();

    lua_pushnil(L);
    while (lua_next(L, valueIdx)) {
        variables->emplace_back(addField(L, registry, scope));
        lua_pop(L, 1);
    }

    lua_pushliteral(L, "__metatable");
    if (lua_getmetatable(L, valueIdx)) {
        variables->emplace_back(addField(L, registry, scope));
    }
}

void LuauVariable::addIterFields(LuauVariableRegistry *registry, lua_State *L,
                                 const LuauScope &scope, int valueIdx) {
    auto variables = registry->getVariables(scope, false);
    if (!variables) {
        return;
    }

    variables->clear();

    lua_pushvalue(L, valueIdx);
    int callResult = lua_pcall(L, 1, 3, 0);
    if (callResult != LUA_OK) {
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
        if (lua_isnil(L, -2)) {
            return;
        }

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
                                   const LuauScope &scope, int valueIdx) {
    auto variables = registry->getVariables(scope, false);
    if (!variables) {
        return;
    }

    variables->clear();

    lua_pushnil(L);
    while (lua_next(L, -2)) {
        lua_pushvalue(L, valueIdx);

        // call getter to retrieve the value
        lua_pcall(L, 1, 1, 0);
        variables->emplace_back(addField(L, registry, scope));
        lua_pop(L, 1);
    }
}

LuauVariable::Ptr LuauVariable::addField(lua_State *L,
                                         LuauVariableRegistry *registry,
                                         const LuauScope &scope) {
    int keyType = lua_type(L, -2);
    auto fieldName = LuauTypes::toString(L, -2);
    if (keyType == LUA_TNUMBER) {
        fieldName = QStringLiteral("[%1]").arg(lua_tointeger(L, -2));
    } else if (keyType == LUA_TINTEGER) {
        fieldName = QStringLiteral("[%1]").arg(lua_tointeger64(L, -2, nullptr));
    }
    auto variable = registry->createVariable(L, fieldName, scope.getLevel());
    if (keyType == LUA_TNUMBER) {
        variable->m_index = lua_tointeger(L, -2);
    } else if (keyType == LUA_TINTEGER) {
        variable->m_index = static_cast<int>(lua_tointeger64(L, -2, nullptr));
    }
    return variable;
}

bool LuauVariable::hasGetters(lua_State *L, int valueIdx) {
    if (luaL_getmetafield(L, valueIdx, "__getters") != 1) {
        return false;
    }

    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return false;
    }

    return true;
}

QString LuauVariable::preprocess(const QString &inputValue) {
    if (m_type == LUA_TSTRING) {
        return QStringLiteral("[[") + inputValue + ("]]");
    } else if (m_type == LUA_TVECTOR) {
        return QStringLiteral("vector.create") + inputValue;
    }

    return inputValue;
}
