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

#ifndef BOOKMARKCLEARCOMMAND_H
#define BOOKMARKCLEARCOMMAND_H

#include "../undocommandbase.h"
#include "document/qhexdocument.h"

#include <QMap>

class BookMarkClearCommand : public UndoCommandBase {
public:
    explicit BookMarkClearCommand(QHexDocument *doc,
                                  const QMap<qsizetype, QString> &bookmarks,
                                  QUndoCommand *parent = nullptr);

    // QUndoCommand interface
public:
    virtual void undo() override;
    virtual void redo() override;

    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand *other) override;

protected:
    QHexDocument *m_doc;
    QMap<qsizetype, QString> m_bookmarks;
};

#endif // BOOKMARKCLEARCOMMAND_H
