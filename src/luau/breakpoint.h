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

#ifndef BREAKPOINT_H
#define BREAKPOINT_H

#include <QString>

#include "lua.h"

class BreakPoint {
public:
    static BreakPoint create(int line);

    int line() const;
    int enable(lua_State *L, int funcIndex, bool enable);

private:
    int m_line = 0;
};

struct BreakContext {
    QString source;
    int line = 0;
    int depth = 0;
    lua_State *state = nullptr;
};

#endif // BREAKPOINT_H