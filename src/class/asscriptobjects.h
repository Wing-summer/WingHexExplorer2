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

#ifndef ASSCRIPTOBJECTS_H
#define ASSCRIPTOBJECTS_H

#include <QCache>

#include "WingPlugin/scriptobjects.h"

class ScriptDictionary : public WingHex::ASScriptDictionary {
public:
    ScriptDictionary(CScriptDictionary *data);

    // ASScriptDictionary interface
public:
    virtual bool set(const QString &key, void *value,
                     std::string_view type) override;
    virtual bool set(const QString &key, const quint64 &value) override;
    virtual bool set(const QString &key, const qint64 &value) override;
    virtual bool set(const QString &key, const double &value) override;
    virtual bool get(const QString &key, void *value,
                     std::string_view type) const override;
    virtual bool get(const QString &key, quint64 &value) const override;
    virtual bool get(const QString &key, qint64 &value) const override;
    virtual bool get(const QString &key, double &value) const override;
    virtual QString getType(const QString &key) const override;
    virtual bool exists(const QString &key) const override;
    virtual bool isEmpty() const override;
    virtual uint size() const override;
    virtual bool remove(const QString &key) override;
    virtual void clear() override;

private:
    inline static QCache<std::string_view, int> typeIDCache{10};
};

class ScriptArray : public WingHex::ASScriptArray {
public:
    ScriptArray(CScriptArray *data);

    // ASScriptArray interface
public:
    virtual QString elementType() const override;
    virtual uint size() const override;
    virtual bool isEmpty() const override;
    virtual void reserve(uint maxElements) override;
    virtual void resize(uint numElements) override;
    virtual void *at(uint index) override;
    virtual const void *at(uint index) const override;
    virtual void setValue(uint index, void *value) override;
    virtual void insert(uint index, void *value) override;
    virtual void insert(uint index, const ASScriptArray &arr) override;
    virtual void append(void *value) override;
    virtual void removeAt(uint index) override;
    virtual void removeLast() override;
    virtual void removeRange(uint start, uint count) override;
    virtual void sortAsc() override;
    virtual void sortDesc() override;
    virtual void sortAsc(uint startAt, uint count) override;
    virtual void sortDesc(uint startAt, uint count) override;
    virtual void sort(uint startAt, uint count, bool asc) override;
    virtual void reverse() override;
    virtual int find(const void *value) const override;
    virtual int find(uint startAt, const void *value) const override;
    virtual int findByRef(const void *ref) const override;
    virtual int findByRef(uint startAt, const void *ref) const override;
    virtual void *buffer() override;
};

class ScriptGrid : public WingHex::ASScriptGrid {
public:
    ScriptGrid(CScriptGrid *data);

    // ASScriptGrid interface
public:
    virtual QString elementType() const override;
    virtual uint width() const override;
    virtual uint height() const override;
    virtual void resize(uint width, uint height) override;
    virtual void *at(uint x, uint y) override;
    virtual const void *at(uint x, uint y) const override;
    virtual void setValue(uint x, uint y, void *value) override;
};

class ScriptAny : public WingHex::ASScriptAny {
public:
    ScriptAny(CScriptAny *data);

    // ASScriptAny interface
public:
    virtual void store(void *ref, std::string_view type) override;
    virtual void store(const quint64 &value) override;
    virtual void store(const qint64 &value) override;
    virtual void store(const double &value) override;
    virtual bool retrieve(void *ref, std::string_view type) const override;
    virtual bool retrieve(quint64 &value) const override;
    virtual bool retrieve(qint64 &value) const override;
    virtual bool retrieve(double &value) const override;

private:
    inline static QCache<std::string_view, int> typeIDCache{10};
};

#endif // ASSCRIPTOBJECTS_H
