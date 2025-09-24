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

#include "hexlineedit.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QEvent>
#include <QMenu>

HexLineEdit::HexLineEdit(QWidget *parent)
    : QLineEdit(parent), m_mode(TextMode), m_undoLimit(100), m_undoIndex(-1),
      m_isUndoing(false), m_isRedoing(false), m_mergeEnabled(true) {
    // monospace looks nicer for hex
    QFont f = font();
    f.setFamily("Monospace");
    f.setStyleHint(QFont::Monospace);
    setFont(f);

    setAttribute(Qt::WA_InputMethodEnabled, false);
}

void HexLineEdit::setMode(Mode m) {
    if (m_mode == m)
        return;

    if (m_mode == HexMode) {
        saveUndoRecord(UndoRecord::ModeChange, m_raw,
                       logicalPosFromDisplayPos(cursorPosition()),
                       logicalPosFromDisplayPos(cursorPosition()));
    }

    m_mode = m;
    if (m_mode == TextMode) {
        // show raw (no spaces) when back to text mode
        setText(m_raw);
    } else {
        // enter hex mode: filter current visible text to raw hex/? and show
        // formatted
        m_raw = filteredRaw(text());
        setText(formatRawToDisplay(m_raw));
        setCursorPosition(displayPosFromLogicalPos(m_raw.size()));
    }
    Q_EMIT modeChanged(m_mode);
}

void HexLineEdit::setHexText(const QString &hexish) {
    auto oldCursor = logicalPosFromDisplayPos(cursorPosition());

    m_raw = filteredRaw(hexish);
    setMode(HexMode);
    setText(formatRawToDisplay(m_raw));
    setCursorPosition(displayPosFromLogicalPos(m_raw.size()));

    saveUndoRecord(UndoRecord::ModeChange, m_raw, oldCursor, m_raw.size());
}

QString HexLineEdit::rawHexText() const { return m_raw; }

bool HexLineEdit::isValid() const {
    if (m_mode == TextMode) {
        return true;
    }

    return (m_raw.size() % 2) == 0;
}

qsizetype HexLineEdit::undoLimit() const { return m_undoLimit; }

void HexLineEdit::undo() {
    if (m_undoIndex < 0 || m_isUndoing || m_isRedoing)
        return;

    m_isUndoing = true;

    const UndoRecord &record = m_undoStack[m_undoIndex];

    UndoRecord reverseRecord = record.reverse();
    m_raw = reverseRecord.applyTo(m_raw);

    refreshDisplayAndSetCursor(reverseRecord.cursorAfter);

    if (reverseRecord.hasSelection) {
        auto selStart = displayPosFromLogicalPos(reverseRecord.selectionStart);
        auto selEnd = displayPosFromLogicalPos(reverseRecord.selectionEnd);
        setSelection(selStart, selEnd - selStart);
    }

    m_undoIndex--;
    Q_EMIT undoAvailable(m_undoIndex >= 0);
    Q_EMIT redoAvailable(m_undoIndex < m_undoStack.size() - 1);

    m_isUndoing = false;
}

void HexLineEdit::redo() {
    if (m_undoIndex >= m_undoStack.size() - 1 || m_isUndoing || m_isRedoing) {
        return;
    }

    m_isRedoing = true;

    m_undoIndex++;
    const UndoRecord &record = m_undoStack[m_undoIndex];

    m_raw = record.applyTo(m_raw);

    refreshDisplayAndSetCursor(record.cursorAfter);

    if (record.hasSelection) {
        auto selStart = displayPosFromLogicalPos(record.selectionStart);
        auto selEnd = displayPosFromLogicalPos(record.selectionEnd);
        setSelection(selStart, selEnd - selStart);
    }

    Q_EMIT undoAvailable(true);
    Q_EMIT redoAvailable(m_undoIndex < m_undoStack.size() - 1);

    m_isRedoing = false;
}

