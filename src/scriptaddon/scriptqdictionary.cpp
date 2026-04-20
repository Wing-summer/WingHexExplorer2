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
#include "define.h"

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
    CScriptDictValue &val = dict[key];

    // Ensure the dictionary value has the engine pointer set (it will be null
    // if newly created in above operation)
    if (val.m_engine == nullptr)
        val.m_engine = engine;

    return &val;
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
        it = dict.insert(dictMap_t::value_type(key, CScriptDictValue(engine)))
                 .first;
        iterGuard++;
    }

    it->second.Set(value, typeId);
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
        return it->second.Get(value, typeId);

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
        it->second.FreeValue();
        dict.erase(it);
        iterGuard++;
        return true;
    }

    return false;
}

void CScriptDictionary::DeleteAll() {
    dictMap_t::iterator it;
    for (it = dict.begin(); it != dict.end(); it++)
        it->second.FreeValue();

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

//-------------------------------------------------------------------------
// CScriptDictValue

CScriptDictValue::CScriptDictValue() {
    m_valueObj = nullptr;
    m_typeId = 0;
    m_engine = nullptr;
}

CScriptDictValue::CScriptDictValue(asIScriptEngine *engine) {
    m_valueObj = nullptr;
    m_typeId = 0;
    m_engine = engine;
}

CScriptDictValue::CScriptDictValue(asIScriptEngine *engine, void *value,
                                   int typeId) {
    m_valueObj = 0;
    m_typeId = 0;

    // Don't increase the reference count, since the engine anyway holds a
    // pointer to the owning dictionary in the GC
    // TODO: If the dictionaryValue is used as a standalone object, then it may
    // be necessary to increase refcount in the engine
    m_engine = engine;
    Set(value, typeId);
}

CScriptDictValue::~CScriptDictValue() { FreeValue(); }

void CScriptDictValue::FreeValue() {
    if (!m_engine) {
        m_valueObj = nullptr;
        m_typeId = 0;
        return;
    }

    if ((m_typeId & asTYPEID_MASK_OBJECT) && m_valueObj) {
        int actualTypeId =
            m_typeId & ~(asTYPEID_OBJHANDLE | asTYPEID_HANDLETOCONST);
        asITypeInfo *ti = m_engine->GetTypeInfoById(actualTypeId);
        if (ti) {
            m_engine->ReleaseScriptObject(m_valueObj, ti);
        }
    }

    m_valueObj = nullptr;
    m_typeId = 0;
}

void CScriptDictValue::ReleaseReferences(asIScriptEngine *) { FreeValue(); }

void CScriptDictValue::EnumReferences(asIScriptEngine *inEngine) {
    assert(inEngine == m_engine);

    // If we're holding a reference, we'll notify the garbage collector of it
    if (m_valueObj)
        inEngine->GCEnumCallback(m_valueObj);

    // The object type itself is also garbage collected
    if (m_typeId)
        inEngine->GCEnumCallback(inEngine->GetTypeInfoById(m_typeId));
}

void CScriptDictValue::Set(void *value, int typeId) {
    FreeValue();

    m_typeId = typeId;
    if (typeId & asTYPEID_OBJHANDLE) {
        // We're receiving a reference to the handle, so we need to dereference
        // it
        m_valueObj = *(void **)value;
        m_engine->AddRefScriptObject(m_valueObj,
                                     m_engine->GetTypeInfoById(typeId));
    } else if (typeId & asTYPEID_MASK_OBJECT) {
        // Create a copy of the object
        m_valueObj = m_engine->CreateScriptObjectCopy(
            value, m_engine->GetTypeInfoById(typeId));
        if (m_valueObj == 0) {
            asIScriptContext *ctx = asGetActiveContext();
            if (ctx)
                ctx->SetException("Cannot create copy of object");
        }
    } else {
        // Copy the primitive value
        // We receive a pointer to the value.
        int size = m_engine->GetSizeOfPrimitiveType(typeId);
        memcpy(&m_valueInt, value, size);
    }
}

void CScriptDictValue::Set(const asQWORD &value) {
    Set(const_cast<asQWORD *>(&value), asTYPEID_UINT64);
}

void CScriptDictValue::Set(CScriptDictValue &value) {
    assert(m_engine == value.m_engine);
    if (value.m_typeId & asTYPEID_OBJHANDLE)
        Set((void *)&value.m_valueObj, value.m_typeId);
    else if (value.m_typeId & asTYPEID_MASK_OBJECT)
        Set((void *)value.m_valueObj, value.m_typeId);
    else
        Set((void *)&value.m_valueInt, value.m_typeId);
}

// This overloaded method is implemented so that all integer and
// unsigned integers types will be stored in the dictionary as int64
// through implicit conversions. This simplifies the management of the
// numeric types when the script retrieves the stored value using a
// different type.
void CScriptDictValue::Set(const asINT64 &value) {
    Set(const_cast<asINT64 *>(&value), asTYPEID_INT64);
}

// This overloaded method is implemented so that all floating point types
// will be stored in the dictionary as double through implicit conversions.
// This simplifies the management of the numeric types when the script
// retrieves the stored value using a different type.
void CScriptDictValue::Set(const double &value) {
    Set(const_cast<double *>(&value), asTYPEID_DOUBLE);
}

namespace {

enum NumericKind { nkSigned, nkUnsigned, nkFloat };

struct NumericValue {
    NumericKind kind;
    union {
        asINT64 s;
        asQWORD u;
        double f;
    };
};

static int resolveBaseTypeId(asIScriptEngine *engine, int typeId) {
    if (typeId > asTYPEID_DOUBLE && (typeId & asTYPEID_MASK_OBJECT) == 0) {
        asITypeInfo *ti = engine->GetTypeInfoById(typeId);
        if (ti) {
            int underlying = ti->GetUnderlyingTypeId();
            if (underlying != 0)
                return underlying;
        }
    }

    return typeId;
}

static bool readNumericValue(asIScriptEngine *engine, asQWORD storedInt,
                             double storedFlt, int sourceTypeId,
                             NumericValue &out) {
    int baseTypeId = resolveBaseTypeId(engine, sourceTypeId);

    out.kind = nkSigned;
    out.s = 0;
    out.u = 0;
    out.f = 0.0L;

    switch (baseTypeId) {
    case asTYPEID_BOOL: {
        char v = 0;
        std::memcpy(&v, &storedInt, sizeof(v));
        out.kind = nkSigned;
        out.s = v ? 1 : 0;
        return true;
    }
    case asTYPEID_INT8: {
        asINT8 v = 0;
        std::memcpy(&v, &storedInt, sizeof(v));
        out.kind = nkSigned;
        out.s = v;
        return true;
    }
    case asTYPEID_INT16: {
        asINT16 v = 0;
        std::memcpy(&v, &storedInt, sizeof(v));
        out.kind = nkSigned;
        out.s = v;
        return true;
    }
    case asTYPEID_INT32: {
        asINT32 v = 0;
        std::memcpy(&v, &storedInt, sizeof(v));
        out.kind = nkSigned;
        out.s = v;
        return true;
    }
    case asTYPEID_INT64: {
        asINT64 v = 0;
        std::memcpy(&v, &storedInt, sizeof(v));
        out.kind = nkSigned;
        out.s = v;
        return true;
    }
    case asTYPEID_UINT8: {
        asBYTE v = 0;
        std::memcpy(&v, &storedInt, sizeof(v));
        out.kind = nkUnsigned;
        out.u = v;
        return true;
    }
    case asTYPEID_UINT16: {
        asWORD v = 0;
        std::memcpy(&v, &storedInt, sizeof(v));
        out.kind = nkUnsigned;
        out.u = v;
        return true;
    }
    case asTYPEID_UINT32: {
        asDWORD v = 0;
        std::memcpy(&v, &storedInt, sizeof(v));
        out.kind = nkUnsigned;
        out.u = v;
        return true;
    }
    case asTYPEID_UINT64: {
        asQWORD v = 0;
        std::memcpy(&v, &storedInt, sizeof(v));
        out.kind = nkUnsigned;
        out.u = v;
        return true;
    }
    case asTYPEID_FLOAT: {
        float v = float(storedFlt);
        out.kind = nkFloat;
        out.f = v;
        return true;
    }
    case asTYPEID_DOUBLE: {
        out.kind = nkFloat;
        out.f = storedFlt;
        return true;
    }
    default:
        return false;
    }
}

static bool writeNumericValue(void *value, asIScriptEngine *engine,
                              int targetTypeId, const NumericValue &src) {
    int baseTypeId = resolveBaseTypeId(engine, targetTypeId);

    switch (baseTypeId) {
    case asTYPEID_BOOL: {
        bool v = false;
        if (src.kind == nkFloat)
            v = (src.f != 0.0L);
        else if (src.kind == nkUnsigned)
            v = (src.u != 0);
        else
            v = (src.s != 0);

        *(bool *)value = v;
        return true;
    }

    case asTYPEID_INT8: {
        if (src.kind == nkFloat) {
            if (src.f != src.f)
                return false;
            if (src.f < std::numeric_limits<asINT8>::min() ||
                src.f > std::numeric_limits<asINT8>::max())
                return false;
            *(asINT8 *)value = asINT8(src.f);
            return true;
        }
        if (src.kind == nkUnsigned) {
            if (src.u > std::numeric_limits<asINT8>::max())
                return false;
            *(asINT8 *)value = asINT8(src.u);
            return true;
        }
        if (src.s < std::numeric_limits<asINT8>::min() ||
            src.s > std::numeric_limits<asINT8>::max())
            return false;
        *(asINT8 *)value = asINT8(src.s);
        return true;
    }

    case asTYPEID_INT16: {
        if (src.kind == nkFloat) {
            if (src.f != src.f)
                return false;
            if (src.f < std::numeric_limits<asINT16>::min() ||
                src.f > std::numeric_limits<asINT16>::max())
                return false;
            *(asINT16 *)value = asINT16(src.f);
            return true;
        }
        if (src.kind == nkUnsigned) {
            if (src.u > std::numeric_limits<asINT16>::max())
                return false;
            *(asINT16 *)value = asINT16(src.u);
            return true;
        }
        if (src.s < std::numeric_limits<asINT16>::min() ||
            src.s > std::numeric_limits<asINT16>::max())
            return false;
        *(asINT16 *)value = asINT16(src.s);
        return true;
    }

    case asTYPEID_INT32: {
        if (src.kind == nkFloat) {
            if (src.f != src.f)
                return false;
            if (src.f < std::numeric_limits<asINT32>::min() ||
                src.f > std::numeric_limits<asINT32>::max())
                return false;
            *(asINT32 *)value = asINT32(src.f);
            return true;
        }
        if (src.kind == nkUnsigned) {
            if (src.u > std::numeric_limits<asINT32>::max())
                return false;
            *(asINT32 *)value = asINT32(src.u);
            return true;
        }
        if (src.s < std::numeric_limits<asINT32>::min() ||
            src.s > std::numeric_limits<asINT32>::max())
            return false;
        *(asINT32 *)value = asINT32(src.s);
        return true;
    }

    case asTYPEID_INT64: {
        if (src.kind == nkFloat) {
            if (src.f != src.f)
                return false;
            if (src.f < std::numeric_limits<asINT64>::min() ||
                src.f > double(std::numeric_limits<asINT64>::max()))
                return false;
            *(asINT64 *)value = asINT64(src.f);
            return true;
        }
        if (src.kind == nkUnsigned) {
            if (src.u > std::numeric_limits<asINT64>::max())
                return false;
            *(asINT64 *)value = asINT64(src.u);
            return true;
        }
        *(asINT64 *)value = src.s;
        return true;
    }

    case asTYPEID_UINT8: {
        if (src.kind == nkFloat) {
            if (src.f != src.f)
                return false;
            if (src.f < 0.0L || src.f > std::numeric_limits<asBYTE>::max())
                return false;
            *(asBYTE *)value = asBYTE(src.f);
            return true;
        }
        if (src.kind == nkSigned) {
            if (src.s < 0 || src.s > std::numeric_limits<asBYTE>::max())
                return false;
            *(asBYTE *)value = asBYTE(src.s);
            return true;
        }
        if (src.u > std::numeric_limits<asBYTE>::max())
            return false;
        *(asBYTE *)value = asBYTE(src.u);
        return true;
    }

    case asTYPEID_UINT16: {
        if (src.kind == nkFloat) {
            if (src.f != src.f)
                return false;
            if (src.f < 0.0L || src.f > std::numeric_limits<asWORD>::max())
                return false;
            *(asWORD *)value = asWORD(src.f);
            return true;
        }
        if (src.kind == nkSigned) {
            if (src.s < 0 || src.s > std::numeric_limits<asWORD>::max())
                return false;
            *(asWORD *)value = asWORD(src.s);
            return true;
        }
        if (src.u > std::numeric_limits<asWORD>::max())
            return false;
        *(asWORD *)value = asWORD(src.u);
        return true;
    }

    case asTYPEID_UINT32: {
        if (src.kind == nkFloat) {
            if (src.f != src.f)
                return false;
            if (src.f < 0.0L || src.f > std::numeric_limits<asDWORD>::max())
                return false;
            *(asDWORD *)value = asDWORD(src.f);
            return true;
        }
        if (src.kind == nkSigned) {
            if (src.s < 0 || src.s > std::numeric_limits<asDWORD>::max())
                return false;
            *(asDWORD *)value = asDWORD(src.s);
            return true;
        }
        if (src.u > std::numeric_limits<asDWORD>::max())
            return false;
        *(asDWORD *)value = asDWORD(src.u);
        return true;
    }

    case asTYPEID_UINT64: {
        if (src.kind == nkFloat) {
            if (src.f != src.f)
                return false;
            if (src.f < 0.0L ||
                src.f > double(std::numeric_limits<asQWORD>::max()))
                return false;
            *(asQWORD *)value = asQWORD(src.f);
            return true;
        }
        if (src.kind == nkSigned) {
            if (src.s < 0)
                return false;
            *(asQWORD *)value = asQWORD(src.s);
            return true;
        }
        *(asQWORD *)value = src.u;
        return true;
    }

    case asTYPEID_FLOAT: {
        if (src.kind == nkFloat) {
            if (src.f != src.f) {
                *(float *)value = float(src.f); // preserve NaN
                return true;
            }
            if (src.f < -std::numeric_limits<float>::max() ||
                src.f > std::numeric_limits<float>::max())
                return false;
            *(float *)value = float(src.f);
            return true;
        }

        double v = (src.kind == nkUnsigned) ? src.u : src.s;
        if (v < -std::numeric_limits<float>::max() ||
            v > std::numeric_limits<float>::max())
            return false;

        *(float *)value = float(v);
        return true;
    }

    case asTYPEID_DOUBLE: {
        if (src.kind == nkFloat)
            *(double *)value = src.f;
        else if (src.kind == nkUnsigned)
            *(double *)value = src.u;
        else
            *(double *)value = src.s;
        return true;
    }

    default:
        return false;
    }
}
} // namespace

bool CScriptDictValue::Get(void *value, int typeId) const {
    // Return the value
    if (typeId & asTYPEID_OBJHANDLE) {
        // A handle can be retrieved if the stored type is a handle of same or
        // compatible type or if the stored type is an object that implements
        // the interface that the handle refer to.
        if ((m_typeId & asTYPEID_MASK_OBJECT)) {
            // Don't allow the get if the stored handle is to a const, but the
            // desired handle is not
            if ((m_typeId & asTYPEID_HANDLETOCONST) &&
                !(typeId & asTYPEID_HANDLETOCONST))
                return false;

            // RefCastObject will increment the refcount if successful
            m_engine->RefCastObject(m_valueObj,
                                    m_engine->GetTypeInfoById(m_typeId),
                                    m_engine->GetTypeInfoById(typeId),
                                    reinterpret_cast<void **>(value));

            return true;
        }
    } else if (typeId & asTYPEID_MASK_OBJECT) {
        // Verify that the copy can be made
        bool isCompatible = false;

        // Allow a handle to be value assigned if the wanted type is not a
        // handle
        if ((m_typeId & ~(asTYPEID_OBJHANDLE | asTYPEID_HANDLETOCONST)) ==
                typeId &&
            m_valueObj != 0)
            isCompatible = true;

        // Copy the object into the given reference
        if (isCompatible) {
            m_engine->AssignScriptObject(value, m_valueObj,
                                         m_engine->GetTypeInfoById(typeId));
            return true;
        }
    } else {
        // Same type: direct copy
        if (m_typeId == typeId) {
            int size = m_engine->GetSizeOfPrimitiveType(
                resolveBaseTypeId(m_engine, typeId));
            std::memcpy(value, &m_valueInt, size);
            return true;
        }

        const bool sourceIsObject = (m_typeId & asTYPEID_MASK_OBJECT) != 0;

        const int sourceBaseTypeId = resolveBaseTypeId(m_engine, m_typeId);
        const int targetBaseTypeId = resolveBaseTypeId(m_engine, typeId);

        // BOOL ~ DOUBLE and enum underlying types (resolved above) are treated
        // as a single numeric conversion domain.
        if (!sourceIsObject && sourceBaseTypeId >= asTYPEID_BOOL &&
            sourceBaseTypeId <= asTYPEID_DOUBLE &&
            targetBaseTypeId >= asTYPEID_BOOL &&
            targetBaseTypeId <= asTYPEID_DOUBLE) {
            NumericValue src;
            if (!readNumericValue(m_engine, m_valueInt, m_valueFlt, m_typeId,
                                  src)) {
                return false;
            }

            if (!writeNumericValue(value, m_engine, typeId, src)) {
                asIScriptContext *active = asGetActiveContext();
                if (active) {
                    active->SetException("Dictionary value safe conversion "
                                         "failed: value out of range");
                }
                return false;
            }
            return true;
        }

        // Do not treat objects or handles as bool.
        if (typeId == asTYPEID_BOOL) {
            if (m_typeId & asTYPEID_MASK_OBJECT) {
                *(bool *)value = false;
                return false;
            }

            // Primitive values keep the current numeric truthiness behavior.
            asQWORD zero = 0;
            int size = m_engine->GetSizeOfPrimitiveType(m_typeId);
            *(bool *)value = std::memcmp(&m_valueInt, &zero, size) != 0;
            return true;
        }
    }

    // It was not possible to retrieve the value using the desired typeId
    return false;
}

bool CScriptDictValue::Get(asQWORD &value) const {
    return Get(&value, asTYPEID_UINT64);
}

const void *CScriptDictValue::GetAddressOfValue() const {
    if ((m_typeId & asTYPEID_MASK_OBJECT) && !(m_typeId & asTYPEID_OBJHANDLE)) {
        // Return the address to the object directly
        return m_valueObj;
    }

    // Return the address of the primitive or the pointer to the object
    return reinterpret_cast<const void *>(&m_valueObj);
}

bool CScriptDictValue::Get(asINT64 &value) const {
    return Get(&value, asTYPEID_INT64);
}

bool CScriptDictValue::Get(double &value) const {
    return Get(&value, asTYPEID_DOUBLE);
}

int CScriptDictValue::GetTypeId() const { return m_typeId; }

static void CScriptDictValue_Destruct(CScriptDictValue *obj) {
    obj->~CScriptDictValue();
}

static CScriptDictValue &CScriptDictValue_opAssign(void *ref, int typeId,
                                                   CScriptDictValue *obj) {
    obj->Set(ref, typeId);
    return *obj;
}

static CScriptDictValue &
CScriptDictValue_opAssign(const CScriptDictValue &other,
                          CScriptDictValue *obj) {
    obj->Set(const_cast<CScriptDictValue &>(other));
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
    obj->Get(ref, typeId);
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
    self->Get(value);
    *(double *)gen->GetAddressOfReturnLocation() = value;
}

static void CScriptDictValue_opConvInt_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    asINT64 value;
    self->Get(value);
    *(asINT64 *)gen->GetAddressOfReturnLocation() = value;
}

