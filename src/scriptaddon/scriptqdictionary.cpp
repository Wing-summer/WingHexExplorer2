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

#include "scriptqdictionary.h"
#include "AngelScript/sdk/add_on/scriptarray/scriptarray.h"
#include <assert.h>

BEGIN_AS_NAMESPACE

//------------------------------------------------------------------------
// Object types are cached as user data to avoid costly runtime lookups

// We just define a number here that we assume nobody else is using for
// object type user data. The add-ons have reserved the numbers 1000
// through 1999 for this purpose, so we should be fine.
const asPWORD DICTIONARY_CACHE = 1003;

// This cache holds the object type of the dictionary type and array type
// so it isn't necessary to look this up each time the dictionary or array
// is created.
struct SDictionaryCache {
    asITypeInfo *dictType;
    asITypeInfo *arrayType;
    asITypeInfo *keyType;

    // This is called from RegisterScriptDictionary
    static void Setup(asIScriptEngine *engine) {
        SDictionaryCache *cache = reinterpret_cast<SDictionaryCache *>(
            engine->GetUserData(DICTIONARY_CACHE));
        if (cache == 0) {
            cache = new SDictionaryCache;
            engine->SetUserData(cache, DICTIONARY_CACHE);
            engine->SetEngineUserDataCleanupCallback(SDictionaryCache::Cleanup,
                                                     DICTIONARY_CACHE);

            cache->dictType = engine->GetTypeInfoByName("dictionary");
            cache->arrayType = engine->GetTypeInfoByDecl("array<string>");
            cache->keyType = engine->GetTypeInfoByDecl("string");
        }
    }

    // This is called from the engine when shutting down
    static void Cleanup(asIScriptEngine *engine) {
        SDictionaryCache *cache = reinterpret_cast<SDictionaryCache *>(
            engine->GetUserData(DICTIONARY_CACHE));
        if (cache)
            delete cache;
    }
};

//--------------------------------------------------------------------------
// CScriptDictionary implementation

CScriptDictionary *CScriptDictionary::Create(asIScriptEngine *engine) {
    // Use the custom memory routine from AngelScript to allow application to
    // better control how much memory is used
    CScriptDictionary *obj =
        (CScriptDictionary *)asAllocMem(sizeof(CScriptDictionary));
    new (obj) CScriptDictionary(engine);
    return obj;
}

CScriptDictionary *CScriptDictionary::Create(asBYTE *buffer) {
    // Use the custom memory routine from AngelScript to allow application to
    // better control how much memory is used
    CScriptDictionary *obj =
        (CScriptDictionary *)asAllocMem(sizeof(CScriptDictionary));
    new (obj) CScriptDictionary(buffer);
    return obj;
}

CScriptDictionary::CScriptDictionary(asIScriptEngine *engine) { Init(engine); }

void CScriptDictionary::Init(asIScriptEngine *e) {
    // We start with one reference
    refCount = 1;
    gcFlag = false;
    iterGuard = 0;

    // Keep a reference to the engine for as long as we live
    // We don't increment the reference counter, because the
    // engine will hold a pointer to the object in the GC.
    engine = e;

    // The dictionary object type is cached to avoid dynamically parsing it each
    // time
    SDictionaryCache *cache = reinterpret_cast<SDictionaryCache *>(
        engine->GetUserData(DICTIONARY_CACHE));

    // Notify the garbage collector of this object
    engine->NotifyGarbageCollectorOfNewObject(this, cache->dictType);
}

CScriptDictionary::CScriptDictionary(asBYTE *buffer) {
    // This constructor will always be called from a script
    // so we can get the engine from the active context
    asIScriptContext *ctx = asGetActiveContext();
    Init(ctx->GetEngine());

    // Determine if the dictionary key type is registered as reference type or
    // value type
    SDictionaryCache &cache = *reinterpret_cast<SDictionaryCache *>(
        engine->GetUserData(DICTIONARY_CACHE));
    bool keyAsRef = cache.keyType->GetFlags() & asOBJ_REF ? true : false;

    // Initialize the dictionary from the buffer
    asUINT length = *(asUINT *)buffer;
    buffer += 4;

    while (length--) {
        // Align the buffer pointer on a 4 byte boundary in
        // case previous value was smaller than 4 bytes
        if (asPWORD(buffer) & 0x3)
            buffer += 4 - (asPWORD(buffer) & 0x3);

        // Get the name value pair from the buffer and insert it in the
        // dictionary
        dictKey_t name;
        if (keyAsRef) {
            name = **(dictKey_t **)buffer;
            buffer += sizeof(dictKey_t *);
        } else {
            name = *(dictKey_t *)buffer;
            buffer += sizeof(dictKey_t);
        }

        // Get the type id of the value
        int typeId = *(int *)buffer;
        buffer += sizeof(int);

        // Depending on the type id, the value will inline in the buffer or a
        // pointer
        void *ref = (void *)buffer;

        if (typeId >= asTYPEID_INT8 && typeId <= asTYPEID_DOUBLE) {
            // Convert primitive values to either int64 or double, so we can use
            // the overloaded Set methods
            asINT64 i64;
            double d;
            switch (typeId) {
            case asTYPEID_INT8:
                i64 = *(char *)ref;
                break;
            case asTYPEID_INT16:
                i64 = *(short *)ref;
                break;
            case asTYPEID_INT32:
                i64 = *(int *)ref;
                break;
            case asTYPEID_INT64:
                i64 = *(asINT64 *)ref;
                break;
            case asTYPEID_UINT8:
                i64 = *(unsigned char *)ref;
                break;
            case asTYPEID_UINT16:
                i64 = *(unsigned short *)ref;
                break;
            case asTYPEID_UINT32:
                i64 = *(unsigned int *)ref;
                break;
            case asTYPEID_UINT64:
                i64 = *(asINT64 *)ref;
                break;
            case asTYPEID_FLOAT:
                d = *(float *)ref;
                break;
            case asTYPEID_DOUBLE:
                d = *(double *)ref;
                break;
            }

            if (typeId >= asTYPEID_FLOAT)
                Set(name, d);
            else
                Set(name, i64);
        } else {
            if ((typeId & asTYPEID_MASK_OBJECT) &&
                !(typeId & asTYPEID_OBJHANDLE) &&
                (engine->GetTypeInfoById(typeId)->GetFlags() & asOBJ_REF)) {
                // Dereference the pointer to get the reference to the actual
                // object
                ref = *(void **)ref;
            }

            Set(name, ref, typeId);
        }

        // Advance the buffer pointer with the size of the value
        if (typeId & asTYPEID_MASK_OBJECT) {
            asITypeInfo *ti = engine->GetTypeInfoById(typeId);
            if (ti->GetFlags() & asOBJ_VALUE)
                buffer += ti->GetSize();
            else
                buffer += sizeof(void *);
        } else if (typeId == 0) {
            // null pointer
            buffer += sizeof(void *);
        } else {
            buffer += engine->GetSizeOfPrimitiveType(typeId);
        }
    }
}

CScriptDictionary::~CScriptDictionary() {
    // Delete all keys and values
    DeleteAll();
}

void CScriptDictionary::AddRef() const {
    // We need to clear the GC flag
    gcFlag = false;
    asAtomicInc(refCount);
}

void CScriptDictionary::Release() const {
    // We need to clear the GC flag
    gcFlag = false;
    if (asAtomicDec(refCount) == 0) {
        this->~CScriptDictionary();
        asFreeMem(const_cast<CScriptDictionary *>(this));
    }
}

int CScriptDictionary::GetRefCount() { return refCount; }

void CScriptDictionary::SetGCFlag() { gcFlag = true; }

bool CScriptDictionary::GetGCFlag() { return gcFlag; }

void CScriptDictionary::EnumReferences(asIScriptEngine *inEngine) {
    // TODO: If garbage collection can be done from a separate thread, then this
    // method must be
    //       protected so that it doesn't get lost during the iteration if the
    //       dictionary is modified

    // Call the gc enum callback for each of the objects
    dictMap_t::iterator it;
    for (it = dict.begin(); it != dict.end(); it++) {
        if (it->second.m_typeId & asTYPEID_MASK_OBJECT) {
            asITypeInfo *subType = engine->GetTypeInfoById(it->second.m_typeId);
            if (subType) {
                if ((subType->GetFlags() & asOBJ_VALUE) &&
                    (subType->GetFlags() & asOBJ_GC)) {
                    // For value types we need to forward the enum callback
                    // to the object so it can decide what to do
                    engine->ForwardGCEnumReferences(it->second.m_valueObj,
                                                    subType);
                } else {
                    // For others, simply notify the GC about the reference
                    inEngine->GCEnumCallback(it->second.m_valueObj);
                }
            }
        }
    }
}

void CScriptDictionary::ReleaseAllReferences(asIScriptEngine * /*engine*/) {
    // We're being told to release all references in
    // order to break circular references for dead objects
    DeleteAll();
}

