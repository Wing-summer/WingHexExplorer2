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

#include "typehelpers.h"

namespace {

std::vector<PropLookup>
lookupProp(const Luau::TypeId &parentType, const Luau::Name &name,
           Luau::DenseHashSet<Luau::TypeId> &seenTypes) {
    if (seenTypes.contains(parentType))
        return {};
    seenTypes.insert(parentType);

    if (auto externType = Luau::get<Luau::ExternType>(parentType)) {
        if (auto property = Luau::lookupExternTypeProp(externType, name)) {
            return {{parentType, *property}};
        }
    } else if (auto tableType = Luau::get<Luau::TableType>(parentType)) {
        if (auto property = tableType->props.find(name);
            property != tableType->props.end()) {
            return {{parentType, property->second}};
        }
    } else if (auto metatableType =
                   Luau::get<Luau::MetatableType>(parentType)) {
        auto baseTableType = Luau::follow(metatableType->table);
        if (auto baseTable = Luau::get<Luau::TableType>(baseTableType)) {
            if (auto property = baseTable->props.find(name);
                property != baseTable->props.end()) {
                return {{baseTableType, property->second}};
            }
        }

        if (auto metatable = Luau::get<Luau::TableType>(
                Luau::follow(metatableType->metatable))) {
            auto index = metatable->props.find("__index");
            if (index != metatable->props.end() && index->second.readTy) {
                auto followed = Luau::follow(*index->second.readTy);
                if ((Luau::get<Luau::TableType>(followed) ||
                     Luau::get<Luau::MetatableType>(followed)) &&
                    followed != parentType)
                    return lookupProp(followed, name, seenTypes);
            }
        }
    } else if (auto intersection =
                   Luau::get<Luau::IntersectionType>(parentType)) {
        for (auto type : intersection->parts) {
            auto properties = lookupProp(Luau::follow(type), name, seenTypes);
            if (!properties.empty())
                return properties;
        }
    } else if (auto unionType = Luau::get<Luau::UnionType>(parentType)) {
        std::vector<PropLookup> properties;
        for (auto type : unionType->options) {
            auto typeProperties =
                lookupProp(Luau::follow(type), name, seenTypes);
            properties.insert(properties.end(), typeProperties.begin(),
                              typeProperties.end());
        }
        return properties;
    }

    return {};
}

} // namespace

namespace LuauLsp {

std::vector<PropLookup> lookupProp(const Luau::TypeId &parentType,
                                   const Luau::Name &name) {
    Luau::DenseHashSet<Luau::TypeId> seenTypes;
    return ::lookupProp(parentType, name, seenTypes);
}

bool isMetamethod(const QString &name) {
    return name == QLatin1String("__index") ||
           name == QLatin1String("__newindex") ||
           name == QLatin1String("__call") ||
           name == QLatin1String("__concat") ||
           name == QLatin1String("__unm") || name == QLatin1String("__add") ||
           name == QLatin1String("__sub") || name == QLatin1String("__mul") ||
           name == QLatin1String("__div") || name == QLatin1String("__mod") ||
           name == QLatin1String("__pow") ||
           name == QLatin1String("__tostring") ||
           name == QLatin1String("__metatable") ||
           name == QLatin1String("__eq") || name == QLatin1String("__lt") ||
           name == QLatin1String("__le") || name == QLatin1String("__mode") ||
           name == QLatin1String("__iter") || name == QLatin1String("__len");
}

} // namespace LuauLsp
