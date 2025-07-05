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

#include "metaaddcommand.h"

MetaAddCommand::MetaAddCommand(QHexMetadata *hexmeta,
                               const QHexMetadataItem &meta,
                               QUndoCommand *parent)
    : MetaCommand(tr("[MetaAdd]"), hexmeta, meta, parent) {
    _brokenMetas = m_hexmeta->mayBrokenMetaData(meta.begin, meta.end);
}

void MetaAddCommand::redo() {
    m_hexmeta->metadata(m_meta.begin, m_meta.end, m_meta.foreground,
                        m_meta.background, m_meta.comment);
}

int MetaAddCommand::id() const { return UndoID_MetaAdd; }

bool MetaAddCommand::mergeWith(const QUndoCommand *other) {
    auto ucmd = static_cast<const MetaAddCommand *>(other);
    if (ucmd) {
        if (this->m_meta.foreground == ucmd->m_meta.foreground &&
            this->m_meta.background == ucmd->m_meta.background &&
            this->m_meta.comment == ucmd->m_meta.comment) {
            return this->m_meta.mergeRegionWithoutMetaCheck(ucmd->m_meta);
        }
    }
    return false;
}

void MetaAddCommand::undo() {
    m_hexmeta->removeMetadata(m_meta.begin, m_meta.end);
    for (auto &meta : _brokenMetas) {
        m_hexmeta->metadata(meta.begin, meta.end, meta.foreground,
                            meta.background, meta.comment);
    }
}
