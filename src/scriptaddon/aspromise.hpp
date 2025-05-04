// url: https://github.com/romanpunia/aspromise
// Licensed under the MIT license. Free for any type of use.

#ifndef AS_PROMISE_HPP
#define AS_PROMISE_HPP

#ifndef PROMISE_CONFIG
#define PROMISE_CONFIG
#define PROMISE_TYPENAME "promise"    // promise type
#define PROMISE_VOIDPOSTFIX "_v"      // promise<void> type (promise_v)
#define PROMISE_WRAP "wrap"           // promise setter function
#define PROMISE_UNWRAP "unwrap"       // promise getter function
#define PROMISE_YIELD "yield"         // promise awaiter function
#define PROMISE_WHEN "when"           // promise callback function
#define PROMISE_EVENT "when_callback" // promise funcdef name
#define PROMISE_PENDING "pending"     // promise status checker
#define PROMISE_AWAIT                                                          \
    "co_await"                 // keyword for await (C++20 coroutines one love)
#define PROMISE_USERID 559     // promise user data identifier (any value)
#define PROMISE_NULLID -1      // empty promise type id
#define PROMISE_CALLBACKS true // allow <when> listener
#endif
#ifndef NDEBUG
#define PROMISE_ASSERT(Expression, Message) assert((Expression) && Message)
#define PROMISE_CHECK(Expression) (assert((Expression) >= 0))
#else
#define PROMISE_ASSERT(Expression, Message)
#define PROMISE_CHECK(Expression) (Expression)
#endif
#ifndef ANGELSCRIPT_H
#include <angelscript.h>
#endif

#include <atomic>
#include <cassert>
#include <cctype>
#include <condition_variable>
#include <cstring>
#include <functional>
#include <mutex>

#ifndef AS_PROMISE_NO_HELPERS
/* Helper function to cleanup the script function */
static void AsClearCallback(asIScriptFunction *Callback) {
    void *DelegateObject = Callback->GetDelegateObject();
    if (DelegateObject != nullptr)
        Callback->GetEngine()->ReleaseScriptObject(
            DelegateObject, Callback->GetDelegateObjectType());
    Callback->Release();
}
/* Helper function to check if context is awaiting on promise */
static bool IsAsyncContextPending(asIScriptContext *Context) {
    return Context->GetUserData(PROMISE_USERID) != nullptr ||
           Context->GetState() == asEXECUTION_SUSPENDED;
}
/* Helper function to check if context is awaiting on promise or active */
static bool IsAsyncContextBusy(asIScriptContext *Context) {
    return IsAsyncContextPending(Context) ||
           Context->GetState() == asEXECUTION_ACTIVE;
}
#endif

/*
        Basic promise class that can be used for non-blocking asynchronous
   operation data exchange between AngelScript and C++ and vice-versa.
*/
template <typename Executor>
class AsBasicPromise {
private:
    /* Basically used from <any> class */
    struct Dynamic {
        union {
            asINT64 Integer;
            double Number;
            void *Object;
        };

