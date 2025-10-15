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

#include "baseaddrcommand.h"

inline QString constructText(quintptr oldaddr, quintptr newaddr) {
    auto buffer = QStringLiteral("[A] {addr: 0x%1 -> 0x%2}")
                      .arg(QString::number(oldaddr, 16).toUpper(),
                           QString::number(newaddr, 16).toUtf8());
    return buffer;
}

BaseAddrCommand::BaseAddrCommand(QHexDocument *doc, quintptr oldaddr,
                                 quintptr newaddr, QUndoCommand *parent)
    : UndoCommandBase(constructText(oldaddr, newaddr), parent), m_doc(doc),
      m_old(oldaddr), m_new(newaddr) {}

void BaseAddrCommand::redo() { m_doc->setBaseAddress(m_new); }

void BaseAddrCommand::undo() { m_doc->setBaseAddress(m_old); }

int BaseAddrCommand::id() const { return UndoID_SetBaseAddr; }

bool BaseAddrCommand::mergeWith(const QUndoCommand *other) {
    auto ucmd = static_cast<const BaseAddrCommand *>(other);
    if (ucmd && ucmd->m_doc == this->m_doc) {
        this->m_new = ucmd->m_new;
        setText(constructText(this->m_old, this->m_new));
        return true;
    }
    return false;
}
