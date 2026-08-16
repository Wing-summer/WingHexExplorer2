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

LuauScope::LuauScope(int key) : key_(key) { type_ = Type::Unknown; }

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
    scope.type_ = Type::Table;
    return scope;
}

LuauScope LuauScope::createUserData(lua_State *L, int index) {
    const TValue *u = luaA_toobject(L, index);
    auto *address = uvalue(u);
    auto scope = createFromAddress(L, index, address);
    scope.type_ = Type::UserData;
    return scope;
}

bool LuauScope::operator==(const LuauScope &other) const {
    return key_ == other.key_;
}

LuauScope &LuauScope::operator=(const LuauScope &other) {
    L_ = other.L_;
    key_ = other.key_;
    name_ = other.name_;
    type_ = other.type_;
    newRef(other.ref_);
    return *this;
}

int LuauScope::getKey() const { return key_; }

QString LuauScope::getName() const { return name_; }

void LuauScope::setName(const QString &name) { name_ = name; }

int LuauScope::getLevel() const { return level_; }

void LuauScope::setLevel(int level) { level_ = level; }

lua_State *LuauScope::getLuaState() const { return L_; }

bool LuauScope::isLocal() const { return type_ == Type::Local; }

bool LuauScope::isUpvalue() const { return type_ == Type::UpValue; }

bool LuauScope::isTable() const { return type_ == Type::Table; }

bool LuauScope::isUserData() const { return type_ == Type::UserData; }

bool LuauScope::isLoaded() const {
    if (!isTable() && !isUserData())
        return true;
    return loaded_;
}

bool LuauScope::markLoaded() const { return loaded_ = true; }

bool LuauScope::markUnloaded() const { return loaded_ = false; }

bool LuauScope::pushRef() const {
    if (ref_ == LUA_REFNIL || L_ == nullptr)
        return false;

    lua_checkstack(L_, 1);
    lua_getref(L_, ref_);
    return true;
}

LuauScope LuauScope::createWithType(lua_State *L, const QString &name,
                                    Type type) {
    LuauScope scope;
    scope.createKey(qHash(name));
    scope.L_ = L;
    scope.type_ = type;
    scope.name_ = name;
    return scope;
}

void LuauScope::createKey(std::size_t hash) { key_ = hash & 0x7FFFFFFF; }

void LuauScope::newRef(int ref) {
    if (ref == LUA_REFNIL)
        return;

    lua_checkstack(L_, 1);
    lua_getref(L_, ref);
    ref_ = lua_ref(L_, -1);
    lua_pop(L_, 1);
}

LuauScope::Type LuauScope::getType() const { return type_; }
