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

#include "scriptfilesystem.h"
#include "AngelScript/sdk/add_on/autowrapper/aswrappedcall.h"

#include "define.h"

#include <QDir>

BEGIN_AS_NAMESPACE

CScriptFileSystem *ScriptFileSystem_Factory() {
    return new CScriptFileSystem();
}

void RegisterScriptFileSystem_Native(asIScriptEngine *engine) {
    int r;

    assert(engine->GetTypeInfoByName("string"));
    assert(engine->GetTypeInfoByDecl("array<string>"));
    assert(engine->GetTypeInfoByName("datetime"));

    r = engine->RegisterObjectType("filesystem", 0, asOBJ_REF);
    assert(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "filesystem", asBEHAVE_FACTORY, "filesystem @f()",
        asFUNCTION(ScriptFileSystem_Factory), asCALL_CDECL);
    assert(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "filesystem", asBEHAVE_ADDREF, "void f()",
        asMETHOD(CScriptFileSystem, AddRef), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "filesystem", asBEHAVE_RELEASE, "void f()",
        asMETHOD(CScriptFileSystem, Release), asCALL_THISCALL);
    assert(r >= 0);

    r = engine->RegisterObjectMethod(
        "filesystem", "bool changeCurrentPath(const string &in)",
        asMETHOD(CScriptFileSystem, ChangeCurrentPath), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "string getCurrentPath() const",
        asMETHOD(CScriptFileSystem, GetCurrentPath), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "array<string> @getDirs() const",
        asMETHOD(CScriptFileSystem, GetDirs), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "array<string> @getFiles() const",
        asMETHOD(CScriptFileSystem, GetFiles), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "bool isDir(const string &in) const",
        asMETHOD(CScriptFileSystem, IsDir), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "bool isLink(const string &in) const",
        asMETHOD(CScriptFileSystem, IsLink), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "int64 getSize(const string &in) const",
        asMETHOD(CScriptFileSystem, GetSize), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "bool makeDir(const string &in)",
        asMETHOD(CScriptFileSystem, MakeDir), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "bool removeDir(const string &in)",
        asMETHOD(CScriptFileSystem, RemoveDir), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "bool deleteFile(const string &in)",
        asMETHOD(CScriptFileSystem, DeleteFile), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "bool copyFile(const string &in, const string &in)",
        asMETHOD(CScriptFileSystem, CopyFile), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "bool moveFile(const string &in, const string &in)",
        asMETHOD(CScriptFileSystem, MoveFile), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "datetime getCreateDateTime(const string &in) const",
        asMETHOD(CScriptFileSystem, GetCreateDateTime), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "datetime getModifyDateTime(const string &in) const",
        asMETHOD(CScriptFileSystem, GetModifyDateTime), asCALL_THISCALL);
    assert(r >= 0);
}

void RegisterScriptFileSystem_Generic(asIScriptEngine *engine) {
    int r;

    assert(engine->GetTypeInfoByName("string"));
    assert(engine->GetTypeInfoByDecl("array<string>"));
    assert(engine->GetTypeInfoByName("datetime"));

    r = engine->RegisterObjectType("filesystem", 0, asOBJ_REF);
    assert(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "filesystem", asBEHAVE_FACTORY, "filesystem @f()",
        WRAP_FN(ScriptFileSystem_Factory), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "filesystem", asBEHAVE_ADDREF, "void f()",
        WRAP_MFN(CScriptFileSystem, AddRef), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "filesystem", asBEHAVE_RELEASE, "void f()",
        WRAP_MFN(CScriptFileSystem, Release), asCALL_GENERIC);
    assert(r >= 0);

    r = engine->RegisterObjectMethod(
        "filesystem", "bool changeCurrentPath(const string &in)",
        WRAP_MFN(CScriptFileSystem, ChangeCurrentPath), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "string getCurrentPath() const",
        WRAP_MFN(CScriptFileSystem, GetCurrentPath), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "array<string> @getDirs() const",
        WRAP_MFN(CScriptFileSystem, GetDirs), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "array<string> @getFiles() const",
        WRAP_MFN(CScriptFileSystem, GetFiles), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "bool isDir(const string &in) const",
        WRAP_MFN(CScriptFileSystem, IsDir), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "bool isLink(const string &in) const",
        WRAP_MFN(CScriptFileSystem, IsLink), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "int64 getSize(const string &in) const",
        WRAP_MFN(CScriptFileSystem, GetSize), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "bool makeDir(const string &in)",
        WRAP_MFN(CScriptFileSystem, MakeDir), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "bool removeDir(const string &in)",
        WRAP_MFN(CScriptFileSystem, RemoveDir), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "bool deleteFile(const string &in)",
        WRAP_MFN(CScriptFileSystem, DeleteFile), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "bool copyFile(const string &in, const string &in)",
        WRAP_MFN(CScriptFileSystem, CopyFile), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "bool moveFile(const string &in, const string &in)",
        WRAP_MFN(CScriptFileSystem, MoveFile), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "datetime getCreateDateTime(const string &in) const",
        WRAP_MFN(CScriptFileSystem, GetCreateDateTime), asCALL_GENERIC);
    assert(r >= 0);
    r = engine->RegisterObjectMethod(
        "filesystem", "datetime getModifyDateTime(const string &in) const",
        WRAP_MFN(CScriptFileSystem, GetModifyDateTime), asCALL_GENERIC);
    assert(r >= 0);
}

void RegisterScriptFileSystem(asIScriptEngine *engine) {
    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY"))
        RegisterScriptFileSystem_Generic(engine);
    else
        RegisterScriptFileSystem_Native(engine);
}

