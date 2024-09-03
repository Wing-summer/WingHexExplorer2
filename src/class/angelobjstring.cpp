#include "angelobjstring.h"
#include "AngelScript/sdk/add_on/datetime/datetime.h"
#include "AngelScript/sdk/add_on/scriptarray/scriptarray.h"
#include "scriptaddon/scriptqdictionary.h"

#include <QString>
#include <QTextStream>

AngelObjString::AngelObjString() {}

QString AngelObjString::stringToString(void *obj, int expandMembers,
                                       asDebugger *dbg) {
    Q_UNUSED(expandMembers);
    Q_UNUSED(dbg);

    // We know the received object is a string
    QString *val = reinterpret_cast<QString *>(obj);

    constexpr auto limit = 100;

    // Format the output string
    if (val->length() < limit) {

        return QStringLiteral("\"") + *val + QStringLiteral("\"");
    } else {
        QString str;
        QTextStream s(&str);
        s << tr("(len=") << val->length() << QStringLiteral(") \"") << *val
          << QStringLiteral("...");

        return str;
    }
}

QString AngelObjString::arrayToString(void *obj, int expandMembers,
                                      asDebugger *dbg) {
    CScriptArray *arr = reinterpret_cast<CScriptArray *>(obj);

    QString str;
    QTextStream s(&str);
    s << tr("(len=") << arr->GetSize() << QStringLiteral(")");

    if (expandMembers > 0) {
        s << QStringLiteral(" [");
        for (asUINT n = 0; n < arr->GetSize(); n++) {
            s << dbg->ToString(arr->At(n), arr->GetElementTypeId(),
                               expandMembers - 1,
                               arr->GetArrayObjectType()->GetEngine());
            if (n < arr->GetSize() - 1)
                s << ", ";
        }
        s << QStringLiteral("]");
    }

    return str;
}

QString AngelObjString::dictionaryToString(void *obj, int expandMembers,
                                           asDebugger *dbg) {
    CScriptDictionary *dic = reinterpret_cast<CScriptDictionary *>(obj);

    QString str;
    QTextStream s(&str);
    s << tr("(len=") << dic->GetSize() << ")";

    if (expandMembers > 0) {
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
            asIScriptContext *ctx = asGetActiveContext();

            s << dbg->ToString(const_cast<void *>(val), typeId,
                               expandMembers - 1, ctx ? ctx->GetEngine() : 0);

            if (n < dic->GetSize() - 1)
                s << ", ";
        }
        s << "]";
    }

    return str;
}

QString AngelObjString::dateTimeToString(void *obj, int expandMembers,
                                         asDebugger *dbg) {
    Q_UNUSED(expandMembers);
    Q_UNUSED(dbg);

    CDateTime *dt = reinterpret_cast<CDateTime *>(obj);

    QString str;
    QTextStream s(&str);
    s << "{" << dt->getYear() << "-" << dt->getMonth() << "-" << dt->getDay()
      << " ";
    s << dt->getHour() << ":" << dt->getMinute() << ":" << dt->getSecond()
      << "}";

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
