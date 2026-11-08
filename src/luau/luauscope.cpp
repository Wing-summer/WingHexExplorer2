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

#include "luauscope.h"

#include <QHash>

LuauScope::LuauScope(int key) : m_key(key) { m_type = Type::Unknown; }

LuauScope LuauScope::createLocal(lua_State *L, const QString &name) {
    return createWithType(L, name, Type::Local);
}

LuauScope LuauScope::createUpvalue(lua_State *L, const QString &name) {
    return createWithType(L, name, Type::UpValue);
}

LuauScope LuauScope::createGlobal(lua_State *L, const QString &name) {
    return createWithType(L, name, Type::Global);
}

LuauScope LuauScope::createTable(lua_State *L, int index) {
    const TValue *t = luaA_toobject(L, index);
    auto *address = hvalue(t);
    auto scope = createFromAddress(L, index, address);
    scope.m_type = Type::Table;
    return scope;
}

LuauScope LuauScope::createUserData(lua_State *L, int index) {
    const TValue *u = luaA_toobject(L, index);
    auto *address = uvalue(u);
    auto scope = createFromAddress(L, index, address);
    scope.m_type = Type::UserData;
    return scope;
}

bool LuauScope::operator==(const LuauScope &other) const {
    return m_key == other.m_key;
}

LuauScope &LuauScope::operator=(const LuauScope &other) {
    m_state = other.m_state;
    m_key = other.m_key;
    m_name = other.m_name;
    m_type = other.m_type;
    newRef(other.m_ref);
    return *this;
}

int LuauScope::getKey() const { return m_key; }

QString LuauScope::getName() const { return m_name; }

void LuauScope::setName(const QString &name) { m_name = name; }

int LuauScope::getLevel() const { return m_level; }

void LuauScope::setLevel(int level) { m_level = level; }

lua_State *LuauScope::getLuaState() const { return m_state; }

bool LuauScope::isLocal() const { return m_type == Type::Local; }

bool LuauScope::isUpvalue() const { return m_type == Type::UpValue; }

bool LuauScope::isTable() const { return m_type == Type::Table; }

bool LuauScope::isUserData() const { return m_type == Type::UserData; }

bool LuauScope::isLoaded() const {
    if (!isTable() && !isUserData()) {
        return true;
    }
    return m_loaded;
}

bool LuauScope::markLoaded() const { return m_loaded = true; }

bool LuauScope::markUnloaded() const { return m_loaded = false; }

bool LuauScope::pushRef() const {
    if (m_ref == LUA_REFNIL || m_state == nullptr) {
        return false;
    }

    lua_checkstack(m_state, 1);
    lua_getref(m_state, m_ref);
    return true;
}

LuauScope LuauScope::createWithType(lua_State *L, const QString &name,
                                    Type type) {
    LuauScope scope;
    scope.createKey(qHash(name));
    scope.m_state = L;
    scope.m_type = type;
    scope.m_name = name;
    return scope;
}

void LuauScope::createKey(std::size_t hash) { m_key = hash & 0x7FFFFFFF; }

void LuauScope::newRef(int ref) {
    if (ref == LUA_REFNIL) {
        return;
    }

    lua_checkstack(m_state, 1);
    lua_getref(m_state, ref);
    m_ref = lua_ref(m_state, -1);
    lua_pop(m_state, 1);
}

LuauScope::Type LuauScope::getType() const { return m_type; }
