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

#include "lua.h"

#include <QString>

class BreakPoint {
public:
    static BreakPoint create(int line);

    int line() const;
    int targetLine() const;
    int enable(lua_State *L, int func_index, bool enable);

private:
    int line_ = 0;
    int target_line_ = -1;
};

struct BreakContext {
    QString source_;
    int line_ = 0;
    int depth_ = 0;
    lua_State *L_ = nullptr;
};

#endif // BREAKPOINT_H