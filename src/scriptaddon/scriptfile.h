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

#include <QFile>
#include <QString>

BEGIN_AS_NAMESPACE

class CScriptFile {
public:
    CScriptFile();

    void AddRef() const;
    void Release() const;

    bool Open(const QString &filename);
    bool Close();
    asINT64 GetSize() const;
    bool IsEOF() const;

    // Reading
    QString ReadString(unsigned int length);
    QString ReadLine();
    asINT64 ReadInt(asUINT bytes);
    asQWORD ReadUInt(asUINT bytes);
    float ReadFloat();
    double ReadDouble();

    // Cursor
    int GetPos() const;
    bool SetPos(asINT64 pos);
    bool MovePos(asINT64 delta);

    // Big-endian = most significant byte first
    bool mostSignificantByteFirst;

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

// Use this one instead if native calling conventions
// are not supported on the target platform
void RegisterScriptFile_Generic(asIScriptEngine *engine);

END_AS_NAMESPACE

#endif
