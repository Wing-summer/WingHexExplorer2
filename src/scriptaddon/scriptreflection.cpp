/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include "scriptreflection.h"
#include "define.h"

#include "scriptaddon/scriptqdictionary.h"

#include <QString>
#include <QStringList>

class ASRef {
public:
    ASRef() : m_ref(1) {}
    void AddRef() { ++m_ref; }
    void Release() {
        if (--m_ref == 0) {
            deleteSelf();
        }
    }

protected:
    virtual void deleteSelf() = 0;
    virtual ~ASRef() {}

private:
    int m_ref;
};

class ASEnum final : public ASRef {
public:
    ASEnum(asITypeInfo *t) : type(t) {
        if (type)
            type->AddRef();
    }
    ~ASEnum() {
        if (type)
            type->Release();
    }

    QString name() const {
        return type ? QString::fromUtf8(type->GetName()) : QString{};
    }
    asUINT valueCount() const { return type ? type->GetEnumValueCount() : 0; }
    QString valueName(asUINT idx) const {
        if (!type) {
            return {};
        }
        return QString::fromUtf8(type->GetEnumValueByIndex(idx, nullptr));
    }
    asINT64 value(asUINT idx) const {
        if (!type)
            return 0;
        asINT64 v = 0;
        type->GetEnumValueByIndex(idx, &v);
        return v;
    }

    asINT64 keyToValue(const QString &key) const {
        QString scope;
        auto rkey = key;
        auto s = key.lastIndexOf(':');
        if (s > 0 && key.at(s - 1) == ':') {
            scope = key.sliced(0, s - 1);
            rkey.slice(s + 1);
        }

        auto count = valueCount();
        auto ename = this->name();
        for (int i = 0; i < count; ++i) {
            asINT64 v = 0;
            auto name = type->GetEnumValueByIndex(i, &v);
            if (scope.isEmpty()) {
                if (name == rkey) {
                    return v;
                }
            } else {
                if (scope == ename && name == rkey) {
                    return v;
                }
            }
        }
        return -1;
    }

    QString valueToKey(asINT64 value, bool scope) const {
        if (!type) {
            return {};
        }

        auto total = valueCount();
        auto name = this->name();
        for (int i = 0; i < total; ++i) {
            asINT64 v = 0;
            auto name = type->GetEnumValueByIndex(i, &v);
            if (v == value) {
                auto r = QString::fromUtf8(name);
                if (scope) {
                    r.prepend(name + QStringLiteral("::"));
                }
                return r;
            }
        }

        return {};
    }

    asINT64 keysToValue(const QString &keys) const {
        auto l = keys.split('|');
        auto total = l.size();
        asINT64 value = 0;
        for (int i = 0; i < total; ++i) {
            auto key = l.at(i).trimmed();
            QString scope;
            auto rkey = key;
            auto s = key.lastIndexOf(':');
            if (s > 0 && key.at(s - 1) == ':') {
                scope = key.sliced(0, s - 1);
                rkey.slice(s + 1);
            }

            auto count = valueCount();
            auto ename = this->name();
            for (int i = 0; i < count; ++i) {
                asINT64 v = 0;
                auto name = type->GetEnumValueByIndex(i, &v);
                if (scope.isEmpty()) {
                    if (name == rkey) {
                        return v;
                    }
                } else {
                    if (scope == ename && name == rkey) {
                        return v;
                    }
                }
            }
        }
        return value;
    }

    QString valueToKeys(asINT64 value, bool scope) const {
        if (!type) {
            return {};
        }

        QString keys;
        auto total = valueCount();
        auto ename = this->name();
        auto v = value;
        for (int i = 0; i < total; ++i) {
            asINT64 k = 0;
            auto name = type->GetEnumValueByIndex(i, &k);
            if ((k != 0 && (v & k) == k) || (k == value)) {
                v = v & ~k;
                if (!keys.isEmpty()) {
                    keys += '|';
                }
                if (scope) {
                    keys += ename + QStringLiteral("::");
                }
                keys += name;
            }
        }
        return keys;
    }

protected:
    void deleteSelf() override {
        this->~ASEnum();
        asFreeMem(const_cast<ASEnum *>(this));
    }

private:
    asITypeInfo *type;
};

