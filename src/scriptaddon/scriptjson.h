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

#ifndef SCRIPTJSON_H
#define SCRIPTJSON_H

#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already include before
#include "AngelScript/sdk/angelscript/include/angelscript.h"
#endif

BEGIN_AS_NAMESPACE

void RegisterQJson(asIScriptEngine *engine);

END_AS_NAMESPACE
#endif // SCRIPTJSON_H