        int TypeId = PROMISE_NULLID;
    };
#if PROMISE_CALLBACKS
    /* Callbacks storage */
    struct {
        std::function<void(AsBasicPromise<Executor> *)> Native;
        asIScriptFunction *Wrapper = nullptr;
    } Callbacks;
#else
    std::condition_variable Ready;
#endif
private:
    asIScriptEngine *Engine;
    asIScriptContext *Context;
    std::atomic<uint32_t> RefCount;
    std::atomic<uint32_t> RefMark;
    std::mutex Update;
    Dynamic Value;

public:
    /* Thread safe release */
    void Release() {
        PROMISE_ASSERT(RefCount > 0, "promise is already released");
        RefMark = 0;
        if (!--RefCount) {
            ReleaseReferences(nullptr);
            this->~AsBasicPromise();
            asFreeMem((void *)this);
        }
    }
    /* Thread safe add reference */
    void AddRef() {
        PROMISE_ASSERT(RefCount < std::numeric_limits<uint32_t>::max(),
                       "too many references to this promise");
        RefMark = 0;
        ++RefCount;
    }
    /* For garbage collector to detect references */
    void EnumReferences(asIScriptEngine *OtherEngine) {
        if (Value.Object != nullptr && (Value.TypeId & asTYPEID_MASK_OBJECT)) {
            asITypeInfo *SubType = Engine->GetTypeInfoById(Value.TypeId);
            if ((SubType->GetFlags() & asOBJ_REF))
                OtherEngine->GCEnumCallback(Value.Object);
            else if ((SubType->GetFlags() & asOBJ_VALUE) &&
                     (SubType->GetFlags() & asOBJ_GC))
                Engine->ForwardGCEnumReferences(Value.Object, SubType);

            asITypeInfo *Type = OtherEngine->GetTypeInfoById(Value.TypeId);
            if (Type != nullptr)
                OtherEngine->GCEnumCallback(Type);
        }
#if PROMISE_CALLBACKS
        if (Callbacks.Wrapper != nullptr) {
            void *DelegateObject = Callbacks.Wrapper->GetDelegateObject();
            if (DelegateObject != nullptr)
                OtherEngine->GCEnumCallback(DelegateObject);
            OtherEngine->GCEnumCallback(Callbacks.Wrapper);
        }
#endif
    }
    /* For garbage collector to release references */
    void ReleaseReferences(asIScriptEngine *) {
        if (Value.TypeId & asTYPEID_MASK_OBJECT) {
            asITypeInfo *Type = Engine->GetTypeInfoById(Value.TypeId);
            Engine->ReleaseScriptObject(Value.Object, Type);
            if (Type != nullptr)
                Type->Release();
            Clean();
        }
#if PROMISE_CALLBACKS
        if (Callbacks.Wrapper != nullptr) {
            AsClearCallback(Callbacks.Wrapper);
            Callbacks.Wrapper = nullptr;
        }
#endif
    }
    /* For garbage collector to mark */
    void MarkRef() { RefMark = 0; }
    /* For garbage collector to check mark */
    bool IsRefMarked() { return RefMark == 1; }
    /* For garbage collector to check reference count */
    uint32_t GetRefCount() { return RefCount; }
    /* Receive stored type id of future value */
    int GetTypeIdOfObject() { return Value.TypeId; }
    /* Provide a native callback that should be fired when promise will be
     * settled */
    void When(std::function<void(AsBasicPromise<Executor> *)> &&NewCallback) {
#if PROMISE_CALLBACKS
        std::unique_lock<std::mutex> Unique(Update);
        Callbacks.Native = std::move(NewCallback);
        if (Callbacks.Native && !IsPending()) {
            auto Callback = std::move(Callbacks.Native);
            Unique.unlock();
            Callback(this);
        }
#else
        PROMISE_ASSERT(false,
                       "native callback binder for <when> is not allowed");
#endif
    }
    /* Provide a script callback that should be fired when promise will be
     * settled */
    void When(asIScriptFunction *NewCallback) {
#if PROMISE_CALLBACKS
        std::unique_lock<std::mutex> Unique(Update);
        if (Callbacks.Wrapper != nullptr)
            AsClearCallback(Callbacks.Wrapper);

        Callbacks.Wrapper = NewCallback;
        if (Callbacks.Wrapper != nullptr) {
            void *DelegateObject = Callbacks.Wrapper->GetDelegateObject();
            if (DelegateObject != nullptr)
                Context->GetEngine()->AddRefScriptObject(
                    DelegateObject, Callbacks.Wrapper->GetDelegateObjectType());
        }

        if (Callbacks.Wrapper != nullptr && !IsPending()) {
            Callbacks.Wrapper = nullptr;
            Unique.unlock();
            Executor()(this, Context, NewCallback);
        }
#else
        PROMISE_ASSERT(false,
                       "script callback binder for <when> is not allowed");
#endif
    }
    /*
            Thread safe store function, this is used as promise resolver
       function, will either only store the result or store result and execute
       callback that will resume suspended context and then release the promise
       (won't destroy)
    */
    void Store(void *RefPointer, int RefTypeId) {
        std::unique_lock<std::mutex> Unique(Update);
        PROMISE_ASSERT(Value.TypeId == PROMISE_NULLID,
                       "promise should be settled only once");
        PROMISE_ASSERT(RefPointer != nullptr || RefTypeId == asTYPEID_VOID,
                       "input pointer should not be null");
        PROMISE_ASSERT(Engine != nullptr,
                       "promise is malformed (engine is null)");
        PROMISE_ASSERT(Context != nullptr,
                       "promise is malformed (context is null)");

        if (Value.TypeId != PROMISE_NULLID) {
            asIScriptContext *ThisContext = asGetActiveContext();
            if (!ThisContext)
                ThisContext = Context;
            ThisContext->SetException("promise is already fulfilled");
            return;
        }

        if ((RefTypeId & asTYPEID_MASK_OBJECT)) {
            asITypeInfo *Type = Engine->GetTypeInfoById(RefTypeId);
            if (Type != nullptr)
                Type->AddRef();
        }

        Value.TypeId = RefTypeId;
        if (Value.TypeId & asTYPEID_OBJHANDLE) {
            Value.Object = *(void **)RefPointer;
        } else if (Value.TypeId & asTYPEID_MASK_OBJECT) {
            Value.Object = Engine->CreateScriptObjectCopy(
                RefPointer, Engine->GetTypeInfoById(Value.TypeId));
        } else if (RefPointer != nullptr) {
            Value.Integer = 0;
            int Size = Engine->GetSizeOfPrimitiveType(Value.TypeId);
            memcpy(&Value.Integer, RefPointer, Size);
        }

        bool SuspendOwned =
            Context->GetUserData(PROMISE_USERID) == (void *)this;
        if (SuspendOwned)
            Context->SetUserData(nullptr, PROMISE_USERID);

        bool WantsResume =
            (Context->GetState() == asEXECUTION_SUSPENDED && SuspendOwned);
#if PROMISE_CALLBACKS
        auto NativeCallback = std::move(Callbacks.Native);
        auto *WrapperCallback = Callbacks.Wrapper;
        Callbacks.Wrapper = nullptr;
        Unique.unlock();

        if (NativeCallback != nullptr)
            NativeCallback(this);

        if (WrapperCallback != nullptr)
            Executor()(this, Context, WrapperCallback);
#else
        Ready.notify_all();
        Unique.unlock();
#endif
        if (WantsResume)
            Executor()(this, Context);
    }
    /* Thread safe store function, a little easier for C++ usage */
    void Store(void *RefPointer, const char *TypeName) {
        PROMISE_ASSERT(Engine != nullptr,
                       "promise is malformed (engine is null)");
        PROMISE_ASSERT(TypeName != nullptr, "typename should not be null");
        Store(RefPointer, Engine->GetTypeIdByDecl(TypeName));
    }
    /* Thread safe store function, for promise<void> */
    void StoreVoid() { Store(nullptr, asTYPEID_VOID); }
    /* Thread safe retrieve function, non-blocking try-retrieve future value */
    bool Retrieve(void *RefPointer, int RefTypeId) {
        PROMISE_ASSERT(Engine != nullptr,
                       "promise is malformed (engine is null)");
        PROMISE_ASSERT(RefPointer != nullptr,
                       "output pointer should not be null");
        if (Value.TypeId == PROMISE_NULLID)
            return false;

        if (RefTypeId & asTYPEID_OBJHANDLE) {
            if ((Value.TypeId & asTYPEID_MASK_OBJECT)) {
                if ((Value.TypeId & asTYPEID_HANDLETOCONST) &&
                    !(RefTypeId & asTYPEID_HANDLETOCONST))
                    return false;

                Engine->RefCastObject(Value.Object,
                                      Engine->GetTypeInfoById(Value.TypeId),
                                      Engine->GetTypeInfoById(RefTypeId),
                                      reinterpret_cast<void **>(RefPointer));
                if (*(asPWORD *)RefPointer == 0)
                    return false;

                return true;
            }
        } else if (RefTypeId & asTYPEID_MASK_OBJECT) {
            if (Value.TypeId == RefTypeId) {
                Engine->AssignScriptObject(
                    RefPointer, Value.Object,
                    Engine->GetTypeInfoById(Value.TypeId));
                return true;
            }
        } else {
            int Size1 = Engine->GetSizeOfPrimitiveType(Value.TypeId);
            int Size2 = Engine->GetSizeOfPrimitiveType(RefTypeId);
            PROMISE_ASSERT(Size1 == Size2,
                           "cannot map incompatible primitive types");

            if (Size1 == Size2) {
                memcpy(RefPointer, &Value.Integer, Size1);
                return true;
            }
        }

        return false;
    }
    /* Thread safe retrieve function, also non-blocking, another syntax is used
     */
    void *Retrieve() {
        RetrieveVoid();
        if (Value.TypeId == PROMISE_NULLID)
            return nullptr;

        if (Value.TypeId & asTYPEID_OBJHANDLE)
            return &Value.Object;
        else if (Value.TypeId & asTYPEID_MASK_OBJECT)
            return Value.Object;
        else if (Value.TypeId <= asTYPEID_DOUBLE ||
                 Value.TypeId & asTYPEID_MASK_SEQNBR)
            return &Value.Integer;

        return nullptr;
    }
    /* Thread safe retrieve function */
    void RetrieveVoid() {
        std::unique_lock<std::mutex> Unique(Update);
        asIScriptContext *ThisContext = asGetActiveContext();
        if (ThisContext != nullptr && IsPending())
            ThisContext->SetException("promise is still pending");
    }
    /* Can be used to check if promise is still pending */
    bool IsPending() { return Value.TypeId == PROMISE_NULLID; }
    /*
            This function should be called before retrieving the value
            from promise, it will either suspend current context and add
            reference to this promise if it is still pending or do nothing
            if promise was already settled
    */
    AsBasicPromise *YieldIf() {
        std::unique_lock<std::mutex> Unique(Update);
        if (Value.TypeId == PROMISE_NULLID && Context != nullptr &&
            Context->Suspend() >= 0)
            Context->SetUserData(this, PROMISE_USERID);

        return this;
    }
    /*
            This function can be used to await for promise
            within C++ code (blocking style)
    */
    AsBasicPromise *WaitIf() {
        if (!IsPending())
            return this;

        std::unique_lock<std::mutex> Unique(Update);
#if PROMISE_CALLBACKS
        if (IsPending()) {
            std::condition_variable Ready;
            Callbacks.Native = [&Ready](AsBasicPromise<Executor> *) {
                Ready.notify_all();
            };
            Ready.wait(Unique, [this]() { return !IsPending(); });
        }
#else
        if (IsPending())
            Ready.wait(Unique, [this]() { return !IsPending(); });
#endif
        return this;
    }

private:
    /*
            Construct a promise, notify GC, set value to none,
            grab a reference to script context
    */
    AsBasicPromise(asIScriptContext *NewContext) noexcept
        : Engine(nullptr), Context(NewContext), RefCount(1), RefMark(0) {
        PROMISE_ASSERT(Context != nullptr, "context should not be null");
        Engine = Context->GetEngine();
        Engine->NotifyGarbageCollectorOfNewObject(
            this, Engine->GetTypeInfoByName(PROMISE_TYPENAME));
        Clean();
    }
    /* Reset value to none */
    void Clean() {
        memset(&Value, 0, sizeof(Value));
        Value.TypeId = PROMISE_NULLID;
    }

public:
    /* AsBasicPromise creation function, for use within C++ */
    static AsBasicPromise *
    Create(asIScriptContext *Context = asGetActiveContext()) {
        return new (asAllocMem(sizeof(AsBasicPromise))) AsBasicPromise(Context);
    }
    /* AsBasicPromise creation function, for use within AngelScript */
    static AsBasicPromise *CreateFactory(void *_Ref, int TypeId) {
        AsBasicPromise *Future = new (asAllocMem(sizeof(AsBasicPromise)))
            AsBasicPromise(asGetActiveContext());
        if (TypeId != asTYPEID_VOID)
            Future->Store(_Ref, TypeId);

        return Future;
    }
    /* AsBasicPromise creation function, for use within AngelScript (void
     * promise) */
    static AsBasicPromise *CreateFactoryVoid(void *_Ref, int TypeId) {
        return Create();
    }
    /*
            Interface registration, note: promise<void> is not supported,
            instead use promise_v when internal datatype is not intended,
            promise will be an object handle with GC behaviours, default
            constructed promise will be pending otherwise early settled
    */
    static void Register(asIScriptEngine *Engine) {
        using Type = AsBasicPromise<Executor>;
        PROMISE_ASSERT(Engine != nullptr, "script engine should not be null");
        PROMISE_CHECK(
            Engine->RegisterObjectType(PROMISE_TYPENAME "<class T>", 0,
                                       asOBJ_REF | asOBJ_GC | asOBJ_TEMPLATE));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME "<T>", asBEHAVE_FACTORY,
            PROMISE_TYPENAME "<T>@ f(?&in)", asFUNCTION(Type::CreateFactory),
            asCALL_CDECL));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME "<T>", asBEHAVE_TEMPLATE_CALLBACK,
            "bool f(int&in, bool&out)", asFUNCTION(Type::TemplateCallback),
            asCALL_CDECL));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME "<T>", asBEHAVE_ADDREF, "void f()",
            asMETHOD(Type, AddRef), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME "<T>", asBEHAVE_RELEASE, "void f()",
            asMETHOD(Type, Release), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME "<T>", asBEHAVE_SETGCFLAG, "void f()",
            asMETHOD(Type, MarkRef), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME "<T>", asBEHAVE_GETGCFLAG, "bool f()",
            asMETHOD(Type, IsRefMarked), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME "<T>", asBEHAVE_GETREFCOUNT, "int f()",
            asMETHOD(Type, GetRefCount), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME "<T>", asBEHAVE_ENUMREFS, "void f(int&in)",
            asMETHOD(Type, EnumReferences), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME "<T>", asBEHAVE_RELEASEREFS, "void f(int&in)",
            asMETHOD(Type, ReleaseReferences), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectMethod(
            PROMISE_TYPENAME "<T>", "void " PROMISE_WRAP "(?&in)",
            asMETHODPR(Type, Store, (void *, int), void), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectMethod(
            PROMISE_TYPENAME "<T>", "T& " PROMISE_UNWRAP "()",
            asMETHODPR(Type, Retrieve, (), void *), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectMethod(
            PROMISE_TYPENAME "<T>",
            PROMISE_TYPENAME "<T>@+ " PROMISE_YIELD "()",
            asMETHOD(Type, YieldIf), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectMethod(
            PROMISE_TYPENAME "<T>", "bool " PROMISE_PENDING "()",
            asMETHOD(Type, IsPending), asCALL_THISCALL));
#if PROMISE_CALLBACKS
        PROMISE_CHECK(Engine->RegisterFuncdef(
            "void " PROMISE_TYPENAME "<T>::" PROMISE_EVENT "(promise<T>@+)"));
        PROMISE_CHECK(Engine->RegisterObjectMethod(
            PROMISE_TYPENAME "<T>", "void " PROMISE_WHEN "(" PROMISE_EVENT "@)",
            asMETHODPR(Type, When, (asIScriptFunction *), void),
            asCALL_THISCALL));
#endif
        PROMISE_CHECK(Engine->RegisterObjectType(
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX, 0, asOBJ_REF | asOBJ_GC));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX, asBEHAVE_FACTORY,
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX "@ f()",
            asFUNCTION(Type::CreateFactoryVoid), asCALL_CDECL));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX, asBEHAVE_ADDREF, "void f()",
            asMETHOD(Type, AddRef), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX, asBEHAVE_RELEASE, "void f()",
            asMETHOD(Type, Release), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX, asBEHAVE_SETGCFLAG,
            "void f()", asMETHOD(Type, MarkRef), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX, asBEHAVE_GETGCFLAG,
            "bool f()", asMETHOD(Type, IsRefMarked), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX, asBEHAVE_GETREFCOUNT,
            "int f()", asMETHOD(Type, GetRefCount), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX, asBEHAVE_ENUMREFS,
            "void f(int&in)", asMETHOD(Type, EnumReferences), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectBehaviour(
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX, asBEHAVE_RELEASEREFS,
            "void f(int&in)", asMETHOD(Type, ReleaseReferences),
            asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectMethod(
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX, "void " PROMISE_WRAP "()",
            asMETHODPR(Type, StoreVoid, (), void), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectMethod(
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX, "void " PROMISE_UNWRAP "()",
            asMETHODPR(Type, RetrieveVoid, (), void), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectMethod(
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX,
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX "@+ " PROMISE_YIELD "()",
            asMETHOD(Type, YieldIf), asCALL_THISCALL));
        PROMISE_CHECK(Engine->RegisterObjectMethod(
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX, "bool " PROMISE_PENDING "()",
            asMETHOD(Type, IsPending), asCALL_THISCALL));
#if PROMISE_CALLBACKS
        PROMISE_CHECK(
            Engine->RegisterFuncdef("void " PROMISE_TYPENAME PROMISE_VOIDPOSTFIX
                                    "::" PROMISE_EVENT "(promise_v@+)"));
        PROMISE_CHECK(Engine->RegisterObjectMethod(
            PROMISE_TYPENAME PROMISE_VOIDPOSTFIX,
            "void " PROMISE_WHEN "(" PROMISE_EVENT "@)",
            asMETHODPR(Type, When, (asIScriptFunction *), void),
            asCALL_THISCALL));
#endif
    }

private:
    /* Template callback function for compiler, copy-paste from <array> class */
    static bool TemplateCallback(asITypeInfo *Info, bool &DontGarbageCollect) {
        int TypeId = Info->GetSubTypeId();
        if (TypeId == asTYPEID_VOID)
            return false;

        if ((TypeId & asTYPEID_MASK_OBJECT) && !(TypeId & asTYPEID_OBJHANDLE)) {
            asIScriptEngine *Engine = Info->GetEngine();
            asITypeInfo *SubType = Engine->GetTypeInfoById(TypeId);
            asDWORD Flags = SubType->GetFlags();

            if ((Flags & asOBJ_VALUE) && !(Flags & asOBJ_POD)) {
                bool Found = false;
                for (size_t i = 0; i < SubType->GetBehaviourCount(); i++) {
                    asEBehaviours Behaviour;
                    asIScriptFunction *Func =
                        SubType->GetBehaviourByIndex((int)i, &Behaviour);
                    if (Behaviour != asBEHAVE_CONSTRUCT)
                        continue;

                    if (Func->GetParamCount() == 0) {
                        Found = true;
                        break;
                    }
                }

                if (!Found) {
                    Engine->WriteMessage(
                        PROMISE_TYPENAME, 0, 0, asMSGTYPE_ERROR,
                        "The subtype has no default constructor");
                    return false;
                }
            } else if ((Flags & asOBJ_REF)) {
                bool Found = false;
                if (!Engine->GetEngineProperty(
                        asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE)) {
                    for (size_t i = 0; i < SubType->GetFactoryCount(); i++) {
                        asIScriptFunction *Function =
                            SubType->GetFactoryByIndex((int)i);
                        if (Function->GetParamCount() == 0) {
                            Found = true;
                            break;
                        }
                    }
                }

                if (!Found) {
                    Engine->WriteMessage(PROMISE_TYPENAME, 0, 0,
                                         asMSGTYPE_ERROR,
                                         "The subtype has no default factory");
                    return false;
                }
            }

            if (!(Flags & asOBJ_GC))
                DontGarbageCollect = true;
        } else if (!(TypeId & asTYPEID_OBJHANDLE)) {
            DontGarbageCollect = true;
        } else {
            asITypeInfo *SubType = Info->GetEngine()->GetTypeInfoById(TypeId);
            asDWORD Flags = SubType->GetFlags();

            if (!(Flags & asOBJ_GC)) {
                if ((Flags & asOBJ_SCRIPT_OBJECT)) {
                    if ((Flags & asOBJ_NOINHERIT))
                        DontGarbageCollect = true;
                } else
                    DontGarbageCollect = true;
            }
        }

        return true;
    }
};

