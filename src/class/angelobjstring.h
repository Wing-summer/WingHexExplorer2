#ifndef ANGELOBJSTRING_H
#define ANGELOBJSTRING_H

#include "AngelScript/add_on/debugger/debugger.h"

#include <QObject>
#include <QString>
#include <QtGlobal>

class AngelObjString : public QObject {
    Q_OBJECT

public:
    // for debugger use
    static std::string stringToString(void *obj, int expandMembers,
                                      CDebugger *dbg);

    static std::string arrayToString(void *obj, int expandMembers,
                                     CDebugger *dbg);

    static std::string dictionaryToString(void *obj, int expandMembers,
                                          CDebugger *dbg);

    static std::string dateTimeToString(void *obj, int expandMembers,
                                        CDebugger *dbg);

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
