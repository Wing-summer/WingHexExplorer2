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

#include <limits>
#include <stdexcept>
#include <utility>

#include "qtluau/callback.hpp"

ScriptDocumentRegistry::ScriptDocumentRegistry(
    ScriptDocumentContext::CurrentView current,
    ScriptDocumentContext::OpenView open,
    ScriptDocumentContext::CloseView close)
    : m_current(std::move(current)), m_open(std::move(open)),
      m_close(std::move(close)) {}

ScriptDocumentRegistry::~ScriptDocumentRegistry() {
    for (const auto &context : std::as_const(m_contexts)) {
        delete context.data();
    }
}

ScriptDocumentContext *ScriptDocumentRegistry::findCaller(lua_State *L) const {
    return m_contexts.value(qtluau::luaContextKey(L)).data();
}

ScriptDocumentContext *ScriptDocumentRegistry::forCaller(lua_State *L) {
    const auto key = qtluau::luaContextKey(L);
    auto context = m_contexts.value(key);
    if (!context) {
        if (m_ids->context == std::numeric_limits<int>::max()) {
            throw std::runtime_error("Script document context limit reached");
        }
        for (auto it = m_contexts.begin(); it != m_contexts.end();) {
            if (!it.value()) {
                it = m_contexts.erase(it);
            } else {
                ++it;
            }
        }
        context = new ScriptDocumentContext(m_current, m_open, m_close, m_ids,
                                            ++m_ids->context);
        qtluau::trackContextObject(L, context);
        m_contexts.insert(key, context);
    }
    return context.data();
}
