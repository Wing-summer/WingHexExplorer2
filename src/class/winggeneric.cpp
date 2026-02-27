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

#include "winggeneric.h"

#include "angelscript.h"

WingGeneric::WingGeneric(asIScriptGeneric *gen) : _gen(gen) {}

void *WingGeneric::auxiliary() const { return _gen->GetAuxiliary(); }

void *WingGeneric::object() const { return _gen->GetObject(); }

int WingGeneric::objectTypeId() const { return _gen->GetObjectTypeId(); }

int WingGeneric::argCount() const { return _gen->GetArgCount(); }

int WingGeneric::argTypeId(uint arg, quint32 *flags) const {
    static_assert(sizeof(quint32) == sizeof(asDWORD),
                  "asDWORD is same with quint32");
    return _gen->GetArgTypeId(arg, reinterpret_cast<asDWORD *>(flags));
}

QString WingGeneric::argTypeName(uint arg, quint32 *flags) {
    auto e = _gen->GetEngine();
    auto str = e->GetTypeDeclaration(argTypeId(arg, flags));
    if (str) {
        return QString::fromUtf8(str);
    } else {
        return {};
    }
}

uchar WingGeneric::argByte(uint arg) { return _gen->GetArgByte(arg); }

quint16 WingGeneric::argWord(uint arg) { return _gen->GetArgWord(arg); }

quint32 WingGeneric::argDWord(uint arg) { return _gen->GetArgDWord(arg); }

quint64 WingGeneric::argQWord(uint arg) { return _gen->GetArgQWord(arg); }

float WingGeneric::argFloat(uint arg) { return _gen->GetArgFloat(arg); }

double WingGeneric::argDouble(uint arg) { return _gen->GetArgDouble(arg); }

void *WingGeneric::argAddress(uint arg) { return _gen->GetArgAddress(arg); }

void *WingGeneric::argObject(uint arg) { return _gen->GetArgObject(arg); }

void *WingGeneric::addressOfArg(uint arg) { return _gen->GetAddressOfArg(arg); }

int WingGeneric::returnTypeId(quint32 *flags) const {
    static_assert(sizeof(quint32) == sizeof(asDWORD),
                  "asDWORD is same with quint32");
    return _gen->GetReturnTypeId(reinterpret_cast<asDWORD *>(flags));
}

WingHex::asRetCodes WingGeneric::setReturnByte(uchar val) {
    return WingHex::asRetCodes(_gen->SetReturnByte(val));
}

WingHex::asRetCodes WingGeneric::setReturnWord(quint16 val) {
    return WingHex::asRetCodes(_gen->SetReturnWord(val));
}

WingHex::asRetCodes WingGeneric::setReturnDWord(quint32 val) {
    return WingHex::asRetCodes(_gen->SetReturnDWord(val));
}

WingHex::asRetCodes WingGeneric::setReturnQWord(quint64 val) {
    return WingHex::asRetCodes(_gen->SetReturnQWord(val));
}

WingHex::asRetCodes WingGeneric::setReturnFloat(float val) {
    return WingHex::asRetCodes(_gen->SetReturnFloat(val));
}

WingHex::asRetCodes WingGeneric::setReturnDouble(double val) {
    return WingHex::asRetCodes(_gen->SetReturnDouble(val));
}

WingHex::asRetCodes WingGeneric::setReturnAddress(void *addr) {
    return WingHex::asRetCodes(_gen->SetReturnAddress(addr));
}

WingHex::asRetCodes WingGeneric::setReturnObject(void *obj) {
    return WingHex::asRetCodes(_gen->SetReturnObject(obj));
}

void *WingGeneric::addressOfReturnLocation() {
    return _gen->GetAddressOfReturnLocation();
}
