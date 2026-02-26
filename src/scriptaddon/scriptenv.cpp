/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
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

#include "scriptenv.h"
#include "define.h"

#include <QString>
#include <QtGlobal>

static QString readEnvironmentVar(const QString &env, const QString &defValue) {
    return qEnvironmentVariable(env.toUtf8(), defValue);
}

static bool isEnvironmentVarSet(const QString &env) {
    return qEnvironmentVariableIsSet(env.toUtf8());
}

static bool isEnvironmentVarEmpty(const QString &env) {
    return qEnvironmentVariableIsEmpty(env.toUtf8());
}

static int readEnvironmentVarInt(const QString &env, bool *ok) {
    return qEnvironmentVariableIntValue(env.toUtf8(), ok);
}

void RegisterEnv(asIScriptEngine *engine) {
    auto r = engine->SetDefaultNamespace("env");
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "string readEnvironmentVar(const string &in name, const string "
        "&in defaultValue = \"\")",
        asFUNCTION(readEnvironmentVar), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "bool isEnvironmentVarSet(const string &in name)",
        asFUNCTION(isEnvironmentVarSet), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "bool isEnvironmentVarEmpty(const string &in name)",
        asFUNCTION(isEnvironmentVarEmpty), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction("int readEnvironmentVarInt(const string "
                                       "&in name, bool &out ok = void)",
                                       asFUNCTION(readEnvironmentVarInt),
                                       asCALL_CDECL);
    ASSERT(r >= 0);

    engine->SetDefaultNamespace("");
}
