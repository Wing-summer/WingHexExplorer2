#ifndef ANGELSCRIPTHELPER_H
#define ANGELSCRIPTHELPER_H

#include "AngelScript/add_on/scriptarray/scriptarray.h"
#include <QMetaEnum>
#include <QtGlobal>

#if Q_PROCESSOR_WORDSIZE == 4
#define QSIZETYPE_WRAP(decl) "int " decl
#define QPTR_WRAP(decl) "uint " decl
#define QPTR "uint"
#define QSIZETYPE "int"
#elif Q_PROCESSOR_WORDSIZE == 8
#define QSIZETYPE_WRAP(decl) "int64 " decl
#define QPTR_WRAP(decl) "uint64 " decl
#define QPTR "uint64"
#define QSIZETYPE "int64"
#else
#error "Processor with unexpected word size"
#endif

// a helper function to register Qt enums to AngelScript
template <typename T>
void registerAngelType(asIScriptEngine *engine, const char *enumName) {
    auto e = QMetaEnum::fromType<T>();
    auto r = engine->RegisterEnum(enumName);
    Q_ASSERT(r >= 0);

    for (int i = 0; i < e.keyCount(); ++i) {
        r = engine->RegisterEnumValue(enumName, e.key(i), e.value(i));
        Q_ASSERT(r >= 0);
    }
}

template <typename Func>
CScriptArray *retarrayWrapperFunction(Func &&getRet, const char *angelType) {
    // context, which can be used to obtain a pointer to the
    // engine.
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();

        auto ret = getRet();

        // The script array needs to know its type to properly handle the
        // elements. Note that the object type should be cached to avoid
        // performance issues if the function is called frequently.
        asITypeInfo *t = engine->GetTypeInfoByDecl(angelType);
        Q_ASSERT(t);

        auto array = CScriptArray::Create(t, ret.size());
        for (typename decltype(ret)::size_type i = 0; i < ret.size(); ++i) {
            auto v = ret.at(i);
            array->SetValue(i, &v);
        }
        return array;
    } else {
        return nullptr;
    }
}

template <typename Func>
CScriptArray *byteArrayWrapperFunction(Func &&fn) {
    return retarrayWrapperFunction(fn, "array<byte>");
}

#endif // ANGELSCRIPTHELPER_H
