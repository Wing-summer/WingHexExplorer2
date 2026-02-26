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

#ifndef SCRIPTANY_H
#define SCRIPTANY_H

#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif

#include "scriptqdictionary.h"

BEGIN_AS_NAMESPACE

// we using CScriptDictValue as a replacement of any

class CScriptAny : public CScriptDictValue {
public:
    // Constructors
    CScriptAny(asIScriptEngine *engine);
    CScriptAny(asIScriptEngine *engine, void *value, int typeId);

    // Memory management
    int AddRef() const;
    int Release() const;

    // Copy the stored value from another any object
    CScriptAny &operator=(const CScriptAny &);
    int CopyFrom(const CScriptAny *other);

    // Store the value, either as variable type, integer number, or real number
    void Store(void *ref, int refTypeId);
    void Store(const asQWORD &value);
    void Store(const asINT64 &value);
    void Store(const double &value);

    // Retrieve the stored value, either as variable type, integer number, or
    // real number
    bool Retrieve(void *ref, int refTypeId) const;
    bool Retrieve(asQWORD &value) const;
    bool Retrieve(asINT64 &value) const;
    bool Retrieve(double &value) const;

    // GC methods
    int GetRefCount();
    void SetFlag();
    bool GetFlag();
    void ReleaseAllHandles(asIScriptEngine *engine);

protected:
    virtual ~CScriptAny();
    void FreeObject();

    mutable int refCount;
    mutable bool gcFlag;
    asIScriptEngine *engine;
};

void RegisterScriptAny(asIScriptEngine *engine);

END_AS_NAMESPACE

#endif
