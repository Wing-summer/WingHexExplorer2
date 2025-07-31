#ifndef SCRIPTENV_H
#define SCRIPTENV_H

#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif

BEGIN_AS_NAMESPACE

void RegisterEnv(asIScriptEngine *engine);

END_AS_NAMESPACE

#endif // SCRIPTENV_H