CScriptDictionary &
CScriptDictionary::operator=(const CScriptDictionary &other) {
    // Clear everything we had before
    DeleteAll();

    // Do a shallow copy of the dictionary
    dictMap_t::const_iterator it;
    for (it = other.dict.begin(); it != other.dict.end(); it++) {
        if (it->second.m_typeId & asTYPEID_OBJHANDLE)
            Set(it->first, (void *)const_cast<void **>(&it->second.m_valueObj),
                it->second.m_typeId);
        else if (it->second.m_typeId & asTYPEID_MASK_OBJECT)
            Set(it->first, (void *)it->second.m_valueObj, it->second.m_typeId);
        else
            Set(it->first,
                (void *)const_cast<asINT64 *>(&it->second.m_valueInt),
                it->second.m_typeId);
    }

    return *this;
}

CScriptDictValue *CScriptDictionary::operator[](const dictKey_t &key) {
    // Return the existing value if it exists, else insert an empty value
    return &dict[key];
}

const CScriptDictValue *
CScriptDictionary::operator[](const dictKey_t &key) const {
    // Return the existing value if it exists
    dictMap_t::const_iterator it;
    it = dict.find(key);
    if (it != dict.end())
        return &it->second;

    // Else raise an exception
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx)
        ctx->SetException("Invalid access to non-existing value");

    return 0;
}

void CScriptDictionary::Set(const dictKey_t &key, void *value, int typeId) {
    dictMap_t::iterator it;
    it = dict.find(key);
    if (it == dict.end()) {
        it = dict.insert(dictMap_t::value_type(key, CScriptDictValue())).first;
        iterGuard++;
    }

    it->second.Set(engine, value, typeId);
}

void CScriptDictionary::Set(const dictKey_t &key, const asQWORD &value) {
    Set(key, const_cast<asQWORD *>(&value), asTYPEID_UINT64);
}

// This overloaded method is implemented so that all integer and
// unsigned integers types will be stored in the dictionary as int64
// through implicit conversions. This simplifies the management of the
// numeric types when the script retrieves the stored value using a
// different type.
void CScriptDictionary::Set(const dictKey_t &key, const asINT64 &value) {
    Set(key, const_cast<asINT64 *>(&value), asTYPEID_INT64);
}

// This overloaded method is implemented so that all floating point types
// will be stored in the dictionary as double through implicit conversions.
// This simplifies the management of the numeric types when the script
// retrieves the stored value using a different type.
void CScriptDictionary::Set(const dictKey_t &key, const double &value) {
    Set(key, const_cast<double *>(&value), asTYPEID_DOUBLE);
}

// Returns true if the value was successfully retrieved
bool CScriptDictionary::Get(const dictKey_t &key, void *value,
                            int typeId) const {
    dictMap_t::const_iterator it;
    it = dict.find(key);
    if (it != dict.end())
        return it->second.Get(engine, value, typeId);

    // AngelScript has already initialized the value with a default value,
    // so we don't have to do anything if we don't find the element, or if
    // the element is incompatible with the requested type.

    return false;
}

bool CScriptDictionary::Get(const dictKey_t &key, asQWORD &value) const {
    return Get(key, &value, asTYPEID_UINT64);
}

// Returns the type id of the stored value
int CScriptDictionary::GetTypeId(const dictKey_t &key) const {
    dictMap_t::const_iterator it;
    it = dict.find(key);
    if (it != dict.end())
        return it->second.m_typeId;

    return -1;
}

bool CScriptDictionary::Get(const dictKey_t &key, asINT64 &value) const {
    return Get(key, &value, asTYPEID_INT64);
}

bool CScriptDictionary::Get(const dictKey_t &key, double &value) const {
    return Get(key, &value, asTYPEID_DOUBLE);
}

bool CScriptDictionary::Exists(const dictKey_t &key) const {
    dictMap_t::const_iterator it;
    it = dict.find(key);
    if (it != dict.end())
        return true;

    return false;
}

bool CScriptDictionary::IsEmpty() const {
    if (dict.size() == 0)
        return true;

    return false;
}

asUINT CScriptDictionary::GetSize() const { return asUINT(dict.size()); }

bool CScriptDictionary::Delete(const dictKey_t &key) {
    dictMap_t::iterator it;
    it = dict.find(key);
    if (it != dict.end()) {
        it->second.FreeValue(engine);
        dict.erase(it);
        iterGuard++;
        return true;
    }

    return false;
}

void CScriptDictionary::DeleteAll() {
    dictMap_t::iterator it;
    for (it = dict.begin(); it != dict.end(); it++)
        it->second.FreeValue(engine);

    dict.clear();
    iterGuard++;
}

CScriptArray *CScriptDictionary::GetKeys() const {
    // Retrieve the object type for the array<string> from the cache
    SDictionaryCache *cache = reinterpret_cast<SDictionaryCache *>(
        engine->GetUserData(DICTIONARY_CACHE));
    asITypeInfo *ti = cache->arrayType;

    // Create the array object
    CScriptArray *array = CScriptArray::Create(ti, asUINT(dict.size()));
    long current = -1;
    dictMap_t::const_iterator it;
    for (it = dict.begin(); it != dict.end(); it++) {
        current++;
        *(dictKey_t *)array->At(current) = it->first;
    }

    return array;
}

asIScriptEngine *CScriptDictionary::GetEngine() const { return engine; }

//--------------------------------------------------------------------------
// Generic wrappers

void ScriptDictionaryFactory_Generic(asIScriptGeneric *gen) {
    *(CScriptDictionary **)gen->GetAddressOfReturnLocation() =
        CScriptDictionary::Create(gen->GetEngine());
}

void ScriptDictionaryListFactory_Generic(asIScriptGeneric *gen) {
    asBYTE *buffer = (asBYTE *)gen->GetArgAddress(0);
    *(CScriptDictionary **)gen->GetAddressOfReturnLocation() =
        CScriptDictionary::Create(buffer);
}

void ScriptDictionaryAddRef_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    dict->AddRef();
}

void ScriptDictionaryRelease_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    dict->Release();
}

void ScriptDictionaryAssign_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    CScriptDictionary *other = *(CScriptDictionary **)gen->GetAddressOfArg(0);
    *dict = *other;
    *(CScriptDictionary **)gen->GetAddressOfReturnLocation() = dict;
}

void ScriptDictionarySet_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    dictKey_t *key = *(dictKey_t **)gen->GetAddressOfArg(0);
    void *ref = *(void **)gen->GetAddressOfArg(1);
    int typeId = gen->GetArgTypeId(1);
    dict->Set(*key, ref, typeId);
}

void ScriptDictionarySetInt_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    dictKey_t *key = *(dictKey_t **)gen->GetAddressOfArg(0);
    void *ref = *(void **)gen->GetAddressOfArg(1);
    dict->Set(*key, *(asINT64 *)ref);
}

void ScriptDictionarySetUInt_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    dictKey_t *key = *(dictKey_t **)gen->GetAddressOfArg(0);
    void *ref = *(void **)gen->GetAddressOfArg(1);
    dict->Set(*key, *(asQWORD *)ref);
}

void ScriptDictionarySetFlt_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    dictKey_t *key = *(dictKey_t **)gen->GetAddressOfArg(0);
    void *ref = *(void **)gen->GetAddressOfArg(1);
    dict->Set(*key, *(double *)ref);
}

void ScriptDictionaryGet_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    dictKey_t *key = *(dictKey_t **)gen->GetAddressOfArg(0);
    void *ref = *(void **)gen->GetAddressOfArg(1);
    int typeId = gen->GetArgTypeId(1);
    *(bool *)gen->GetAddressOfReturnLocation() = dict->Get(*key, ref, typeId);
}

void ScriptDictionaryGetInt_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    dictKey_t *key = *(dictKey_t **)gen->GetAddressOfArg(0);
    void *ref = *(void **)gen->GetAddressOfArg(1);
    *(bool *)gen->GetAddressOfReturnLocation() =
        dict->Get(*key, *(asINT64 *)ref);
}

void ScriptDictionaryGetUInt_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    dictKey_t *key = *(dictKey_t **)gen->GetAddressOfArg(0);
    void *ref = *(void **)gen->GetAddressOfArg(1);
    *(bool *)gen->GetAddressOfReturnLocation() =
        dict->Get(*key, *(asQWORD *)ref);
}

void ScriptDictionaryGetFlt_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    dictKey_t *key = *(dictKey_t **)gen->GetAddressOfArg(0);
    void *ref = *(void **)gen->GetAddressOfArg(1);
    *(bool *)gen->GetAddressOfReturnLocation() =
        dict->Get(*key, *(double *)ref);
}

void ScriptDictionaryExists_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    dictKey_t *key = *(dictKey_t **)gen->GetAddressOfArg(0);
    bool ret = dict->Exists(*key);
    *(bool *)gen->GetAddressOfReturnLocation() = ret;
}

void ScriptDictionaryIsEmpty_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    bool ret = dict->IsEmpty();
    *(bool *)gen->GetAddressOfReturnLocation() = ret;
}

void ScriptDictionaryGetSize_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    asUINT ret = dict->GetSize();
    *(asUINT *)gen->GetAddressOfReturnLocation() = ret;
}

void ScriptDictionaryDelete_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    dictKey_t *key = *(dictKey_t **)gen->GetAddressOfArg(0);
    *(bool *)gen->GetAddressOfReturnLocation() = dict->Delete(*key);
}

void ScriptDictionaryDeleteAll_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    dict->DeleteAll();
}

