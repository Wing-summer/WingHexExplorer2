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

#include "define.h"

#include <QDir>
#include <QFileInfo>

BEGIN_AS_NAMESPACE

CScriptFile *ScriptFile_Factory() { return new CScriptFile(); }

void RegisterScriptFile_Native(asIScriptEngine *engine) {
    int r;

    r = engine->RegisterObjectType("file", 0, asOBJ_REF);
    assert(r >= 0);
    r = engine->RegisterObjectBehaviour("file", asBEHAVE_FACTORY, "file @f()",
                                        asFUNCTION(ScriptFile_Factory),
                                        asCALL_CDECL);
    assert(r >= 0);
    r = engine->RegisterObjectBehaviour("file", asBEHAVE_ADDREF, "void f()",
                                        asMETHOD(CScriptFile, AddRef),
                                        asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectBehaviour("file", asBEHAVE_RELEASE, "void f()",
                                        asMETHOD(CScriptFile, Release),
                                        asCALL_THISCALL);
    assert(r >= 0);

    r = engine->RegisterObjectMethod(
        "file", "bool open(const string &in path, int mode)",
        asMETHOD(CScriptFile, Open), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "file", "void close()", asMETHOD(CScriptFile, Close), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int64 getSize() const",
                                     asMETHOD(CScriptFile, GetSize),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "bool IsOpen() const",
                                     asMETHOD(CScriptFile, IsOpen),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "bool isEndOfFile() const",
                                     asMETHOD(CScriptFile, IsEOF),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "string readString()",
                                     asMETHOD(CScriptFile, ReadString),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "array<uint8>@ readByteArray()",
                                     asMETHOD(CScriptFile, ReadByteArray),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "string readLine()",
                                     asMETHOD(CScriptFile, ReadLine),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "file", "array<uint8>@ readLineByteArray()",
        asMETHOD(CScriptFile, ReadLineByteArray), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int8 readInt8()",
                                     asMETHOD(CScriptFile, ReadInt8),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int16 readInt16()",
                                     asMETHOD(CScriptFile, ReadInt16),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int32 readInt32()",
                                     asMETHOD(CScriptFile, ReadInt32),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int64 readInt64()",
                                     asMETHOD(CScriptFile, ReadInt64),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "uint8 readUInt8()",
                                     asMETHOD(CScriptFile, ReadUInt8),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "uint16 readUInt16()",
                                     asMETHOD(CScriptFile, ReadUInt16),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "uint32 readUInt32()",
                                     asMETHOD(CScriptFile, ReadUInt32),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "uint64 readUInt64()",
                                     asMETHOD(CScriptFile, ReadUInt64),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "float readFloat()",
                                     asMETHOD(CScriptFile, ReadFloat),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "double readDouble()",
                                     asMETHOD(CScriptFile, ReadDouble),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "file", "int writeString(const string &in)",
        asMETHOD(CScriptFile, WriteString), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "file", "int writeByteArray(const array<uint8> &in)",
        asMETHOD(CScriptFile, WriteByteArray), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int64 writeInt8(int8)",
                                     asMETHOD(CScriptFile, WriteInt8),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int64 writeInt16(int16)",
                                     asMETHOD(CScriptFile, WriteInt16),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int64 writeInt32(int32)",
                                     asMETHOD(CScriptFile, WriteInt32),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int64 writeInt64(int64)",
                                     asMETHOD(CScriptFile, WriteInt64),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int64 writeUInt8(uint8)",
                                     asMETHOD(CScriptFile, WriteUInt8),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int64 writeUInt16(uint16)",
                                     asMETHOD(CScriptFile, WriteUInt16),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int64 writeUInt32(uint32)",
                                     asMETHOD(CScriptFile, WriteUInt32),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int64 writeUInt64(uint64)",
                                     asMETHOD(CScriptFile, WriteUInt64),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int64 writeFloat(float)",
                                     asMETHOD(CScriptFile, WriteFloat),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int64 writeDouble(double)",
                                     asMETHOD(CScriptFile, WriteDouble),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int64 getPos() const",
                                     asMETHOD(CScriptFile, GetPos),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "bool setPos(int64)",
                                     asMETHOD(CScriptFile, SetPos),
                                     asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod("file", "int64 movePos(int64)",
                                     asMETHOD(CScriptFile, MovePos),
                                     asCALL_THISCALL);
    assert(r >= 0);

    r = engine->RegisterObjectProperty(
        "file", "bool mostSignificantByteFirst",
        asOFFSET(CScriptFile, mostSignificantByteFirst));
    assert(r >= 0);

    // register enum flags
    r = engine->SetDefaultNamespace("file");
    assert(r >= 0);
    r = engine->RegisterEnum("mode");
    assert(r >= 0);
    r = engine->RegisterEnumValue("mode", "NotOpen", QFile::NotOpen);
    assert(r >= 0);
    r = engine->RegisterEnumValue("mode", "ReadOnly", QFile::ReadOnly);
    assert(r >= 0);
    r = engine->RegisterEnumValue("mode", "WriteOnly", QFile::WriteOnly);
    assert(r >= 0);
    r = engine->RegisterEnumValue("mode", "ReadWrite", QFile::ReadWrite);
    assert(r >= 0);
    r = engine->RegisterEnumValue("mode", "Append", QFile::Append);
    assert(r >= 0);
    r = engine->RegisterEnumValue("mode", "Truncate", QFile::Truncate);
    assert(r >= 0);
    r = engine->RegisterEnumValue("mode", "Text", QFile::Text);
    assert(r >= 0);
    r = engine->RegisterEnumValue("mode", "Unbuffered", QFile::Unbuffered);
    assert(r >= 0);
    r = engine->RegisterEnumValue("mode", "NewOnly", QFile::NewOnly);
    assert(r >= 0);
    r = engine->RegisterEnumValue("mode", "ExistingOnly", QFile::ExistingOnly);
    assert(r >= 0);
    engine->SetDefaultNamespace("");
}

void RegisterScriptFile(asIScriptEngine *engine) {
    // we only support native call
    RegisterScriptFile_Native(engine);
}

CScriptFile::CScriptFile() {
    refCount = 1;
#ifdef Q_LITTLE_ENDIAN
    mostSignificantByteFirst = false;
#else
    mostSignificantByteFirst = true;
#endif
}

CScriptFile::~CScriptFile() { Close(); }

void CScriptFile::AddRef() const { asAtomicInc(refCount); }

void CScriptFile::Release() const {
    if (asAtomicDec(refCount) == 0)
        delete this;
}

bool CScriptFile::Open(const QString &filename, int mode) {
    auto ctx = asGetActiveContext();
    if (ctx == nullptr) {
        return false;
    }

    // Close the previously opened file handle
    if (file.isOpen())
        Close();

    auto ptr = static_cast<const QString *>(
        ctx->GetUserData(AsUserDataType::UserData_Section_StringPtr));
    if (ptr) {
        QFileInfo sinfo(*ptr);
        QFileInfo finfo(sinfo.absoluteDir(), filename);
        file.setFileName(finfo.absoluteFilePath());
    } else {
        file.setFileName(filename);
    }

    if (!file.open(QFile::OpenMode(mode))) {
        return false;
    }

    return true;
}

bool CScriptFile::IsOpen() const { return file.isOpen(); }

void CScriptFile::Close() { return file.close(); }

asINT64 CScriptFile::GetSize() const { return file.size(); }

asINT64 CScriptFile::GetPos() const { return file.pos(); }

bool CScriptFile::SetPos(asINT64 pos) { return file.seek(pos); }

asINT64 CScriptFile::MovePos(asINT64 delta) { return file.skip(delta); }

QString CScriptFile::ReadLine() { return QString::fromUtf8(file.readLine()); }

CScriptArray *CScriptFile::ReadLineByteArray() {
    auto ctx = asGetActiveContext();
    if (ctx) {
        auto raw = file.readLine();
        auto engine = ctx->GetEngine();
        auto arrType = engine->GetTypeInfoByDecl("array<uint8>");
        auto total = raw.size();
        CScriptArray *arr = CScriptArray::Create(arrType, total);
        for (qsizetype i = 0; i < total; ++i) {
            *reinterpret_cast<asBYTE *>(arr->At(i)) = raw.at(i);
        }
        return arr;
    }
    return nullptr;
}

asINT8 CScriptFile::ReadInt8() { return __read<qint8>(); }

asINT16 CScriptFile::ReadInt16() { return __read<qint16>(); }

asINT32 CScriptFile::ReadInt32() { return __read<qint32>(); }

asINT64 CScriptFile::ReadInt64() { return __read<qint64>(); }

asBYTE CScriptFile::ReadUInt8() { return __read<quint8>(); }

asWORD CScriptFile::ReadUInt16() { return __read<quint16>(); }

asUINT CScriptFile::ReadUInt32() { return __read<quint32>(); }

asQWORD CScriptFile::ReadUInt64() { return __read<quint64>(); }

float CScriptFile::ReadFloat() {
    float data = 0;
    file.read(reinterpret_cast<char *>(&data), sizeof(data));
    return data;
}

double CScriptFile::ReadDouble() {
    double data = 0;
    file.read(reinterpret_cast<char *>(&data), sizeof(data));
    return data;
}

bool CScriptFile::IsEOF() const { return file.atEnd(); }

QString CScriptFile::ReadString() { return __read<QString>(); }

CScriptArray *CScriptFile::ReadByteArray(asINT64 length) {
    auto ctx = asGetActiveContext();
    if (ctx) {
        auto raw = file.read(length);
        auto engine = ctx->GetEngine();
        auto arrType = engine->GetTypeInfoByDecl("array<uint8>");
        auto total = raw.size();
        CScriptArray *arr = CScriptArray::Create(arrType, total);
        for (qsizetype i = 0; i < total; ++i) {
            *reinterpret_cast<asBYTE *>(arr->At(i)) = raw.at(i);
        }
        return arr;
    }
    return nullptr;
}

asINT64 CScriptFile::WriteFloat(float f) { return __write(f); }

asINT64 CScriptFile::WriteDouble(double d) { return __write(d); }

asINT64 CScriptFile::WriteString(const QString &str) { return __write(str); }

asINT64 CScriptFile::WriteByteArray(CScriptArray *array) {
    auto total = array->GetSize();
    return file.write(static_cast<const char *>(array->GetBuffer()), total);
}

asINT64 CScriptFile::WriteInt8(asINT8 v) { return __write<qint8>(v); }

asINT64 CScriptFile::WriteInt16(asINT16 v) { return __write<qint16>(v); }

asINT64 CScriptFile::WriteInt32(asINT32 v) { return __write<qint32>(v); }

asINT64 CScriptFile::WriteInt64(asINT64 v) { return __write<qint64>(v); }

asINT64 CScriptFile::WriteUInt8(asBYTE v) { return __write<quint8>(v); }

asINT64 CScriptFile::WriteUInt16(asWORD v) { return __write<quint16>(v); }

asINT64 CScriptFile::WriteUInt32(asUINT v) { return __write<quint32>(v); }

asINT64 CScriptFile::WriteUInt64(asQWORD v) { return __write<quint64>(v); }

END_AS_NAMESPACE
