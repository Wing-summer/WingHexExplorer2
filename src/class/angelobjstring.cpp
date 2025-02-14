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

#include "angelobjstring.h"
#include "AngelScript/sdk/add_on/scriptarray/scriptarray.h"
#include "scriptaddon/scriptqdictionary.h"

#include <QColor>
#include <QString>
#include <QTextStream>

AngelObjString::AngelObjString() {}

QString AngelObjString::stringToString(void *obj, asDebugger *dbg) {
    Q_UNUSED(dbg);

    // We know the received object is a string
    QString *val = reinterpret_cast<QString *>(obj);
    return *val;
}

QString AngelObjString::arrayToString(void *obj, asDebugger *dbg) {
    CScriptArray *arr = reinterpret_cast<CScriptArray *>(obj);

    QString str;
    QTextStream s(&str);
    s << tr("(len=") << arr->GetSize() << QStringLiteral(")");

    s << QStringLiteral(" [");
    for (asUINT n = 0; n < arr->GetSize(); n++) {
        s << dbg->toString(arr->At(n), arr->GetElementTypeId(),
                           arr->GetArrayObjectType()->GetEngine());
        if (n < arr->GetSize() - 1)
            s << ", ";
    }
    s << QStringLiteral("]");

    return str;
}

QString AngelObjString::charToString(void *obj, asDebugger *dbg) {

    Q_UNUSED(dbg);

    // We know the received object is a char
    QChar *val = reinterpret_cast<QChar *>(obj);
    return QString(*val);
}

QString AngelObjString::dictionaryToString(void *obj, asDebugger *dbg) {
    CScriptDictionary *dic = reinterpret_cast<CScriptDictionary *>(obj);

    QString str;
    QTextStream s(&str);

    auto engine = dic->GetEngine();

    s << " [";
    asUINT n = 0;
    for (CScriptDictionary::CIterator it = dic->begin(); it != dic->end();
         it++, n++) {
        s << "[" << it.GetKey() << "] = ";

        // Get the type and address of the value
        const void *val = it.GetAddressOfValue();
        int typeId = it.GetTypeId();

        // Use the engine from the currently active context (if none is
        // active, the debugger will use the engine held inside it by
        // default, but in an environment where there multiple engines this
        // might not be the correct instance).
        s << dbg->toString(const_cast<void *>(val), typeId, engine);

        if (n < dic->GetSize() - 1)
            s << ", ";
    }
    s << "]";

    return str;
}

QString AngelObjString::colorToString(void *obj, asDebugger *dbg) {
    Q_UNUSED(dbg);

    auto color = reinterpret_cast<QColor *>(obj);
    QString str;
    QTextStream s(&str);
    s << QStringLiteral("color(") << color->name() << QStringLiteral(")");
    return str;
}

QString AngelObjString::getEscapedString(const ushort *begin, int length,
                                         bool isUnicode) {
    Q_STATIC_ASSERT(sizeof(ushort) == sizeof(QChar));

    auto quote = QStringLiteral("\"");

    QString buffer = quote;

    bool lastWasHexEscape = false;
    const ushort *end = begin + length;
    for (const ushort *p = begin; p != end; ++p) {
        // check if we need to insert "" to break an hex escape sequence
        if (Q_UNLIKELY(lastWasHexEscape)) {
            if (fromHex(*p) != -1) {
                // yes, insert it
                buffer += "\"\"";
            }
            lastWasHexEscape = false;
        }

        // Surrogate characters are category Cs (Other_Surrogate), so
        // isPrintable = false for them
        int runLength = 0;
        while (p + runLength != end && QChar::isPrint(p[runLength]) &&
               p[runLength] != '\\' && p[runLength] != '"')
            ++runLength;
        if (runLength) {
            buffer += QString(reinterpret_cast<const QChar *>(p), runLength);
            p += runLength - 1;
            continue;
        }

        // print as an escape sequence
        // (maybe, see below for surrogate pairs)
        int buflen = 2;
        ushort buf[sizeof "\\U12345678" - 1];
        buf[0] = '\\';
        switch (*p) {
        case '"':
        case '\\':
            buf[1] = *p;
            break;
        case '\b':
            buf[1] = 'b';
            break;
        case '\f':
            buf[1] = 'f';
            break;
        case '\n':
            buf[1] = 'n';
            break;
        case '\r':
            buf[1] = 'r';
            break;
        case '\t':
            buf[1] = 't';
            break;
        default:
            if (!isUnicode) {
                // print as hex escape
                buf[1] = 'x';
                buf[2] = toHexUpper(uchar(*p) >> 4);
                buf[3] = toHexUpper(uchar(*p));
                buflen = 4;
                lastWasHexEscape = true;
                break;
            }
            if (QChar::isHighSurrogate(*p)) {
                if ((p + 1) != end && QChar::isLowSurrogate(p[1])) {
                    // properly-paired surrogates
                    uint ucs4 = QChar::surrogateToUcs4(*p, p[1]);
                    if (QChar::isPrint(ucs4)) {
                        buf[0] = *p;
                        buf[1] = p[1];
                        buflen = 2;
                    } else {
                        buf[1] = 'U';
                        buf[2] = '0'; // toHexUpper(ucs4 >> 32);
                        buf[3] = '0'; // toHexUpper(ucs4 >> 28);
                        buf[4] = toHexUpper(ucs4 >> 20);
                        buf[5] = toHexUpper(ucs4 >> 16);
                        buf[6] = toHexUpper(ucs4 >> 12);
                        buf[7] = toHexUpper(ucs4 >> 8);
                        buf[8] = toHexUpper(ucs4 >> 4);
                        buf[9] = toHexUpper(ucs4);
                        buflen = 10;
                    }
                    ++p;
                    break;
                }
                // improperly-paired surrogates, fall through
            }
            buf[1] = 'u';
            buf[2] = toHexUpper(ushort(*p) >> 12);
            buf[3] = toHexUpper(ushort(*p) >> 8);
            buf[4] = toHexUpper(*p >> 4);
            buf[5] = toHexUpper(*p);
            buflen = 6;
        }
        buffer += QString(reinterpret_cast<const QChar *>(buf), buflen);
    }

    buffer += quote;

    return buffer;
}
