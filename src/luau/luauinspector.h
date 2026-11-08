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

#include <lua.h>

enum class InspectMode { Compact, Pretty };

struct InspectOptions {
    int depth = 3;
    int maxItems = 100;
    bool quoteString = true;

    InspectOptions(InspectMode m = InspectMode::Pretty) { setMode(m); }

    void setMode(InspectMode m) {
        m_mode = m;
        if (m_mode == InspectMode::Compact) {
            m_newline = {};
            m_indent = {};
        } else {
            m_newline = QByteArray(1, '\n');
            m_indent = QByteArray(2, ' ');
        }
    }

    QByteArray getIndent() const { return m_indent; }

    QByteArray getNewline() const { return m_newline; }

    InspectMode getMode() const { return m_mode; }

private:
    InspectMode m_mode = InspectMode::Pretty;
    QByteArray m_newline;
    QByteArray m_indent;
};

class LuauInspector {
public:
    static QString inspect(lua_State *L, int index,
                           const InspectOptions &options = {});
    static QString inspect(lua_State *L, int index, InspectMode mode);
};

#endif // LUAUINSPECTOR_H
