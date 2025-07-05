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

#include "appendcommand.h"

AppendCommand::AppendCommand(QHexDocument *doc, QHexCursor *cursor,
                             const QByteArray &data, int nibbleindex,
                             QUndoCommand *parent)
    : HexCommand(tr("[HexAppend]"), doc, cursor, nibbleindex, parent) {
    m_offset = -1;
    m_data = data;
    m_length = data.length();
}

void AppendCommand::undo() {
    auto offset = m_doc->length() - m_length;
    m_doc->_remove(offset, m_length);
    m_doc->insertBookMarkAdjustRevert(offset, m_length);
    m_doc->metadata()->insertAdjustRevert(offset, m_length);
    m_cursor->setPos(offset, m_nibbleindex);
}

void AppendCommand::redo() {
    auto offset = m_doc->length();
    m_doc->_insert(offset, m_data);
    m_doc->insertBookMarkAdjust(offset, m_length);
    m_doc->metadata()->insertAdjust(offset, m_length);
    if (m_data.length() == 1 && m_nibbleindex) {
        m_cursor->setPos(offset, 0);
    } else {
        m_cursor->setPos(offset + m_length, m_nibbleindex);
    }
}

int AppendCommand::id() const { return UndoID_HexAppend; }

bool AppendCommand::mergeWith(const QUndoCommand *other) {
    auto ucmd = static_cast<const AppendCommand *>(other);
    if (ucmd) {
        this->m_data.append(ucmd->m_data);
        return true;
    }
    return false;
}
