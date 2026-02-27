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
#include "class/cryptographichash.h"
#include "define.h"

static CScriptArray *crypto_hash(const CScriptArray &data,
                                 CryptographicHash::Algorithm method) {
    bool ok = false;
    auto bab = cArray2ByteArray(data, &ok);
    if (!ok) {
        return nullptr;
    }
    return byteArrayWrapper(CryptographicHash::hash(bab, method));
}

void RegisterScriptCrypto(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("crypto");
    ASSERT(r >= 0);

    registerAngelType<CryptographicHash::Algorithm>(engine, "algorithm");

    r = engine->RegisterGlobalFunction(
        "array<byte>@ hash(const array<byte> &in data, "
        "crypto::algorithm method)",
        asFUNCTION(crypto_hash), asCALL_CDECL);
    ASSERT(r >= 0);

    engine->SetDefaultNamespace("");
}
