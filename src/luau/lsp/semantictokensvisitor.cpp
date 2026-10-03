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

#include "semantictokensvisitor.h"

#include "Luau/AstQuery.h"
#include "typehelpers.h"

static bool isMethod(const Luau::FunctionType *ftv) {
    if (ftv->hasSelf) {
        return true;
    }

    // Some Luau function types do not set hasSelf, so also check the
    // conventional first-parameter name.
    if (ftv->argNames.size() > 0 && ftv->argNames[0].has_value() &&
        ftv->argNames[0]->name == "self") {
        return true;
    }

    return false;
}

static bool isMethod(Luau::TypeId ty) {
    auto ftv = Luau::get<Luau::FunctionType>(Luau::follow(ty));
    if (!ftv) {
        return false;
    }

    return isMethod(ftv);
}

static bool isOverloadedMethod(Luau::TypeId ty) {
    if (!Luau::get<Luau::IntersectionType>(Luau::follow(ty))) {
        return false;
    }

    auto isOverloadedMethod = [](Luau::TypeId part) -> bool {
        return isMethod(part);
    };

    std::vector<Luau::TypeId> parts = Luau::flattenIntersection(ty);
    return std::all_of(parts.begin(), parts.end(), isOverloadedMethod);
}

static lsp::SemanticTokenTypes inferTokenType(const Luau::TypeId ty,
                                              lsp::SemanticTokenTypes base) {
    if (!ty) {
        return base;
    }
    auto followedTy = Luau::follow(ty);

    if (auto ftv = Luau::get<Luau::FunctionType>(followedTy)) {
        if (isMethod(ftv)) {
            return lsp::SemanticTokenTypes::Method;
        } else {
            return lsp::SemanticTokenTypes::Function;
        }
    } else if (Luau::get<Luau::IntersectionType>(followedTy)) {
        if (isOverloadedMethod(followedTy)) {
            return lsp::SemanticTokenTypes::Method;
        } else if (Luau::isOverloadedFunction(followedTy)) {
            return lsp::SemanticTokenTypes::Function;
        }
    }

    return base;
}

SemanticTokensVisitor::SemanticTokensVisitor(
    const Luau::ModulePtr &module,
    const std::unordered_map<Luau::AstName, Luau::TypeId> &builtinGlobals)
    : module(module), builtinGlobals(builtinGlobals) {}

bool SemanticTokensVisitor::visit(Luau::AstTypeTable *table) {
    // If the indexer location is the same as a result type, the indexer was
    // synthetically added
    if (table->indexer && table->indexer->indexType->location ==
                              table->indexer->resultType->location) {
        syntheticTypes.emplace(table->indexer->indexType);
    }
    return true;
}

bool SemanticTokensVisitor::visit(Luau::AstTypeUnion *unionType) {
    // If the union contains a "nil" part, but it has the the size of only 1
    // column, then it is synthetic Note that the union could contain > 2
    // parts:
    //  T? -> T | nil
    //  U | V? -> U | V | nil
    for (const auto &ty : unionType->types)
        if (auto ref = ty->as<Luau::AstTypeReference>())
            if (!ref->prefix && !ref->hasParameterList && ref->name == "nil" &&
                ref->location.end.column == ref->location.begin.column + 1) {
                syntheticTypes.emplace(ref);
            }
    return true;
}

bool SemanticTokensVisitor::visit(Luau::AstType *type) { return true; }

bool SemanticTokensVisitor::visit(Luau::AstTypeReference *ref) {
    // Ignore synthetic type references
    if (syntheticTypes.find(ref) != syntheticTypes.end()) {
        return false;
    }

    // HACK: The location information provided only gives location for the
    // whole reference but we do not want to highlight punctuation inside of
    // it (Module.Type<Param> should not highlight . < >) So we use the
    // start position and consider the end positions separately. Here, we
    // make the assumption that there is no comments or newlines present in
    // between the punctuation

    auto startPosition = ref->location.begin;
    // Highlight prefix if exists
    if (ref->prefix) {
        Luau::Position endPosition{
            startPosition.line,
            startPosition.column +
                static_cast<unsigned int>(strlen(ref->prefix->value))};
        tokens.emplace_back(SemanticToken{startPosition, endPosition,
                                          lsp::SemanticTokenTypes::Namespace,
                                          lsp::SemanticTokenModifiers::None});
        startPosition = {endPosition.line, endPosition.column + 1};
    }

    // Highlight name
    Luau::Position endPosition{
        startPosition.line,
        startPosition.column +
            static_cast<unsigned int>(strlen(ref->name.value))};
    tokens.emplace_back(SemanticToken{startPosition, endPosition,
                                      lsp::SemanticTokenTypes::Type,
                                      lsp::SemanticTokenModifiers::None});

    // Do not highlight parameters as they will be visited later
    return true;
}

