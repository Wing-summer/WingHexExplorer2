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
** =============================================================================
*/

#include "qdocumentswaptextcommand.h"

#include "qdocument_p.h"

QDocumentSwapTextCommand::QDocumentSwapTextCommand(const QString &text,
                                                   QDocument *doc,
                                                   QDocumentCommand *p)
    : QDocumentCommand(Command::Custom, doc, p), oldtext(m_doc->text()),
      newtext(text) {}

void QDocumentSwapTextCommand::undo() {
    m_doc->setText(oldtext);
    for (int i = 0; i < m_doc->lineCount(); ++i) {
        auto line = m_doc->line(i);
        markUndone(line.handle());
    }
}

void QDocumentSwapTextCommand::redo() {
    m_doc->setText(newtext);
    for (int i = 0; i < m_doc->lineCount(); ++i) {
        auto line = m_doc->line(i);
        markRedone(line.handle(), m_first);
    }
}
