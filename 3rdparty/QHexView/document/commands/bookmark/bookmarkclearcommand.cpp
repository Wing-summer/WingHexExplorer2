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

#include "bookmarkclearcommand.h"

BookMarkClearCommand::BookMarkClearCommand(
    QHexDocument *doc, const QMap<qsizetype, QString> &bookmarks,
    QUndoCommand *parent)
    : UndoCommandBase(tr("[ClearBookMark]"), parent), m_doc(doc),
      m_bookmarks(bookmarks) {}

void BookMarkClearCommand::redo() { m_doc->clearBookMark(); }

int BookMarkClearCommand::id() const { return UndoID_BookMarkClear; }

bool BookMarkClearCommand::mergeWith(const QUndoCommand *other) {
    Q_UNUSED(other);
    return true;
}

void BookMarkClearCommand::undo() { m_doc->applyBookMarks(m_bookmarks); }