bool SemanticTokensVisitor::visit(Luau::AstTypePack *type) { return true; }

bool SemanticTokensVisitor::visit(Luau::AstTypePackGeneric *type) {
    // HACK: do not highlight punctuation
    Luau::Position endPosition{
        type->location.begin.line,
        type->location.begin.column +
            static_cast<unsigned int>(strlen(type->genericName.value))};
    tokens.emplace_back(SemanticToken{type->location.begin, endPosition,
                                      lsp::SemanticTokenTypes::Namespace,
                                      lsp::SemanticTokenModifiers::None});
    return true;
}

bool SemanticTokensVisitor::visit(Luau::AstStatLocal *local) {
    auto scope = Luau::findScopeAtPosition(*module, local->location.begin);
    if (!scope) {
        return true;
    }

    for (auto var : local->vars) {
        auto ty = scope->lookup(var);
        if (ty) {
            auto type = inferTokenType(*ty, lsp::SemanticTokenTypes::Variable);
            if (type == lsp::SemanticTokenTypes::Variable) {
                // No special semantic token needed, fall back
                // to syntax highlighting
                return true;
            }
            tokens.emplace_back(
                SemanticToken{var->location.begin, var->location.end, type,
                              lsp::SemanticTokenModifiers::None});
        }
    }

    return true;
}

bool SemanticTokensVisitor::visit(Luau::AstStatFunction *func) { return true; }

bool SemanticTokensVisitor::visit(Luau::AstStatLocalFunction *func) {
    tokens.emplace_back(SemanticToken{
        func->name->location.begin, func->name->location.end,
        lsp::SemanticTokenTypes::Function, lsp::SemanticTokenModifiers::None});
    return true;
}

bool SemanticTokensVisitor::visit(Luau::AstExprFunction *func) {
    if (func->self) {
        localMap.insert_or_assign(func->self, AstLocalInfo::Self);
    }

    bool first = true;
    for (auto arg : func->args) {
        if (first && !func->self && arg->name == "self") {
            tokens.emplace_back(
                SemanticToken{arg->location.begin, arg->location.end,
                              lsp::SemanticTokenTypes::Property,
                              lsp::SemanticTokenModifiers::DefaultLibrary});
            localMap.insert_or_assign(arg, AstLocalInfo::Self);
        } else {
            tokens.emplace_back(
                SemanticToken{arg->location.begin, arg->location.end,
                              lsp::SemanticTokenTypes::Parameter,
                              lsp::SemanticTokenModifiers::None});
            localMap.insert_or_assign(arg, AstLocalInfo::Parameter);
        }

        first = false;
    }
    return true;
}

bool SemanticTokensVisitor::visit(Luau::AstExprLocal *local) {
    auto defaultType = lsp::SemanticTokenTypes::Variable;
    if (localMap.find(local->local) != localMap.end()) {
        auto localInfo = localMap.at(local->local);
        if (localInfo == AstLocalInfo::Self) {
            tokens.emplace_back(
                SemanticToken{local->location.begin, local->location.end,
                              lsp::SemanticTokenTypes::Property,
                              lsp::SemanticTokenModifiers::DefaultLibrary});

            return true;
        } else if (localInfo == AstLocalInfo::Parameter) {
            defaultType = lsp::SemanticTokenTypes::Parameter;
        }
    }

    auto type = defaultType;
    if (auto ty = module->astTypes.find(local)) {
        type = inferTokenType(*ty, defaultType);
    }

    if (type == lsp::SemanticTokenTypes::Variable)
        return true;

    tokens.emplace_back(SemanticToken{local->location.begin,
                                      local->location.end, type,
                                      lsp::SemanticTokenModifiers::None});
    return true;
}