static void ScriptDictionaryGetRefCount_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *self = (CScriptDictionary *)gen->GetObject();
    *(int *)gen->GetAddressOfReturnLocation() = self->GetRefCount();
}

static void ScriptDictionarySetGCFlag_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *self = (CScriptDictionary *)gen->GetObject();
    self->SetGCFlag();
}

static void ScriptDictionaryGetGCFlag_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *self = (CScriptDictionary *)gen->GetObject();
    *(bool *)gen->GetAddressOfReturnLocation() = self->GetGCFlag();
}

static void ScriptDictionaryEnumReferences_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *self = (CScriptDictionary *)gen->GetObject();
    asIScriptEngine *engine = *(asIScriptEngine **)gen->GetAddressOfArg(0);
    self->EnumReferences(engine);
}

static void
ScriptDictionaryReleaseAllReferences_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *self = (CScriptDictionary *)gen->GetObject();
    asIScriptEngine *engine = *(asIScriptEngine **)gen->GetAddressOfArg(0);
    self->ReleaseAllReferences(engine);
}

static void CScriptDictionaryGetKeys_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *self = (CScriptDictionary *)gen->GetObject();
    *(CScriptArray **)gen->GetAddressOfReturnLocation() = self->GetKeys();
}

static void CScriptDictionary_opIndex_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *self = (CScriptDictionary *)gen->GetObject();
    dictKey_t *key = *(dictKey_t **)gen->GetAddressOfArg(0);
    *(CScriptDictValue **)gen->GetAddressOfReturnLocation() =
        self->operator[](*key);
}

static void CScriptDictionary_opIndex_const_Generic(asIScriptGeneric *gen) {
    const CScriptDictionary *self = (const CScriptDictionary *)gen->GetObject();
    dictKey_t *key = *(dictKey_t **)gen->GetAddressOfArg(0);
    *(const CScriptDictValue **)gen->GetAddressOfReturnLocation() =
        self->operator[](*key);
}

//-------------------------------------------------------------------------
// CScriptDictValue

CScriptDictValue::CScriptDictValue() {
    m_valueObj = 0;
    m_typeId = 0;
}

CScriptDictValue::CScriptDictValue(asIScriptEngine *engine, void *value,
                                   int typeId) {
    m_valueObj = 0;
    m_typeId = 0;
    Set(engine, value, typeId);
}

CScriptDictValue::~CScriptDictValue() {
    if (m_valueObj && m_typeId) {
        asIScriptContext *ctx = asGetActiveContext();
        if (ctx)
            FreeValue(ctx->GetEngine());
        else {
            // Must not hold an object when destroyed, as then the object will
            // never be freed
            assert((m_typeId & asTYPEID_MASK_OBJECT) == 0);
        }
    }
}

void CScriptDictValue::FreeValue(asIScriptEngine *engine) {
    // If it is a handle or a ref counted object, call release
    if (m_typeId & asTYPEID_MASK_OBJECT) {
        // Let the engine release the object
        engine->ReleaseScriptObject(m_valueObj,
                                    engine->GetTypeInfoById(m_typeId));
        m_valueObj = 0;
        m_typeId = 0;
    }

    // For primitives, there's nothing to do
}

void CScriptDictValue::EnumReferences(asIScriptEngine *inEngine) {
    // If we're holding a reference, we'll notify the garbage collector of it
    if (m_valueObj)
        inEngine->GCEnumCallback(m_valueObj);

    // The object type itself is also garbage collected
    if (m_typeId)
        inEngine->GCEnumCallback(inEngine->GetTypeInfoById(m_typeId));
}

void CScriptDictValue::Set(asIScriptEngine *engine, void *value, int typeId) {
    FreeValue(engine);

    m_typeId = typeId;
    if (typeId & asTYPEID_OBJHANDLE) {
        // We're receiving a reference to the handle, so we need to dereference
        // it
        m_valueObj = *(void **)value;
        engine->AddRefScriptObject(m_valueObj, engine->GetTypeInfoById(typeId));
    } else if (typeId & asTYPEID_MASK_OBJECT) {
        // Create a copy of the object
        m_valueObj = engine->CreateScriptObjectCopy(
            value, engine->GetTypeInfoById(typeId));
        if (m_valueObj == 0) {
            asIScriptContext *ctx = asGetActiveContext();
            if (ctx)
                ctx->SetException("Cannot create copy of object");
        }
    } else {
        // Copy the primitive value
        // We receive a pointer to the value.
        int size = engine->GetSizeOfPrimitiveType(typeId);
        memcpy(&m_valueInt, value, size);
    }
}

void CScriptDictValue::Set(asIScriptEngine *engine, const asQWORD &value) {
    Set(engine, const_cast<asQWORD *>(&value), asTYPEID_UINT64);
}

void CScriptDictValue::Set(asIScriptEngine *engine, CScriptDictValue &value) {
    if (value.m_typeId & asTYPEID_OBJHANDLE)
        Set(engine, (void *)&value.m_valueObj, value.m_typeId);
    else if (value.m_typeId & asTYPEID_MASK_OBJECT)
        Set(engine, (void *)value.m_valueObj, value.m_typeId);
    else
        Set(engine, (void *)&value.m_valueInt, value.m_typeId);
}

// This overloaded method is implemented so that all integer and
// unsigned integers types will be stored in the dictionary as int64
// through implicit conversions. This simplifies the management of the
// numeric types when the script retrieves the stored value using a
// different type.
void CScriptDictValue::Set(asIScriptEngine *engine, const asINT64 &value) {
    Set(engine, const_cast<asINT64 *>(&value), asTYPEID_INT64);
}

// This overloaded method is implemented so that all floating point types
// will be stored in the dictionary as double through implicit conversions.
// This simplifies the management of the numeric types when the script
// retrieves the stored value using a different type.
void CScriptDictValue::Set(asIScriptEngine *engine, const double &value) {
    Set(engine, const_cast<double *>(&value), asTYPEID_DOUBLE);
}

static inline void SetScriptExceptionRange() {
    asIScriptContext *active = asGetActiveContext();
    if (active)
        active->SetException("Safe conversion failed: value out of range");
}

