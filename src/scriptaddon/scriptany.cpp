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

#include "scriptany.h"
#include "define.h"

#include <new>

BEGIN_AS_NAMESPACE

// We'll use the generic interface for the factories as we need the engine
// pointer
static void ScriptAnyFactory_Generic(asIScriptGeneric *gen) {
    asIScriptEngine *engine = gen->GetEngine();

    auto mem = static_cast<CScriptAny *>(asAllocMem(sizeof(CScriptAny)));
    *(CScriptAny **)gen->GetAddressOfReturnLocation() =
        new (mem) CScriptAny(engine);
}

static void ScriptAnyFactory2_Generic(asIScriptGeneric *gen) {
    asIScriptEngine *engine = gen->GetEngine();
    void *ref = (void *)gen->GetArgAddress(0);
    int refType = gen->GetArgTypeId(0);

    auto mem = static_cast<CScriptAny *>(asAllocMem(sizeof(CScriptAny)));
    *(CScriptAny **)gen->GetAddressOfReturnLocation() =
        new (mem) CScriptAny(engine, ref, refType);
}

static CScriptAny &ScriptAnyAssignment(CScriptAny *other, CScriptAny *self) {
    return *self = *other;
}

void RegisterScriptAny_Native(asIScriptEngine *engine);

void RegisterScriptAny(asIScriptEngine *engine) {
    RegisterScriptAny_Native(engine);
}

static void CScriptAny_opCast(void *ref, int typeId, CScriptAny *obj) {
    obj->Retrieve(ref, typeId);
}

static asINT64 CScriptAny_opConvInt(CScriptAny *obj) {
    asINT64 value = 0;
    CScriptAny_opCast(&value, asTYPEID_INT64, obj);
    return value;
}

static asQWORD CScriptAny_opConvUInt(CScriptAny *obj) {
    asQWORD value = 0;
    CScriptAny_opCast(&value, asTYPEID_UINT64, obj);
    return value;
}

static double CScriptAny_opConvDouble(CScriptAny *obj) {
    double value = 0;
    CScriptAny_opCast(&value, asTYPEID_DOUBLE, obj);
    return value;
}

static CScriptAny &CScriptAny_opAssign(void *ref, int typeId, CScriptAny *obj) {
    obj->Store(ref, typeId);
    return *obj;
}

static CScriptAny &CScriptAny_opAssign(double val, CScriptAny *obj) {
    return CScriptAny_opAssign(&val, asTYPEID_DOUBLE, obj);
}

static CScriptAny &CScriptAny_opAssign(asINT64 val, CScriptAny *obj) {
    return CScriptAny_opAssign(&val, asTYPEID_INT64, obj);
}

static CScriptAny &CScriptAny_opAssign(asQWORD val, CScriptAny *obj) {
    return CScriptAny_opAssign(&val, asTYPEID_UINT64, obj);
}

