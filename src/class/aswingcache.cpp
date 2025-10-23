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

#include "aswingcache.h"

#include "asdebugger.h"
#include "astype_evaluator.h"

#include <QtGlobal>

asWingCache::asWingCache(asDebugger &dbg, asIScriptContext *ctx)
    : asIDBCache(dbg, ctx) {
    auto engine = dbg.workspace->engine;
    Q_ASSERT(engine);
    if (_evaluators.empty()) {
        registerEvaluator<asIDBCharTypeEvaluator>(engine, "char");
        registerEvaluator<asIDBStringTypeEvaluator>(engine, "string");
        registerEvaluator<asIDBArrayTypeEvaluator>(engine, "array");
        registerEvaluator<asIDBDictionaryTypeEvaluator>(engine, "dictionary");
        registerEvaluator<asIDBAnyTypeEvaluator>(engine, "any");
        registerEvaluator<asIDBColorTypeEvaluator>(engine, "color");
        registerEvaluator<asIDBRegExTypeEvaluator>(engine, "regex::exp");
        registerEvaluator<asIDBRegMatchTypeEvaluator>(engine, "regex::match");
        registerEvaluator<asIDBJsonDocumentTypeEvaluator>(engine,
                                                          "Json::JsonDocument");
        registerEvaluator<asIDBJsonArrayTypeEvaluator>(engine,
                                                       "Json::JsonArray");
        registerEvaluator<asIDBJsonObjectTypeEvaluator>(engine,
                                                        "Json::JsonObject");
        registerEvaluator<asIDBJsonValueTypeEvaluator>(engine,
                                                       "Json::JsonValue");
    }
}

const asIDBTypeEvaluator &
asWingCache::GetEvaluator(const asIDBVarAddr &id) const {
    if (id.ResolveAs<void>() == nullptr)
        return asIDBCache::GetEvaluator(id);

    // do we have a custom evaluator?
    if (auto f = _evaluators.find(
            id.typeId & (asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR));
        f != _evaluators.end()) {
        return *f->second;
    }

    auto type = ctx->GetEngine()->GetTypeInfoById(id.typeId);
    if (type) {
        auto &d = static_cast<asDebugger &>(dbg);
        std::string name = d.cache->GetTypeNameFromType(id.typeId);

        // are we a template?
        if (id.typeId & asTYPEID_TEMPLATE) {
            // fetch the base type, see if we have a
            // evaluator for that one
            auto baseType = ctx->GetEngine()->GetTypeInfoByName(name.c_str());
            if (baseType) {
                if (auto f = _evaluators.find(
                        baseType->GetTypeId() &
                        (asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR));
                    f != _evaluators.end()) {
                    return *f->second;
                }
            }
        } else {
            static const asIDBCustomTypeEvaluator uninitType(d._customEvals);
            if (d._customEvals.contains(name)) {
                return uninitType;
            }
        }
    }

    return asIDBCache::GetEvaluator(id);
}

void asWingCache::registerEvaluator(
    int typeId, std::unique_ptr<asIDBTypeEvaluator> evaluator) {
    Q_ASSERT(typeId);
    typeId &= asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR;
    _evaluators.insert_or_assign(typeId, std::move(evaluator));
}