static void CScriptDictValue_opConvUInt_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    asQWORD value;
    self->Get(value);
    *(asQWORD *)gen->GetAddressOfReturnLocation() = value;
}

static void CScriptDictValue_opCast_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    self->Get(gen->GetArgAddress(0), gen->GetArgTypeId(0));
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
    new (self) CScriptDictValue(gen->GetEngine());
}

static void CScriptDictValue_Destruct_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    CScriptDictValue_Destruct(self);
}

static void CScriptDictValue_EnumReferences_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    self->EnumReferences(gen->GetEngine());
}

static void CScriptDictValue_ReleaseReferences_Generic(asIScriptGeneric *gen) {
    CScriptDictValue *self = (CScriptDictValue *)gen->GetObject();
    self->ReleaseReferences(gen->GetEngine());
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
    return m_it->second.Get(&value, asTYPEID_INT64);
}

bool CScriptDictionary::CIterator::GetValue(double &value) const {
    return m_it->second.Get(&value, asTYPEID_DOUBLE);
}

bool CScriptDictionary::CIterator::GetValue(void *value, int typeId) const {
    return m_it->second.Get(value, typeId);
}

const void *CScriptDictionary::CIterator::GetAddressOfValue() const {
    return m_it->second.GetAddressOfValue();
}