void HexLineEdit::clearUndoHistory() {
    m_undoStack.clear();
    m_undoIndex = -1;
    Q_EMIT undoAvailable(false);
    Q_EMIT redoAvailable(false);
}

void HexLineEdit::setUndoLimit(qsizetype limit) {
    m_undoLimit = limit;
    if (m_undoStack.size() > m_undoLimit) {
        m_undoStack = m_undoStack.sliced(m_undoStack.size() - m_undoLimit);
        m_undoIndex = qMin(m_undoIndex, m_undoStack.size() - 1);
    }
}

void HexLineEdit::contextMenuEvent(QContextMenuEvent *event) {
    QMenu *menu = createStandardContextMenu();

    if (m_mode == HexMode) {
        QList<QAction *> actions = menu->actions();
        for (auto &action : actions) {
            menu->removeAction(action);
            delete action;
        }

        QAction *undoAction = new QAction(tr("Undo"), menu);
        undoAction->setShortcut(QKeySequence::Undo);
        undoAction->setEnabled(m_undoIndex >= 0);
        connect(undoAction, &QAction::triggered, this, &HexLineEdit::undo);
        connect(this, &HexLineEdit::undoAvailable, undoAction,
                &QAction::setEnabled);

        QAction *redoAction = new QAction(tr("Redo"), menu);
        redoAction->setShortcut(QKeySequence::Redo);
        redoAction->setEnabled(m_undoIndex < m_undoStack.size() - 1);
        connect(redoAction, &QAction::triggered, this, &HexLineEdit::redo);
        connect(this, &HexLineEdit::redoAvailable, redoAction,
                &QAction::setEnabled);

        menu->addAction(undoAction);
        menu->addAction(redoAction);
        menu->addSeparator();

        QAction *copyAction = new QAction(tr("Copy"), menu);
        copyAction->setShortcut(QKeySequence::Copy);
        copyAction->setEnabled(hasSelectedText());
        connect(copyAction, &QAction::triggered, this,
                &HexLineEdit::copySelectedHex);

        QAction *cutAction = new QAction(tr("Cut"), menu);
        cutAction->setShortcut(QKeySequence::Cut);
        cutAction->setEnabled(hasSelectedText());
        connect(cutAction, &QAction::triggered, this,
                &HexLineEdit::cutSelectedHex);

        QAction *pasteAction = new QAction(tr("Paste"), menu);
        pasteAction->setShortcut(QKeySequence::Paste);
        connect(pasteAction, &QAction::triggered, this, [this]() {
            handlePasteText(QApplication::clipboard()->text());
        });

        QAction *selectAllAction = new QAction(tr("Select all"), menu);
        selectAllAction->setShortcut(QKeySequence::SelectAll);
        connect(selectAllAction, &QAction::triggered, this,
                &QLineEdit::selectAll);

        menu->addAction(copyAction);
        menu->addAction(cutAction);
        menu->addAction(pasteAction);
        menu->addSeparator();
        menu->addAction(selectAllAction);
    }

    menu->exec(event->globalPos());
    delete menu;
}

void HexLineEdit::copySelectedHex() {
    if (!hasSelectedText())
        return;

    auto selStart = selectionStart();
    auto selEnd = selStart + selectionLength();
    auto logicalStart = logicalPosFromDisplayPos(selStart);
    auto logicalEnd = logicalPosFromDisplayPos(selEnd);

    auto selectedHex = m_raw.sliced(logicalStart, logicalEnd - logicalStart);
    QApplication::clipboard()->setText(selectedHex);
}

void HexLineEdit::cutSelectedHex() {
    if (!hasSelectedText())
        return;

    auto selStart = selectionStart();
    auto selEnd = selStart + selectionLength();
    auto logicalStart = logicalPosFromDisplayPos(selStart);
    auto logicalEnd = logicalPosFromDisplayPos(selEnd);

    QString removedText = m_raw.sliced(logicalStart, logicalEnd - logicalStart);
    auto oldCursor = logicalPosFromDisplayPos(cursorPosition());

    saveUndoRecord(UndoRecord::Replace, logicalStart, removedText, {},
                   oldCursor, logicalStart, true, logicalStart, logicalEnd);

    copySelectedHex();
    m_raw.remove(logicalStart, logicalEnd - logicalStart);
    refreshDisplayAndSetCursor(logicalStart);
}

