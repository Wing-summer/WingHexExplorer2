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

#include "scriptcrypto.h"

#include "class/angelscripthelper.h"
#include "class/crcalgorithm.h"
#include "class/cryptographichash.h"

static CScriptArray *crypto_hash(const CScriptArray &data,
                                 CryptographicHash::Algorithm method) {
    bool ok = false;
    auto bab = cArray2ByteArray(data, &ok);
    if (!ok) {
        return nullptr;
    }
    return byteArrayWrapperFunction(
        [&]() -> QByteArray { return CryptographicHash::hash(bab, method); });
}

static quint32 crypto_crc32(const CScriptArray &data) {
    bool ok = false;
    auto bab = cArray2ByteArray(data, &ok);
    if (!ok) {
        return 0;
    }
    return CrcAlgorithm::__hash(bab);
}

static quint64 crypto_crc64(const CScriptArray &data) {
    bool ok = false;
    auto bab = cArray2ByteArray(data, &ok);
    if (!ok) {
        return 0;
    }
    return CrcAlgorithm::__hash64(bab);
}

static quint16 crypto_crc16(const CScriptArray &data) {
    bool ok = false;
    auto bab = cArray2ByteArray(data, &ok);
    if (!ok) {
        return 0;
    }
    return CrcAlgorithm::__hash16(bab);
}

void RegisterScriptCrypto(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("crypto");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    registerAngelType<CryptographicHash::Algorithm>(engine, "algorithm");

    r = engine->RegisterGlobalFunction(
        "array<byte>@ hash(const array<byte> &in data, "
        "crypto::algorithm method)",
        asFUNCTION(crypto_hash), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "uint16 crc16(const array<byte> &in data)", asFUNCTION(crypto_crc16),
        asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "uint32 crc32(const array<byte> &in data)", asFUNCTION(crypto_crc32),
        asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "uint64 crc64(const array<byte> &in data)", asFUNCTION(crypto_crc64),
        asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    engine->SetDefaultNamespace("");
}
