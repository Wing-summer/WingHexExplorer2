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

#include "luauvariableregistry.h"

#include "luau/luautypes.h"
#include "luauutil.h"

void LuauVariableRegistry::clear() { variables_.clear(); }

void LuauVariableRegistry::update(const QVector<lua_State *> &vm_chain) {
    if (vm_chain.isEmpty())
        return;

    auto *scope_state = vm_chain.first();
    LuauUtil::DisableDebugStep _(scope_state);
    depth_ = 0;
    for (lua_State *L : vm_chain) {
        fetch(L, scope_state);
    }
    fetchGlobals(scope_state);
    clearDirtyScopes();
}

LuauScope LuauVariableRegistry::getLocalScope(lua_State *L, int level) {
    return LuauScope::createLocal(
        L, QStringLiteral("___locals__%1_%2").arg(level).arg(quintptr(L)));
}

LuauScope LuauVariableRegistry::getUpvalueScope(lua_State *L, int level) {
    return LuauScope::createLocal(
        L, QStringLiteral("___upvalues__%1_%2").arg(level).arg(quintptr(L)));
}

LuauScope LuauVariableRegistry::getGlobalScope(lua_State *L) {
    return LuauScope::createGlobal(L, QStringLiteral("___globals__"));
}

LuauVariable::Ptr LuauVariableRegistry::createVariable(lua_State *L,
                                                       const QString &name,
                                                       int level) {
    return LuauVariable::Ptr::create(this, L, name, level);
}

bool LuauVariableRegistry::registerVariables(LuauScope scope,
                                             LuauVariableList variables) {
    if (variables_.contains(scope)) {
        return false;
    }
    Q_ASSERT(variables);
    variables_.insert(scope, variables);
    return true;
}

void LuauVariableRegistry::registerOrUpdateVariables(
    LuauScope scope, LuauVariableList variables) {
    Q_ASSERT(variables);
    variables_.insert(scope, variables);
}

bool LuauVariableRegistry::isRegistered(LuauScope scope) const {
    return variables_.contains(scope);
}

LuauVariableList LuauVariableRegistry::getVariables(const LuauScope &scope,
                                                    bool load) {
    auto it = variables_.find(scope);
    if (it == variables_.end()) {
        return {};
    }

    auto &s = it.key();
    if (load && !s.isLoaded()) {
        LuauVariable::loadFields(this, s);
        s.markLoaded();
    }

    return it.value();
}

QPair<const LuauScope, LuauVariableList>
LuauVariableRegistry::getVariables(int reference) {
    LuauScope scope(reference);
    auto it = variables_.find(scope);
    if (it == variables_.end()) {
        return {};
    }
    return qMakePair(it.key(), it.value());
}

void LuauVariableRegistry::fetchGlobals(lua_State *L) {
    LuauUtil::StackGuard guard(L);
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    auto globals = LuauVariableList::create();
    lua_pushnil(L);
    while (lua_next(L, -2)) {
        auto name = LuauTypes::toString(L, -2);
        globals->append(createVariable(L, name, -1));
        lua_pop(L, 1);
    }
    registerOrUpdateVariables(getGlobalScope(L), globals);
}

void LuauVariableRegistry::clearDirtyScopes() {
    for (auto &&[scope, variables] : variables_.asKeyValueRange()) {
        if (scope.isTable() || scope.isUserData()) {
            variables->clear();
            scope.markUnloaded();
        }
    }
}

void LuauVariableRegistry::fetch(lua_State *L, lua_State *src) {
    LuauUtil::StackGuard guard(L);
    lua_Debug ar;
    for (int level = 0; lua_getinfo(L, level, "sln", &ar); ++level) {
        if (ar.what[0] == 'C') {
            continue;
        }
        fetchFromStack(L, level, src);
        ++depth_;
    }
}

void LuauVariableRegistry::fetchFromStack(lua_State *L, int level,
                                          lua_State *src) {
    // Register local variables
    auto variables = LuauVariableList::create();
    int index = 1;
    while (const char *name = lua_getlocal(L, level, index++)) {
        variables->append(createVariable(L, QString::fromUtf8(name), level));
        lua_pop(L, 1);
    }
    registerOrUpdateVariables(getLocalScope(src, depth_), variables);

    // Register upvalues
    lua_Debug ar = {};
    lua_getinfo(L, level, "f", &ar);
    auto upvalues = LuauVariableList::create();
    index = 1;
    while (const char *name = lua_getupvalue(L, -1, index++)) {
        upvalues->append(createVariable(L, QString::fromUtf8(name), level));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    registerOrUpdateVariables(getUpvalueScope(src, depth_), upvalues);
}
