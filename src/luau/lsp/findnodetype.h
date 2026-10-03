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

#ifndef FINDNODETYPE_H
#define FINDNODETYPE_H

#include "Luau/Ast.h"

struct FindNodeType : public Luau::AstVisitor {
    Luau::Position pos;
    Luau::Position documentEnd;
    Luau::AstNode *best = nullptr;
    bool closed = false;

public:
    explicit FindNodeType(Luau::Position pos, Luau::Position documentEnd,
                          bool closed);

public:
    bool isCloserMatch(Luau::Location &newLocation) const;

public:
    virtual bool visit(Luau::AstNode *node) override;

    virtual bool visit(class Luau::AstType *node) override;

    virtual bool visit(class Luau::AstTypePack *node) override;

    virtual bool visit(Luau::AstGenericType *node) override;

    virtual bool visit(Luau::AstGenericTypePack *node) override;

    virtual bool visit(Luau::AstStatBlock *block) override;
};

#endif // FINDNODETYPE_H
