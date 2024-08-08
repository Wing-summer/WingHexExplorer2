#include "scriptregex.h"

BEGIN_AS_NAMESPACE

#include <QRegularExpression>

QRegularExpression *ScriptFileSystem_Factory() {
    return new QRegularExpression();
}

void RegisterScriptRegex(asIScriptEngine *engine) {
    int r = engine->RegisterObjectType(
        "regex", sizeof(QRegularExpression),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<QRegularExpression>());
    assert(r >= 0);
}

END_AS_NAMESPACE