void HexLineEdit::keyPressEvent(QKeyEvent *ev) {
    if (m_mode == TextMode) {
        if (ev->matches(QKeySequence::Undo)) {
            QLineEdit::undo();
            m_raw = text();
            return;
        }
        if (ev->matches(QKeySequence::Redo)) {
            QLineEdit::redo();
            m_raw = text();
            return;
        }

        QLineEdit::keyPressEvent(ev);
        m_raw = text();
        return;
    }

    int key = ev->key();
    bool ctrl = ev->modifiers() & (Qt::ControlModifier | Qt::MetaModifier);

    if (ctrl && key == Qt::Key_Z) {
        if (ev->modifiers() & Qt::ShiftModifier) {
            redo();
        } else {
            undo();
        }
        return;
    }

    if (ctrl && key == Qt::Key_Y) {
        redo();
        return;
    }

    if (ctrl && key == Qt::Key_C) {
        copySelectedHex();
        return;
    }

    if (ctrl && key == Qt::Key_X) {
        cutSelectedHex();
        return;
    }

    if (ctrl && key == Qt::Key_V) {
        handlePasteText(QApplication::clipboard()->text());
        return;
    }

    if (ctrl && key == Qt::Key_A) {
        selectAll();
        return;
    }

    auto txt = ev->text();

    if (!txt.isEmpty() && txt[0] == '?') {
        auto disp = cursorPosition();
        auto logical = logicalPosFromDisplayPos(disp);
        auto oldCursor = logical;

        if (hasSelection()) {
            auto selStart = selectionStart();
            auto selEnd = selStart + selectionLength();
            auto logicalStart = logicalPosFromDisplayPos(selStart);
            auto logicalEnd = logicalPosFromDisplayPos(selEnd);
            QString removedText =
                m_raw.sliced(logicalStart, logicalEnd - logicalStart);

            replaceSelectionWithRaw(QString('?'));

            saveUndoRecord(UndoRecord::Replace, logicalStart, removedText,
                           QStringLiteral("?"), oldCursor, logicalStart + 1,
                           true, logicalStart, logicalEnd);
        } else {
            insertAtLogicalPos(QString('?'), logical);
            refreshDisplayAndSetCursor(logical + 1);

            saveUndoRecord(UndoRecord::Insert, logical, {}, QStringLiteral("?"),
                           oldCursor, logical + 1);
        }
        return;
    }

    // allow single ASCII hex digit
    if (!txt.isEmpty() && isHexOrWildcard(txt[0]) &&
        (ev->modifiers() == Qt::NoModifier ||
         ev->modifiers() == Qt::ShiftModifier)) {
        auto disp = cursorPosition();
        auto logical = logicalPosFromDisplayPos(disp);
        auto oldCursor = logical;
        QChar ch = txt[0].toUpper();

        if (hasSelection()) {
            auto selStart = selectionStart();
            auto selEnd = selStart + selectionLength();
            auto logicalStart = logicalPosFromDisplayPos(selStart);
            auto logicalEnd = logicalPosFromDisplayPos(selEnd);
            QString removedText =
                m_raw.sliced(logicalStart, logicalEnd - logicalStart);

            replaceSelectionWithRaw(QString(ch));

            saveUndoRecord(UndoRecord::Replace, logicalStart, removedText,
                           QString(ch), oldCursor, logicalStart + 1, true,
                           logicalStart, logicalEnd);
        } else {
            insertAtLogicalPos(QString(ch), logical);
            refreshDisplayAndSetCursor(logical + 1);

            saveUndoRecord(UndoRecord::Insert, logical, "", QString(ch),
                           oldCursor, logical + 1);
        }
        return;
    }

    // Backspace
    if (key == Qt::Key_Backspace) {
        auto oldCursor = logicalPosFromDisplayPos(cursorPosition());

        if (hasSelection()) {
            auto selStart = selectionStart();
            auto selEnd = selStart + selectionLength();
            auto logicalStart = logicalPosFromDisplayPos(selStart);
            auto logicalEnd = logicalPosFromDisplayPos(selEnd);
            QString removedText =
                m_raw.sliced(logicalStart, logicalEnd - logicalStart);

            removeLogicalRange(logicalStart, logicalEnd - logicalStart);
            refreshDisplayAndSetCursor(logicalStart);

            saveUndoRecord(UndoRecord::Replace, logicalStart, removedText, {},
                           oldCursor, logicalStart, true, logicalStart,
                           logicalEnd);
        } else {
            auto disp = cursorPosition();
            auto logical = logicalPosFromDisplayPos(disp);
            if (logical > 0) {
                QChar removedChar = m_raw.at(logical - 1);
                removeLogicalRange(logical - 1, 1);
                refreshDisplayAndSetCursor(logical - 1);

                saveUndoRecord(UndoRecord::Delete, logical - 1,
                               QString(removedChar), {}, oldCursor,
                               logical - 1);
            }
        }
        return;
    }

    // Delete
    if (key == Qt::Key_Delete) {
        auto oldCursor = logicalPosFromDisplayPos(cursorPosition());

        if (hasSelection()) {
            auto selStart = selectionStart();
            auto selEnd = selStart + selectionLength();
            auto logicalStart = logicalPosFromDisplayPos(selStart);
            auto logicalEnd = logicalPosFromDisplayPos(selEnd);
            QString removedText =
                m_raw.sliced(logicalStart, logicalEnd - logicalStart);

            removeLogicalRange(logicalStart, logicalEnd - logicalStart);
            refreshDisplayAndSetCursor(logicalStart);

            saveUndoRecord(UndoRecord::Replace, logicalStart, removedText, {},
                           oldCursor, logicalStart, true, logicalStart,
                           logicalEnd);
        } else {
            auto disp = cursorPosition();
            auto logical = logicalPosFromDisplayPos(disp);
            if (logical < m_raw.size()) {
                QChar removedChar = m_raw.at(logical);
                removeLogicalRange(logical, 1);
                refreshDisplayAndSetCursor(logical);

                saveUndoRecord(UndoRecord::Delete, logical,
                               QString(removedChar), {}, oldCursor, logical);
            }
        }
        return;
    }

    // Left / Right navigation by logical nibble
    if (key == Qt::Key_Left || key == Qt::Key_Right) {
        auto disp = cursorPosition();
        auto logical = logicalPosFromDisplayPos(disp);
        if (key == Qt::Key_Left) {
            if (logical > 0) {
                logical--;
            }
        } else {
            if (logical < m_raw.size()) {
                logical++;
            }
        }
        setCursorPosition(displayPosFromLogicalPos(logical));
        return;
    }

    // Home/End fallback
    if (key == Qt::Key_Home) {
        setCursorPosition(0);
        return;
    }
    if (key == Qt::Key_End) {
        setCursorPosition(text().size());
        return;
    }

    ev->ignore();
}

