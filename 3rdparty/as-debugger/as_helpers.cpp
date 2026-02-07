// MIT Licensed
// see https://github.com/Paril/angelscript-debugger

#include "as_helpers.h"
#include <stdexcept>

asIDBValue::asIDBValue(asIScriptEngine *engine, void *ptr, int typeId,
                       bool reference)
    : engine(engine), typeId(typeId) {
    if (!typeId)
        return;

    type = engine->GetTypeInfoById(typeId);

    if (type)
        type->AddRef();

    if (reference)
        ptr = *reinterpret_cast<void **>(ptr);

    if (typeId & asTYPEID_OBJHANDLE) {
        value.obj = *reinterpret_cast<void **>(ptr);
        engine->AddRefScriptObject(value.obj, type);
    } else if (typeId & asTYPEID_MASK_OBJECT) {
        value.obj = engine->CreateScriptObjectCopy(ptr, type);
    } else {
        value.u64 = 0;
        int size = engine->GetSizeOfPrimitiveType(typeId);
        memcpy(&value.u64, ptr, size);
    }
}

asIDBValue::asIDBValue(const asIDBValue &other)
    : engine(other.engine), typeId(other.typeId), type(other.type) {
    if (!typeId)
        return;

    if (type)
        type->AddRef();

    if (typeId & asTYPEID_OBJHANDLE) {
        value.obj = other.value.obj;
        engine->AddRefScriptObject(value.obj, type);
    } else if (typeId & asTYPEID_MASK_OBJECT) {
        value.obj = engine->CreateScriptObjectCopy(other.value.obj, type);
    } else
        value.u64 = other.value.u64;
}

asIDBValue::asIDBValue(asIDBValue &&other) noexcept
    : engine(other.engine), typeId(other.typeId), type(other.type) {
    if (!typeId)
        return;

    value.u64 = other.value.u64;

    other.type = nullptr;
    other.typeId = 0;
    other.value.u64 = 0;
}

asIDBValue &asIDBValue::operator=(const asIDBValue &other) {
    engine = other.engine;
    typeId = other.typeId;
    type = other.type;
    value.u64 = 0;

    if (!typeId)
        return *this;

    if (type)
        type->AddRef();

    if (typeId & asTYPEID_OBJHANDLE) {
        value.obj = other.value.obj;
        engine->AddRefScriptObject(value.obj, type);
    } else if (typeId & asTYPEID_MASK_OBJECT) {
        value.obj = engine->CreateScriptObjectCopy(other.value.obj, type);
    } else
        value.u64 = other.value.u64;

    return *this;
}

asIDBValue &asIDBValue::operator=(asIDBValue &&other) noexcept {
    engine = other.engine;
    typeId = other.typeId;
    type = other.type;
    value.u64 = 0;

    if (!typeId)
        return *this;

    value.u64 = other.value.u64;

    other.type = nullptr;
    other.typeId = 0;
    other.value.u64 = 0;

    return *this;
}

asIDBValue::~asIDBValue() { Release(); }

void asIDBValue::Release() {
    if (typeId & asTYPEID_MASK_OBJECT)
        engine->ReleaseScriptObject(value.obj, type);

    if (type)
        type->Release();

    type = nullptr;
    typeId = 0;
    value.u64 = 0;
}

void asIDBValue::SetArgument(asIScriptContext *ctx, asUINT index) const {
    if (typeId & asTYPEID_MASK_OBJECT)
        ctx->SetArgObject(index, GetPointer<void *>());
    else if (typeId == asTYPEID_BOOL || typeId == asTYPEID_INT8 ||
             typeId == asTYPEID_UINT8)
        ctx->SetArgByte(index, *GetPointer<uint8_t>());
    else if (typeId == asTYPEID_INT16 || typeId == asTYPEID_UINT16)
        ctx->SetArgWord(index, *GetPointer<uint16_t>());
    else if (typeId == asTYPEID_INT32 || typeId == asTYPEID_UINT32)
        ctx->SetArgDWord(index, *GetPointer<uint32_t>());
    else if (typeId == asTYPEID_INT64 || typeId == asTYPEID_UINT64)
        ctx->SetArgQWord(index, *GetPointer<uint64_t>());
    else if (typeId == asTYPEID_FLOAT)
        ctx->SetArgFloat(index, *GetPointer<float>());
    else if (typeId == asTYPEID_DOUBLE)
        ctx->SetArgDouble(index, *GetPointer<double>());
    else
        throw std::runtime_error("invalid type");
}

