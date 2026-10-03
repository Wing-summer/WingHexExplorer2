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

#ifndef ATTACHCOMMENTSVISITOR_H
#define ATTACHCOMMENTSVISITOR_H

#include "Luau/Ast.h"
#include "Luau/ParseResult.h"

struct AttachCommentsVisitor : public Luau::AstVisitor {
    Luau::Position pos;
    std::vector<Luau::Comment>
        moduleComments; // A list of all comments in the module
    Luau::Position closestPreviousNode{0, 0};

public:
    explicit AttachCommentsVisitor(const Luau::Location node,
                                   std::vector<Luau::Comment> moduleComments);

public:
    std::vector<Luau::Comment> attachComments();

public:
    virtual bool visit(Luau::AstExprTable *tbl) override;

    virtual bool visit(Luau::AstTypeTable *tbl) override;

    virtual bool visit(Luau::AstStatDeclareExternType *klass) override;

    virtual bool visit(Luau::AstStatBlock *block) override;

    virtual bool visit(Luau::AstType *ty) override;

    virtual bool visit(Luau::AstTypePack *ty) override;
};

#endif // ATTACHCOMMENTSVISITOR_H
