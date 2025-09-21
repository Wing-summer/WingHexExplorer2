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

#ifndef ASTYPE_EVALUATOR_H
#define ASTYPE_EVALUATOR_H

#include "scriptaddon/scriptany.h"

#include "as-debugger/as_debugger.h"
#include "src/scriptaddon/scriptqdictionary.h"

#include <QChar>
#include <QColor>
#include <QString>

class asIDBArrayTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    virtual void Expand(asIDBVariable::Ptr var) const override {
        QueryVariableForEach(var, 0);
    }
};

class asIDBStringTypeEvaluator : public asIDBTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        const QString *s = var->address.ResolveAs<const QString>();

        if (s->isEmpty()) {
            var->value = "<empty>";
        } else {
            if (var->owner.expired()) {
                var->value = s->toStdString();
            } else {
                var->value = fmt::format("\"{}\"", s->toStdString());
            }
        }
    }
};

class asIDBCharTypeEvaluator : public asIDBTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        const QChar *s = var->address.ResolveAs<const QChar>();
        var->value = s->unicode();
    }
};

class asIDBColorTypeEvaluator : public asIDBTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        const QColor *s = var->address.ResolveAs<const QColor>();
        var->value = s->name().toStdString();
    }
};

class asIDBAnyTypeEvaluator : public asIDBTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        const CScriptAny *v = var->address.ResolveAs<const CScriptAny>();

        if (v->GetTypeId() == 0) {
            var->value = "<no stored value>";
            return;
        }

        var->value = fmt::format("any<{}>", var->dbg.cache->GetTypeNameFromType(
                                                {v->GetTypeId(), asTM_NONE}));
        var->expandable = true;
    }

    virtual void Expand(asIDBVariable::Ptr var) const override {
        const CScriptAny *v = var->address.ResolveAs<const CScriptAny>();

        auto tid = v->GetTypeId();

        asIDBVarAddr id(tid, false, nullptr);

        if (tid == asTYPEID_DOUBLE)
            id.address = (void *)&v->value.valueFlt;
        else if (tid == asTYPEID_INT64)
            id.address = (void *)&v->value.valueInt;
        else if (tid & (asTYPEID_HANDLETOCONST | asTYPEID_OBJHANDLE))
            id.address = (void *)&v->value.valueObj;
        else
            id.address = v->value.valueObj;

        var->CreateChildVariable(
            "value", id,
            var->dbg.cache->GetTypeNameFromType({v->GetTypeId(), asTM_NONE}));
    }
};

class asIDBDictionaryTypeEvaluator : public asIDBTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        const CScriptDictionary *v =
            var->address.ResolveAs<const CScriptDictionary>();

        size_t size = v->GetSize();

        var->value = fmt::format("{{{} key/value pairs}}", size);
        var->expandable = size != 0;
    }

    virtual void Expand(asIDBVariable::Ptr var) const override {
        const CScriptDictionary *v =
            var->address.ResolveAs<const CScriptDictionary>();

        for (auto &kvp : *v) {
            auto child = var->CreateChildVariable(
                fmt::format("[\"{}\"]", kvp.GetKey().toStdString()),
                {kvp.GetTypeId(), false,
                 const_cast<void *>(kvp.GetAddressOfValue())},
                var->dbg.cache->GetTypeNameFromType(
                    {kvp.GetTypeId(), asTM_NONE}));
            child->Evaluate();
        }
    }
};

#endif // ASTYPE_EVALUATOR_H