asIDBObjectIteratorHelper::asIDBObjectIteratorHelper(asITypeInfo *type,
                                                     void *obj)
    : type(type), obj(obj), opForBegin(type->GetMethodByName("opForBegin")),
      opForEnd(type->GetMethodByName("opForEnd")),
      opForNext(type->GetMethodByName("opForNext")) {
    if (!Validate()) {
        opForBegin = nullptr;
        return;
    }

    // TODO: don't need opForValues unless we're iterating
    // TODO: verify opForValue{n} selection
    auto opForValue = type->GetMethodByName("opForValue");

    if (!opForValue) {
        for (int i = 0;; i++) {
            auto f = type->GetMethodByName(
                fmt::format(FMT_STRING("opForValue{}"), i).c_str());

            if (!f)
                break;

            opForValues.push_back(f);
        }
    } else
        opForValues.push_back(opForValue);
}

asIDBValue asIDBObjectIteratorHelper::Begin(asIScriptContext *ctx) const {
    ctx->Prepare(opForBegin);
    ctx->SetObject(obj);
    ctx->Execute();

    asDWORD flags;
    int typeId = opForBegin->GetReturnTypeId(&flags);
    return asIDBValue(ctx->GetEngine(), ctx->GetAddressOfReturnValue(), typeId,
                      (asETypeModifiers)flags);
}

void asIDBObjectIteratorHelper::Value(asIScriptContext *ctx,
                                      const asIDBValue &val,
                                      size_t index) const {
    ctx->Prepare(opForValues[index]);
    ctx->SetObject(obj);
    val.SetArgument(ctx, 0);
    ctx->Execute();
}

asIDBValue asIDBObjectIteratorHelper::Next(asIScriptContext *ctx,
                                           const asIDBValue &val) const {
    ctx->Prepare(opForNext);
    ctx->SetObject(obj);
    val.SetArgument(ctx, 0);
    ctx->Execute();

    asDWORD flags;
    int typeId = opForBegin->GetReturnTypeId(&flags);
    return asIDBValue(ctx->GetEngine(), ctx->GetAddressOfReturnValue(), typeId,
                      (asETypeModifiers)flags);
}

bool asIDBObjectIteratorHelper::End(asIScriptContext *ctx,
                                    const asIDBValue &val) const {
    ctx->Prepare(opForEnd);
    ctx->SetObject(obj);
    val.SetArgument(ctx, 0);
    ctx->Execute();
    return !!ctx->GetReturnByte();
}

size_t asIDBObjectIteratorHelper::CalculateLength(asIScriptContext *ctx) const {
    ctx->PushState();

    size_t length = 0;
    for (auto it = Begin(ctx); !End(ctx, it); length++, it = Next(ctx, it))
        ;

    ctx->PopState();
    return length;
}

bool asIDBObjectIteratorHelper::Validate() {
    if (!opForBegin || !opForEnd || !opForNext)
        return false;

    iteratorTypeId = opForBegin->GetReturnTypeId();

    if (!iteratorTypeId) {
        error = "bad iterator return type";
        return false;
    }

    if (!(iteratorTypeId & asTYPEID_MASK_OBJECT) &&
        iteratorTypeId > asTYPEID_DOUBLE) {
        error = "unsupported iterator type";
        return false;
    }

    iteratorType = opForBegin->GetEngine()->GetTypeInfoById(iteratorTypeId);

    // TODO: more validation

    return true;
}