#ifndef AS_PROMISE_NO_GENERATOR
/*
        A fast and minimal code generator function for custom syntax of promise
   class, it takes raw code input with <await> syntax and returns code that use
   un-wrappers.
*/
static char *
AsGeneratePromiseEntrypoints(const char *Text, size_t *InoutTextSize,
                             void *(*AllocateMemory)(size_t) = &asAllocMem,
                             void (*FreeMemory)(void *) = &asFreeMem) {
    PROMISE_ASSERT(Text != nullptr, "script code should not be null");
    PROMISE_ASSERT(InoutTextSize != nullptr,
                   "script code size should not be null");
    PROMISE_ASSERT(AllocateMemory != nullptr,
                   "memory allocation function should not be null");
    PROMISE_ASSERT(FreeMemory != nullptr,
                   "memory deallocation function should not be null");
    const char Match[] = PROMISE_AWAIT " ";
    size_t Size = *InoutTextSize;
    char *Code = (char *)AllocateMemory(Size + 1);
    size_t MatchSize = sizeof(Match) - 1;
    size_t Offset = 0;
    memcpy(Code, Text, Size);
    Code[Size] = '\0';

    while (Offset < Size) {
        char U = Code[Offset];
        if (U == '/' && Offset + 1 < Size &&
            (Code[Offset + 1] == '/' || Code[Offset + 1] == '*')) {
            if (Code[++Offset] == '*') {
                while (Offset + 1 < Size) {
                    char N = Code[Offset++];
                    if (N == '*' && Code[Offset++] == '/')
                        break;
                }
            } else {
                while (Offset < Size) {
                    char N = Code[Offset++];
                    if (N == '\r' || N == '\n')
                        break;
                }
            }

            continue;
        } else if (U == '\"' || U == '\'') {
            ++Offset;
            while (Offset < Size) {
                size_t LastOffset = Offset++;
                if (Code[LastOffset] != U)
                    continue;

                if (LastOffset < 1 || Code[LastOffset - 1] != '\\')
                    break;

                if (LastOffset > 1 && Code[LastOffset - 2] == '\\')
                    break;
            }

            continue;
        } else if (Size - Offset < MatchSize ||
                   memcmp(Code + Offset, Match, MatchSize) != 0) {
            ++Offset;
            continue;
        }

        size_t Start = Offset + MatchSize;
        while (Start < Size) {
            if (!isspace((uint8_t)Code[Start]))
                break;
            ++Start;
        }

        int32_t Brackets = 0;
        size_t End = Start;
        while (End < Size) {
            char V = Code[End];
            if (V == ')') {
                if (--Brackets < 0)
                    break;
            } else if (V == '\"' || V == '\'') {
                ++End;
                while (End < Size) {
                    size_t LastEnd = End++;
                    if (Code[LastEnd] != V)
                        continue;

                    if (LastEnd < 1 || Code[LastEnd - 1] != '\\')
                        break;

                    if (LastEnd > 1 && Code[LastEnd - 2] == '\\')
                        break;
                }
                --End;
            } else if (V == ';')
                break;
            else if (V == '(')
                ++Brackets;
            End++;
        }

        if (End == Start) {
            Offset = End;
            continue;
        }

        const char Generator[] = ")." PROMISE_YIELD "()." PROMISE_UNWRAP "()";
        char *Left = Code, *Middle = Code + Start, *Right = Code + End;
        size_t LeftSize = Offset;
        size_t MiddleSize = End - Start;
        size_t GeneratorSize = sizeof(Generator) - 1;
        size_t RightSize = Size - Offset;
        size_t SubstringSize =
            LeftSize + MiddleSize + GeneratorSize + RightSize;
        size_t PrevSize = End - Offset;
        size_t NewSize = MiddleSize + GeneratorSize + 1;

        char *Substring = (char *)AllocateMemory(SubstringSize + 1);
        memcpy(Substring, Left, LeftSize);
        memcpy(Substring + LeftSize, "(", 1);
        memcpy(Substring + LeftSize + 1, Middle, MiddleSize);
        memcpy(Substring + LeftSize + 1 + MiddleSize, Generator, GeneratorSize);
        memcpy(Substring + LeftSize + 1 + MiddleSize + GeneratorSize, Right,
               RightSize);
        Substring[SubstringSize] = '\0';
        FreeMemory(Code);

        size_t NestedSize = Offset + MiddleSize;
        char Prev = Substring[NestedSize];
        Substring[NestedSize] = '\0';
        bool IsRecursive =
            (strstr(Substring + Offset, PROMISE_AWAIT) != nullptr);
        Substring[NestedSize] = Prev;

        Code = Substring;
        Size -= PrevSize;
        Size += NewSize;
        if (!IsRecursive)
            Offset += MiddleSize + GeneratorSize;
    }

    *InoutTextSize = Size;
    return Code;
}
#endif
#ifndef AS_PROMISE_NO_DEFAULTS
/*
        Basic promise settle executor, will
        resume context at thread that has
        settled the promise.
*/
struct AsDirectExecutor {
    /* Called after suspend, this method will probably be inlined anyways */
    inline void operator()(AsBasicPromise<AsDirectExecutor> *Promise,
                           asIScriptContext *Context) {
        /*
                Context should be suspended at this moment but if for
                some reason it went active between function calls
           (multithreaded) then user is responsible for this task to be properly
           queued or exception should thrown if possible
        */
        Context->Execute();
    }
    /* Called after suspend, for callback execution */
    inline void operator()(AsBasicPromise<AsDirectExecutor> *Promise,
                           asIScriptContext *Context,
                           asIScriptFunction *Callback) {
        /*
                Callback control flow:
                        If main context is active: execute nested call on
           current context If main context is suspended: execute on newly
           created context Otherwise: execute on current context
        */
        asEContextState State = Context->GetState();
        auto Execute = [&Promise, &Context, &Callback]() {
            PROMISE_CHECK(Context->Prepare(Callback));
            PROMISE_CHECK(Context->SetArgObject(0, Promise));
            Context->Execute();
        };
        if (State == asEXECUTION_ACTIVE) {
            PROMISE_CHECK(Context->PushState());
            Execute();
            PROMISE_CHECK(Context->PopState());
        } else if (State == asEXECUTION_SUSPENDED) {
            asIScriptEngine *Engine = Context->GetEngine();
            Context = Engine->RequestContext();
            Execute();
            Engine->ReturnContext(Context);
        } else
            Execute();

        /* Cleanup referenced resources */
        AsClearCallback(Callback);
    }
};