bool SemanticTokensVisitor::visit(Luau::AstExprGlobal *global) {
    auto it = builtinGlobals.find(global->name);
    if (it != builtinGlobals.end() && strlen(global->name.value) > 0) {
        // SPECIAL CASE: if name is "Enum", classify it as an enum
        if (global->name == "Enum") {
            tokens.emplace_back(
                SemanticToken{global->location.begin, global->location.end,
                              lsp::SemanticTokenTypes::Enum,
                              lsp::SemanticTokenModifiers::DefaultLibrary});
        }
        // If it starts with an uppercase letter, flag it as a class
        // Otherwise, flag it as a builtin
        else if (isupper(global->name.value[0])) {
            tokens.emplace_back(
                SemanticToken{global->location.begin, global->location.end,
                              lsp::SemanticTokenTypes::Class,
                              lsp::SemanticTokenModifiers::DefaultLibrary});
        } else {
            auto type =
                inferTokenType(it->second, lsp::SemanticTokenTypes::Variable);
            tokens.emplace_back(SemanticToken{
                global->location.begin, global->location.end, type,
                lsp::SemanticTokenModifiers(
                    lsp::SemanticTokenModifiers::DefaultLibrary |
                    lsp::SemanticTokenModifiers::Readonly)});
        }
    } else {
        auto ty = module->astTypes.find(global);
        if (!ty) {
            return true;
        }

        auto type = inferTokenType(*ty, lsp::SemanticTokenTypes::Variable);
        if (type == lsp::SemanticTokenTypes::Variable)
            return true;

        tokens.emplace_back(SemanticToken{global->location.begin,
                                          global->location.end, type,
                                          lsp::SemanticTokenModifiers::None});
    }

    return true;
}

bool SemanticTokensVisitor::visit(Luau::AstExprIndexName *index) {
    auto parentTy = module->astTypes.find(index->expr);
    if (!parentTy)
        return true;

    auto parentIsBuiltin = false;
    auto parentIsEnum = false;
    if (auto global = index->expr->as<Luau::AstExprGlobal>()) {
        parentIsBuiltin =
            builtinGlobals.find(global->name) != builtinGlobals.end();
        if (parentIsBuiltin && global->name == "Enum")
            parentIsEnum = true;
    }

    auto ty = Luau::follow(*parentTy);
    if (auto propInformation =
            LuauLsp::lookupProp(ty, std::string(index->index.value));
        propInformation.size() == 1 && propInformation[0].property.readTy) {
        auto prop = propInformation[0].property;

        auto defaultType = lsp::SemanticTokenTypes::Property;
        if (parentIsEnum)
            defaultType = lsp::SemanticTokenTypes::Enum;
        else if (Luau::hasTag(prop.tags, "EnumItem"))
            defaultType = lsp::SemanticTokenTypes::EnumMember;

        auto type = inferTokenType(*prop.readTy, defaultType);
        auto modifiers = lsp::SemanticTokenModifiers::None;
        if (parentIsBuiltin) {
            modifiers = lsp::SemanticTokenModifiers(
                modifiers | lsp::SemanticTokenModifiers::DefaultLibrary |
                lsp::SemanticTokenModifiers::Readonly);
        } else if (LuauLsp::isMetamethod(
                       QString::fromUtf8(index->index.value))) {
            modifiers = lsp::SemanticTokenModifiers(
                modifiers | lsp::SemanticTokenModifiers::DefaultLibrary);
        }
        tokens.emplace_back(SemanticToken{index->indexLocation.begin,
                                          index->indexLocation.end, type,
                                          modifiers});
    }

    return true;
}

bool SemanticTokensVisitor::visit(Luau::AstExprTable *tbl) {
    for (const auto &item : tbl->items) {
        if (item.kind == Luau::AstExprTable::Item::Kind::Record) {
            if (auto ty = module->astTypes.find(item.value)) {
                auto type =
                    inferTokenType(*ty, lsp::SemanticTokenTypes::Property);
                tokens.emplace_back(SemanticToken{
                    item.key->location.begin, item.key->location.end, type,
                    lsp::SemanticTokenModifiers::None});
            }
        }
    }

    return true;
}

bool SemanticTokensVisitor::visit(Luau::AstStatBlock *block) {
    for (Luau::AstStat *stat : block->body) {
        stat->visit(this);
    }

    return false;
}
