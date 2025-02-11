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

#include "scriptfile.h"

BEGIN_AS_NAMESPACE

CScriptFile *ScriptFile_Factory() { return new CScriptFile(); }

void ScriptFile_Factory_Generic(asIScriptGeneric *gen) {
    *(CScriptFile **)gen->GetAddressOfReturnLocation() = ScriptFile_Factory();
}

void ScriptFile_AddRef_Generic(asIScriptGeneric *gen) {
    CScriptFile *file = (CScriptFile *)gen->GetObject();
    file->AddRef();
}

void ScriptFile_Release_Generic(asIScriptGeneric *gen) {
    CScriptFile *file = (CScriptFile *)gen->GetObject();
    file->Release();
}

void ScriptFile_Open_Generic(asIScriptGeneric *gen) {
    CScriptFile *file = (CScriptFile *)gen->GetObject();
    QString *f = (QString *)gen->GetArgAddress(0);
    auto r = file->Open(*f);
    gen->SetReturnByte(r);
}

void ScriptFile_Close_Generic(asIScriptGeneric *gen) {
    CScriptFile *file = (CScriptFile *)gen->GetObject();
    auto r = file->Close();
    gen->SetReturnByte(r);
}

void ScriptFile_GetSize_Generic(asIScriptGeneric *gen) {
    CScriptFile *file = (CScriptFile *)gen->GetObject();
    qint64 r = file->GetSize();
    gen->SetReturnQWord(r);
}

void ScriptFile_ReadString_Generic(asIScriptGeneric *gen) {
    CScriptFile *file = (CScriptFile *)gen->GetObject();
    int len = gen->GetArgDWord(0);
    auto str = file->ReadString(len);
    gen->SetReturnObject(&str);
}

void ScriptFile_ReadLine_Generic(asIScriptGeneric *gen) {
    CScriptFile *file = (CScriptFile *)gen->GetObject();
    auto str = file->ReadLine();
    gen->SetReturnObject(&str);
}

void ScriptFile_ReadInt_Generic(asIScriptGeneric *gen) {
    CScriptFile *file = (CScriptFile *)gen->GetObject();
    asUINT bytes = *(asUINT *)gen->GetAddressOfArg(0);
    *(asINT64 *)gen->GetAddressOfReturnLocation() = file->ReadInt(bytes);
}

void ScriptFile_ReadUInt_Generic(asIScriptGeneric *gen) {
    CScriptFile *file = (CScriptFile *)gen->GetObject();
    asUINT bytes = *(asUINT *)gen->GetAddressOfArg(0);
    *(asQWORD *)gen->GetAddressOfReturnLocation() = file->ReadUInt(bytes);
}

void ScriptFile_ReadFloat_Generic(asIScriptGeneric *gen) {
    CScriptFile *file = (CScriptFile *)gen->GetObject();
    *(float *)gen->GetAddressOfReturnLocation() = file->ReadFloat();
}

void ScriptFile_ReadDouble_Generic(asIScriptGeneric *gen) {
    CScriptFile *file = (CScriptFile *)gen->GetObject();
    *(double *)gen->GetAddressOfReturnLocation() = file->ReadDouble();
}

void ScriptFile_IsEOF_Generic(asIScriptGeneric *gen) {
    CScriptFile *file = (CScriptFile *)gen->GetObject();
    bool r = file->IsEOF();
    gen->SetReturnByte(r);
}

void ScriptFile_GetPos_Generic(asIScriptGeneric *gen) {
    CScriptFile *file = (CScriptFile *)gen->GetObject();
    gen->SetReturnDWord(file->GetPos());
}

void ScriptFile_SetPos_Generic(asIScriptGeneric *gen) {
    CScriptFile *file = (CScriptFile *)gen->GetObject();
    auto pos = qint64(gen->GetArgQWord(0));
    gen->SetReturnByte(file->SetPos(pos));
}

void ScriptFile_MovePos_Generic(asIScriptGeneric *gen) {
    CScriptFile *file = (CScriptFile *)gen->GetObject();
    auto delta = qint64(gen->GetArgQWord(0));
    gen->SetReturnByte(file->MovePos(delta));
}

