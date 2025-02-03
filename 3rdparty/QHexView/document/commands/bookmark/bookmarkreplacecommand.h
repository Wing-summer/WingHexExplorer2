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

#ifndef BOOKMARKREPLACECOMMAND_H
#define BOOKMARKREPLACECOMMAND_H

#include "bookmarkcommand.h"

class BookMarkReplaceCommand : public BookMarkCommand {
public:
    BookMarkReplaceCommand(QHexDocument *doc, qsizetype pos, QString comment,
                           QString oldcomment, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

protected:
    QString m_oldcomment;
};

#endif // BOOKMARKREPLACECOMMAND_H