/*
        Executor that notifies prepared context
        whenever promise settles.
*/
struct AsReactiveExecutor {
    typedef std::function<void(AsBasicPromise<AsReactiveExecutor> *,
                               asIScriptFunction *)>
        ReactiveCallback;

    /* Called after suspend, this method will probably be inlined anyways */
    inline void operator()(AsBasicPromise<AsReactiveExecutor> *Promise,
                           asIScriptContext *Context) {
        ReactiveCallback &Execute = GetCallback(Context);
        Execute(Promise, nullptr);
    }
    /* Called after suspend, for callback execution */
    inline void operator()(AsBasicPromise<AsReactiveExecutor> *Promise,
                           asIScriptContext *Context,
                           asIScriptFunction *Callback) {
        ReactiveCallback &Execute = GetCallback(Context);
        Execute(Promise, Callback);
    }
    static void SetCallback(asIScriptContext *Context,
                            ReactiveCallback *Callback) {
        PROMISE_ASSERT(!Callback || *Callback, "invalid reactive callback");
        Context->SetUserData((void *)Callback, 1022);
    }
    static ReactiveCallback &GetCallback(asIScriptContext *Context) {
        ReactiveCallback *Callback =
            (ReactiveCallback *)Context->GetUserData(1022);
        PROMISE_ASSERT(Callback != nullptr,
                       "missing reactive callback on context");
        return *Callback;
    }
};

using AsDirectPromise = AsBasicPromise<AsDirectExecutor>;
using AsReactivePromise = AsBasicPromise<AsReactiveExecutor>;
#endif
#endif
