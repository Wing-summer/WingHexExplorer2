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
    : QDocumentCommand(Command::Custom, doc, p) {
    auto textl = text.split('\n');
    diffs = DiffUtil::compareFiles(doc->textLines(), textl);
    auto r = std::find_if(
        diffs.begin(), diffs.end(), [](const DiffUtil::DiffEntry &diff) {
            return diff.changeType != DiffUtil::DiffEntry::UNCHANGED;
        });
    if (r == diffs.end()) {
        setObsolete(true);
        return;
    }

    header = textl.takeFirst();
    oldheader = doc->line(0).text();
    for (auto &s : textl) {
        auto lh = new QDocumentLineHandle(s, m_doc);
        _handles.append(lh);
    }

    for (auto p = std::next(doc->begin()); p != doc->end(); ++p) {
        _oldhandles.append(*p);
    }

    auto cursor = doc->editCursor();
    setTargetCursor(cursor->handle());
}

void QDocumentSwapTextCommand::undo() {
    removeLines(0, m_doc->lineCount() - 1);
    removeText(0, 0, header.length());
    insertText(0, 0, oldheader);
    insertLines(0, _oldhandles);

    m_doc->impl()->emitContentsChange(0, _oldhandles.count() + 1);
    for (auto &item : diffs) {
        if (item.changeType == DiffUtil::DiffEntry::REMOVED) {
            auto line = m_doc->line(item.lineNumberA - 1);
            markUndone(line.handle());
        } else if (item.changeType == DiffUtil::DiffEntry::ADDED) {
            auto line = m_doc->line(item.lineNumberB - 1);
            markUndone(line.handle());
        }
    }
}

void QDocumentSwapTextCommand::redo() {
    removeLines(0, m_doc->lineCount() - 1);
    removeText(0, 0, oldheader.length());
    insertText(0, 0, header);
    insertLines(0, _handles);

    m_doc->impl()->emitContentsChange(0, _handles.count() + 1);
    for (auto &item : diffs) {
        if (item.changeType == DiffUtil::DiffEntry::REMOVED) {
            auto line = m_doc->line(item.lineNumberA - 1);
            markRedone(line.handle(), m_first);
        } else if (item.changeType == DiffUtil::DiffEntry::ADDED) {
            auto line = m_doc->line(item.lineNumberB - 1);
            markRedone(line.handle(), m_first);
        }
    }
}
