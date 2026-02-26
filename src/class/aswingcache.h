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

#ifndef ASWINGCACHE_H
#define ASWINGCACHE_H

#include "as-debugger/as_debugger.h"

class asDebugger;

class asWingCache : public asIDBCache {
public:
    explicit asWingCache(asDebugger &dbg, asIScriptContext *ctx);

public:
    const asIDBTypeEvaluator &GetEvaluator(const asIDBVarAddr &id) const;

private:
    // Register an evaluator.
    void registerEvaluator(int typeId,
                           std::unique_ptr<asIDBTypeEvaluator> evaluator);

    // A quick shortcut to make a templated instantiation
    // of T from the given type name.
    template <typename T>
    inline void registerEvaluator(asIScriptEngine *engine, const char *name) {
        auto info = engine->GetTypeInfoByName(name);
        assert(info);
        registerEvaluator(info->GetTypeId(), std::make_unique<T>());
    }

    // _evaluators don't take up much memory so we'll just
    // always keep them around.
    // we only have only one engine, so it's ok to make it static
    inline static std::unordered_map<int, std::unique_ptr<asIDBTypeEvaluator>>
        _evaluators;
};

#endif // ASWINGCACHE_H
