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

#include "replacecommand.h"
#include "document/qhexdocument.h"

ReplaceCommand::ReplaceCommand(QHexDocument *doc, qsizetype offset,
                               const QByteArray &data, QHexCursor *cursor,
                               int nibbleindex, QUndoCommand *parent)
    : HexCommand(tr("[HexReplace] pos: %1").arg(offset), doc, cursor,
                 nibbleindex, parent) {
    m_offset = offset;
    m_data = data;
    m_length = data.length();
    m_olddata = doc->read(m_offset, m_length);
}

void ReplaceCommand::undo() {
    m_doc->_replace(m_offset, m_olddata);
    m_cursor->setPos(m_offset, m_nibbleindex);
}

void ReplaceCommand::redo() {
    m_doc->_replace(m_offset, m_data);
    if (m_data.length() == 1 && m_nibbleindex) {
        m_cursor->setPos(m_offset, 0);
    } else {
        m_cursor->setPos(m_offset + m_length, !m_nibbleindex);
    }
}

int ReplaceCommand::id() const { return UndoID_HexReplaceInsert; }

bool ReplaceCommand::mergeWith(const QUndoCommand *other) {
    auto ucmd = dynamic_cast<const ReplaceCommand *>(other);
    if (ucmd) {
        if (this->m_offset == ucmd->m_offset) {
            if (this->m_length <= ucmd->m_length) {
                this->m_olddata = ucmd->m_olddata;
                this->m_data = ucmd->m_data;
                this->m_length = ucmd->m_length;
            } else {
                this->m_data.replace(0, ucmd->m_length, ucmd->m_data);
            }
            return true;
        }

        if (this->m_offset + this->m_length == ucmd->m_offset) {
            this->m_length += ucmd->m_offset;
            this->m_data.append(ucmd->m_data);
            this->m_olddata.append(ucmd->m_olddata);
            this->m_nibbleindex = ucmd->m_nibbleindex;
            return true;
        }

        if (this->m_offset <= ucmd->m_offset &&
            this->m_offset + m_length >= ucmd->m_offset + ucmd->m_length) {
            auto dis = ucmd->m_offset - this->m_offset;
            this->m_data.replace(dis, ucmd->m_length, ucmd->m_data);
            return true;
        }
        if (this->m_offset >= ucmd->m_offset &&
            this->m_offset + m_length <= ucmd->m_offset + ucmd->m_length) {
            this->m_offset = ucmd->m_offset;
            this->m_data = ucmd->m_data;
            this->m_length = ucmd->m_length;
            this->m_olddata = ucmd->m_olddata;
            this->m_nibbleindex = ucmd->m_nibbleindex;
            setText(tr("[HexReplace] pos: %1").arg(this->m_offset));
            return true;
        }
    }
    return false;
}
