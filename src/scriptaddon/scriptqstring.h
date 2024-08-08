#ifndef SCRIPTQSTRING_H
#define SCRIPTQSTRING_H

#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already include before
#include "AngelScript/angelscript/include/angelscript.h"
#endif

//---------------------------
// Compilation settings
//

// Some prefer to use property accessors to get/set the length of the string
// This option registers the accessors instead of the method length()
#ifndef AS_USE_ACCESSORS
#define AS_USE_ACCESSORS 0
#endif

// This option disables the implicit operators with primitives
#ifndef AS_NO_IMPL_OPS_WITH_STRING_AND_PRIMITIVE
#define AS_NO_IMPL_OPS_WITH_STRING_AND_PRIMITIVE 0
#endif

BEGIN_AS_NAMESPACE

void RegisterQString(asIScriptEngine *engine);
void RegisterQStringUtils(asIScriptEngine *engine);

END_AS_NAMESPACE

#endif // SCRIPTQSTRING_H
