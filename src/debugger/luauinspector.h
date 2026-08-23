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

#ifndef LUAUINSPECTOR_H
#define LUAUINSPECTOR_H

#include <QByteArray>
#include <QString>
#include <limits.h>
#include <lua.h>

enum class InspectMode { Compact, Pretty };

struct InspectOptions {
    int depth = 3;
    int maxItems = 100;
    bool quoteString = true;

    InspectOptions(InspectMode m = InspectMode::Pretty) { setMode(m); }

    void setMode(InspectMode m) {
        mode = m;
        if (mode == InspectMode::Compact) {
            newline = {};
            indent = {};
        } else {
            newline = QByteArray(1, '\n');
            indent = QByteArray(2, ' ');
        }
    }

    QByteArray getIndent() const { return indent; }
    QByteArray getNewline() const { return newline; }

    InspectMode getMode() const { return mode; }

private:
    InspectMode mode = InspectMode::Pretty;
    QByteArray newline;
    QByteArray indent;
};

class LuauInspector {
public:
    static QString inspect(lua_State *L, int index,
                           const InspectOptions &options = {});
    static QString inspect(lua_State *L, int index, InspectMode mode);
};

#endif // LUAUINSPECTOR_H
