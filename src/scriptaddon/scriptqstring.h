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

#ifndef SCRIPTQSTRING_H
#define SCRIPTQSTRING_H

#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already include before
#include "AngelScript/sdk/angelscript/include/angelscript.h"
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
void RegisterQStringRegExSupport(asIScriptEngine *engine);

END_AS_NAMESPACE

#endif // SCRIPTQSTRING_H
