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
** The original License is LPGL from Andres6936/QHexEdit. I have modified a lot
** so I decide to change the Open Source License. You can use the original
** library under LPGL. Thanks for Andres6936's efforts.
** =============================================================================
*/

#ifndef BASEADDRCOMMAND_H
#define BASEADDRCOMMAND_H

#include "document/qhexdocument.h"
#include "undocommandbase.h"

class BaseAddrCommand : public UndoCommandBase {
public:
    explicit BaseAddrCommand(QHexDocument *doc, quintptr oldaddr,
                             quintptr newaddr, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    virtual int id() const override;
    bool mergeWith(const QUndoCommand *other) override;

private:
    QHexDocument *m_doc;
    quintptr m_old, m_new;
};

#endif // BASEADDRCOMMAND_H