void HexLineEdit::mousePressEvent(QMouseEvent *ev) {
    QLineEdit::mousePressEvent(ev);
    if (m_mode == HexMode) {
        auto disp = cursorPosition();
        auto logical = logicalPosFromDisplayPos(disp);
        setCursorPosition(displayPosFromLogicalPos(logical));
    }
}

void HexLineEdit::inputMethodEvent(QInputMethodEvent *event) {
    if (m_mode == TextMode) {
        QLineEdit::inputMethodEvent(event);
        return;
    }
    event->ignore();
}

QVariant HexLineEdit::inputMethodQuery(Qt::InputMethodQuery query) const {
    if (m_mode == HexMode && query == Qt::ImEnabled) {
        return false;
    }
    return QLineEdit::inputMethodQuery(query);
}

void HexLineEdit::saveUndoRecord(UndoRecord::OperationType type,
                                 const QString &fullText, int cursorBefore,
                                 int cursorAfter) {
    saveUndoRecord(UndoRecord(type, fullText, cursorBefore, cursorAfter));
}

void HexLineEdit::saveUndoRecord(UndoRecord::OperationType type, int position,
                                 const QString &removed,
                                 const QString &inserted, int cursorBefore,
                                 int cursorAfter, bool hasSelection,
                                 int selStart, int selEnd) {
    saveUndoRecord(UndoRecord(type, position, removed, inserted, cursorBefore,
                              cursorAfter, hasSelection, selStart, selEnd));
}

