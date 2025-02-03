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

#include "insertcommand.h"

InsertCommand::InsertCommand(QHexDocument *doc, QHexCursor *cursor,
                             qsizetype offset, const QByteArray &data,
                             int nibbleindex, QUndoCommand *parent)
    : HexCommand(doc, cursor, nibbleindex, parent) {
    m_offset = offset;
    m_data = data;
    m_length = data.length();
}

void InsertCommand::undo() {
    m_doc->_remove(m_offset, m_length);
    m_doc->insertBookMarkAdjustRevert(m_offset, m_length);
    m_doc->metadata()->insertAdjustRevert(m_offset, m_length);
    m_cursor->setPos(m_offset, m_nibbleindex);
    HexCommand::undo();
}
void InsertCommand::redo() {
    m_doc->_insert(m_offset, m_data);
    m_doc->insertBookMarkAdjust(m_offset, m_length);
    m_doc->metadata()->insertAdjust(m_offset, m_length);
    if (m_data.length() == 1 && m_nibbleindex) {
        m_cursor->setPos(m_offset, 0);
    } else {
        m_cursor->setPos(m_offset + m_length, m_nibbleindex);
    }
    HexCommand::redo();
}
