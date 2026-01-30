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

#ifndef SCRIPTFILE_H
#define SCRIPTFILE_H

//---------------------------
// Declaration
//

#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif

#include "AngelScript/sdk/add_on/scriptarray/scriptarray.h"

#include <QDataStream>
#include <QFile>
#include <QTextStream>

BEGIN_AS_NAMESPACE

class CScriptFile {
public:
    CScriptFile();

    void AddRef() const;
    void Release() const;

    // open mode
    bool Open(const QString &filename, int mode);
    bool IsOpen() const;
    void Close();
    asINT64 GetSize() const;
    bool IsEOF() const;

    // Reading
    QString ReadString();
    CScriptArray *ReadByteArray(asINT64 length);
    QString ReadLine();
    CScriptArray *ReadLineByteArray();
    asINT8 ReadInt8();
    asINT16 ReadInt16();
    asINT32 ReadInt32();
    asINT64 ReadInt64();
    asBYTE ReadUInt8();
    asWORD ReadUInt16();
    asUINT ReadUInt32();
    asQWORD ReadUInt64();
    float ReadFloat();
    double ReadDouble();

    // Writing
    asINT64 WriteString(const QString &str);
    asINT64 WriteByteArray(CScriptArray *array);
    asINT64 WriteInt8(asINT8 v);
    asINT64 WriteInt16(asINT16 v);
    asINT64 WriteInt32(asINT32 v);
    asINT64 WriteInt64(asINT64 v);
    asINT64 WriteUInt8(asBYTE v);
    asINT64 WriteUInt16(asWORD v);
    asINT64 WriteUInt32(asUINT v);
    asINT64 WriteUInt64(asQWORD v);
    asINT64 WriteFloat(float v);
    asINT64 WriteDouble(double v);

    // Cursor
    asINT64 GetPos() const;
    bool SetPos(asINT64 pos);
    asINT64 MovePos(asINT64 delta);

    // Big-endian = most significant byte first
    bool mostSignificantByteFirst;

protected:
    template <typename T>
    T __read() {
        if (!file.isOpen()) {
            return {};
        }

        T buffer;
        if (file.openMode().testFlag(QFile::Text)) {
            QTextStream s(&file);
            if constexpr (std::is_same_v<T, qint8> ||
                          std::is_same_v<T, quint8>) {
                QDataStream s(&file);
                s >> buffer;
            } else {
                s >> buffer;
            }
        } else {
            QDataStream s(&file);
            s.setByteOrder(mostSignificantByteFirst
                               ? QDataStream::BigEndian
                               : QDataStream::LittleEndian);
            s >> buffer;
        }
        return buffer;
    }

    template <typename T>
    asINT64 __write(const T &data) {
        if (!file.isOpen()) {
            return 0;
        }

        auto oldPos = file.pos();
        if (file.openMode().testFlag(QFile::Text)) {
            QTextStream s(&file);
            s << data;
        } else {
            QDataStream s(&file);
            s.setByteOrder(mostSignificantByteFirst
                               ? QDataStream::BigEndian
                               : QDataStream::LittleEndian);
            s << data;
        }
        return file.pos() - oldPos;
    }

protected:
    ~CScriptFile();

    mutable int refCount;
    QFile file;
};

// This function will determine the configuration of the engine
// and use one of the two functions below to register the file type
void RegisterScriptFile(asIScriptEngine *engine);

// Call this function to register the file type
// using native calling conventions
void RegisterScriptFile_Native(asIScriptEngine *engine);

END_AS_NAMESPACE

#endif
