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

#ifndef HEXCOMMAND_H
#define HEXCOMMAND_H

#include "document/qhexcursor.h"
#include "document/qhexdocument.h"

#include <QUndoCommand>

class HexCommand : public QUndoCommand {
public:
    HexCommand(QHexDocument *doc, QHexCursor *cursor, int nibbleindex,
               QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

protected:
    QHexDocument *m_doc;
    QHexCursor *m_cursor;

    qsizetype m_offset;
    qsizetype m_length;
    QByteArray m_data;

    int m_nibbleindex;
};

#endif // HEXCOMMAND_H
