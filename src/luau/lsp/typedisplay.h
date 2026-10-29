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

#ifndef TYPEDISPLAY_H
#define TYPEDISPLAY_H

#include "Luau/ToString.h"
#include "Luau/Type.h"
#include "Luau/TypePack.h"
#include <algorithm>
#include <functional>

namespace LuauLsp {

// Presentation belongs to the host; never change the analysis type graph.
struct TypeDisplayOptions : Luau::ToStringOptions {
    std::function<std::optional<std::string>(Luau::TypeId)> typeNameOverride;
};

inline std::string displayTypeResult(Luau::ToStringResult result,
                                     const TypeDisplayOptions &options) {
    if (!options.typeNameOverride) {
        return result.name;
    }
    // Replace only identity-bearing spans, so literals and property names stay
    // untouched. Work backwards to preserve the original byte offsets.
    std::sort(result.typeSpans.begin(), result.typeSpans.end(),
              [](const auto &left, const auto &right) {
                  return left.startPos > right.startPos;
              });
    size_t boundary = result.name.size();
    for (const auto &span : result.typeSpans) {
        if (span.endPos > boundary || span.startPos >= span.endPos) {
            continue;
        }
        if (auto name = options.typeNameOverride(span.type)) {
            result.name.replace(span.startPos, span.endPos - span.startPos,
                                *name);
            boundary = span.startPos;
        }
    }
    return result.name;
}

inline std::string displayType(Luau::TypeId type, TypeDisplayOptions &options) {
    if (options.typeNameOverride) {
        if (auto name = options.typeNameOverride(Luau::follow(type))) {
            return *name;
        }
    }
    return displayTypeResult(Luau::toStringDetailed(type, options), options);
}

inline std::string displayType(Luau::TypePackId pack,
                               TypeDisplayOptions &options) {
    return displayTypeResult(Luau::toStringDetailed(pack, options), options);
}

inline std::string displayNamedFunction(const std::string &name,
                                        const Luau::FunctionType &function,
                                        TypeDisplayOptions &options) {
    if (!options.typeNameOverride) {
        return Luau::toStringNamedFunction(name, function, options);
    }
    // Let Luau format generics, then render parameter and return types through
    // its public detailed API, which retains the identities of named types.
    const auto original = Luau::toStringNamedFunction("", function, options);
    auto result = name + original.substr(0, original.find('(')) + "(";
    auto it = Luau::begin(function.argTypes);
    size_t index = 0;
    bool first = true;
    for (; it != Luau::end(function.argTypes); ++it, ++index) {
        if (index == 0 && function.hasSelf &&
            options.hideFunctionSelfArgument) {
            continue;
        }
        if (!first) {
            result += ", ";
        }
        first = false;
        if (index < options.namedFunctionOverrideArgNames.size()) {
            result += options.namedFunctionOverrideArgNames[index];
        } else if (index < function.argNames.size() &&
                   function.argNames[index]) {
            result += function.argNames[index]->name;
        } else {
            result += "_";
        }
        result += ": " + displayType(*it, options);
    }
    if (auto tail = it.tail()) {
        const auto variadic = Luau::get<Luau::VariadicTypePack>(*tail);
        if (!variadic || !variadic->hidden) {
            if (!first) {
                result += ", ";
            }
            result += "...: ";
            result += variadic ? displayType(variadic->ty, options)
                               : displayType(*tail, options);
        }
    }
    result += "): ";
    const auto count = Luau::size(function.retTypes);
    const bool tail = !Luau::finite(function.retTypes);
    const bool wrap =
        Luau::get<Luau::TypePack>(Luau::follow(function.retTypes)) &&
        (tail ? count != 0 : count > 1);
    if (wrap) {
        result += "(";
    }
    result += displayType(function.retTypes, options);
    if (wrap) {
        result += ")";
    }
    return result;
}

} // namespace LuauLsp

#endif // TYPEDISPLAY_H
