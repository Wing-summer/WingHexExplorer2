#ifndef ASTYPE_EVALUATOR_H
#define ASTYPE_EVALUATOR_H

#include "as-debugger/as_debugger.h"
#include "src/scriptaddon/scriptqdictionary.h"

class asIDBArrayTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    virtual void Expand(asIDBVariable::Ptr var) const override {
        QueryVariableForEach(var, 0);
    }
};

// class asIDBAnyTypeEvaluator : public asIDBTypeEvaluator {
// public:
//     virtual void Evaluate(asIDBVariable::Ptr var) const override {
//         const CScriptAny *v = var->address.ResolveAs<const CScriptAny>();

//         if (v->GetTypeId() == 0) {
//             var->value = "(no stored value)";
//             return;
//         }

//         var->value = fmt::format("any<{}>",
//         var->dbg.cache->GetTypeNameFromType(
//                                                 {v->GetTypeId(),
//                                                 asTM_NONE}));
//         var->expandable = true;
//     }

//     virtual void Expand(asIDBVariable::Ptr var) const override {
//         const CScriptAny *v = var->address.ResolveAs<const CScriptAny>();

//         auto tid = v->GetTypeId();

//         asIDBVarAddr id(tid, false, nullptr);

//         if (tid == asTYPEID_DOUBLE)
//             id.address = (void *)&v->value.valueFlt;
//         else if (tid == asTYPEID_INT64)
//             id.address = (void *)&v->value.valueInt;
//         else if (tid & (asTYPEID_HANDLETOCONST | asTYPEID_OBJHANDLE))
//             id.address = (void *)&v->value.valueObj;
//         else
//             id.address = v->value.valueObj;

//         var->CreateChildVariable(
//             "value", id,
//             var->dbg.cache->GetTypeNameFromType({v->GetTypeId(),
//             asTM_NONE}));
//     }
// };

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
                fmt::format("[{}]", kvp.GetKey()),
                {kvp.GetTypeId(), false,
                 const_cast<void *>(kvp.GetAddressOfValue())},
                var->dbg.cache->GetTypeNameFromType(
                    {kvp.GetTypeId(), asTM_NONE}));
            child->Evaluate();
        }
    }
};

#endif // ASTYPE_EVALUATOR_H
