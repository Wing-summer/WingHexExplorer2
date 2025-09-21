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

#include "astype_evaluator.h"

#include <QtGlobal>

asWingCache::asWingCache(asIDBDebugger &dbg, asIScriptContext *ctx)
    : asIDBCache(dbg, ctx) {
    auto engine = dbg.workspace->engine;
    Q_ASSERT(engine);
    registerEvaluator<asIDBCharTypeEvaluator>(engine, "char");
    registerEvaluator<asIDBStringTypeEvaluator>(engine, "string");
    registerEvaluator<asIDBArrayTypeEvaluator>(engine, "array");
    registerEvaluator<asIDBDictionaryTypeEvaluator>(engine, "dictionary");
    registerEvaluator<asIDBAnyTypeEvaluator>(engine, "any");
    registerEvaluator<asIDBColorTypeEvaluator>(engine, "color");
}

const asIDBTypeEvaluator &
asWingCache::GetEvaluator(const asIDBVarAddr &id) const {
    if (id.ResolveAs<void>() == nullptr)
        return asIDBCache::GetEvaluator(id);

    auto type = ctx->GetEngine()->GetTypeInfoById(id.typeId);

    // do we have a custom evaluator?
    if (auto f = _evaluators.find(
            id.typeId & (asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR));
        f != _evaluators.end())
        return *f->second.get();

    // are we a template?
    if (id.typeId & asTYPEID_TEMPLATE) {
        // fetch the base type, see if we have a
        // evaluator for that one
        auto baseType = ctx->GetEngine()->GetTypeInfoByName(type->GetName());

        if (auto f =
                _evaluators.find(baseType->GetTypeId() &
                                 (asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR));
            f != _evaluators.end())
            return *f->second.get();
    }

    return asIDBCache::GetEvaluator(id);
}

void asWingCache::registerEvaluator(
    int typeId, std::unique_ptr<asIDBTypeEvaluator> evaluator) {
    typeId &= asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR;
    _evaluators.insert_or_assign(typeId, std::move(evaluator));
}
