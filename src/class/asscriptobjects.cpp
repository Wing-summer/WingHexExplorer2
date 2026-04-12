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

#include "asscriptobjects.h"

#include "AngelScript/sdk/add_on/scriptarray/scriptarray.h"
#include "AngelScript/sdk/add_on/scriptgrid/scriptgrid.h"
#include "class/angelscripthelper.h"
#include "scriptaddon/scriptany.h"
#include "scriptaddon/scriptqdictionary.h"

ScriptDictionary::ScriptDictionary(CScriptDictionary *data)
    : WingHex::ASScriptDictionary(data) {}

bool ScriptDictionary::set(const QString &key, void *value,
                           std::string_view type) {
    auto ctx = asGetActiveContext();
    if (ctx) {
        if (isValid()) {
            int typeId;
            if (typeIDCache.contains(type)) {
                typeId = typeIDCache.object(type);
            } else {
                typeId = ctx->GetEngine()->GetTypeIdByDecl(type.data());
                if (typeId == asINVALID_TYPE) {
                    return false;
                }
                typeIDCache.insert(type, typeId);
            }
            _data->Set(key, value, typeId);
            return true;
        }
    }
    return false;
}

bool ScriptDictionary::set(const QString &key, const quint64 &value) {
    if (isValid()) {
        _data->Set(key, asQWORD(value));
        return true;
    }
    return false;
}

bool ScriptDictionary::set(const QString &key, const qint64 &value) {
    if (isValid()) {
        _data->Set(key, asINT64(value));
        return true;
    }
    return false;
}

bool ScriptDictionary::set(const QString &key, const double &value) {
    if (isValid()) {
        _data->Set(key, value);
        return true;
    }
    return false;
}

bool ScriptDictionary::get(const QString &key, void *value,
                           std::string_view type) const {
    auto ctx = asGetActiveContext();
    if (ctx) {
        if (isValid()) {
            int typeId;
            if (typeIDCache.contains(type)) {
                typeId = typeIDCache.object(type);
            } else {
                typeId = ctx->GetEngine()->GetTypeIdByDecl(type.data());
                if (typeId == asINVALID_TYPE) {
                    return false;
                }
                typeIDCache.insert(type, typeId);
            }
            return _data->Get(key, value, typeId);
        }
    }
    return false;
}

bool ScriptDictionary::get(const QString &key, quint64 &value) const {
    static_assert(sizeof(quint64) == sizeof(asQWORD) &&
                  std::is_unsigned_v<quint64> && std::is_unsigned_v<asQWORD>);
    if (isValid()) {
        return _data->Get(key, reinterpret_cast<asQWORD &>(value));
    }
    return false;
}

bool ScriptDictionary::get(const QString &key, qint64 &value) const {
    static_assert(sizeof(qint64) == sizeof(asINT64) &&
                  std::is_signed_v<qint64> && std::is_signed_v<asINT64>);
    if (isValid()) {
        return _data->Get(key, reinterpret_cast<asINT64 &>(value));
    }
    return false;
}

bool ScriptDictionary::get(const QString &key, double &value) const {
    if (isValid()) {
        return _data->Get(key, value);
    }
    return false;
}

QString ScriptDictionary::getType(const QString &key) const {
    auto ctx = asGetActiveContext();
    if (ctx) {
        if (isValid()) {
            auto id = _data->GetTypeId(key);
            if (id != -1) {
                return QString::fromStdString(
                    getAsTypeNameById(ctx->GetEngine(), id));
            }
        }
    }
    return {};
}

bool ScriptDictionary::exists(const QString &key) const {
    if (isValid()) {
        return _data->Exists(key);
    }
    return false;
}

bool ScriptDictionary::isEmpty() const {
    if (isValid()) {
        return _data->IsEmpty();
    }
    return true;
}

uint ScriptDictionary::size() const {
    if (isValid()) {
        return _data->GetSize();
    }
    return 0;
}

bool ScriptDictionary::remove(const QString &key) {
    if (isValid()) {
        return _data->Delete(key);
    }
    return false;
}

void ScriptDictionary::clear() {
    if (isValid()) {
        _data->DeleteAll();
    }
}

ScriptArray::ScriptArray(CScriptArray *data) : WingHex::ASScriptArray(data) {}

QString ScriptArray::elementType() const {
    auto ctx = asGetActiveContext();
    if (ctx) {
        if (isValid()) {
            auto typeID = _data->GetElementTypeId();
            return QString::fromStdString(
                getAsTypeNameById(ctx->GetEngine(), typeID));
        }
    }
    return {};
}

uint ScriptArray::size() const {
    if (isValid()) {
        return _data->GetSize();
    }
    return 0;
}

bool ScriptArray::isEmpty() const {
    if (isValid()) {
        return _data->IsEmpty();
    }
    return true;
}

void ScriptArray::reserve(uint maxElements) {
    if (isValid()) {
        _data->Reserve(maxElements);
    }
}

void ScriptArray::resize(uint numElements) {
    if (isValid()) {
        _data->Resize(numElements);
    }
}

void *ScriptArray::at(uint index) {
    if (isValid()) {
        return _data->At(index);
    }
    return nullptr;
}

const void *ScriptArray::at(uint index) const {
    if (isValid()) {
        return _data->At(index);
    }
    return nullptr;
}

void ScriptArray::setValue(uint index, void *value) {
    if (isValid()) {
        _data->SetValue(index, value);
    }
}

void ScriptArray::insert(uint index, void *value) {
    if (isValid()) {
        _data->InsertAt(index, value);
    }
}

void ScriptArray::insert(uint index, const ASScriptArray &arr) {
    if (isValid() && arr.isValid()) {
        _data->InsertAt(index, *arr._data);
    }
}