class ASFunc final : public ASRef {
public:
    ASFunc(asIScriptFunction *f) : func(f) {
        if (func)
            func->AddRef();
    }
    ~ASFunc() {
        if (func)
            func->Release();
    }

    QString declaration(bool includeReturn = true,
                        bool includeNamespace = true) const {
        return func ? QString::fromUtf8(
                          func->GetDeclaration(includeReturn, includeNamespace))
                    : QString{};
    }
    asUINT paramCount() const { return func ? func->GetParamCount() : 0; }
    int paramTypeId(unsigned i) const {
        if (!func) {
            return -1;
        }
        int id;
        if (func->GetParam(i, &id) == asSUCCESS) {
            return id;
        }
        return -1;
    }
    int returnTypeId() const { return func ? func->GetReturnTypeId() : -1; }

protected:
    void deleteSelf() override {
        this->~ASFunc();
        asFreeMem(const_cast<ASFunc *>(this));
    }

private:
    asIScriptFunction *func;
};

class ASType final : public ASRef {
public:
    ASType(asITypeInfo *t) : type(t) {
        if (type)
            type->AddRef();
    }
    ~ASType() {
        if (type)
            type->Release();
    }

    QString name() const {
        return type ? QString::fromUtf8(type->GetName()) : QString{};
    }
    asUINT methodCount() const { return type ? type->GetMethodCount() : 0; }
    ASFunc *method(unsigned idx) const {
        if (!type)
            return nullptr;
        asIScriptFunction *f = type->GetMethodByIndex(idx, false);
        if (!f)
            return nullptr;
        auto mem = static_cast<ASFunc *>(asAllocMem(sizeof(ASFunc)));
        return new (mem) ASFunc(f);
    }
    asUINT propertyCount() const { return type ? type->GetPropertyCount() : 0; }
    QString propertyDeclaration(unsigned idx) const {
        if (!type)
            return {};
        return QString::fromUtf8(type->GetPropertyDeclaration(idx));
    }

    // enum-like access if the type is an enum
    asUINT enumValueCount() const {
        return type ? type->GetEnumValueCount() : 0;
    }
    QString enumValueName(unsigned idx) const {
        if (!type)
            return {};
        return QString::fromUtf8(type->GetEnumValueByIndex(idx, nullptr));
    }
    asINT64 enumValue(unsigned idx) const {
        if (!type)
            return 0;
        asINT64 v = 0;
        type->GetEnumValueByIndex(idx, &v);
        return v;
    }

protected:
    void deleteSelf() override {
        this->~ASType();
        asFreeMem(const_cast<ASType *>(this));
    }

private:
    asITypeInfo *type;
};

static ASEnum *FromEnum_ByName(const QString &enumName) {
    auto ctx = asGetActiveContext();
    if (!ctx) {
        return nullptr;
    }

    auto eName = enumName.toUtf8();
    auto engine = ctx->GetEngine();
    asITypeInfo *t = engine->GetTypeInfoByName(eName);

    if (!t) {
        return nullptr;
    }

    // check enum flag
    asDWORD flags = t->GetFlags();
    if ((flags & asOBJ_ENUM) == 0) {
        return nullptr;
    }

    auto mem = static_cast<ASEnum *>(asAllocMem(sizeof(ASEnum)));
    return new (mem) ASEnum(t);
}

static ASType *FromType_ByName(const QString &typeName) {
    auto ctx = asGetActiveContext();
    if (!ctx) {
        return nullptr;
    }

    auto tName = typeName.toUtf8();
    auto engine = ctx->GetEngine();
    asITypeInfo *t = engine->GetTypeInfoByName(tName);

    if (!t) {
        return nullptr;
    }

    auto mem = static_cast<ASType *>(asAllocMem(sizeof(ASType)));
    return new (mem) ASType(t);
}

