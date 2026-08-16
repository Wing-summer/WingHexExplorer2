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

#ifndef LUAUVARIABLEREGISTRY_H
#define LUAUVARIABLEREGISTRY_H

#include "luauscope.h"
#include "luauvariable.h"

#include <QHash>
#include <QVector>

class LuauVariableRegistry {
public:
    LuauVariableRegistry() = default;

public:
    void clear();
    void update(const QVector<lua_State *> &vm_chain);

    static LuauScope getLocalScope(lua_State *L, int level);
    static LuauScope getUpvalueScope(lua_State *L, int level);
    static LuauScope getGlobalScope(lua_State *L);

    LuauVariable::Ptr createVariable(lua_State *L, const QString &name,
                                     int level);

    bool registerVariables(LuauScope scope, LuauVariableList variables);
    void registerOrUpdateVariables(LuauScope scope, LuauVariableList variables);
    bool isRegistered(LuauScope scope) const;
    LuauVariableList getVariables(const LuauScope &scope, bool load);
    QPair<const LuauScope, LuauVariableList> getVariables(int reference);

    void fetchGlobals(lua_State *L);
    void clearDirtyScopes();

private:
    void fetch(lua_State *L, lua_State *src);
    void fetchFromStack(lua_State *L, int level, lua_State *src);

private:
    QHash<LuauScope, LuauVariableList> variables_;
    int depth_ = 0;
};

#endif // LUAUVARIABLEREGISTRY_H