//--------------------------------------------------------------------------
// Register the type

void RegisterScriptDictionary(asIScriptEngine *engine) {
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
    ASSERT(r >= 0);

#else
    r = engine->RegisterObjectType("dictionaryValue", sizeof(CScriptDictValue),
                                   asOBJ_VALUE | asOBJ_ASHANDLE | asOBJ_GC |
                                       asOBJ_APP_CLASS_CD);
    ASSERT(r >= 0);

#endif
    r = engine->RegisterObjectBehaviour(
        "dictionaryValue", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION(CScriptDictValue_Construct_Generic), asCALL_GENERIC);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "dictionaryValue", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTION(CScriptDictValue_Destruct), asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "dictionaryValue", asBEHAVE_ENUMREFS, "void f(int&in)",
        asMETHOD(CScriptDictValue, EnumReferences), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "dictionaryValue", asBEHAVE_RELEASEREFS, "void f(int&in)",
        asMETHOD(CScriptDictValue, ReleaseReferences), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionaryValue",
        "dictionaryValue &opAssign(const dictionaryValue &in)",
        asFUNCTIONPR(CScriptDictValue_opAssign,
                     (const CScriptDictValue &, CScriptDictValue *),
                     CScriptDictValue &),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionaryValue", "dictionaryValue &opHndlAssign(const ?&in)",
        asFUNCTIONPR(CScriptDictValue_opAssign,
                     (void *, int, CScriptDictValue *), CScriptDictValue &),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionaryValue",
        "dictionaryValue &opHndlAssign(const dictionaryValue &in)",
        asFUNCTIONPR(CScriptDictValue_opAssign,
                     (const CScriptDictValue &, CScriptDictValue *),
                     CScriptDictValue &),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionaryValue", "dictionaryValue &opAssign(const ?&in)",
        asFUNCTIONPR(CScriptDictValue_opAssign,
                     (void *, int, CScriptDictValue *), CScriptDictValue &),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionaryValue", "dictionaryValue &opAssign(double)",
        asFUNCTIONPR(CScriptDictValue_opAssign, (double, CScriptDictValue *),
                     CScriptDictValue &),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionaryValue", "dictionaryValue &opAssign(int64)",
        asFUNCTIONPR(CScriptDictValue_opAssign, (asINT64, CScriptDictValue *),
                     CScriptDictValue &),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionaryValue", "dictionaryValue &opAssign(uint64)",
        asFUNCTIONPR(CScriptDictValue_opAssign, (asQWORD, CScriptDictValue *),
                     CScriptDictValue &),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionaryValue", "void opCast(?&out)",
        asFUNCTIONPR(CScriptDictValue_opCast, (void *, int, CScriptDictValue *),
                     void),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionaryValue", "void opConv(?&out)",
        asFUNCTIONPR(CScriptDictValue_opCast, (void *, int, CScriptDictValue *),
                     void),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionaryValue", "int64 opConv()",
        asFUNCTIONPR(CScriptDictValue_opConvInt, (CScriptDictValue *), asINT64),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("dictionaryValue", "uint64 opConv()",
                                     asFUNCTIONPR(CScriptDictValue_opConvUInt,
                                                  (CScriptDictValue *),
                                                  asQWORD),
                                     asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("dictionaryValue", "double opConv()",
                                     asFUNCTIONPR(CScriptDictValue_opConvDouble,
                                                  (CScriptDictValue *), double),
                                     asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectType("dictionary", sizeof(CScriptDictionary),
                                   asOBJ_REF | asOBJ_GC);
    ASSERT(r >= 0);

    // Use the generic interface to construct the object since we need the
    // engine pointer, we could also have retrieved the engine pointer from the
    // active context
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_FACTORY, "dictionary@ f()",
        asFUNCTION(ScriptDictionaryFactory_Generic), asCALL_GENERIC);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_LIST_FACTORY,
        "dictionary @f(int &in) {repeat {string, ?}}",
        asFUNCTION(ScriptDictionaryListFactory_Generic), asCALL_GENERIC);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_ADDREF, "void f()",
        asMETHOD(CScriptDictionary, AddRef), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_RELEASE, "void f()",
        asMETHOD(CScriptDictionary, Release), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "dictionary &opAssign(const dictionary &in)",
        asMETHODPR(CScriptDictionary, operator=, (const CScriptDictionary &),
                   CScriptDictionary &),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "void set(const string &in, const ?&in)",
        asMETHODPR(CScriptDictionary, Set, (const dictKey_t &, void *, int),
                   void),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "bool get(const string &in, ?&out) const",
        asMETHODPR(CScriptDictionary, Get,
                   (const dictKey_t &, void *, int) const, bool),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "void set(const string &in, const int64&in)",
        asMETHODPR(CScriptDictionary, Set, (const dictKey_t &, const asINT64 &),
                   void),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "void set(const string &in, const uint64&in)",
        asMETHODPR(CScriptDictionary, Set, (const dictKey_t &, const asQWORD &),
                   void),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "bool get(const string &in, int64&out) const",
        asMETHODPR(CScriptDictionary, Get, (const dictKey_t &, asINT64 &) const,
                   bool),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "bool get(const string &in, uint64&out) const",
        asMETHODPR(CScriptDictionary, Get, (const dictKey_t &, asQWORD &) const,
                   bool),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "void set(const string &in, const double&in)",
        asMETHODPR(CScriptDictionary, Set, (const dictKey_t &, const double &),
                   void),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "bool get(const string &in, double&out) const",
        asMETHODPR(CScriptDictionary, Get, (const dictKey_t &, double &) const,
                   bool),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "bool exists(const string &in) const",
        asMETHOD(CScriptDictionary, Exists), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("dictionary", "bool isEmpty() const",
                                     asMETHOD(CScriptDictionary, IsEmpty),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("dictionary", "uint getSize() const",
                                     asMETHOD(CScriptDictionary, GetSize),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "bool delete(const string &in)",
        asMETHOD(CScriptDictionary, Delete), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("dictionary", "void deleteAll()",
                                     asMETHOD(CScriptDictionary, DeleteAll),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "array<string> @getKeys() const",
        asMETHOD(CScriptDictionary, GetKeys), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "dictionaryValue &opIndex(const string &in)",
        asMETHODPR(CScriptDictionary, operator[], (const dictKey_t &),
                   CScriptDictValue *),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "const dictionaryValue &opIndex(const string &in) const",
        asMETHODPR(CScriptDictionary, operator[], (const dictKey_t &) const,
                   const CScriptDictValue *),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    // Register GC behaviours
    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_GETREFCOUNT, "int f()",
        asMETHOD(CScriptDictionary, GetRefCount), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_SETGCFLAG, "void f()",
        asMETHOD(CScriptDictionary, SetGCFlag), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_GETGCFLAG, "bool f()",
        asMETHOD(CScriptDictionary, GetGCFlag), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_ENUMREFS, "void f(int&in)",
        asMETHOD(CScriptDictionary, EnumReferences), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "dictionary", asBEHAVE_RELEASEREFS, "void f(int&in)",
        asMETHOD(CScriptDictionary, ReleaseAllReferences), asCALL_THISCALL);
    ASSERT(r >= 0);

    // Support foreach
    r = engine->RegisterObjectType("dictionaryIter", 0, asOBJ_REF);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "dictionaryIter", asBEHAVE_ADDREF, "void f()",
        asMETHOD(CScriptDictionary::CScriptDictIter, AddRef), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "dictionaryIter", asBEHAVE_RELEASE, "void f()",
        asMETHOD(CScriptDictionary::CScriptDictIter, Release), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "dictionaryIter @opForBegin() const",
        asMETHODPR(CScriptDictionary, opForBegin, () const,
                   CScriptDictionary::CScriptDictIter *),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "bool opForEnd(dictionaryIter @+) const",
        asMETHODPR(CScriptDictionary, opForEnd,
                   (const CScriptDictionary::CScriptDictIter &) const, bool),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "dictionaryIter @+ opForNext(dictionaryIter @+) const",
        asMETHODPR(CScriptDictionary, opForNext,
                   (CScriptDictionary::CScriptDictIter &) const,
                   CScriptDictionary::CScriptDictIter *),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary",
        "const dictionaryValue &opForValue0(dictionaryIter @+) const",
        asMETHODPR(CScriptDictionary, opForValue0,
                   (const CScriptDictionary::CScriptDictIter &) const,
                   const CScriptDictValue &),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "dictionary", "const string &opForValue1(dictionaryIter @+) const",
        asMETHODPR(CScriptDictionary, opForValue1,
                   (const CScriptDictionary::CScriptDictIter &) const,
                   const dictKey_t &),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    // Cache some things the dictionary will need at runtime
    SDictionaryCache::Setup(engine);
}

END_AS_NAMESPACE
