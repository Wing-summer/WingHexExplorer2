#include "scriptenv.h"

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
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "string readEnvironmentVar(const string &in name, const string "
        "&in defaultValue = \"\")",
        asFUNCTION(readEnvironmentVar), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "bool isEnvironmentVarSet(const string &in name)",
        asFUNCTION(isEnvironmentVarSet), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "bool isEnvironmentVarEmpty(const string &in name)",
        asFUNCTION(isEnvironmentVarEmpty), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction("int readEnvironmentVarInt(const string "
                                       "&in name, bool &out ok = void)",
                                       asFUNCTION(readEnvironmentVarInt),
                                       asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    engine->SetDefaultNamespace("");
}
