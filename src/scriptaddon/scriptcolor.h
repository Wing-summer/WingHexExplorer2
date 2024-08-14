#ifndef SCRIPTCOLOR_H
#define SCRIPTCOLOR_H

#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif

BEGIN_AS_NAMESPACE

void RegisterColor(asIScriptEngine *engine);

END_AS_NAMESPACE

#endif // SCRIPTCOLOR_H
