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
    bp.m_line = line;
    return bp;
}

int BreakPoint::line() const { return m_line; }

int BreakPoint::enable(lua_State *L, int funcIndex, bool enable) {
    lua_checkstack(L, 1);
    lua_getref(L, funcIndex);
    int result = lua_breakpoint(L, -1, m_line, enable);
    auto old = m_line;
    if (result == -1) {
        return -1;
    } else {
        m_line = result;
    }
    lua_pop(L, 1);
    return old;
}