bool CScriptDictValue::Get(asIScriptEngine *engine, void *value,
                           int typeId) const {
    // ---------------------
    // Helpers
    // ---------------------
    struct CastResult {
        bool ok;
        bool isFloat;
        double dval;
        asQWORD qword;
        CastResult() : ok(false), isFloat(false), dval(0.0), qword(0) {}
    };

    // object or handle -> primitive via opImplConv / opConv
    auto TryObjectToPrimitive = [this](asIScriptEngine *eng,
                                       void *storedObjOrHandle,
                                       int storedTypeId) -> CastResult {
        CastResult res;
        if (!eng || !storedObjOrHandle)
            return res;
        if ((storedTypeId & asTYPEID_MASK_OBJECT) == 0)
            return res;

        int actualTypeId = storedTypeId;
        void *objPtr = nullptr;

        // If stored as handle, dereference to actual object pointer
        if (storedTypeId & asTYPEID_OBJHANDLE) {
            actualTypeId =
                storedTypeId & ~(asTYPEID_OBJHANDLE | asTYPEID_HANDLETOCONST);
            void **maybePtr = reinterpret_cast<void **>(storedObjOrHandle);
            if (!maybePtr)
                return res;
            objPtr = *maybePtr;
            if (!objPtr)
                return res;
        } else {
            objPtr = storedObjOrHandle;
        }
        if (!objPtr)
            return res;

        asITypeInfo *ti = eng->GetTypeInfoById(actualTypeId);
        if (!ti)
            return res;

        // Find opImplConv() first, then opConv()
        asIScriptFunction *chosen = nullptr;
        asUINT methodCount = ti->GetMethodCount();
        for (asUINT i = 0; i < methodCount; ++i) {
            asIScriptFunction *mf = ti->GetMethodByIndex(i, true);
            if (!mf)
                continue;
            const char *name = mf->GetName();
            if (!name)
                continue;
            if (strcmp(name, "opImplConv") == 0 && mf->GetParamCount() == 0) {
                chosen = mf;
                break;
            }
        }
        if (!chosen) {
            for (asUINT i = 0; i < methodCount; ++i) {
                asIScriptFunction *mf = ti->GetMethodByIndex(i, true);
                if (!mf)
                    continue;
                const char *name = mf->GetName();
                if (!name)
                    continue;
                if (strcmp(name, "opConv") == 0 && mf->GetParamCount() == 0) {
                    chosen = mf;
                    break;
                }
            }
        }
        if (!chosen)
            return res;

        asIScriptContext *ctx = eng->CreateContext();
        if (!ctx)
            return res;
        if (ctx->Prepare(chosen) < 0) {
            ctx->Release();
            return res;
        }
        ctx->SetObject(objPtr);
        int exec = ctx->Execute();
        if (exec != asEXECUTION_FINISHED) {
            asIScriptContext *active = asGetActiveContext();
            if (active) {
                const char *ex = ctx->GetExceptionString();
                if (ex)
                    active->SetException(ex);
            }
            ctx->Release();
            return res;
        }

        int retTypeId = chosen->GetReturnTypeId(nullptr);
        if (retTypeId & asTYPEID_MASK_OBJECT) {
            ctx->Release();
            return res;
        }

        if (retTypeId == asTYPEID_DOUBLE) {
            res.dval = ctx->GetReturnDouble();
            res.isFloat = true;
            res.ok = true;
        } else if (retTypeId == asTYPEID_FLOAT) {
            res.dval = double(ctx->GetReturnFloat());
            res.isFloat = true;
            res.ok = true;
        } else {
            res.qword = ctx->GetReturnQWord();
            res.isFloat = false;
            res.ok = true;
        }
        ctx->Release();
        return res;
    };

    // Read stored primitive into normalized container
    struct StoredPrim {
        bool isFloat;
        double dval;
        asINT64 sval;
        asQWORD uval;
    };
    auto ReadStoredPrimitive = [this]() -> StoredPrim {
        StoredPrim out;
        out.isFloat = false;
        out.dval = 0.0;
        out.sval = 0;
        out.uval = 0;
        if (m_typeId == asTYPEID_DOUBLE) {
            out.isFloat = true;
            out.dval = m_valueFlt;
            return out;
        }
        if (m_typeId == asTYPEID_FLOAT) {
            out.isFloat = true;
            out.dval = double(m_valueFlt);
            return out;
        }
        if (m_typeId == asTYPEID_BOOL) {
            char b;
            memcpy(&b, &m_valueInt, sizeof(char));
            out.sval = b ? 1 : 0;
            out.uval = out.sval;
            return out;
        }
        if (m_typeId == asTYPEID_INT64 || m_typeId == asTYPEID_UINT64) {
            out.sval = m_valueInt;
            out.uval = (asQWORD)m_valueInt;
            return out;
        }
        if (m_typeId > asTYPEID_DOUBLE &&
            (m_typeId & asTYPEID_MASK_OBJECT) == 0) {
            int tmp;
            memcpy(&tmp, &m_valueInt, sizeof(int));
            out.sval = (asINT64)tmp;
            out.uval = (asQWORD)(uint32_t)tmp;
            return out;
        }
        switch (m_typeId) {
        case asTYPEID_INT32: {
            int tmp;
            memcpy(&tmp, &m_valueInt, sizeof(int));
            out.sval = tmp;
            out.uval = (asQWORD)(uint32_t)tmp;
            break;
        }
        case asTYPEID_UINT32: {
            unsigned tmp;
            memcpy(&tmp, &m_valueInt, sizeof(unsigned));
            out.sval = tmp;
            out.uval = (asQWORD)tmp;
            break;
        }
        case asTYPEID_INT16: {
            int16_t tmp;
            memcpy(&tmp, &m_valueInt, sizeof(int16_t));
            out.sval = (asINT64)tmp;
            out.uval = (asQWORD)(uint16_t)tmp;
            break;
        }
        case asTYPEID_UINT16: {
            uint16_t tmp;
            memcpy(&tmp, &m_valueInt, sizeof(uint16_t));
            out.sval = (asINT64)tmp;
            out.uval = (asQWORD)tmp;
            break;
        }
        case asTYPEID_INT8: {
            int8_t tmp;
            memcpy(&tmp, &m_valueInt, sizeof(int8_t));
            out.sval = (asINT64)tmp;
            out.uval = (asQWORD)(uint8_t)tmp;
            break;
        }
        case asTYPEID_UINT8: {
            uint8_t tmp;
            memcpy(&tmp, &m_valueInt, sizeof(uint8_t));
            out.sval = (asINT64)tmp;
            out.uval = (asQWORD)tmp;
            break;
        }
        default: {
            out.sval = m_valueInt;
            out.uval = (asQWORD)m_valueInt;
            break;
        }
        }
        return out;
    };

    // ---------------------
    // Main logic
    // ---------------------
    if (typeId & asTYPEID_OBJHANDLE) {
        if ((m_typeId & asTYPEID_MASK_OBJECT)) {
            if ((m_typeId & asTYPEID_HANDLETOCONST) &&
                !(typeId & asTYPEID_HANDLETOCONST))
                return false;
            engine->RefCastObject(m_valueObj, engine->GetTypeInfoById(m_typeId),
                                  engine->GetTypeInfoById(typeId),
                                  reinterpret_cast<void **>(value));
            return true;
        }
    } else if (typeId & asTYPEID_MASK_OBJECT) {
        // Requested an object type.
        bool isCompatible = false;
        if ((m_typeId & ~(asTYPEID_OBJHANDLE | asTYPEID_HANDLETOCONST)) ==
                typeId &&
            m_valueObj != 0)
            isCompatible = true;
        if (isCompatible) {
            engine->AssignScriptObject(value, m_valueObj,
                                       engine->GetTypeInfoById(typeId));
            return true;
        }

        // If stored is primitive, try primitive -> object (factory -> opAssign)
        if ((m_typeId & asTYPEID_MASK_OBJECT) == 0) {
            asITypeInfo *tgtTi = engine->GetTypeInfoById(typeId);
            if (!tgtTi)
                return false;
            StoredPrim stored = ReadStoredPrimitive();

            // Strategy 1: try factory functions (GetFactoryCount /
            // GetFactoryByIndex)
            asUINT facCount = tgtTi->GetFactoryCount();
            for (asUINT fi = 0; fi < facCount; ++fi) {
                asIScriptFunction *fac = tgtTi->GetFactoryByIndex(fi);
                if (!fac)
                    continue;
                if (fac->GetParamCount() != 1)
                    continue;

                // Use GetParam to query param type (GetParam(index, &typeId,
                // &flags, &name))
                int paramTypeId = 0;
                asDWORD paramFlags = 0;
                fac->GetParam(0, &paramTypeId, &paramFlags, nullptr, nullptr);

                // Skip object-typed params for this path
                if (paramTypeId & asTYPEID_MASK_OBJECT)
                    continue;

                // Basic compatibility: numeric/bool -> numeric param allowed
                auto isIntLike = [&](int tid) -> bool {
                    if (tid == asTYPEID_INT8 || tid == asTYPEID_UINT8 ||
                        tid == asTYPEID_INT16 || tid == asTYPEID_UINT16 ||
                        tid == asTYPEID_INT32 || tid == asTYPEID_UINT32 ||
                        tid == asTYPEID_INT64 || tid == asTYPEID_UINT64)
                        return true;
                    if (tid > asTYPEID_DOUBLE &&
                        ((tid & asTYPEID_MASK_OBJECT) == 0))
                        return true; // enum
                    return false;
                };
                auto isFloatLike = [&](int tid) -> bool {
                    return tid == asTYPEID_FLOAT || tid == asTYPEID_DOUBLE;
                };

                bool compatible = false;
                if (paramTypeId == m_typeId)
                    compatible = true;
                else if (isIntLike(m_typeId) &&
                         (isIntLike(paramTypeId) || isFloatLike(paramTypeId)))
                    compatible = true;
                else if (isFloatLike(m_typeId) && isFloatLike(paramTypeId))
                    compatible = true;
                else if (m_typeId == asTYPEID_BOOL && isIntLike(paramTypeId))
                    compatible = true;
                if (!compatible)
                    continue;

                // Prepare context and set arg
                asIScriptContext *ctx = engine->CreateContext();
                if (!ctx)
                    continue;
                if (ctx->Prepare(fac) < 0) {
                    ctx->Release();
                    continue;
                }

                // Set argument by type
                if (paramTypeId == asTYPEID_DOUBLE)
                    ctx->SetArgDouble(0, stored.isFloat ? stored.dval
                                                        : double(stored.sval));
                else if (paramTypeId == asTYPEID_FLOAT)
                    ctx->SetArgFloat(0, stored.isFloat ? (float)stored.dval
                                                       : (float)stored.sval);
                else if (paramTypeId == asTYPEID_INT64 ||
                         paramTypeId == asTYPEID_UINT64)
                    ctx->SetArgQWord(0, (asQWORD)stored.sval);
                else
                    ctx->SetArgDWord(0,
                                     (asDWORD)stored.sval); // 32-bit or smaller

                int exec = ctx->Execute();
                if (exec == asEXECUTION_FINISHED) {
                    void *retObj = ctx->GetReturnObject();
                    if (retObj) {
                        engine->AssignScriptObject(value, retObj, tgtTi);
                        ctx->Release();
                        return true;
                    }
                } else {
                    asIScriptContext *active = asGetActiveContext();
                    if (active) {
                        const char *ex = ctx->GetExceptionString();
                        if (ex)
                            active->SetException(ex);
                    }
                }
                ctx->Release();
            } // end factories

            // Strategy 2: try opAssign on target type
            asIScriptFunction *assignMethod = nullptr;
            asUINT methodCount = tgtTi->GetMethodCount();
            for (asUINT mi = 0; mi < methodCount; ++mi) {
                asIScriptFunction *mf = tgtTi->GetMethodByIndex(mi, true);
                if (!mf)
                    continue;
                const char *name = mf->GetName();
                if (!name)
                    continue;
                if (strcmp(name, "opAssign") != 0)
                    continue;
                if (mf->GetParamCount() != 1)
                    continue;
                int ptype = 0;
                asDWORD pflags = 0;
                mf->GetParam(0, &ptype, &pflags, nullptr, nullptr);
                if (ptype & asTYPEID_MASK_OBJECT)
                    continue;

                // same compatibility test as above
                bool compatible = false;
                if (ptype == m_typeId)
                    compatible = true;
                else {
                    auto isI = [&](int tid) -> bool {
                        if (tid == asTYPEID_INT8 || tid == asTYPEID_UINT8 ||
                            tid == asTYPEID_INT16 || tid == asTYPEID_UINT16 ||
                            tid == asTYPEID_INT32 || tid == asTYPEID_UINT32 ||
                            tid == asTYPEID_INT64 || tid == asTYPEID_UINT64)
                            return true;
                        if (tid > asTYPEID_DOUBLE &&
                            ((tid & asTYPEID_MASK_OBJECT) == 0))
                            return true;
                        return false;
                    };
                    auto isF = [&](int tid) -> bool {
                        return tid == asTYPEID_FLOAT || tid == asTYPEID_DOUBLE;
                    };
                    if (isI(m_typeId) && (isI(ptype) || isF(ptype)))
                        compatible = true;
                    if (isF(m_typeId) && isF(ptype))
                        compatible = true;
                    if (m_typeId == asTYPEID_BOOL && isI(ptype))
                        compatible = true;
                }
                if (compatible) {
                    assignMethod = mf;
                    break;
                }
            }

            if (assignMethod) {
                // Create default object
                void *obj = engine->CreateScriptObject(tgtTi);
                if (!obj)
                    return false;
                asIScriptContext *ctx = engine->CreateContext();
                if (!ctx) {
                    engine->ReleaseScriptObject(obj, tgtTi);
                    return false;
                }
                if (ctx->Prepare(assignMethod) < 0) {
                    ctx->Release();
                    engine->ReleaseScriptObject(obj, tgtTi);
                    return false;
                }
                ctx->SetObject(obj);
                int ptype = 0;
                asDWORD pflags = 0;
                assignMethod->GetParam(0, &ptype, &pflags, nullptr, nullptr);
                if (ptype == asTYPEID_DOUBLE)
                    ctx->SetArgDouble(0, stored.isFloat ? stored.dval
                                                        : double(stored.sval));
                else if (ptype == asTYPEID_FLOAT)
                    ctx->SetArgFloat(0, stored.isFloat ? (float)stored.dval
                                                       : (float)stored.sval);
                else if (ptype == asTYPEID_INT64 || ptype == asTYPEID_UINT64)
                    ctx->SetArgQWord(0, (asQWORD)stored.sval);
                else
                    ctx->SetArgDWord(0, (asDWORD)stored.sval);

                int exec = ctx->Execute();
                if (exec == asEXECUTION_FINISHED) {
                    engine->AssignScriptObject(value, obj, tgtTi);
                    ctx->Release();
                    engine->ReleaseScriptObject(obj,
                                                tgtTi); // release temporary
                    return true;
                } else {
                    asIScriptContext *active = asGetActiveContext();
                    if (active) {
                        const char *ex = ctx->GetExceptionString();
                        if (ex)
                            active->SetException(ex);
                    }
                }
                ctx->Release();
                engine->ReleaseScriptObject(obj, tgtTi);
            }

            return false; // nothing matched
        } // end primitive->object attempt
    } else {
        // Requested primitive type
        // 1) exact match
        if (m_typeId == typeId) {
            int size = engine->GetSizeOfPrimitiveType(typeId);
            memcpy(value, &m_valueInt, size);
            return true;
        }

        // 2) stored is object/handle: try object->primitive conversion
        if (m_typeId & asTYPEID_MASK_OBJECT) {
            CastResult cr = TryObjectToPrimitive(engine, m_valueObj, m_typeId);
            if (!cr.ok)
                return false;

            // Convert CastResult -> requested primitive (truncate float->int)
            if (typeId == asTYPEID_DOUBLE) {
                *(double *)value =
                    cr.isFloat ? cr.dval : double((asINT64)cr.qword);
                return true;
            }
            if (typeId == asTYPEID_FLOAT) {
                *(float *)value =
                    cr.isFloat ? (float)cr.dval : (float)(asINT64)cr.qword;
                return true;
            }
            if (typeId == asTYPEID_INT64 || typeId == asTYPEID_UINT64) {
                *(asINT64 *)value =
                    cr.isFloat ? (asINT64)cr.dval : (asINT64)cr.qword;
                return true;
            }
            if (typeId == asTYPEID_BOOL) {
                if (cr.isFloat)
                    *(bool *)value = (cr.dval != 0.0);
                else
                    *(bool *)value = ((asINT64)cr.qword != 0);
                return true;
            }

            // smaller ints: range-check (float->int truncates)
            asINT64 src = cr.isFloat ? (asINT64)cr.dval : (asINT64)cr.qword;
            if (typeId == asTYPEID_INT32) {
                if (src < INT32_MIN || src > INT32_MAX) {
                    SetScriptExceptionRange();
                    return false;
                }
                *(int *)value = (int)src;
                return true;
            }
            if (typeId == asTYPEID_UINT32) {
                if (src < 0 || (asQWORD)src > 0xFFFFFFFFULL) {
                    SetScriptExceptionRange();
                    return false;
                }
                *(unsigned *)value = (unsigned)src;
                return true;
            }
            if (typeId == asTYPEID_INT16) {
                if (src < INT16_MIN || src > INT16_MAX) {
                    SetScriptExceptionRange();
                    return false;
                }
                *(short *)value = (short)src;
                return true;
            }
            if (typeId == asTYPEID_UINT16) {
                if (src < 0 || src > 0xFFFF) {
                    SetScriptExceptionRange();
                    return false;
                }
                *(uint16_t *)value = (uint16_t)src;
                return true;
            }
            if (typeId == asTYPEID_INT8) {
                if (src < INT8_MIN || src > INT8_MAX) {
                    SetScriptExceptionRange();
                    return false;
                }
                *(int8_t *)value = (int8_t)src;
                return true;
            }
            if (typeId == asTYPEID_UINT8) {
                if (src < 0 || src > 0xFF) {
                    SetScriptExceptionRange();
                    return false;
                }
                *(uint8_t *)value = (uint8_t)src;
                return true;
            }
            if (typeId > asTYPEID_DOUBLE &&
                (typeId & asTYPEID_MASK_OBJECT) == 0) {
                if (src < INT32_MIN || src > INT32_MAX) {
                    SetScriptExceptionRange();
                    return false;
                }
                *(int *)value = (int)src;
                return true;
            }
            return false;
        }

        // 3) both are primitives: convert stored primitive -> requested
        // primitive
        StoredPrim stored = ReadStoredPrimitive();

        if (typeId == asTYPEID_DOUBLE) {
            if (stored.isFloat)
                *(double *)value = stored.dval;
            else
                *(double *)value = double(stored.sval);
            return true;
        }
        if (typeId == asTYPEID_FLOAT) {
            if (stored.isFloat)
                *(float *)value = (float)stored.dval;
            else
                *(float *)value = (float)stored.sval;
            return true;
        }
        if (typeId == asTYPEID_INT64 || typeId == asTYPEID_UINT64) {
            if (stored.isFloat)
                *(asINT64 *)value = (asINT64)stored.dval;
            else
                *(asINT64 *)value = stored.sval;
            return true;
        }
        if (typeId == asTYPEID_BOOL) {
            if (stored.isFloat)
                *(bool *)value = (stored.dval != 0.0);
            else
                *(bool *)value = (stored.sval != 0);
            return true;
        }

        // smaller ints with range checks (float->int truncates)
        if (typeId == asTYPEID_INT32) {
            asINT64 v = stored.isFloat ? (asINT64)stored.dval : stored.sval;
            if (v < INT32_MIN || v > INT32_MAX) {
                SetScriptExceptionRange();
                return false;
            }
            *(int *)value = (int)v;
            return true;
        }
        if (typeId == asTYPEID_UINT32) {
            if (stored.isFloat) {
                asINT64 v = (asINT64)stored.dval;
                if (v < 0 || (asQWORD)v > 0xFFFFFFFFULL) {
                    SetScriptExceptionRange();
                    return false;
                }
                *(unsigned *)value = (unsigned)v;
                return true;
            } else {
                if (stored.uval > 0xFFFFFFFFULL) {
                    SetScriptExceptionRange();
                    return false;
                }
                *(unsigned *)value = (unsigned)stored.uval;
                return true;
            }
        }
        if (typeId == asTYPEID_INT16) {
            asINT64 v = stored.isFloat ? (asINT64)stored.dval : stored.sval;
            if (v < INT16_MIN || v > INT16_MAX) {
                SetScriptExceptionRange();
                return false;
            }
            *(short *)value = (short)v;
            return true;
        }
        if (typeId == asTYPEID_UINT16) {
            asINT64 v = stored.isFloat ? (asINT64)stored.dval : stored.sval;
            if (v < 0 || v > 0xFFFF) {
                SetScriptExceptionRange();
                return false;
            }
            *(uint16_t *)value = (uint16_t)v;
            return true;
        }
        if (typeId == asTYPEID_INT8) {
            asINT64 v = stored.isFloat ? (asINT64)stored.dval : stored.sval;
            if (v < INT8_MIN || v > INT8_MAX) {
                SetScriptExceptionRange();
                return false;
            }
            *(int8_t *)value = (int8_t)v;
            return true;
        }
        if (typeId == asTYPEID_UINT8) {
            asINT64 v = stored.isFloat ? (asINT64)stored.dval : stored.sval;
            if (v < 0 || v > 0xFF) {
                SetScriptExceptionRange();
                return false;
            }
            *(uint8_t *)value = (uint8_t)v;
            return true;
        }
        if (typeId > asTYPEID_DOUBLE && (typeId & asTYPEID_MASK_OBJECT) == 0) {
            asINT64 v = stored.isFloat ? (asINT64)stored.dval : stored.sval;
            if (v < INT32_MIN || v > INT32_MAX) {
                SetScriptExceptionRange();
                return false;
            }
            *(int *)value = (int)v;
            return true;
        }
    }

    // Could not retrieve as requested type
    return false;
}