void HexLineEdit::saveUndoRecord(const UndoRecord &newRecord) {
    if (m_isUndoing || m_isRedoing)
        return;

    if (m_mergeEnabled && m_undoIndex >= 0 &&
        m_undoIndex < m_undoStack.size()) {
        UndoRecord &lastRecord = m_undoStack[m_undoIndex];

        if (lastRecord.canMergeWith(newRecord)) {
            lastRecord.mergeWith(newRecord);
            Q_EMIT undoAvailable(true);
            Q_EMIT redoAvailable(false);
            return;
        }
    }

    if (m_undoIndex < m_undoStack.size() - 1) {
        m_undoStack = m_undoStack.sliced(0, m_undoIndex + 1);
    }

    m_undoStack.append(newRecord);
    m_undoIndex++;

    if (m_undoStack.size() > m_undoLimit) {
        m_undoStack.removeFirst();
        m_undoIndex--;
    }

    Q_EMIT undoAvailable(true);
    Q_EMIT redoAvailable(false);
}

bool HexLineEdit::isHexOrWildcard(QChar c) {
    if (c == QChar('?'))
        return true;
    ushort u = c.unicode();
    if ((u >= '0' && u <= '9') || (u >= 'A' && u <= 'F') ||
        (u >= 'a' && u <= 'f'))
        return true;
    return false;
}

quint8 HexLineEdit::hexCharToValue(QChar c) {
    ushort u = c.toUpper().unicode();
    if (u >= '0' && u <= '9')
        return quint8(u - '0');
    return quint8(10 + (u - 'A'));
}

QString HexLineEdit::filteredRaw(const QString &s) const {
    QString out;
    out.reserve(s.size());
    for (QChar c : s) {
        if (isHexOrWildcard(c)) {
            out.append(c.toUpper());
        }
    }
    return out;
}

QString HexLineEdit::formatRawToDisplay(const QString &raw) const {
    QString out;
    out.reserve(raw.size() + raw.size() / 2 + 2);
    for (int i = 0; i < raw.size(); ++i) {
        out.append(raw[i]);
        if ((i % 2) == 1 && i != raw.size() - 1) {
            out.append(' ');
        }
    }
    return out;
}

qsizetype HexLineEdit::logicalPosFromDisplayPos(qsizetype displayPos) const {
    QString disp = text();
    auto upto = qBound(qsizetype(0), displayPos, disp.size());
    auto count = 0;
    for (qsizetype i = 0; i < upto; ++i) {
        if (disp[i] != ' ') {
            ++count;
        }
    }
    return count;
}

qsizetype HexLineEdit::displayPosFromLogicalPos(qsizetype logicalPos) const {
    QString disp = text();
    qsizetype nonspace = 0;
    for (qsizetype i = 0; i < disp.size(); ++i) {
        if (disp[i] != ' ') {
            if (nonspace == logicalPos) {
                return i;
            }
            ++nonspace;
        }
    }
    return disp.size();
}

void HexLineEdit::insertAtLogicalPos(const QString &hexChars,
                                     qsizetype logicalPos) {
    if (hexChars.isEmpty()) {
        return;
    }
    QString filtered;
    for (auto &c : hexChars) {
        if (isHexOrWildcard(c)) {
            filtered.append(c.toUpper());
        }
    }
    if (filtered.isEmpty()) {
        return;
    }
    qsizetype lp = qBound(qsizetype(0), logicalPos, m_raw.size());
    m_raw.insert(lp, filtered);
}