void RegisterScriptFile_Native(asIScriptEngine *engine) {
    int r;

    r = engine->RegisterObjectType("file", 0, asOBJ_REF);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour("file", asBEHAVE_FACTORY, "file @f()",
                                        asFUNCTION(ScriptFile_Factory),
                                        asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour("file", asBEHAVE_ADDREF, "void f()",
                                        asMETHOD(CScriptFile, AddRef),
                                        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour("file", asBEHAVE_RELEASE, "void f()",
                                        asMETHOD(CScriptFile, Release),
                                        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("file", "bool open(const string &in)",
                                     asMETHOD(CScriptFile, Open),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "file", "bool close()", asMETHOD(CScriptFile, Close), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "int64 getSize() const",
                                     asMETHOD(CScriptFile, GetSize),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "bool isEndOfFile() const",
                                     asMETHOD(CScriptFile, IsEOF),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "string readString(uint)",
                                     asMETHOD(CScriptFile, ReadString),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "string readLine()",
                                     asMETHOD(CScriptFile, ReadLine),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "int64 readInt(uint)",
                                     asMETHOD(CScriptFile, ReadInt),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "uint64 readUInt(uint)",
                                     asMETHOD(CScriptFile, ReadUInt),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "float readFloat()",
                                     asMETHOD(CScriptFile, ReadFloat),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "double readDouble()",
                                     asMETHOD(CScriptFile, ReadDouble),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("file", "int getPos() const",
                                     asMETHOD(CScriptFile, GetPos),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "bool setPos(int64)",
                                     asMETHOD(CScriptFile, SetPos),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "bool movePos(int64)",
                                     asMETHOD(CScriptFile, MovePos),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectProperty(
        "file", "bool mostSignificantByteFirst",
        asOFFSET(CScriptFile, mostSignificantByteFirst));
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
}

void RegisterScriptFile_Generic(asIScriptEngine *engine) {
    int r;

    r = engine->RegisterObjectType("file", 0, asOBJ_REF);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour("file", asBEHAVE_FACTORY, "file @f()",
                                        asFUNCTION(ScriptFile_Factory_Generic),
                                        asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour("file", asBEHAVE_ADDREF, "void f()",
                                        asFUNCTION(ScriptFile_AddRef_Generic),
                                        asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour("file", asBEHAVE_RELEASE, "void f()",
                                        asFUNCTION(ScriptFile_Release_Generic),
                                        asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("file", "bool open(const string &in)",
                                     asFUNCTION(ScriptFile_Open_Generic),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "bool close()",
                                     asFUNCTION(ScriptFile_Close_Generic),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "int64 getSize() const",
                                     asFUNCTION(ScriptFile_GetSize_Generic),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "bool isEndOfFile() const",
                                     asFUNCTION(ScriptFile_IsEOF_Generic),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "string readString(uint)",
                                     asFUNCTION(ScriptFile_ReadString_Generic),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "string readLine()",
                                     asFUNCTION(ScriptFile_ReadLine_Generic),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "int64 readInt(uint)",
                                     asFUNCTION(ScriptFile_ReadInt_Generic),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "uint64 readUInt(uint)",
                                     asFUNCTION(ScriptFile_ReadUInt_Generic),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "float readFloat()",
                                     asFUNCTION(ScriptFile_ReadFloat_Generic),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "double readDouble()",
                                     asFUNCTION(ScriptFile_ReadDouble_Generic),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("file", "int getPos() const",
                                     asFUNCTION(ScriptFile_GetPos_Generic),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "bool setPos(int64)",
                                     asFUNCTION(ScriptFile_SetPos_Generic),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("file", "bool movePos(int64)",
                                     asFUNCTION(ScriptFile_MovePos_Generic),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectProperty(
        "file", "bool mostSignificantByteFirst",
        asOFFSET(CScriptFile, mostSignificantByteFirst));
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
}

void RegisterScriptFile(asIScriptEngine *engine) {
    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY"))
        RegisterScriptFile_Generic(engine);
    else
        RegisterScriptFile_Native(engine);
}

CScriptFile::CScriptFile() {
    refCount = 1;
    mostSignificantByteFirst = false;
}

CScriptFile::~CScriptFile() { Close(); }

void CScriptFile::AddRef() const { asAtomicInc(refCount); }

void CScriptFile::Release() const {
    if (asAtomicDec(refCount) == 0)
        delete this;
}

bool CScriptFile::Open(const QString &filename) {
    // Close the previously opened file handle
    if (file.isOpen())
        Close();

    file.setFileName(filename);
    return file.open(QFile::ReadOnly);
}

bool CScriptFile::Close() {
    if (!file.isOpen())
        return false;

    file.close();
    file.setFileName({});
    return true;
}

asINT64 CScriptFile::GetSize() const {
    if (!file.isOpen())
        return -1;

    return file.size();
}

int CScriptFile::GetPos() const {
    if (!file.isOpen())
        return -1;

    return file.pos();
}

bool CScriptFile::SetPos(asINT64 pos) {
    if (!file.isOpen())
        return false;

    return file.seek(pos);
}

bool CScriptFile::MovePos(asINT64 delta) {
    if (!file.isOpen())
        return false;

    return file.seek(file.pos() + delta);
}

QString CScriptFile::ReadString(unsigned int length) {
    if (!file.isOpen())
        return {};

    return QString::fromUtf8(file.read(length));
}

QString CScriptFile::ReadLine() {
    if (!file.isOpen())
        return {};

    return QString::fromUtf8(file.readLine());
}

asINT64 CScriptFile::ReadInt(asUINT bytes) {
    if (!file.isOpen())
        return 0;

    if (bytes > 8)
        bytes = 8;
    if (bytes == 0)
        return 0;

    char buf[8];
    size_t r = file.read(buf, 8);
    if (r == 0)
        return 0;

    asINT64 val = 0;
    if (mostSignificantByteFirst) {
        unsigned int n = 0;
        for (; n < bytes; n++)
            val |= asQWORD(buf[n]) << ((bytes - n - 1) * 8);

        // Check the most significant byte to determine if the rest
        // of the qword must be filled to give a negative value
        if (buf[0] & 0x80)
            for (; n < 8; n++)
                val |= asQWORD(0xFF) << (n * 8);
    } else {
        unsigned int n = 0;
        for (; n < bytes; n++)
            val |= asQWORD(buf[n]) << (n * 8);

        // Check the most significant byte to determine if the rest
        // of the qword must be filled to give a negative value
        if (buf[bytes - 1] & 0x80)
            for (; n < 8; n++)
                val |= asQWORD(0xFF) << (n * 8);
    }

    return val;
}

asQWORD CScriptFile::ReadUInt(asUINT bytes) {
    if (!file.isOpen())
        return 0;

    if (bytes > 8)
        bytes = 8;
    if (bytes == 0)
        return 0;

    char buf[8];
    size_t r = file.read(buf, 8);
    if (r == 0)
        return 0;

    asQWORD val = 0;
    if (mostSignificantByteFirst) {
        unsigned int n = 0;
        for (; n < bytes; n++)
            val |= asQWORD(buf[n]) << ((bytes - n - 1) * 8);
    } else {
        unsigned int n = 0;
        for (; n < bytes; n++)
            val |= asQWORD(buf[n]) << (n * 8);
    }

    return val;
}

float CScriptFile::ReadFloat() {
    if (!file.isOpen())
        return 0;

    char buf[4];
    size_t r = file.read(buf, 4);
    if (r == 0)
        return 0;

    union conv {
        asUINT val;
        float fp;
    } value = {0};
    if (mostSignificantByteFirst) {
        unsigned int n = 0;
        for (; n < 4; n++)
            value.val |= asUINT(buf[n]) << ((3 - n) * 8);
    } else {
        unsigned int n = 0;
        for (; n < 4; n++)
            value.val |= asUINT(buf[n]) << (n * 8);
    }

    return value.fp;
}

double CScriptFile::ReadDouble() {
    if (!file.isOpen())
        return 0;

    char buf[8];
    size_t r = file.read(buf, 8);
    if (r == 0)
        return 0;

    union conv {
        asQWORD val;
        double fp;
    } value = {0};
    if (mostSignificantByteFirst) {
        unsigned int n = 0;
        for (; n < 8; n++)
            value.val |= asQWORD(buf[n]) << ((7 - n) * 8);
    } else {
        unsigned int n = 0;
        for (; n < 8; n++)
            value.val |= asQWORD(buf[n]) << (n * 8);
    }

    return value.fp;
}

bool CScriptFile::IsEOF() const {
    if (!file.isOpen())
        return true;

    return file.atEnd();
}

END_AS_NAMESPACE
