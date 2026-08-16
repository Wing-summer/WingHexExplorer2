/*==============================================================================
 ** Copyright (C) 2026-2029 WingSummer
 **
 ** This program is free software: you can redistribute it and/or modify it
 ** under the terms of the GNU Affero General Public License as published by the
 ** Free Software Foundation, version 3.
 **
 ** This program is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 ** for more details.
 **
 ** You should have received a copy of the GNU Affero General Public License
 ** along with this program. If not, see <https://www.gnu.org/licenses/>.
 ** =============================================================================
 */

#include "luautypes.h"

#include "debugger/luauinspector.h"
#include "debugger/luauutil.h"
#include "lualib.h"

QString LuauTypes::toString(lua_State *L, int index) {
    LuauUtil::DisableDebugStep _(L);
    lua_checkstack(L, 1);
    QString result;
    size_t len;
    const char *s = luaL_tolstring(L, index, &len);
    if (s) {
        result = QString::fromUtf8(s, len);
    }
    lua_pop(L, 1);
    return result;
}

QString LuauTypes::toDbgString(lua_State *L, int index) {
    LuauInspector::Options options;
    options.maxDepth = 3;
    options.maxItems = 20;
    options.maxStringLength = 512;
    options.stringMode = LuauInspector::StringMode::Quote;
    return LuauInspector::inspect(L, index, options);
}

QString LuauTypes::getTypeName(int type) {
    return QString::fromUtf8(lua_typename(nullptr, type));
}
