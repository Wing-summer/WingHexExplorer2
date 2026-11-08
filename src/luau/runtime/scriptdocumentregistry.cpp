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

#include "scriptdocumentregistry.h"

#include "qtluau/callback.hpp"
#include <limits>
#include <stdexcept>
#include <utility>

ScriptDocumentRegistry::ScriptDocumentRegistry(
    ScriptDocumentContext::CurrentView current,
    ScriptDocumentContext::OpenView open,
    ScriptDocumentContext::CloseView close)
    : _current(std::move(current)), _open(std::move(open)),
      _close(std::move(close)) {}

ScriptDocumentRegistry::~ScriptDocumentRegistry() {
    for (const auto &context : std::as_const(_contexts)) {
        delete context.data();
    }
}

ScriptDocumentContext *ScriptDocumentRegistry::findCaller(lua_State *L) const {
    return _contexts.value(qtluau::luaContextKey(L)).data();
}

ScriptDocumentContext *ScriptDocumentRegistry::forCaller(lua_State *L) {
    const auto key = qtluau::luaContextKey(L);
    auto context = _contexts.value(key);
    if (!context) {
        if (_ids->context == std::numeric_limits<int>::max()) {
            throw std::runtime_error("Script document context limit reached");
        }
        for (auto it = _contexts.begin(); it != _contexts.end();) {
            if (!it.value()) {
                it = _contexts.erase(it);
            } else {
                ++it;
            }
        }
        context = new ScriptDocumentContext(_current, _open, _close, _ids,
                                            ++_ids->context);
        qtluau::trackContextObject(L, context);
        _contexts.insert(key, context);
    }
    return context.data();
}
