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

#ifndef SCRIPTDOCUMENTREGISTRY_H
#define SCRIPTDOCUMENTREGISTRY_H

#include <memory>

#include <QHash>
#include <QPointer>

#include "scriptdocumentcontext.h"

struct lua_State;

// The VM shares allocation counters; each execution owns its own resources.
class ScriptDocumentRegistry final {
public:
    ScriptDocumentRegistry(ScriptDocumentContext::CurrentView current,
                           ScriptDocumentContext::OpenView open,
                           ScriptDocumentContext::CloseView close);
    ~ScriptDocumentRegistry();
    ScriptDocumentContext *findCaller(lua_State *L) const;
    ScriptDocumentContext *forCaller(lua_State *L);

private:
    ScriptDocumentContext::CurrentView m_current;
    ScriptDocumentContext::OpenView m_open;
    ScriptDocumentContext::CloseView m_close;
    std::shared_ptr<ScriptDocumentIds> m_ids =
        std::make_shared<ScriptDocumentIds>();
    QHash<const void *, QPointer<ScriptDocumentContext>> m_contexts;
};

#endif // SCRIPTDOCUMENTREGISTRY_H