void HexLineEdit::removeLogicalRange(qsizetype logicalFrom,
                                     qsizetype logicalLen) {
    if (logicalLen <= 0) {
        return;
    }
    qsizetype from = qBound(qsizetype(0), logicalFrom, m_raw.size());
    qsizetype len = qBound(qsizetype(0), logicalLen, m_raw.size() - from);
    m_raw.remove(from, len);
}

void HexLineEdit::refreshDisplayAndSetCursor(qsizetype logicalCursorPos) {
    QString formatted = formatRawToDisplay(m_raw);
    blockSignals(true);
    setText(formatted);
    blockSignals(false);
    auto disp = displayPosFromLogicalPos(
        qBound(qsizetype(0), logicalCursorPos, m_raw.size()));
    setCursorPosition(disp);
}

bool HexLineEdit::hasSelection() const {
    return selectionStart() >= 0 && selectionLength() > 0;
}

void HexLineEdit::replaceSelectionWithRaw(const QString &newRawChunk) {
    if (!hasSelection())
        return;
    auto selStart = selectionStart();
    auto selLen = selectionLength();
    auto selEnd = selStart + selLen;
    auto lstart = logicalPosFromDisplayPos(selStart);
    auto lend = logicalPosFromDisplayPos(selEnd);
    removeLogicalRange(lstart, lend - lstart);
    if (!newRawChunk.isEmpty())
        insertAtLogicalPos(newRawChunk, lstart);
    refreshDisplayAndSetCursor(lstart + newRawChunk.size());
}

void HexLineEdit::handlePasteText(const QString &txt) {
    if (txt.isEmpty())
        return;

    int oldCursor = logicalPosFromDisplayPos(cursorPosition());
    QString filtered = filteredRaw(txt);
    if (filtered.isEmpty())
        return;

    if (hasSelection()) {
        auto selStart = selectionStart();
        auto selEnd = selStart + selectionLength();
        auto logicalStart = logicalPosFromDisplayPos(selStart);
        auto logicalEnd = logicalPosFromDisplayPos(selEnd);
        QString removedText =
            m_raw.sliced(logicalStart, logicalEnd - logicalStart);

        replaceSelectionWithRaw(filtered);

        saveUndoRecord(UndoRecord::Paste, logicalStart, removedText, filtered,
                       oldCursor, logicalStart + filtered.size(), true,
                       logicalStart, logicalEnd);
    } else {
        auto disp = cursorPosition();
        auto logical = logicalPosFromDisplayPos(disp);

        insertAtLogicalPos(filtered, logical);
        refreshDisplayAndSetCursor(logical + filtered.size());

        saveUndoRecord(UndoRecord::Paste, logical, "", filtered, oldCursor,
                       logical + filtered.size());
    }
}

QVector<HexLineEdit::HexByte> HexLineEdit::hexBytes() const {
    QVector<HexByte> out;
    int i = 0;
    while (i < m_raw.size()) {
        HexByte b;
        QChar c1 = m_raw.at(i++);
        if (c1 == QChar('?')) {
            b.high.hasValue = false;
            b.high.isWildcard = true;
        } else {
            b.high.hasValue = true;
            b.high.isWildcard = false;
            b.high.value = hexCharToValue(c1);
        }
        if (i < m_raw.size()) {
            QChar c2 = m_raw.at(i++);
            if (c2 == QChar('?')) {
                b.low.hasValue = false;
                b.low.isWildcard = true;
            } else {
                b.low.hasValue = true;
                b.low.isWildcard = false;
                b.low.value = hexCharToValue(c2);
            }
        } else {
            b.low.hasValue = false;
            b.low.isWildcard = false;
        }
        out.append(b);
    }
    return out;
}

HexLineEdit::Mode HexLineEdit::mode() const { return m_mode; }