bool CScriptDictValue::Get(asIScriptEngine *engine, asQWORD &value) const {
    return Get(engine, &value, asTYPEID_UINT64);
}

const void *CScriptDictValue::GetAddressOfValue() const {
    if ((m_typeId & asTYPEID_MASK_OBJECT) && !(m_typeId & asTYPEID_OBJHANDLE)) {
        // Return the address to the object directly
        return m_valueObj;
    }

    // Return the address of the primitive or the pointer to the object
    return reinterpret_cast<const void *>(&m_valueObj);
}

bool CScriptDictValue::Get(asIScriptEngine *engine, asINT64 &value) const {
    return Get(engine, &value, asTYPEID_INT64);
}

bool CScriptDictValue::Get(asIScriptEngine *engine, double &value) const {
    return Get(engine, &value, asTYPEID_DOUBLE);
}

int CScriptDictValue::GetTypeId() const { return m_typeId; }

static void CScriptDictValue_Construct(void *mem) {
    new (mem) CScriptDictValue();
}

static void CScriptDictValue_Destruct(CScriptDictValue *obj) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();
        obj->FreeValue(engine);
    }
    obj->~CScriptDictValue();
}

static CScriptDictValue &CScriptDictValue_opAssign(void *ref, int typeId,
                                                   CScriptDictValue *obj) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();
        obj->Set(engine, ref, typeId);
    }
    return *obj;
}

