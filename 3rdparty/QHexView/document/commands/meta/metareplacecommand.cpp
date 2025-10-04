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

#include "metareplacecommand.h"

inline QString constructText(const QHexMetadataItem &meta,
                             const QHexMetadataItem &oldmeta) {
    auto buffer = QStringLiteral("[M*] {pos: %1-0x%2} ")
                      .arg(QString::number(meta.begin),
                           QString::number(meta.begin, 16).toUpper());

    if (meta.foreground != oldmeta.foreground) {
        buffer.append('F');
        if (meta.foreground.isValid() && !oldmeta.foreground.isValid()) {
            buffer.append('+');
        } else if (!meta.foreground.isValid() && oldmeta.foreground.isValid()) {
            buffer.append('-');
        } else {
            buffer.append('*');
        }
    }

    if (meta.background != oldmeta.background) {
        buffer.append('B');
        if (meta.background.isValid() && !oldmeta.background.isValid()) {
            buffer.append('+');
        } else if (!meta.background.isValid() && oldmeta.background.isValid()) {
            buffer.append('-');
        } else {
            buffer.append('*');
        }
    }

    if (meta.comment != oldmeta.comment) {
        buffer.append('C');
        if (meta.comment.isEmpty() && !oldmeta.comment.isEmpty()) {
            buffer.append('-');
        } else if (!meta.comment.isEmpty() && oldmeta.comment.isEmpty()) {
            buffer.append('+');
        } else {
            buffer.append('*');
        }
    }

    return buffer;
}

MetaReplaceCommand::MetaReplaceCommand(QHexMetadata *hexmeta,
                                       const QHexMetadataItem &meta,
                                       const QHexMetadataItem &oldmeta,
                                       QUndoCommand *parent)
    : MetaCommand(constructText(meta, oldmeta), hexmeta, meta, parent),
      m_old(oldmeta) {}

void MetaReplaceCommand::undo() { m_hexmeta->modifyMetadata(m_old, m_meta); }

void MetaReplaceCommand::redo() { m_hexmeta->modifyMetadata(m_meta, m_old); }

int MetaReplaceCommand::id() const { return UndoID_MetaReplace; }

bool MetaReplaceCommand::mergeWith(const QUndoCommand *other) {
    auto ucmd = static_cast<const MetaReplaceCommand *>(other);
    if (ucmd) {
        if (this->m_old == ucmd->m_old) {
            this->m_meta = ucmd->m_meta;
            setText(constructText(this->m_meta, this->m_old));
            return true;
        }
    }
    return false;
}
