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

#include "metaremoveposcommand.h"

inline QString constructText(qsizetype pos) {
    return QStringLiteral("[M-] {pos: %1-0x%2} ")
        .arg(QString::number(pos), QString::number(pos, 16).toUpper());
}

MetaRemovePosCommand::MetaRemovePosCommand(QHexMetadata *hexmeta, qsizetype pos,
                                           QUndoCommand *parent)
    : MetaCommand(constructText(pos), hexmeta, {}, parent), m_pos(pos) {
    auto po = m_hexmeta->get(pos);
    if (po.has_value()) {
        m_meta = po.value();
    }
}

void MetaRemovePosCommand::redo() { m_hexmeta->removeMetadata(m_pos); }

int MetaRemovePosCommand::id() const { return UndoID_MetaRemovePos; }

bool MetaRemovePosCommand::mergeWith(const QUndoCommand *other) {
    auto ucmd = static_cast<const MetaRemovePosCommand *>(other);
    if (ucmd) {
        if (this->m_pos == ucmd->m_pos) {
            return true;
        }
    }
    return false;
}

void MetaRemovePosCommand::undo() {
    m_hexmeta->metadata(m_meta.begin, m_meta.end, m_meta.foreground,
                        m_meta.background, m_meta.comment);
}