static CScriptDictValue &
CScriptDictValue_opAssign(const CScriptDictValue &other,
                          CScriptDictValue *obj) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();
        obj->Set(engine, const_cast<CScriptDictValue &>(other));
    }

    return *obj;
}

static CScriptDictValue &CScriptDictValue_opAssign(double val,
                                                   CScriptDictValue *obj) {
    return CScriptDictValue_opAssign(&val, asTYPEID_DOUBLE, obj);
}

static CScriptDictValue &CScriptDictValue_opAssign(asINT64 val,
                                                   CScriptDictValue *obj) {
    return CScriptDictValue_opAssign(&val, asTYPEID_INT64, obj);
}

static CScriptDictValue &CScriptDictValue_opAssign(asQWORD val,
                                                   CScriptDictValue *obj) {
    return CScriptDictValue_opAssign(&val, asTYPEID_UINT64, obj);
}

static void CScriptDictValue_opCast(void *ref, int typeId,
                                    CScriptDictValue *obj) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();
        obj->Get(engine, ref, typeId);
    }
}

static asINT64 CScriptDictValue_opConvInt(CScriptDictValue *obj) {
    asINT64 value = 0;
    CScriptDictValue_opCast(&value, asTYPEID_INT64, obj);
    return value;
}

static asQWORD CScriptDictValue_opConvUInt(CScriptDictValue *obj) {
    asQWORD value = 0;
    CScriptDictValue_opCast(&value, asTYPEID_UINT64, obj);
    return value;
}

static double CScriptDictValue_opConvDouble(CScriptDictValue *obj) {
    double value = 0;
    CScriptDictValue_opCast(&value, asTYPEID_DOUBLE, obj);
    return value;
}

//-------------------------------------------------------------------
// generic wrapper for CScriptDictValue

static void CScriptDictValue_opConvDouble_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    double value;
    self->Get(gen->GetEngine(), value);
    *(double *)gen->GetAddressOfReturnLocation() = value;
}

static void CScriptDictValue_opConvInt_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    asINT64 value;
    self->Get(gen->GetEngine(), value);
    *(asINT64 *)gen->GetAddressOfReturnLocation() = value;
}

static void CScriptDictValue_opConvUInt_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    asQWORD value;
    self->Get(gen->GetEngine(), value);
    *(asQWORD *)gen->GetAddressOfReturnLocation() = value;
}

static void CScriptDictValue_opCast_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    self->Get(gen->GetEngine(), gen->GetArgAddress(0), gen->GetArgTypeId(0));
}

static void CScriptDictValue_opAssign_int64_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    *(CScriptDictValue **)gen->GetAddressOfReturnLocation() =
        &CScriptDictValue_opAssign((asINT64)gen->GetArgQWord(0), self);
}

static void CScriptDictValue_opAssign_uint64_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    *(CScriptDictValue **)gen->GetAddressOfReturnLocation() =
        &CScriptDictValue_opAssign(gen->GetArgQWord(0), self);
}

static void CScriptDictValue_opAssign_double_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    *(CScriptDictValue **)gen->GetAddressOfReturnLocation() =
        &CScriptDictValue_opAssign(gen->GetArgDouble(0), self);
}

static void CScriptDictValue_opAssign_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    *(CScriptDictValue **)gen->GetAddressOfReturnLocation() =
        &CScriptDictValue_opAssign(gen->GetArgAddress(0), gen->GetArgTypeId(0),
                                   self);
}

static void CScriptDictValue_opCopyAssign_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    *(CScriptDictValue **)gen->GetAddressOfReturnLocation() =
        &CScriptDictValue_opAssign(
            *reinterpret_cast<CScriptDictValue *>(gen->GetArgAddress(0)), self);
}

static void CScriptDictValue_Construct_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    CScriptDictValue_Construct(self);
}

static void CScriptDictValue_Destruct_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    CScriptDictValue_Destruct(self);
}

static void CScriptDictValue_EnumReferences_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    self->EnumReferences(gen->GetEngine());
}

static void CScriptDictValue_FreeValue_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    self->FreeValue(gen->GetEngine());
}

//----------------------------------------------------------------------------
// Foreach support
CScriptDictionary::CScriptDictIter::CScriptDictIter(
    const CScriptDictionary *dict)
    : iter(dict->begin()), refCount(1), iterGuard(dict->iterGuard) {}
CScriptDictionary::CScriptDictIter::~CScriptDictIter() {}

void CScriptDictionary::CScriptDictIter::AddRef() const {
    asAtomicInc(refCount);
}

void CScriptDictionary::CScriptDictIter::Release() const {
    if (asAtomicDec(refCount) == 0) {
        this->~CScriptDictIter();
        asFreeMem(const_cast<CScriptDictIter *>(this));
    }
}

CScriptDictionary::CScriptDictIter *CScriptDictionary::opForBegin() const {
    // Use the custom memory routine from AngelScript to allow application to
    // better control how much memory is used
    CScriptDictionary::CScriptDictIter *iter =
        (CScriptDictionary::CScriptDictIter *)asAllocMem(
            sizeof(CScriptDictionary::CScriptDictIter));
    new (iter) CScriptDictionary::CScriptDictIter(this);
    return iter;
}

bool CScriptDictionary::opForEnd(
    const CScriptDictionary::CScriptDictIter &iter) const {
    if (iter.iterGuard != iterGuard)
        return true;

    if (iter.iter == end())
        return true;

    return false;
}

CScriptDictionary::CScriptDictIter *
CScriptDictionary::opForNext(CScriptDictionary::CScriptDictIter &iter) const {
    if (iter.iterGuard != iterGuard)
        iter.iter = end();
    else
        ++iter.iter;
    return &iter;
}

const CScriptDictValue &CScriptDictionary::opForValue0(
    const CScriptDictionary::CScriptDictIter &iter) const {
    return iter.iter.m_it->second;
}

const dictKey_t &CScriptDictionary::opForValue1(
    const CScriptDictionary::CScriptDictIter &iter) const {
    return iter.iter.m_it->first;
}

void ScriptDictIterAddRef_Generic(asIScriptGeneric *gen) {
    CScriptDictionary::CScriptDictIter *iter =
        (CScriptDictionary::CScriptDictIter *)gen->GetObject();
    iter->AddRef();
}

void ScriptDictIterRelease_Generic(asIScriptGeneric *gen) {
    CScriptDictionary::CScriptDictIter *iter =
        (CScriptDictionary::CScriptDictIter *)gen->GetObject();
    iter->Release();
}

void ScriptDictionary_opForBegin_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    *(CScriptDictionary::CScriptDictIter **)gen->GetAddressOfReturnLocation() =
        dict->opForBegin();
}

void ScriptDictionary_opForEnd_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    CScriptDictionary::CScriptDictIter *iter =
        *(CScriptDictionary::CScriptDictIter **)gen->GetAddressOfArg(0);
    *(bool *)gen->GetAddressOfReturnLocation() = dict->opForEnd(*iter);
}

void ScriptDictionary_opForNext_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    CScriptDictionary::CScriptDictIter *iter =
        *(CScriptDictionary::CScriptDictIter **)gen->GetAddressOfArg(0);
    *(CScriptDictionary::CScriptDictIter **)gen->GetAddressOfReturnLocation() =
        dict->opForNext(*iter);
}

