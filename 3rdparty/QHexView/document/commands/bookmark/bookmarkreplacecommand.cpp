/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
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
**
** The original License is MIT from Dax89/QHexView. I have modified a lot so I
** decide to change the Open Source License. You can use the original library
** under MIT. Thanks for Dax89's efforts.
** =============================================================================
*/

#include "bookmarkreplacecommand.h"

BookMarkReplaceCommand::BookMarkReplaceCommand(QHexDocument *doc, qsizetype pos,
                                               QString comment,
                                               QString oldcomment,
                                               QUndoCommand *parent)
    : BookMarkCommand(tr("[ReplaceBookMark] pos: %1").arg(pos), doc, pos,
                      comment, parent),
      m_oldcomment(oldcomment) {}

void BookMarkReplaceCommand::redo() { m_doc->modBookMark(m_pos, m_comment); }

int BookMarkReplaceCommand::id() const { return UndoID_BookMarkReplace; }

bool BookMarkReplaceCommand::mergeWith(const QUndoCommand *other) {
    auto ucmd = static_cast<const BookMarkReplaceCommand *>(other);
    if (ucmd) {
        if (this->m_pos == ucmd->m_pos) {
            this->m_comment = m_comment;
            return true;
        }
    }
    return false;
}

void BookMarkReplaceCommand::undo() { m_doc->modBookMark(m_pos, m_oldcomment); }
