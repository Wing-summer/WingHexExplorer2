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
#include "qeditor.h"

QDocumentSwapTextCommand::QDocumentSwapTextCommand(const QString &text,
                                                   QDocument *doc, QEditor *e,
                                                   QDocumentCommand *p)
    : QDocumentCommand(Command::Custom, doc, p), _e(e) {
    Q_ASSERT(e);

    auto textl = text.split(doc->lineEndingString());
    diffs = DiffUtil::compareFiles(doc->textLines(), textl, false);
    if (diffs.isEmpty()) {
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

    _vBarValue = e->verticalScrollBar()->value();
    _hBarValue = e->horizontalScrollBar()->value();

    QVector<int> rmLineNrs;
    QVector<int> addLineNrs;
    for (auto &item : diffs) {
        if (item.changeType == DiffUtil::DiffEntry::REMOVED) {
            auto line = m_doc->line(item.lineNumberA - 1);
            rmLineNrs.append(item.lineNumberA);
        } else if (item.changeType == DiffUtil::DiffEntry::ADDED) {
            auto line = m_doc->line(item.lineNumberB - 1);
            addLineNrs.append(item.lineNumberB);
        }
    }

    removeCommonElements(rmLineNrs, addLineNrs);
    _rmRanges = getRange(rmLineNrs);
    _addRanges = getRange(addLineNrs);
}

void QDocumentSwapTextCommand::undo() {
    // breakpoints will be removed if the line handle is removed
    auto mrkinfo = QLineMarksInfoCenter::instance();
    auto marks = backupMarks();
    mrkinfo->removeMarks(_e->fileName());

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

    // so reapply is nessary
    undoApplyLineMarks(marks);

    // reset the scrollbar
    _e->verticalScrollBar()->setValue(_vBarValue);
    _e->horizontalScrollBar()->setValue(_hBarValue);
}

void QDocumentSwapTextCommand::redo() {
    // breakpoints will be removed if the line handle is removed
    auto mrkinfo = QLineMarksInfoCenter::instance();
    auto marks = backupMarks();
    mrkinfo->removeMarks(_e->fileName());

    removeLines(0, m_doc->lineCount() - 1);
    removeText(0, 0, oldheader.length());
    insertText(0, 0, header);
    insertLines(0, _handles);

    m_doc->impl()->emitContentsChange(0, _handles.count() + 1);

    // so reapply is nessary
    applyLineMarks(marks);

    _e->verticalScrollBar()->setValue(_vBarValue);
    _e->horizontalScrollBar()->setValue(_hBarValue);
}

QLineMarkList QDocumentSwapTextCommand::backupMarks() {
    auto mrkinfo = QLineMarksInfoCenter::instance();
    auto filename = _e->fileName();
    return mrkinfo->marks(filename);
}

void QDocumentSwapTextCommand::applyLineMarks(const QLineMarkList &marks) {
    auto mrkinfo = QLineMarksInfoCenter::instance();

    for (auto &m : marks) {
        int total = 0;

        auto pend = _rmRanges.lowerBound(m.line);
        for (auto p = _rmRanges.begin(); p != pend; ++p) {
            total -= *p;
        }

        pend = _addRanges.lowerBound(m.line);
        for (auto p = _addRanges.begin(); p != pend; ++p) {
            total += *p;
        }

        auto mark = m;
        mark.line += total;
        mrkinfo->addLineMark(mark);
    }
}

void QDocumentSwapTextCommand::undoApplyLineMarks(const QLineMarkList &marks) {
    auto mrkinfo = QLineMarksInfoCenter::instance();

    for (auto &m : marks) {
        int total = 0;

        auto pend = _rmRanges.lowerBound(m.line);
        for (auto p = _rmRanges.begin(); p != pend; ++p) {
            total += *p;
        }

        pend = _addRanges.lowerBound(m.line);
        for (auto p = _addRanges.begin(); p != pend; ++p) {
            total -= *p;
        }

        auto mark = m;
        mark.line += total;
        mrkinfo->addLineMark(mark);
    }
}

void QDocumentSwapTextCommand::removeCommonElements(QVector<int> &vec1,
                                                    QVector<int> &vec2) {
    int i = 0, j = 0;

    // Two-pointer technique on sorted vectors
    while (i < vec1.size() && j < vec2.size()) {
        if (vec1[i] == vec2[j]) {
            // If both elements are equal, remove them from both vectors
            vec1.remove(i);
            vec2.remove(j);
            // No need to increment i and j since we removed elements
        } else if (vec1[i] < vec2[j]) {
            // If vec1[i] is smaller, move to the next element in vec1
            ++i;
        } else {
            // If vec2[j] is smaller, move to the next element in vec2
            ++j;
        }
    }
}

QDocumentSwapTextCommand::Ranges
QDocumentSwapTextCommand::getRange(const QVector<int> &vec) {
    if (vec.isEmpty()) {
        return {};
    }

    Ranges ranges;
    auto start = vec.front();
    auto end = start;

    for (int i = 1; i < vec.size(); ++i) {
        if (vec[i] == end + 1) {
            // Continue the current range
            end = vec[i];
        } else {
            // Finalize the current range
            ranges.insert(start, end - start + 1);

            // Start a new range
            start = vec[i];
            end = start;
        }
    }

    ranges.insert(start, end - start + 1);
    return ranges;
}