void ScriptDictionary_opForValue0_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    CScriptDictionary::CScriptDictIter *iter =
        *(CScriptDictionary::CScriptDictIter **)gen->GetAddressOfArg(0);
    *reinterpret_cast<const CScriptDictValue **>(
        gen->GetAddressOfReturnLocation()) = &dict->opForValue0(*iter);
}

void ScriptDictionary_opForValue1_Generic(asIScriptGeneric *gen) {
    CScriptDictionary *dict = (CScriptDictionary *)gen->GetObject();
    CScriptDictionary::CScriptDictIter *iter =
        *(CScriptDictionary::CScriptDictIter **)gen->GetAddressOfArg(0);
    *reinterpret_cast<const dictKey_t **>(gen->GetAddressOfReturnLocation()) =
        &dict->opForValue1(*iter);
}

//------------------------------------------------------------------
// Iterator implementation

CScriptDictionary::CIterator CScriptDictionary::begin() const {
    return CIterator(*this, dict.begin());
}

CScriptDictionary::CIterator CScriptDictionary::end() const {
    return CIterator(*this, dict.end());
}

CScriptDictionary::CIterator
CScriptDictionary::find(const dictKey_t &key) const {
    return CIterator(*this, dict.find(key));
}

CScriptDictionary::CIterator::CIterator(const CScriptDictionary &dict,
                                        dictMap_t::const_iterator it)
    : m_it(it), m_dict(dict) {}

void CScriptDictionary::CIterator::operator++() { ++m_it; }

void CScriptDictionary::CIterator::operator++(int) {
    ++m_it;

    // Normally the post increment would return a copy of the object with the
    // original state, but it is rarely used so we skip this extra copy to avoid
    // unnecessary overhead
}

CScriptDictionary::CIterator &CScriptDictionary::CIterator::operator*() {
    return *this;
}

bool CScriptDictionary::CIterator::operator==(const CIterator &other) const {
    return m_it == other.m_it;
}

bool CScriptDictionary::CIterator::operator!=(const CIterator &other) const {
    return m_it != other.m_it;
}

const dictKey_t &CScriptDictionary::CIterator::GetKey() const {
    return m_it->first;
}

int CScriptDictionary::CIterator::GetTypeId() const {
    return m_it->second.m_typeId;
}

bool CScriptDictionary::CIterator::GetValue(asINT64 &value) const {
    return m_it->second.Get(m_dict.engine, &value, asTYPEID_INT64);
}

bool CScriptDictionary::CIterator::GetValue(double &value) const {
    return m_it->second.Get(m_dict.engine, &value, asTYPEID_DOUBLE);
}

bool CScriptDictionary::CIterator::GetValue(void *value, int typeId) const {
    return m_it->second.Get(m_dict.engine, value, typeId);
}

const void *CScriptDictionary::CIterator::GetAddressOfValue() const {
    return m_it->second.GetAddressOfValue();
}

//--------------------------------------------------------------------------
// Register the type

void RegisterScriptDictionary(asIScriptEngine *engine) {
    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY"))
        RegisterScriptDictionary_Generic(engine);
    else
        RegisterScriptDictionary_Native(engine);
}

