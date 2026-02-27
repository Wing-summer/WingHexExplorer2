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

#ifndef ANGELSCRIPTHELPER_H
#define ANGELSCRIPTHELPER_H

#include "AngelScript/sdk/add_on/scriptarray/scriptarray.h"
#include "define.h"

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
inline void registerAngelType(asIScriptEngine *engine, const char *enumName) {
    static_assert(sizeof(T) == sizeof(int), "sizeof(T) != sizeof(int)");
    auto e = QMetaEnum::fromType<T>();
    auto r = engine->RegisterEnum(enumName);
    Q_ASSERT(r >= 0);

    for (int i = 0; i < e.keyCount(); ++i) {
        r = engine->RegisterEnumValue(enumName, e.key(i), e.value(i));
        Q_ASSERT(r >= 0);
    }
}

template <typename T>
inline CScriptArray *retarrayWrapper(const T &ret, asITypeInfo *t) {
    if (!t) {
        return nullptr;
    }

    // context, which can be used to obtain a pointer to the
    // engine.
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();
        auto array = CScriptArray::Create(t, ret.size());
        for (decltype(ret.size()) i = 0; i < ret.size(); ++i) {
            auto v = ret.at(i);
            array->SetValue(i, &v);
        }
        return array;
    } else {
        return nullptr;
    }
}

template <typename Func>
inline CScriptArray *retArrayWrapperFunction(Func &&getRet, asITypeInfo *t) {
    return retarrayWrapper(getRet(), t);
}

inline CScriptArray *byteArrayWrapper(const QByteArray &array) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        return retarrayWrapper(
            array, static_cast<asITypeInfo *>(ctx->GetEngine()->GetUserData(
                       AsUserDataType::UserData_ByteArrayTypeInfo)));
    } else {
        return nullptr;
    }
}

template <typename Func>
inline CScriptArray *byteArrayWrapperFunction(Func &&getRet) {
    return byteArrayWrapper(getRet());
}

inline CScriptArray *stringListWrapper(const QStringList &array) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        return retarrayWrapper(
            array, static_cast<asITypeInfo *>(ctx->GetEngine()->GetUserData(
                       AsUserDataType::UserData_StringListTypeInfo)));
    } else {
        return nullptr;
    }
}

inline QStringList cArray2QStringList(const CScriptArray &array,
                                      bool *ok = nullptr) {
    // If called from the script, there will always be an active
    // context, which can be used to obtain a pointer to the engine.
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        auto engine = ctx->GetEngine();
        Q_ASSERT(engine);
        auto type = static_cast<asITypeInfo *>(
            engine->GetUserData(AsUserDataType::UserData_StringTypeInfo));
        auto stringID = type->GetTypeId();
        Q_ASSERT(stringID >= 0);
        bool b = array.GetElementTypeId() == stringID;
        if (ok) {
            *ok = b;
        }
        if (!b) {
            return {};
        }

        QStringList buffer;
        buffer.reserve(array.GetSize());
        for (asUINT i = 0; i < array.GetSize(); ++i) {
            auto item = reinterpret_cast<const QString *>(array.At(i));
            buffer.append(*item);
        }
        return buffer;
    }
    return {};
}

inline QByteArray cArray2ByteArray(const CScriptArray &array,
                                   bool *ok = nullptr) {
    // If called from the script, there will always be an active
    // context, which can be used to obtain a pointer to the engine.
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        constexpr auto byteID = asTYPEID_UINT8;
        bool b = array.GetElementTypeId() == byteID;
        if (ok) {
            *ok = b;
        }
        if (!b) {
            return {};
        }

        auto len = array.GetSize();

        QByteArray buffer;
        buffer.resize(len);
        array.AddRef();

        std::memcpy(buffer.data(),
                    const_cast<CScriptArray &>(array).GetBuffer(), len);

        array.Release();
        return buffer;
    }
    return {};
}

#endif // ANGELSCRIPTHELPER_H
