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

#include "removecommand.h"

RemoveCommand::RemoveCommand(QHexDocument *doc, qsizetype offset,
                             qsizetype length, QHexCursor *cursor,
                             int nibbleindex, QUndoCommand *parent)
    : HexCommand(tr("[HexRemove] pos: %1").arg(offset), doc, cursor,
                 nibbleindex, parent) {
    m_offset = offset;
    m_length = length;
    m_data = doc->read(m_offset, m_length);
}

void RemoveCommand::undo() {
    m_doc->_insert(m_offset, m_data);
    m_doc->removeBookMarkAdjustRevert(_rmbms, m_offset, m_length);
    m_doc->metadata()->removeAdjustRevert(_rmMetas, m_offset, m_length);
    if (m_length > 1) {
        m_cursor->setPos(m_offset + m_length - 1, 1);
    } else {
        if (m_nibbleindex) {
            m_cursor->setPos(m_offset + 1, 1);
        } else {
            m_cursor->setPos(m_offset, 0);
        }
    }
}

void RemoveCommand::redo() {
    m_cursor->setPos(m_offset, m_nibbleindex);
    m_doc->_remove(m_offset, m_length);
    _rmbms = m_doc->removeBookMarkAdjust(m_offset, m_length);
    _rmMetas = m_doc->metadata()->removeAdjust(m_offset, m_length);
}

int RemoveCommand::id() const { return UndoID_HexRemove; }

bool RemoveCommand::mergeWith(const QUndoCommand *other) {
    auto ucmd = static_cast<const RemoveCommand *>(other);
    if (ucmd) {
        if (this->m_offset == ucmd->m_offset) {
            this->m_data.append(ucmd->m_data);
            auto bms = ucmd->_rmbms;
            for (auto &&b : bms.asKeyValueRange()) {
                this->_rmbms.insert(b.first + this->m_length, b.second);
            }
            auto metas = ucmd->_rmMetas;
            for (auto &m : metas) {
                m.begin += this->m_length;
                m.end += this->m_length;
            }
            this->_rmMetas.append(ucmd->_rmMetas);
            this->m_length += ucmd->m_length;
            return true;
        }
    }
    return false;
}
