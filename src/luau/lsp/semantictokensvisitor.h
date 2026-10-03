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

#ifndef SEMANTICTOKENSVISITOR_H
#define SEMANTICTOKENSVISITOR_H

#include "Luau/Ast.h"
#include "Luau/Module.h"
#include "lsp.h"

#include <unordered_set>

#include <QVector>

struct SemanticToken {
    Luau::Position start;
    Luau::Position end;
    lsp::SemanticTokenTypes tokenType;
    lsp::SemanticTokenModifiers tokenModifiers;
};

enum struct AstLocalInfo {
    // local is self
    Self,
    // local is a function parameter
    Parameter,
};

struct SemanticTokensVisitor : public Luau::AstVisitor {
    const Luau::ModulePtr &module;
    const std::unordered_map<Luau::AstName, Luau::TypeId> &builtinGlobals;
    QVector<SemanticToken> tokens;
    std::unordered_map<Luau::AstLocal *, AstLocalInfo> localMap{};
    std::unordered_set<Luau::AstType *> syntheticTypes{};

public:
    explicit SemanticTokensVisitor(
        const Luau::ModulePtr &module,
        const std::unordered_map<Luau::AstName, Luau::TypeId> &builtinGlobals);

public:
    // HACK: Luau introduces some synthetic tokens in the AST for types
    // { T } gets converted to { [number]: T } (where number is an introduced
    // AstTypeReference) string? gets converted to string | nil (where nil is an
    // introduced AstTypeReference) We do not want to highlight these synthetic
    // tokens, as they don't exist in real code. We apply visitors in these
    // locations to try and check for synthetic tokens, and mark them as such
    bool visit(Luau::AstTypeTable *table) override;

    bool visit(Luau::AstTypeUnion *unionType) override;

    bool visit(Luau::AstType *type) override;

    bool visit(Luau::AstTypeReference *ref) override;

    bool visit(Luau::AstTypePack *type) override;

    bool visit(Luau::AstTypePackGeneric *type) override;

    bool visit(Luau::AstStatLocal *local) override;

    bool visit(Luau::AstStatFunction *func) override;

    bool visit(Luau::AstStatLocalFunction *func) override;

    bool visit(Luau::AstExprFunction *func) override;

    bool visit(Luau::AstExprLocal *local) override;

    bool visit(Luau::AstExprGlobal *global) override;

    bool visit(Luau::AstExprIndexName *index) override;

    bool visit(Luau::AstExprTable *tbl) override;

    bool visit(Luau::AstStatBlock *block) override;
};

#endif // SEMANTICTOKENSVISITOR_H
