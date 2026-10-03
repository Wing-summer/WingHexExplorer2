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

#ifndef LUAU_LSP_TYPEHELPERS_H
#define LUAU_LSP_TYPEHELPERS_H

#include "Luau/Type.h"

#include <qobject.h>
#include <vector>

struct PropLookup {
    Luau::TypeId baseTableTy;
    Luau::Property property;
};

namespace LuauLsp {

std::vector<PropLookup> lookupProp(const Luau::TypeId &parentType,
                                   const Luau::Name &name);

bool isMetamethod(const QString &name);

} // namespace LuauLsp

#endif // LUAU_LSP_TYPEHELPERS_H