// Generic for: ASEnum@ fromEnum<T>()
static void Generic_FromEnum_Template(asIScriptGeneric *gen) {
    // The called (instantiated) function can expose its template subtypes
    auto inst = gen->GetFunction(); // the actual instantiated template function
    if (!inst) {
        gen->SetReturnObject(nullptr);
        return;
    }

    auto subCount = inst->GetSubTypeCount();
    if (subCount == 0) {
        gen->SetReturnObject(nullptr);
        return;
    }

    // We take the first template subtype as the T for GetEnum<T>()
    auto subtype = inst->GetSubType(0);
    if (!subtype) {
        gen->SetReturnObject(nullptr);
        return;
    }

    // ensure it's an enum
    if ((subtype->GetFlags() & asOBJ_ENUM) == 0) {
        gen->SetReturnObject(nullptr);
        return;
    }

    auto mem = static_cast<ASEnum *>(asAllocMem(sizeof(ASEnum)));
    gen->SetReturnObject(new (mem) ASEnum(subtype));
}

// Generic for: ASType@ fromType<T>()
static void Generic_FromType_Template(asIScriptGeneric *gen) {
    auto inst = gen->GetFunction();
    if (!inst) {
        gen->SetReturnObject(nullptr);
        return;
    }

    auto subCount = inst->GetSubTypeCount();
    if (subCount == 0) {
        gen->SetReturnObject(nullptr);
        return;
    }

    asITypeInfo *subtype = inst->GetSubType(0);
    if (!subtype) {
        gen->SetReturnObject(nullptr);
        return;
    }

    auto mem = static_cast<ASType *>(asAllocMem(sizeof(ASType)));
    gen->SetReturnObject(new (mem) ASType(subtype));
}

// Generic entry for: dictionary@ getClassMembers(const ?&in obj)
static void getClassMembers_Generic(asIScriptGeneric *gen) {
    asIScriptEngine *engine = gen->GetEngine();
    CScriptDictionary *dict = CScriptDictionary::Create(engine);

    int argTypeId = gen->GetArgTypeId(0);

    const int realTypeId = argTypeId & ~asTYPEID_OBJHANDLE;
    asITypeInfo *ti = engine->GetTypeInfoById(realTypeId);
    if (!ti) {
        gen->SetReturnObject(dict);
        return;
    }

    asDWORD typeFlags = ti->GetFlags();
    if (!(typeFlags & asOBJ_SCRIPT_OBJECT)) {
        gen->SetReturnObject(dict);
        return;
    }

    bool isHandle = (argTypeId & asTYPEID_OBJHANDLE) != 0;
    asIScriptObject *scriptObj = nullptr;
    if (isHandle) {
        scriptObj = reinterpret_cast<asIScriptObject *>(gen->GetArgObject(0));
    } else {
        scriptObj = reinterpret_cast<asIScriptObject *>(gen->GetArgAddress(0));
    }

    if (scriptObj == nullptr) {
        gen->SetReturnObject(dict);
        return;
    }

    asUINT propCount = ti->GetPropertyCount();
    for (asUINT i = 0; i < propCount; ++i) {
        const char *propName = nullptr;
        int propTypeId = 0;
        ti->GetProperty(i, &propName, &propTypeId);
        if (!propName)
            continue;

        void *propAddr = scriptObj->GetAddressOfProperty(i);
        if (!propAddr) {
            continue;
        }

        dict->Set(QString::fromUtf8(propName), propAddr, propTypeId);
    }
    gen->SetReturnObject(dict);
}

