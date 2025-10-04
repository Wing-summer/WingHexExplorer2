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

#include "metaclearcommand.h"

inline QString constructText(const QVector<QHexMetadataItem> &metas) {
    return QStringLiteral("[M~] {cnt: %1}").arg(metas.size());
}

MetaClearCommand::MetaClearCommand(QHexMetadata *hexmeta,
                                   const QVector<QHexMetadataItem> &metas,
                                   QUndoCommand *parent)
    : UndoCommandBase(constructText(metas), parent), m_hexmeta(hexmeta),
      m_metas(metas) {}

void MetaClearCommand::redo() { m_hexmeta->clear(); }

int MetaClearCommand::id() const { return UndoID_MetaClear; }

bool MetaClearCommand::mergeWith(const QUndoCommand *other) {
    Q_UNUSED(other);
    return true;
}

void MetaClearCommand::undo() { m_hexmeta->applyMetas(m_metas); }
