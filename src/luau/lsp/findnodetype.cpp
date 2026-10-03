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

#include "findnodetype.h"

FindNodeType::FindNodeType(Luau::Position pos, Luau::Position documentEnd,
                           bool closed)
    : pos(pos), documentEnd(documentEnd), closed(closed) {}

bool FindNodeType::isCloserMatch(Luau::Location &newLocation) const {
    return (closed ? newLocation.containsClosed(pos)
                   : newLocation.contains(pos)) &&
           (!best || best->location.encloses(newLocation));
}

bool FindNodeType::visit(Luau::AstNode *node) {
    if (isCloserMatch(node->location)) {
        best = node;
        return true;
    }

    // Edge case: If we ask for the node at the position that is the very
    // end of the document return the innermost AST element that ends at
    // that position.

    if (node->location.end == documentEnd && pos >= documentEnd) {
        best = node;
        return true;
    }

    return false;
}

bool FindNodeType::visit(Luau::AstType *node) {
    return visit(static_cast<Luau::AstNode *>(node));
}

bool FindNodeType::visit(Luau::AstTypePack *node) {
    return visit(static_cast<Luau::AstNode *>(node));
}

bool FindNodeType::visit(Luau::AstGenericType *node) { return false; }

bool FindNodeType::visit(Luau::AstGenericTypePack *node) { return false; }

bool FindNodeType::visit(Luau::AstStatBlock *block) {
    visit(static_cast<Luau::AstNode *>(block));

    for (Luau::AstStat *stat : block->body) {
        if (stat->location.end < pos)
            continue;
        if (stat->location.begin > pos)
            break;

        stat->visit(this);
    }

    return false;
}