CScriptFileSystem::CScriptFileSystem() {
    refCount = 1;

    auto ctx = asGetActiveContext();
    if (ctx) {
        auto ptr = static_cast<const QString *>(
            ctx->GetUserData(AsUserDataType::UserData_Section_StringPtr));
        if (ptr) {
            QFileInfo info(*ptr);
            currentPath = info.absolutePath();
        } else {
            currentPath = QDir::homePath();
        }
    } else {
        currentPath = QDir::homePath();
    }
}

CScriptFileSystem::~CScriptFileSystem() {}

void CScriptFileSystem::AddRef() const { asAtomicInc(refCount); }

void CScriptFileSystem::Release() const {
    if (asAtomicDec(refCount) == 0)
        delete this;
}

CScriptArray *CScriptFileSystem::GetFiles() const {
    // Obtain a pointer to the engine
    asIScriptContext *ctx = asGetActiveContext();
    asIScriptEngine *engine = ctx->GetEngine();

    // TODO: This should only be done once
    // TODO: This assumes that CScriptArray was already registered
    asITypeInfo *arrayType = engine->GetTypeInfoByDecl("array<string>");

    QDir dir(currentPath);
    auto files = dir.entryList(QDir::Files);
    auto total = files.size();

    // Create the array object
    CScriptArray *array = CScriptArray::Create(arrayType, total);
    for (int i = 0; i < total; ++i) {
        reinterpret_cast<QString *>(array->At(i))->assign(files.at(i));
    }

    return array;
}

CScriptArray *CScriptFileSystem::GetDirs() const {
    // Obtain a pointer to the engine
    asIScriptContext *ctx = asGetActiveContext();
    asIScriptEngine *engine = ctx->GetEngine();

    // TODO: This should only be done once
    // TODO: This assumes that CScriptArray was already registered
    asITypeInfo *arrayType = engine->GetTypeInfoByDecl("array<string>");

    QDir dir(currentPath);
    auto files = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    auto total = files.size();

    // Create the array object
    CScriptArray *array = CScriptArray::Create(arrayType, total);
    for (int i = 0; i < total; ++i) {
        reinterpret_cast<QString *>(array->At(i))->assign(files.at(i));
    }

    return array;
}

// Doesn't change anything if the new path is not valid
bool CScriptFileSystem::ChangeCurrentPath(const QString &path) {
    QFileInfo info(currentPath, path);
    if (!info.isDir()) {
        return false;
    }

    currentPath = info.absoluteFilePath();
    return true;
}

bool CScriptFileSystem::IsDir(const QString &path) const {
    QFileInfo info(currentPath, path);
    return info.isDir();
}

bool CScriptFileSystem::IsLink(const QString &path) const {
    QFileInfo info(currentPath, path);
    return info.isSymLink();
}

asINT64 CScriptFileSystem::GetSize(const QString &path) const {
    QFileInfo info(currentPath, path);
    return info.size();
}

bool CScriptFileSystem::MakeDir(const QString &path) {
    QDir dir(currentPath);
    return dir.mkpath(path);
}

bool CScriptFileSystem::RemoveDir(const QString &path) {
    if (!ENABLE_WRITE) {
        constexpr auto msg = "filesystem::removeDir is prohibited by settings";
        auto ctx = asGetActiveContext();
        if (ctx) {
            ctx->SetException(msg);
        }
        qCritical(msg);
        return false;
    }
    QDir dir(currentPath);
    return dir.rmpath(path);
}

bool CScriptFileSystem::DeleteFile(const QString &path) {
    if (!ENABLE_WRITE) {
        constexpr auto msg = "filesystem::deleteFile is prohibited by settings";
        auto ctx = asGetActiveContext();
        if (ctx) {
            ctx->SetException(msg);
        }
        qCritical(msg);
        return false;
    }
    return QFile::remove(getRealAbsPath(path));
}

bool CScriptFileSystem::CopyFile(const QString &source, const QString &target) {
    return QFile::copy(getRealAbsPath(source), getRealAbsPath(target));
}

bool CScriptFileSystem::MoveFile(const QString &source, const QString &target) {
    if (!ENABLE_WRITE) {
        constexpr auto msg = "filesystem::moveFile is prohibited by settings";
        auto ctx = asGetActiveContext();
        if (ctx) {
            ctx->SetException(msg);
        }
        qCritical(msg);
        return false;
    }
    auto src = getRealAbsPath(source);
    auto dest = getRealAbsPath(target);
    return QFile::rename(source, dest);
}

QString CScriptFileSystem::GetCurrentPath() const { return currentPath; }

CDateTime CScriptFileSystem::GetCreateDateTime(const QString &path) const {
    QFileInfo info(currentPath, path);
    auto dt = info.fileTime(QFile::FileBirthTime);
    auto d = dt.date();
    auto t = dt.time();
    return CDateTime(d.year(), d.month(), d.day(), t.hour(), t.minute(),
                     t.second());
}

CDateTime CScriptFileSystem::GetModifyDateTime(const QString &path) const {
    QFileInfo info(currentPath, path);
    auto dt = info.fileTime(QFile::FileModificationTime);
    auto d = dt.date();
    auto t = dt.time();
    return CDateTime(d.year(), d.month(), d.day(), t.hour(), t.minute(),
                     t.second());
}

QString CScriptFileSystem::getRealAbsPath(const QString &path) {
    QFileInfo info(currentPath, path);
    return info.absoluteFilePath();
}

END_AS_NAMESPACE
