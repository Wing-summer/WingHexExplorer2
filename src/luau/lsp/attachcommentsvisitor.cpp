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

#include "attachcommentsvisitor.h"

#include <algorithm>

AttachCommentsVisitor::AttachCommentsVisitor(
    const Luau::Location node, std::vector<Luau::Comment> moduleComments)
    : pos(node.begin), moduleComments(std::move(moduleComments)) {}

std::vector<Luau::Comment> AttachCommentsVisitor::attachComments() {
    std::vector<Luau::Comment> candidates{};

    // Gather candidate comments (between previous node and target)
    for (auto &comment : moduleComments) { // The comment needs to be present
                                           // before the node for it to be
        // attached
        if (comment.location.begin <= pos) {
            // They should be after the closest previous node
            if (comment.location.begin >= closestPreviousNode) {
                candidates.emplace_back(comment);
            }
        }
    }

    if (candidates.empty()) {
        return {};
    }

    // Sort by end position descending (closest to target first)
    std::sort(candidates.begin(), candidates.end(),
              [](const Luau::Comment &a, const Luau::Comment &b) {
                  return a.location.end > b.location.end;
              });

    // Filter to only adjacent comments
    // A comment is adjacent if there's no blank line between it and the
    // target
    std::vector<Luau::Comment> result;
    unsigned int adjacentLine = pos.line;

    for (const auto &comment : candidates) {
        if (comment.location.end.line + 1 >= adjacentLine) {
            result.emplace_back(comment);
            adjacentLine = comment.location.begin.line;
        } else {
            break;
        }
    }

    // Reverse to restore chronological order
    std::reverse(result.begin(), result.end());
    return result;
}

bool AttachCommentsVisitor::visit(Luau::AstExprTable *tbl) {
    if (tbl->location.begin >= pos) {
        return false;
    }
    if (tbl->location.begin > closestPreviousNode) {
        closestPreviousNode = tbl->location.begin;
    }

    for (Luau::AstExprTable::Item item : tbl->items) {
        if (item.value->location.begin >= pos) {
            continue;
        }
        if (item.value->location.begin > closestPreviousNode) {
            closestPreviousNode = item.value->location.begin;
        }
        item.value->visit(this);
        if (item.value->location.end <= pos &&
            item.value->location.end > closestPreviousNode) {
            closestPreviousNode = item.value->location.end;
        }
    }

    return false;
}

bool AttachCommentsVisitor::visit(Luau::AstTypeTable *tbl) {
    if (tbl->location.begin >= pos) {
        return false;
    }
    if (tbl->location.begin > closestPreviousNode) {
        closestPreviousNode = tbl->location.begin;
    }

    for (Luau::AstTableProp item : tbl->props) {
        if (item.type->location.begin >= pos) {
            continue;
        }
        if (item.type->location.begin > closestPreviousNode) {
            closestPreviousNode = item.type->location.begin;
        }
        item.type->visit(this);
        if (item.type->location.end <= pos &&
            item.type->location.end > closestPreviousNode) {
            closestPreviousNode = item.type->location.end;
        }
    }

    return false;
}

bool AttachCommentsVisitor::visit(Luau::AstStatDeclareExternType *klass) {
    if (klass->location.begin >= pos) {
        return false;
    }
    if (klass->location.begin > closestPreviousNode) {
        closestPreviousNode = klass->location.begin;
    }

    for (const auto &item : klass->props) {
        if (item.ty->location.begin >= pos) {
            continue;
        }
        closestPreviousNode =
            std::max(closestPreviousNode, item.ty->location.begin);
        item.ty->visit(this);
        if (item.ty->location.end <= pos) {
            closestPreviousNode =
                std::max(closestPreviousNode, item.ty->location.end);
        }
    }

    return false;
}

bool AttachCommentsVisitor::visit(Luau::AstStatBlock *block) {
    // If the position is after the block, then it can be ignored
    // If the position is within the block, then we know we can cut
    // anything before the block, so set the previous node location to
    // the block entry
    if (block->location.begin >= pos) {
        return false;
    }
    if (block->location.begin > closestPreviousNode) {
        closestPreviousNode = block->location.begin;
    }

    for (Luau::AstStat *stat : block->body) {
        if (stat->location.begin >= pos) {
            continue;
        }
        stat->visit(this);
        if (stat->location.end <= pos &&
            stat->location.end > closestPreviousNode) {
            closestPreviousNode = stat->location.end;
        }
    }

    return false;
}

bool AttachCommentsVisitor::visit(Luau::AstType *ty) { return true; }

bool AttachCommentsVisitor::visit(Luau::AstTypePack *ty) { return true; }