void RegisterScriptAny_Native(asIScriptEngine *engine) {
    int r;
    r = engine->RegisterObjectType("any", sizeof(CScriptAny),
                                   asOBJ_REF | asOBJ_GC);
    ASSERT(r >= 0);

    // We'll use the generic interface for the constructor as we need the engine
    // pointer
    r = engine->RegisterObjectBehaviour("any", asBEHAVE_FACTORY, "any@ f()",
                                        asFUNCTION(ScriptAnyFactory_Generic),
                                        asCALL_GENERIC);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "any", asBEHAVE_FACTORY, "any@ f(?&in) explicit",
        asFUNCTION(ScriptAnyFactory2_Generic), asCALL_GENERIC);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "any", asBEHAVE_FACTORY, "any@ f(const int64&in) explicit",
        asFUNCTION(ScriptAnyFactory2_Generic), asCALL_GENERIC);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "any", asBEHAVE_FACTORY, "any@ f(const double&in) explicit",
        asFUNCTION(ScriptAnyFactory2_Generic), asCALL_GENERIC);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour("any", asBEHAVE_ADDREF, "void f()",
                                        asMETHOD(CScriptAny, AddRef),
                                        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour("any", asBEHAVE_RELEASE, "void f()",
                                        asMETHOD(CScriptAny, Release),
                                        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("any", "any &opAssign(any&in)",
                                     asFUNCTION(ScriptAnyAssignment),
                                     asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "any", "void store(?&in)",
        asMETHODPR(CScriptAny, Store, (void *, int), void), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "any", "void store(const uint64&in)",
        asMETHODPR(CScriptAny, Store, (const asQWORD &), void),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "any", "void store(const int64&in)",
        asMETHODPR(CScriptAny, Store, (const asINT64 &), void),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "any", "void store(const double&in)",
        asMETHODPR(CScriptAny, Store, (const double &), void), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "any", "bool retrieve(?&out) const",
        asMETHODPR(CScriptAny, Retrieve, (void *, int) const, bool),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "any", "bool retrieve(uint64&out) const",
        asMETHODPR(CScriptAny, Retrieve, (asQWORD &) const, bool),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "any", "bool retrieve(int64&out) const",
        asMETHODPR(CScriptAny, Retrieve, (asINT64 &) const, bool),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "any", "bool retrieve(double&out) const",
        asMETHODPR(CScriptAny, Retrieve, (double &) const, bool),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "any", "void opCast(?&out)",
        asFUNCTIONPR(CScriptAny_opCast, (void *, int, CScriptAny *), void),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "any", "void opConv(?&out)",
        asFUNCTIONPR(CScriptAny_opCast, (void *, int, CScriptAny *), void),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "any", "int64 opConv()",
        asFUNCTIONPR(CScriptAny_opConvInt, (CScriptAny *), asINT64),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "any", "uint64 opConv()",
        asFUNCTIONPR(CScriptAny_opConvUInt, (CScriptAny *), asQWORD),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "any", "double opConv()",
        asFUNCTIONPR(CScriptAny_opConvDouble, (CScriptAny *), double),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("any", "any &opAssign(const ?&in)",
                                     asFUNCTIONPR(CScriptAny_opAssign,
                                                  (void *, int, CScriptAny *),
                                                  CScriptAny &),
                                     asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "any", "any &opAssign(double)",
        asFUNCTIONPR(CScriptAny_opAssign, (double, CScriptAny *), CScriptAny &),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("any", "any &opAssign(int64)",
                                     asFUNCTIONPR(CScriptAny_opAssign,
                                                  (asINT64, CScriptAny *),
                                                  CScriptAny &),
                                     asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("any", "any &opAssign(uint64)",
                                     asFUNCTIONPR(CScriptAny_opAssign,
                                                  (asQWORD, CScriptAny *),
                                                  CScriptAny &),
                                     asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    // Register GC behaviours
    r = engine->RegisterObjectBehaviour("any", asBEHAVE_GETREFCOUNT, "int f()",
                                        asMETHOD(CScriptAny, GetRefCount),
                                        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour("any", asBEHAVE_SETGCFLAG, "void f()",
                                        asMETHOD(CScriptAny, SetFlag),
                                        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour("any", asBEHAVE_GETGCFLAG, "bool f()",
                                        asMETHOD(CScriptAny, GetFlag),
                                        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "any", asBEHAVE_ENUMREFS, "void f(int&in)",
        asMETHOD(CScriptDictValue, EnumReferences), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "any", asBEHAVE_RELEASEREFS, "void f(int&in)",
        asMETHOD(CScriptAny, ReleaseAllHandles), asCALL_THISCALL);
    ASSERT(r >= 0);
}

CScriptAny &CScriptAny::operator=(const CScriptAny &other) {
    // Hold on to the object type reference so it isn't destroyed too early
    if ((other.m_typeId & asTYPEID_MASK_OBJECT)) {
        asITypeInfo *ti = engine->GetTypeInfoById(other.m_typeId);
        if (ti)
            ti->AddRef();
    }

    FreeObject();

    m_typeId = other.m_typeId;
    if (m_typeId & asTYPEID_OBJHANDLE) {
        // For handles, copy the pointer and increment the reference count
        m_valueObj = m_valueObj;
        engine->AddRefScriptObject(m_valueObj,
                                   engine->GetTypeInfoById(m_typeId));
    } else if (m_typeId & asTYPEID_MASK_OBJECT) {
        // Create a copy of the object
        m_valueObj = engine->CreateScriptObjectCopy(
            other.m_valueObj, engine->GetTypeInfoById(m_typeId));
    } else {
        // Primitives can be copied directly
        m_valueInt = m_valueInt;
    }

    return *this;
}

int CScriptAny::CopyFrom(const CScriptAny *other) {
    if (other == 0)
        return asINVALID_ARG;

    *this = *other;

    return 0;
}

CScriptAny::CScriptAny(asIScriptEngine *engine) {
    this->engine = engine;
    refCount = 1;
    gcFlag = false;

    // Notify the garbage collector of this object
    engine->NotifyGarbageCollectorOfNewObject(this,
                                              engine->GetTypeInfoByName("any"));
}

CScriptAny::CScriptAny(asIScriptEngine *engine, void *ref, int refTypeId) {
    this->engine = engine;
    refCount = 1;
    gcFlag = false;

    // Notify the garbage collector of this object
    engine->NotifyGarbageCollectorOfNewObject(this,
                                              engine->GetTypeInfoByName("any"));

    Store(ref, refTypeId);
}

CScriptAny::~CScriptAny() { FreeObject(); }

void CScriptAny::FreeObject() { FreeValue(engine); }

void CScriptAny::ReleaseAllHandles(asIScriptEngine * /*engine*/) {
    FreeObject();
}

int CScriptAny::AddRef() const {
    // Increase counter and clear flag set by GC
    gcFlag = false;
    return asAtomicInc(refCount);
}

int CScriptAny::Release() const {
    // Decrease the ref counter
    gcFlag = false;
    if (asAtomicDec(refCount) == 0) {
        this->~CScriptAny();
        asFreeMem(const_cast<CScriptAny *>(this));
        return 0;
    }

    return refCount;
}

void CScriptAny::Store(void *ref, int refTypeId) {
    Set(engine, ref, refTypeId);
}

void CScriptAny::Store(const asQWORD &value) { Set(engine, value); }

void CScriptAny::Store(const asINT64 &value) { Set(engine, value); }

void CScriptAny::Store(const double &value) { Set(engine, value); }

bool CScriptAny::Retrieve(void *ref, int refTypeId) const {
    return Get(engine, ref, refTypeId);
}

bool CScriptAny::Retrieve(asQWORD &value) const { return Get(engine, value); }

bool CScriptAny::Retrieve(asINT64 &value) const { return Get(engine, value); }

bool CScriptAny::Retrieve(double &value) const { return Get(engine, value); }

int CScriptAny::GetRefCount() { return refCount; }

void CScriptAny::SetFlag() { gcFlag = true; }

bool CScriptAny::GetFlag() { return gcFlag; }

END_AS_NAMESPACE
