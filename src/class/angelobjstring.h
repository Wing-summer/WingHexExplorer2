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

#ifndef ANGELOBJSTRING_H
#define ANGELOBJSTRING_H

#include "class/asdebugger.h"

#include <QObject>
#include <QString>
#include <QtGlobal>

class AngelObjString : public QObject {
    Q_OBJECT

public:
    static QString stringToString(void *obj, asDebugger *dbg, asUINT tag);

    static QString arrayToString(void *obj, asDebugger *dbg, asUINT tag);

    static QString charToString(void *obj, asDebugger *dbg, asUINT tag);

    static QString dictionaryToString(void *obj, asDebugger *dbg, asUINT tag);

    static QString colorToString(void *obj, asDebugger *dbg, asUINT tag);

public:
    // ==================================================
    // the following codes are from QT offical code (LGPL)
    // some modifications are made by wingsummer

    static constexpr inline char toHexUpper(uint value) noexcept {
        return "0123456789ABCDEF"[value & 0xF];
    }

    static constexpr inline int fromHex(uint c) noexcept {
        return ((c >= '0') && (c <= '9'))   ? int(c - '0')
               : ((c >= 'A') && (c <= 'F')) ? int(c - 'A' + 10)
               : ((c >= 'a') && (c <= 'f')) ? int(c - 'a' + 10)
                                            :
                                            /* otherwise */ -1;
    }

    static QString getEscapedString(const ushort *begin, int length,
                                    bool isUnicode);
    // ==================================================

private:
    AngelObjString();

    Q_DISABLE_COPY_MOVE(AngelObjString)
};

#endif // ANGELOBJSTRING_H
