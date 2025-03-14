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

#ifndef METACLEARCOMMAND_H
#define METACLEARCOMMAND_H

#include "../../qhexmetadata.h"
#include <QList>
#include <QObject>
#include <QUndoCommand>
#include <QUndoStack>

// this class is newed by wingsummer

class MetaClearCommand : public QUndoCommand {
public:
    MetaClearCommand(QHexMetadata *hexmeta,
                     const QVector<QHexMetadataItem> &metas,
                     QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

protected:
    QHexMetadata *m_hexmeta;
    QVector<QHexMetadataItem> m_metas;
};

#endif // METACLEARCOMMAND_H