void RegisterScriptDictionary_Native(asIScriptEngine *engine) {
    int r;

    // The array<string> type must be available
    assert(engine->GetTypeInfoByDecl("array<string>"));

#if AS_CAN_USE_CPP11
    // With C++11 it is possible to use asGetTypeTraits to automatically
    // determine the correct flags that represents the C++ class
    r = engine->RegisterObjectType("dictionaryValue", sizeof(CScriptDictValue),
                                   asOBJ_VALUE | asOBJ_ASHANDLE | asOBJ_GC |
                                       asGetTypeTraits<CScriptDictValue>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
#else
    r = engine->RegisterObjectType("dictionaryValue", sizeof(CScriptDictValue),
                                   asOBJ_VALUE | asOBJ_ASHANDLE | asOBJ_GC |
                                       asOBJ_APP_CLASS_CD);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
#endif
    r = engine->RegisterObjectBehaviour(
        "dictionaryValue", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION(CScriptDictValue_Construct), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionaryValue", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTION(CScriptDictValue_Destruct), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionaryValue", asBEHAVE_ENUMREFS, "void f(int&in)",
        asMETHOD(CScriptDictValue, EnumReferences), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionaryValue", asBEHAVE_RELEASEREFS, "void f(int&in)",
        asMETHOD(CScriptDictValue, FreeValue), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue",
        "dictionaryValue &opAssign(const dictionaryValue &in)",
        asFUNCTIONPR(CScriptDictValue_opAssign,
                     (const CScriptDictValue &, CScriptDictValue *),
                     CScriptDictValue &),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "dictionaryValue &opHndlAssign(const ?&in)",
        asFUNCTIONPR(CScriptDictValue_opAssign,
                     (void *, int, CScriptDictValue *), CScriptDictValue &),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue",
        "dictionaryValue &opHndlAssign(const dictionaryValue &in)",
        asFUNCTIONPR(CScriptDictValue_opAssign,
                     (const CScriptDictValue &, CScriptDictValue *),
                     CScriptDictValue &),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "dictionaryValue &opAssign(const ?&in)",
        asFUNCTIONPR(CScriptDictValue_opAssign,
                     (void *, int, CScriptDictValue *), CScriptDictValue &),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "dictionaryValue &opAssign(double)",
        asFUNCTIONPR(CScriptDictValue_opAssign, (double, CScriptDictValue *),
                     CScriptDictValue &),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "dictionaryValue &opAssign(int64)",
        asFUNCTIONPR(CScriptDictValue_opAssign, (asINT64, CScriptDictValue *),
                     CScriptDictValue &),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "dictionaryValue &opAssign(uint64)",
        asFUNCTIONPR(CScriptDictValue_opAssign, (asQWORD, CScriptDictValue *),
                     CScriptDictValue &),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "void opCast(?&out)",
        asFUNCTIONPR(CScriptDictValue_opCast, (void *, int, CScriptDictValue *),
                     void),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "void opConv(?&out)",
        asFUNCTIONPR(CScriptDictValue_opCast, (void *, int, CScriptDictValue *),
                     void),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "int64 opConv()",
        asFUNCTIONPR(CScriptDictValue_opConvInt, (CScriptDictValue *), asINT64),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("dictionaryValue", "uint64 opConv()",
                                     asFUNCTIONPR(CScriptDictValue_opConvUInt,
                                                  (CScriptDictValue *),
                                                  asQWORD),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("dictionaryValue", "double opConv()",
                                     asFUNCTIONPR(CScriptDictValue_opConvDouble,
                                                  (CScriptDictValue *), double),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectType("dictionary", sizeof(CScriptDictionary),
                                   asOBJ_REF | asOBJ_GC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    // Use the generic interface to construct the object since we need the
    // engine pointer, we could also have retrieved the engine pointer from the
    // active context
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_FACTORY, "dictionary@ f()",
        asFUNCTION(ScriptDictionaryFactory_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_LIST_FACTORY,
        "dictionary @f(int &in) {repeat {string, ?}}",
        asFUNCTION(ScriptDictionaryListFactory_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_ADDREF, "void f()",
        asMETHOD(CScriptDictionary, AddRef), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_RELEASE, "void f()",
        asMETHOD(CScriptDictionary, Release), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "dictionary &opAssign(const dictionary &in)",
        asMETHODPR(CScriptDictionary, operator=, (const CScriptDictionary &),
                   CScriptDictionary &),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "void set(const string &in, const ?&in)",
        asMETHODPR(CScriptDictionary, Set, (const dictKey_t &, void *, int),
                   void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "bool get(const string &in, ?&out) const",
        asMETHODPR(CScriptDictionary, Get,
                   (const dictKey_t &, void *, int) const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "void set(const string &in, const int64&in)",
        asMETHODPR(CScriptDictionary, Set, (const dictKey_t &, const asINT64 &),
                   void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "void set(const string &in, const uint64&in)",
        asMETHODPR(CScriptDictionary, Set, (const dictKey_t &, const asQWORD &),
                   void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "bool get(const string &in, int64&out) const",
        asMETHODPR(CScriptDictionary, Get, (const dictKey_t &, asINT64 &) const,
                   bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "bool get(const string &in, uint64&out) const",
        asMETHODPR(CScriptDictionary, Get, (const dictKey_t &, asQWORD &) const,
                   bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "void set(const string &in, const double&in)",
        asMETHODPR(CScriptDictionary, Set, (const dictKey_t &, const double &),
                   void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "bool get(const string &in, double&out) const",
        asMETHODPR(CScriptDictionary, Get, (const dictKey_t &, double &) const,
                   bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "bool exists(const string &in) const",
        asMETHOD(CScriptDictionary, Exists), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("dictionary", "bool isEmpty() const",
                                     asMETHOD(CScriptDictionary, IsEmpty),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("dictionary", "uint getSize() const",
                                     asMETHOD(CScriptDictionary, GetSize),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "bool delete(const string &in)",
        asMETHOD(CScriptDictionary, Delete), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("dictionary", "void deleteAll()",
                                     asMETHOD(CScriptDictionary, DeleteAll),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "array<string> @getKeys() const",
        asMETHOD(CScriptDictionary, GetKeys), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "dictionaryValue &opIndex(const string &in)",
        asMETHODPR(CScriptDictionary, operator[], (const dictKey_t &),
                   CScriptDictValue *),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "const dictionaryValue &opIndex(const string &in) const",
        asMETHODPR(CScriptDictionary, operator[], (const dictKey_t &) const,
                   const CScriptDictValue *),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Register GC behaviours
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_GETREFCOUNT, "int f()",
        asMETHOD(CScriptDictionary, GetRefCount), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_SETGCFLAG, "void f()",
        asMETHOD(CScriptDictionary, SetGCFlag), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_GETGCFLAG, "bool f()",
        asMETHOD(CScriptDictionary, GetGCFlag), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_ENUMREFS, "void f(int&in)",
        asMETHOD(CScriptDictionary, EnumReferences), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_RELEASEREFS, "void f(int&in)",
        asMETHOD(CScriptDictionary, ReleaseAllReferences), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Support foreach
    r = engine->RegisterObjectType("dictionaryIter", 0, asOBJ_REF);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionaryIter", asBEHAVE_ADDREF, "void f()",
        asMETHOD(CScriptDictionary::CScriptDictIter, AddRef), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionaryIter", asBEHAVE_RELEASE, "void f()",
        asMETHOD(CScriptDictionary::CScriptDictIter, Release), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "dictionaryIter @opForBegin() const",
        asMETHODPR(CScriptDictionary, opForBegin, () const,
                   CScriptDictionary::CScriptDictIter *),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "bool opForEnd(dictionaryIter @+) const",
        asMETHODPR(CScriptDictionary, opForEnd,
                   (const CScriptDictionary::CScriptDictIter &) const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "dictionaryIter @+ opForNext(dictionaryIter @+) const",
        asMETHODPR(CScriptDictionary, opForNext,
                   (CScriptDictionary::CScriptDictIter &) const,
                   CScriptDictionary::CScriptDictIter *),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary",
        "const dictionaryValue &opForValue0(dictionaryIter @+) const",
        asMETHODPR(CScriptDictionary, opForValue0,
                   (const CScriptDictionary::CScriptDictIter &) const,
                   const CScriptDictValue &),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "const string &opForValue1(dictionaryIter @+) const",
        asMETHODPR(CScriptDictionary, opForValue1,
                   (const CScriptDictionary::CScriptDictIter &) const,
                   const dictKey_t &),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Cache some things the dictionary will need at runtime
    SDictionaryCache::Setup(engine);
}

void RegisterScriptDictionary_Generic(asIScriptEngine *engine) {
    int r;

    // Register the cleanup callback for the object type cache
    engine->SetEngineUserDataCleanupCallback(SDictionaryCache::Cleanup,
                                             DICTIONARY_CACHE);

#if AS_CAN_USE_CPP11
    // With C++11 it is possible to use asGetTypeTraits to automatically
    // determine the correct flags that represents the C++ class
    r = engine->RegisterObjectType("dictionaryValue", sizeof(CScriptDictValue),
                                   asOBJ_VALUE | asOBJ_ASHANDLE | asOBJ_GC |
                                       asGetTypeTraits<CScriptDictValue>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
#else
    r = engine->RegisterObjectType("dictionaryValue", sizeof(CScriptDictValue),
                                   asOBJ_VALUE | asOBJ_ASHANDLE | asOBJ_GC |
                                       asOBJ_APP_CLASS_CD);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
#endif
    r = engine->RegisterObjectBehaviour(
        "dictionaryValue", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION(CScriptDictValue_Construct_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionaryValue", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTION(CScriptDictValue_Destruct_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionaryValue", asBEHAVE_ENUMREFS, "void f(int&in)",
        asFUNCTION(CScriptDictValue_EnumReferences_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionaryValue", asBEHAVE_RELEASEREFS, "void f(int&in)",
        asFUNCTION(CScriptDictValue_FreeValue_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue",
        "dictionaryValue &opAssign(const dictionaryValue &in)",
        asFUNCTION(CScriptDictValue_opCopyAssign_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "dictionaryValue &opHndlAssign(const ?&in)",
        asFUNCTION(CScriptDictValue_opAssign_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue",
        "dictionaryValue &opHndlAssign(const dictionaryValue &in)",
        asFUNCTION(CScriptDictValue_opCopyAssign_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "dictionaryValue &opAssign(const ?&in)",
        asFUNCTION(CScriptDictValue_opAssign_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "dictionaryValue &opAssign(double)",
        asFUNCTION(CScriptDictValue_opAssign_double_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "dictionaryValue &opAssign(int64)",
        asFUNCTION(CScriptDictValue_opAssign_int64_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "dictionaryValue &opAssign(uint64)",
        asFUNCTION(CScriptDictValue_opAssign_uint64_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "void opCast(?&out)",
        asFUNCTION(CScriptDictValue_opCast_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "void opConv(?&out)",
        asFUNCTION(CScriptDictValue_opCast_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "int64 opConv()",
        asFUNCTION(CScriptDictValue_opConvInt_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "uint64 opConv()",
        asFUNCTION(CScriptDictValue_opConvUInt_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionaryValue", "double opConv()",
        asFUNCTION(CScriptDictValue_opConvDouble_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectType("dictionary", sizeof(CScriptDictionary),
                                   asOBJ_REF | asOBJ_GC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_FACTORY, "dictionary@ f()",
        asFUNCTION(ScriptDictionaryFactory_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_LIST_FACTORY,
        "dictionary @f(int &in) {repeat {string, ?}}",
        asFUNCTION(ScriptDictionaryListFactory_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_ADDREF, "void f()",
        asFUNCTION(ScriptDictionaryAddRef_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_RELEASE, "void f()",
        asFUNCTION(ScriptDictionaryRelease_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "dictionary &opAssign(const dictionary &in)",
        asFUNCTION(ScriptDictionaryAssign_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "void set(const string &in, const ?&in)",
        asFUNCTION(ScriptDictionarySet_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "bool get(const string &in, ?&out) const",
        asFUNCTION(ScriptDictionaryGet_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "void set(const string &in, const int64&in)",
        asFUNCTION(ScriptDictionarySetInt_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "void set(const string &in, const uint64&in)",
        asFUNCTION(ScriptDictionarySetUInt_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "bool get(const string &in, int64&out) const",
        asFUNCTION(ScriptDictionaryGetInt_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "bool get(const string &in, uint64&out) const",
        asFUNCTION(ScriptDictionaryGetUInt_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "void set(const string &in, const double&in)",
        asFUNCTION(ScriptDictionarySetFlt_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "bool get(const string &in, double&out) const",
        asFUNCTION(ScriptDictionaryGetFlt_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "bool exists(const string &in) const",
        asFUNCTION(ScriptDictionaryExists_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "bool isEmpty() const",
        asFUNCTION(ScriptDictionaryIsEmpty_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "uint getSize() const",
        asFUNCTION(ScriptDictionaryGetSize_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "bool delete(const string &in)",
        asFUNCTION(ScriptDictionaryDelete_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "void deleteAll()",
        asFUNCTION(ScriptDictionaryDeleteAll_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "array<string> @getKeys() const",
        asFUNCTION(CScriptDictionaryGetKeys_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "dictionaryValue &opIndex(const string &in)",
        asFUNCTION(CScriptDictionary_opIndex_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "const dictionaryValue &opIndex(const string &in) const",
        asFUNCTION(CScriptDictionary_opIndex_const_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Register GC behaviours
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_GETREFCOUNT, "int f()",
        asFUNCTION(ScriptDictionaryGetRefCount_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_SETGCFLAG, "void f()",
        asFUNCTION(ScriptDictionarySetGCFlag_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_GETGCFLAG, "bool f()",
        asFUNCTION(ScriptDictionaryGetGCFlag_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_ENUMREFS, "void f(int&in)",
        asFUNCTION(ScriptDictionaryEnumReferences_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_RELEASEREFS, "void f(int&in)",
        asFUNCTION(ScriptDictionaryReleaseAllReferences_Generic),
        asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Support foreach
    r = engine->RegisterObjectType("dictionaryIter", 0, asOBJ_REF);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionaryIter", asBEHAVE_ADDREF, "void f()",
        asFUNCTION(ScriptDictIterAddRef_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "dictionaryIter", asBEHAVE_RELEASE, "void f()",
        asFUNCTION(ScriptDictIterRelease_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "dictionary", "dictionaryIter @opForBegin() const",
        asFUNCTION(ScriptDictionary_opForBegin_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "bool opForEnd(dictionaryIter @+) const",
        asFUNCTION(ScriptDictionary_opForEnd_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "dictionaryIter @+ opForNext(dictionaryIter @+) const",
        asFUNCTION(ScriptDictionary_opForNext_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary",
        "const dictionaryValue &opForValue0(dictionaryIter @+) const",
        asFUNCTION(ScriptDictionary_opForValue0_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "dictionary", "const string &opForValue1(dictionaryIter @+) const",
        asFUNCTION(ScriptDictionary_opForValue1_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Cache some things the dictionary will need at runtime
    SDictionaryCache::Setup(engine);
}

END_AS_NAMESPACE