void ScriptArray::append(void *value) {
    if (isValid()) {
        _data->InsertLast(value);
    }
}

void ScriptArray::removeAt(uint index) {
    if (isValid()) {
        _data->RemoveAt(index);
    }
}

void ScriptArray::removeLast() {
    if (isValid()) {
        _data->RemoveLast();
    }
}

void ScriptArray::removeRange(uint start, uint count) {
    if (isValid()) {
        _data->RemoveRange(start, count);
    }
}

void ScriptArray::sortAsc() {
    if (isValid()) {
        _data->SortAsc();
    }
}

void ScriptArray::sortDesc() {
    if (isValid()) {
        _data->SortDesc();
    }
}

void ScriptArray::sortAsc(uint startAt, uint count) {
    if (isValid()) {
        _data->SortAsc(startAt, count);
    }
}

void ScriptArray::sortDesc(uint startAt, uint count) {
    if (isValid()) {
        _data->SortDesc(startAt, count);
    }
}

void ScriptArray::sort(uint startAt, uint count, bool asc) {
    if (isValid()) {
        _data->Sort(startAt, count, asc);
    }
}

void ScriptArray::reverse() {
    if (isValid()) {
        _data->Reverse();
    }
}

int ScriptArray::find(const void *value) const {
    if (isValid()) {
        return _data->Find(value);
    }
    return -1;
}

int ScriptArray::find(uint startAt, const void *value) const {
    if (isValid()) {
        return _data->Find(startAt, value);
    }
    return -1;
}

int ScriptArray::findByRef(const void *ref) const {
    if (isValid()) {
        return _data->FindByRef(ref);
    }
    return -1;
}

int ScriptArray::findByRef(uint startAt, const void *ref) const {
    if (isValid()) {
        return _data->FindByRef(startAt, ref);
    }
    return -1;
}

void *ScriptArray::buffer() {
    if (isValid()) {
        return _data->GetBuffer();
    }
    return nullptr;
}

ScriptGrid::ScriptGrid(CScriptGrid *data) : WingHex::ASScriptGrid(data) {}

QString ScriptGrid::elementType() const {
    auto ctx = asGetActiveContext();
    if (ctx) {
        if (isValid()) {
            auto typeID = _data->GetElementTypeId();
            return QString::fromStdString(
                getAsTypeNameById(ctx->GetEngine(), typeID));
        }
    }
    return {};
}

uint ScriptGrid::width() const {
    if (isValid()) {
        return _data->GetWidth();
    }
    return 0;
}

uint ScriptGrid::height() const {
    if (isValid()) {
        return _data->GetHeight();
    }
    return 0;
}

void ScriptGrid::resize(uint width, uint height) {
    if (isValid()) {
        _data->Resize(width, height);
    }
}

void *ScriptGrid::at(uint x, uint y) {
    if (isValid()) {
        return _data->At(x, y);
    }
    return nullptr;
}

const void *ScriptGrid::at(uint x, uint y) const {
    if (isValid()) {
        return _data->At(x, y);
    }
    return nullptr;
}

void ScriptGrid::setValue(uint x, uint y, void *value) {
    if (isValid()) {
        _data->SetValue(x, y, value);
    }
}

ScriptAny::ScriptAny(CScriptAny *data) : WingHex::ASScriptAny(data) {}

void ScriptAny::store(void *ref, std::string_view type) {
    auto ctx = asGetActiveContext();
    if (ctx) {
        if (isValid()) {
            int typeId;
            if (typeIDCache.contains(type)) {
                typeId = typeIDCache.object(type);
            } else {
                typeId = ctx->GetEngine()->GetTypeIdByDecl(type.data());
                if (typeId == asINVALID_TYPE) {
                    return;
                }
                typeIDCache.insert(type, typeId);
            }
            _data->Store(ref, typeId);
        }
    }
}

void ScriptAny::store(const quint64 &value) {
    if (isValid()) {
        _data->Store(asQWORD(value));
    }
}

void ScriptAny::store(const qint64 &value) {
    if (isValid()) {
        _data->Store(asINT64(value));
    }
}

void ScriptAny::store(const double &value) {
    if (isValid()) {
        _data->Store(value);
    }
}

bool ScriptAny::retrieve(void *ref, std::string_view type) const {
    auto ctx = asGetActiveContext();
    if (ctx) {
        if (isValid()) {
            int typeId;
            if (typeIDCache.contains(type)) {
                typeId = typeIDCache.object(type);
            } else {
                typeId = ctx->GetEngine()->GetTypeIdByDecl(type.data());
                if (typeId == asINVALID_TYPE) {
                    return false;
                }
                typeIDCache.insert(type, typeId);
            }
            return _data->Retrieve(ref, typeId);
        }
    }
    return false;
}

bool ScriptAny::retrieve(quint64 &value) const {
    static_assert(sizeof(quint64) == sizeof(asQWORD) &&
                  std::is_unsigned_v<quint64> && std::is_unsigned_v<asQWORD>);
    if (isValid()) {
        return _data->Retrieve(reinterpret_cast<asQWORD &>(value));
    }
    return false;
}

bool ScriptAny::retrieve(qint64 &value) const {
    static_assert(sizeof(qint64) == sizeof(asINT64) &&
                  std::is_signed_v<qint64> && std::is_signed_v<asINT64>);
    if (isValid()) {
        return _data->Retrieve(reinterpret_cast<asINT64 &>(value));
    }
    return false;
}

bool ScriptAny::retrieve(double &value) const {
    if (isValid()) {
        return _data->Retrieve(value);
    }
    return false;
}
