#ifndef SCRIPTREGEX_H
#define SCRIPTREGEX_H

#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif

BEGIN_AS_NAMESPACE

void RegisterScriptRegex(asIScriptEngine *engine);

END_AS_NAMESPACE

#endif // SCRIPTREGEX_H
