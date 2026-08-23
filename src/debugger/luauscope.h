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

#ifndef LUAUSCOPE_H
#define LUAUSCOPE_H

#include <lua.h>
#include <luau/VM/src/lapi.h>
#include <luau/VM/src/lstate.h>

#include <QHash>
#include <QString>

class LuauScope {
public:
    enum class Type {
        Local,
        UpValue,
        Global,
        Table,
        UserData,
        Unknown,
    };

public:
    LuauScope() = default;
    explicit LuauScope(int key);

    static LuauScope createLocal(lua_State *L, const QString &name);

    static LuauScope createUpvalue(lua_State *L, const QString &name);

    static LuauScope createGlobal(lua_State *L, const QString &name);

    static LuauScope createTable(lua_State *L, int index = -1);

    static LuauScope createUserData(lua_State *L, int index = -1);

    bool operator==(const LuauScope &other) const;
    LuauScope &operator=(const LuauScope &other);

    int getKey() const;
    QString getName() const;
    Type getType() const;
    void setName(const QString &name);
    int getLevel() const;
    void setLevel(int level);
    lua_State *getLuaState() const;

    bool isLocal() const;
    bool isUpvalue() const;
    bool isTable() const;
    bool isUserData() const;
    bool isLoaded() const;
    bool markLoaded() const;
    bool markUnloaded() const;

    bool pushRef() const;

private:
    static LuauScope createWithType(lua_State *L, const QString &name,
                                    Type type);

    template <class T>
    static inline LuauScope createFromAddress(lua_State *L, int index,
                                              const T *address) {
        LuauScope scope;
        scope.L_ = L;
        scope.createKey(std::hash<const T *>{}(address));
        scope.ref_ = lua_ref(L, index);
        return scope;
    }

    void createKey(std::size_t hash);

    void newRef(int ref);

private:
    int key_ = 0;
    QString name_;
    Type type_ = Type::Unknown;
    lua_State *L_ = nullptr;
    int ref_ = LUA_REFNIL;
    mutable bool loaded_ = false;
    int level_ = 0;
};

inline size_t qHash(const LuauScope &key, size_t seed = 0) noexcept {
    return qHash(key.getKey(), seed);
}

#endif // LUAUSCOPE_H
