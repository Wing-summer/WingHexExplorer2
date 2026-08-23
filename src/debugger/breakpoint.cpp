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

#include "breakpoint.h"

BreakPoint BreakPoint::create(int line) {
    BreakPoint bp;
    bp.line_ = line;
    return bp;
}

int BreakPoint::line() const { return line_; }

int BreakPoint::targetLine() const { return target_line_; }

int BreakPoint::enable(lua_State *L, int func_index, bool enable) {
    lua_checkstack(L, 1);
    lua_getref(L, func_index);
    int result = lua_breakpoint(L, -1, line_, enable);
    if (result != -1)
        target_line_ = result;
    lua_pop(L, 1);
    return result;
}
