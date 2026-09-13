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

#ifndef LUAUVARIABLE_H
#define LUAUVARIABLE_H

#include "luauscope.h"

#include "luau/luauinspector.h"

#include <QSharedPointer>
#include <QString>

#include <optional>

class LuauVariableRegistry;

class LuauVariable {
public:
    LuauScope getScope() const;
    bool isTable() const;
    bool isUserData() const;
    bool hasFields() const;
    QString getName() const;
    QString getValue() const;
    QString getType() const;

    QString setValue(LuauScope scope, const QString &value);

    static void loadFields(LuauVariableRegistry *registry,
                           const LuauScope &scope);

public:
    using Ptr = QSharedPointer<LuauVariable>;
    friend Ptr;
    using Ref = QWeakPointer<LuauVariable>;

private:
    friend class LuauVariableRegistry;
    LuauVariable(LuauVariableRegistry *registry, lua_State *L,
                 const QString &name, int level);
    void addScope(LuauVariableRegistry *registry, lua_State *L);

    static void addRawFields(LuauVariableRegistry *registry, lua_State *L,
                             const LuauScope &scope, int value_idx);
    static void addIterFields(LuauVariableRegistry *registry, lua_State *L,
                              const LuauScope &scope, int value_idx);
    static void addCustomFields(LuauVariableRegistry *registry, lua_State *L,
                                const LuauScope &scope, int value_idx);
    static LuauVariable::Ptr addField(lua_State *L,
                                      LuauVariableRegistry *registry,
                                      const LuauScope &scope);

    static bool hasGetters(lua_State *L, int value_idx);

    QString preprocess(const QString &input_value);

private:
    lua_State *L_ = nullptr;
    int level_ = 0;
    QString name_;
    std::optional<int> index_ = std::nullopt;
    QString value_;
    LuauScope scope_;
    int type_ = LUA_TNIL;
};

using LuauVariableList = QSharedPointer<QVector<LuauVariable::Ptr>>;

#endif // LUAUVARIABLE_H