void RegisterASReflection(asIScriptEngine *engine) {
    int r;

    r = engine->SetDefaultNamespace("reflect");
    ASSERT(r >= 0);

    // ASEnum
    r = engine->RegisterObjectType("ASEnum", 0, asOBJ_REF);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour("ASEnum", asBEHAVE_ADDREF, "void f()",
                                        asMETHOD(ASEnum, AddRef),
                                        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour("ASEnum", asBEHAVE_RELEASE, "void f()",
                                        asMETHOD(ASEnum, Release),
                                        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("ASEnum", "string name() const",
                                     asMETHOD(ASEnum, name), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("ASEnum", "uint valueCount() const",
                                     asMETHOD(ASEnum, valueCount),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("ASEnum", "string valueName(uint) const",
                                     asMETHOD(ASEnum, valueName),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("ASEnum", "int64 value(uint) const",
                                     asMETHOD(ASEnum, value), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "ASEnum", "string valueToKey(int64, bool scope = false) const",
        asMETHOD(ASEnum, valueToKey), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "ASEnum", "int64 keyToValue(const string&in) const",
        asMETHOD(ASEnum, keyToValue), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "ASEnum", "string valueToKeys(int64, bool scope = false) const",
        asMETHOD(ASEnum, valueToKeys), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "ASEnum", "int64 keysToValue(const string&in) const",
        asMETHOD(ASEnum, keysToValue), asCALL_THISCALL);
    ASSERT(r >= 0);

    // ASFunc
    r = engine->RegisterObjectType("ASFunc", 0, asOBJ_REF);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour("ASFunc", asBEHAVE_ADDREF, "void f()",
                                        asMETHOD(ASFunc, AddRef),
                                        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour("ASFunc", asBEHAVE_RELEASE, "void f()",
                                        asMETHOD(ASFunc, Release),
                                        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "ASFunc", "string declaration(bool = true, bool = true) const",
        asMETHOD(ASFunc, declaration), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("ASFunc", "uint paramCount() const",
                                     asMETHOD(ASFunc, paramCount),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("ASFunc", "int paramTypeId(uint) const",
                                     asMETHOD(ASFunc, paramTypeId),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("ASFunc", "int returnTypeId() const",
                                     asMETHOD(ASFunc, returnTypeId),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);

    // ASType
    r = engine->RegisterObjectType("ASType", 0, asOBJ_REF);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour("ASType", asBEHAVE_ADDREF, "void f()",
                                        asMETHOD(ASType, AddRef),
                                        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour("ASType", asBEHAVE_RELEASE, "void f()",
                                        asMETHOD(ASType, Release),
                                        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("ASType", "string name() const",
                                     asMETHOD(ASType, name), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("ASType", "uint methodCount() const",
                                     asMETHOD(ASType, methodCount),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("ASType", "ASFunc@ method(uint) const",
                                     asMETHOD(ASType, method), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("ASType", "uint propertyCount() const",
                                     asMETHOD(ASType, propertyCount),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "ASType", "string propertyDeclaration(uint) const",
        asMETHOD(ASType, propertyDeclaration), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("ASType", "uint enumValueCount() const",
                                     asMETHOD(ASType, enumValueCount),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "ASType", "string enumValueName(uint) const",
        asMETHOD(ASType, enumValueName), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("ASType", "int enumValue(uint) const",
                                     asMETHOD(ASType, enumValue),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);

    // --- string-based global factories ---
    r = engine->RegisterGlobalFunction("ASEnum@ fromEnum(const string &in)",
                                       asFUNCTION(FromEnum_ByName),
                                       asCALL_CDECL);
    ASSERT(r >= 0);
    r = engine->RegisterGlobalFunction("ASType@ fromType(const string &in)",
                                       asFUNCTION(FromType_ByName),
                                       asCALL_CDECL);
    ASSERT(r >= 0);

    // --- template-based factories (requires AngelScript that supports template
    // functions) ---
    r = engine->RegisterGlobalFunction("ASEnum@ fromEnum<T>()",
                                       asFUNCTION(Generic_FromEnum_Template),
                                       asCALL_GENERIC);
    ASSERT(r >= 0);
    r = engine->RegisterGlobalFunction("ASType@ fromType<T>()",
                                       asFUNCTION(Generic_FromType_Template),
                                       asCALL_GENERIC);
    ASSERT(r >= 0);

    // for script classes
    r = engine->RegisterGlobalFunction(
        "dictionary@ getClassMembers(const ?&in)",
        asFUNCTION(getClassMembers_Generic), asCALL_GENERIC);
    ASSERT(r >= 0);

    engine->SetDefaultNamespace("");
}
