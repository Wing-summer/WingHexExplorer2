/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include "QConsoleWidget.h"

#include <QAbstractItemView>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QKeyEvent>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QScrollBar>
#include <QStringListModel>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QTextLayout>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

#include "QConsoleIODevice.h"

QConsoleWidget::QConsoleWidget(QWidget *parent)
    : WingCodeEdit(parent), m_mode(Output), m_inpos(0) {
    m_iodevice = new QConsoleIODevice(this, this);

    setTextInteractionFlags(Qt::TextEditorInteraction);
    setUndoRedoEnabled(false);

    setAutoCloseChar(true);
    setMatchBraces(true);

    setDefaultTheme();
}

QConsoleWidget::~QConsoleWidget() {}

QConsoleWidget::ConsoleMode QConsoleWidget::mode() const { return m_mode; }

void QConsoleWidget::setMode(ConsoleMode m) {
    if (m == m_mode) {
        return;
    }

    if (m == Input) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
        m_inpos = cursor.position();
        m_mode = Input;
    }

    if (m == Output) {
        m_history.deactivate();
        m_mode = Output;
    }
}

QIODevice *QConsoleWidget::device() const { return m_iodevice; }

QString QConsoleWidget::getCommandLine() {
    if (m_mode == Output) {
        return {};
    }

    QString code = currentCommandLine();
    code.replace(QChar::ParagraphSeparator, QChar::LineFeed);
    return code.trimmed();
}

void QConsoleWidget::paste() {
    if (mode() != Input || !canPaste()) {
        return;
    }
    QTextCursor textCursor = this->textCursor();
    const QMimeData *const clipboard = QApplication::clipboard()->mimeData();
    const QString text = clipboard->text();
    if (!text.isNull()) {
        m_history.deactivate();
        textCursor.insertText(text);
        setTextCursor(textCursor);
    }
}

void QConsoleWidget::handleReturnKey(Qt::KeyboardModifiers mod) {
    Q_UNUSED(mod);

    QString code = getCommandLine();

    // start new block
    appendPlainText(QString());
    setMode(Output);

    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);
    setTextCursor(textCursor);

    // Update the history
    if (!code.isEmpty()) {
        m_history.add(code);
    }

    // append the newline char and
    // send signal / update iodevice
    if (m_iodevice->isOpen()) {
        m_iodevice->consoleWidgetInput(code);
    }

    Q_EMIT sigOnConsoleCommand(code);
}

void QConsoleWidget::handleTabKey(Qt::KeyboardModifiers mod) {
    const auto key =
        mod.testFlag(Qt::ShiftModifier) ? Qt::Key_Backtab : Qt::Key_Tab;
    QKeyEvent event(QEvent::KeyPress, key, mod, QStringLiteral("\t"));
    WingCodeEdit::keyPressEvent(&event);
}

bool QConsoleWidget::handleBackspaceAtInputStart() { return false; }

void QConsoleWidget::prepareInputNavigation() {}

void QConsoleWidget::handleHistoryKey(QKeyEvent *event) {
    prepareInputNavigation();
    const auto up = event->key() == Qt::Key_Up;
    const auto modifiers = event->modifiers();
    event->accept();
    auto doc = this->document();
    auto cursor = textCursor();
    const auto first = doc->findBlock(m_inpos);
    const auto last = doc->lastBlock();
    auto block = cursor.block();
    doc->documentLayout()->blockBoundingRect(block);
    const auto line =
        block.layout()->lineForTextPosition(cursor.positionInBlock());
    const auto firstLine =
        first.layout()->lineForTextPosition(m_inpos - first.position());
    const auto atBoundary =
        up ? block == first && (!line.isValid() || !firstLine.isValid() ||
                                line.lineNumber() <= firstLine.lineNumber())
           : block == last &&
                 (!line.isValid() ||
                  line.lineNumber() == last.layout()->lineCount() - 1);

    if (modifiers != Qt::NoModifier || !atBoundary) {
        if (cursor.anchor() < m_inpos) {
            cursor.setPosition(m_inpos);
        }
        const auto anchorMode = modifiers.testFlag(Qt::ShiftModifier)
                                    ? QTextCursor::KeepAnchor
                                    : QTextCursor::MoveAnchor;
        cursor.movePosition(up ? QTextCursor::Up : QTextCursor::Down,
                            anchorMode);
        cursor.setPosition(
            qMax(cursor.position(), inputStartForBlock(cursor.block())),
            anchorMode);
        setTextCursor(cursor);
        ensureCursorVisible();
        return;
    }

    if (!m_history.isActive()) {
        if (!up) {
            return;
        }
        auto code = currentCommandLine();
        code.replace(QChar::ParagraphSeparator, QChar::LineFeed);
        m_history.activate(code);
    }
    if (m_history.move(up)) {
        replaceCommandLine(m_history.currentValue());
        cursor = textCursor();
        if (up) {
            cursor.setPosition(m_inpos);
        } else {
            cursor.movePosition(QTextCursor::End);
        }
        setTextCursor(cursor);
        if (m_history.pos == -1) {
            m_history.deactivate();
        }
    } else {
        if (m_history.pos == -1) {
            m_history.deactivate();
        }
        QApplication::beep();
    }
}

void QConsoleWidget::keyPressEvent(QKeyEvent *e) {
    if (processCompletionBegin(e)) {
        return;
    }

    int key = e->key();
    if (mode() == Input && (key == Qt::Key_Up || key == Qt::Key_Down ||
                            key == Qt::Key_Tab || key == Qt::Key_Backtab)) {
        prepareInputNavigation();
    }

    QTextCursor textCursor = this->textCursor();
    bool selectionInEditZone = isSelectionInEditZone();

    auto mods = e->modifiers();

    // check for user abort request
    if (mods == Qt::ControlModifier) {
        switch (key) {
        case Qt::Key_Q:
            Q_EMIT sigOnAbortEvaluation();
            e->accept();
            return;
        case Qt::Key_C:
            // Allow copying anywhere in the console ...
            if (textCursor.hasSelection()) {
                copy();
            }
            e->accept();
            return;
        }
    }

    // the rest of key events are ignored during output mode
    if (mode() != Input) {
        e->ignore();
        return;
    }

    if (mods == Qt::ControlModifier) {
        switch (key) {
        case Qt::Key_X:
            // Allow cut only if the selection is limited to the interactive
            // area
            if (selectionInEditZone) {
                m_history.deactivate();
                cut();
            }
            e->accept();
            return;
        case Qt::Key_V:
            // Allow paste only if the selection is in the interactive area
            if (selectionInEditZone || isCursorInEditZone()) {
                paste();
            }

            e->accept();
            return;
        case Qt::Key_Backspace:
            e->accept();
            m_history.deactivate();
            replaceCommandLine(QString());
            return;
        }
    }

    const auto shiftMod = mods.testFlag(Qt::ShiftModifier);

    const auto navigationKey =
        key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Left ||
        key == Qt::Key_Right || key == Qt::Key_Home || key == Qt::Key_End ||
        key == Qt::Key_PageUp || key == Qt::Key_PageDown ||
        key == Qt::Key_Shift || key == Qt::Key_Control || key == Qt::Key_Alt;
    if (m_history.isActive() && !navigationKey) {
        m_history.deactivate();
    }

    // Force the cursor back to the interactive area
    // for all keys except modifiers
    if (!isCursorInEditZone() && key != Qt::Key_Control &&
        key != Qt::Key_Shift && key != Qt::Key_Alt) {
        auto block = textCursor.block();
        if (block.position() >= document()->findBlock(m_inpos).position()) {
            textCursor.setPosition(inputStartForBlock(block));
        } else {
            textCursor.movePosition(QTextCursor::End);
        }
        setTextCursor(textCursor);
    }

    switch (key) {
    case Qt::Key_Up:
    case Qt::Key_Down:
        handleHistoryKey(e);
        break;

    case Qt::Key_Left: {
        auto pos = textCursor.position();
        auto block = textCursor.block();
        if ((!textCursor.hasSelection() || shiftMod) &&
            pos == inputStartForBlock(block) && pos > m_inpos) {
            textCursor.movePosition(QTextCursor::PreviousBlock,
                                    shiftMod ? QTextCursor::KeepAnchor
                                             : QTextCursor::MoveAnchor);
            textCursor.movePosition(QTextCursor::EndOfBlock,
                                    shiftMod ? QTextCursor::KeepAnchor
                                             : QTextCursor::MoveAnchor);
            setTextCursor(textCursor);
        } else if (pos > inputStartForBlock(block)) {
            processDefaultKeyPressEvent(e);
            auto cursor = this->textCursor();
            if (cursor.position() < inputStartForBlock(block)) {
                cursor.setPosition(inputStartForBlock(block),
                                   shiftMod ? QTextCursor::KeepAnchor
                                            : QTextCursor::MoveAnchor);
                setTextCursor(cursor);
            }
        } else {
            QApplication::beep();
            e->accept();
        }
    } break;

    case Qt::Key_Right: {
        auto block = textCursor.block();
        auto nextBlock = block.next();
        if (textCursor.atBlockEnd() && nextBlock.isValid()) {
            textCursor.setPosition(inputStartForBlock(nextBlock),
                                   shiftMod ? QTextCursor::KeepAnchor
                                            : QTextCursor::MoveAnchor);
            setTextCursor(textCursor);
        } else {
            processDefaultKeyPressEvent(e);
        }
    } break;

    case Qt::Key_Delete:
        e->accept();
        if (selectionInEditZone) {
            cut();
        } else {
            auto block = textCursor.block();
            auto nextBlock = block.next();
            // cursor must be in edit zone
            if (textCursor.hasSelection() ||
                textCursor.selectionStart() < m_inpos) {
                QApplication::beep();
            } else if (!textCursor.hasSelection() && textCursor.atBlockEnd() &&
                       nextBlock.isValid()) {
                textCursor.setPosition(inputStartForBlock(nextBlock),
                                       QTextCursor::KeepAnchor);
                textCursor.removeSelectedText();
                setTextCursor(textCursor);
            } else {
                processDefaultKeyPressEvent(e);
            }
        }
        break;

    case Qt::Key_Backspace: {
        e->accept();
        auto block = textCursor.block();
        if (!textCursor.hasSelection() &&
            textCursor.position() == inputStartForBlock(block) &&
            e->modifiers() == Qt::NoModifier && handleBackspaceAtInputStart()) {
            break;
        }
        if (selectionInEditZone) {
            cut();
        } else {
            // cursor must be in edit zone
            if (textCursor.hasSelection() ||
                textCursor.selectionStart() <= inputStartForBlock(block)) {
                QApplication::beep();
            } else {
                processDefaultKeyPressEvent(e);
            }
        }
    } break;

    case Qt::Key_Tab:
    case Qt::Key_Backtab:
        e->accept();
        if (textCursor.hasSelection() && !selectionInEditZone) {
            return;
        }
        handleTabKey(key == Qt::Key_Backtab ? mods | Qt::ShiftModifier : mods);
        return;

    case Qt::Key_Home:
        e->accept();
        textCursor.setPosition(mods.testFlag(Qt::ControlModifier)
                                   ? m_inpos
                                   : inputStartForBlock(textCursor.block()),
                               shiftMod ? QTextCursor::KeepAnchor
                                        : QTextCursor::MoveAnchor);
        setTextCursor(textCursor);
        break;

    case Qt::Key_Enter:
    case Qt::Key_Return:
        e->accept();
        handleReturnKey(e->modifiers());
        break;

    default:
        e->accept();
        if (textCursor.hasSelection() && !selectionInEditZone &&
            !e->text().isEmpty()) {
            return;
        }
        processDefaultKeyPressEvent(e);
        break;
    }

    if (processCompletionEnd(e)) {
        return;
    }
}

void QConsoleWidget::contextMenuEvent(QContextMenuEvent *event) {
    QMenu *menu = createStandardContextMenu();

    QAction *a;
    if ((a = menu->findChild<QAction *>("edit-cut"))) {
        a->setEnabled(canCut());
    }
    if ((a = menu->findChild<QAction *>("edit-delete"))) {
        a->setEnabled(canCut());
    }
    if ((a = menu->findChild<QAction *>("edit-paste"))) {
        a->setEnabled(canPaste());
    }

    menu->exec(event->globalPos());
    delete menu;
}

bool QConsoleWidget::isSelectionInEditZone() const {
    QTextCursor textCursor = this->textCursor();
    if (!textCursor.hasSelection()) {
        return false;
    }

    return canPaste();
}

bool QConsoleWidget::isCursorInEditZone() const {
    auto textCursor = this->textCursor();
    return textCursor.position() >= inputStartForBlock(textCursor.block());
}

bool QConsoleWidget::canPaste() const {
    QTextCursor textCursor = this->textCursor();
    if (textCursor.position() < inputStartForBlock(textCursor.block())) {
        return false;
    }
    textCursor.setPosition(textCursor.anchor());
    if (textCursor.position() < inputStartForBlock(textCursor.block())) {
        return false;
    }
    return true;
}

bool QConsoleWidget::canCut() const { return isSelectionInEditZone(); }

void QConsoleWidget::replaceCommandLine(const QString &str) {

    // Select the text after the last command prompt ...
    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);
    textCursor.setPosition(m_inpos, QTextCursor::KeepAnchor);

    // ... and replace it with new string.
    textCursor.insertText(str);

    // move to the end of the document
    textCursor.movePosition(QTextCursor::End);
    setTextCursor(textCursor);
}

QString QConsoleWidget::currentCommandLine() const {
    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);
    textCursor.setPosition(m_inpos, QTextCursor::KeepAnchor);
    return textCursor.selectedText();
}

int QConsoleWidget::inputStartForBlock(const QTextBlock &block) const {
    return qMax(m_inpos, block.position());
}

int QConsoleWidget::blockTextStart(const QTextBlock &block) const {
    if (block.position() < document()->findBlock(m_inpos).position()) {
        return -1;
    }
    return inputStartForBlock(block) - block.position();
}

void QConsoleWidget::dragMoveEvent(QDragMoveEvent *e) { e->ignore(); }

int QConsoleWidget::currentHeaderPos() const { return m_inpos; }

int QConsoleWidget::write(const QString &message) {
    QTextCursor tc = textCursor();

    if (mode() == Input) {
        // in Input mode output messages are inserted
        // before the edit block

        // get offset of current pos from the end
        int editpos = tc.position();
        tc.movePosition(QTextCursor::End);
        editpos = tc.position() - editpos;

        // convert the input pos as relative from the end
        m_inpos = tc.position() - m_inpos;

        // insert block
        tc.movePosition(QTextCursor::StartOfBlock);
        tc.insertBlock();
        auto id = tc.blockNumber();
        tc.movePosition(QTextCursor::PreviousBlock);

        tc.insertText(message);
        tc.movePosition(QTextCursor::End);
        // restore input pos
        m_inpos = tc.position() - m_inpos;
        // restore the edit pos
        tc.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, editpos);

        setTextCursor(tc);
        return id;
    } else {
        // in output mode messages are ed
        QTextCursor tc1 = tc;
        tc1.movePosition(QTextCursor::End);

        // check is cursor was not at the end
        // (e.g. had been moved per mouse action)
        bool needsRestore = tc1.position() != tc.position();

        // insert text
        setTextCursor(tc1);
        auto id = tc1.blockNumber();
        tc1.insertText(message);
        ensureCursorVisible();

        // restore cursor if needed
        if (needsRestore) {
            setTextCursor(tc);
        }

        return id;
    }
}

QConsoleWidget::History &QConsoleWidget::history() { return m_history; }

/////////////////// QConsoleWidget::History /////////////////////

QConsoleWidget::History QConsoleWidget::m_history;

QConsoleWidget::History::History(void) : pos(0), active(false), maxSize(100) {}

QConsoleWidget::History::~History(void) {}

void QConsoleWidget::History::add(const QString &str) {
    active = false;
    auto idx = strings.indexOf(str);
    if (idx >= 0) {
        strings.move(idx, 0);
        return;
    }
    if (strings.size() == maxSize) {
        strings.pop_back();
    }
    strings.push_front(str);
}

void QConsoleWidget::History::activate(const QString &tk) {
    active = true;
    token = tk;
    pos = -1;
}

bool QConsoleWidget::History::move(bool dir) {
    if (active) {
        int next = indexOf(dir, pos);
        if (pos != next) {
            pos = next;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

int QConsoleWidget::History::indexOf(bool dir, int from) const {
    return qBound(-1, from + (dir ? 1 : -1), int(strings.size()) - 1);
}

/////////////////// Stream manipulators /////////////////////

QTextStream &waitForInput(QTextStream &s) {
    QConsoleIODevice *d = qobject_cast<QConsoleIODevice *>(s.device());
    if (d) {
        d->waitForReadyRead(-1);
    }
    return s;
}

QTextStream &inputMode(QTextStream &s) {
    QConsoleIODevice *d = qobject_cast<QConsoleIODevice *>(s.device());
    if (d && d->widget()) {
        d->widget()->setMode(QConsoleWidget::Input);
    }
    return s;
}

QTextStream &outChannel(QTextStream &s) {
    QConsoleIODevice *d = qobject_cast<QConsoleIODevice *>(s.device());
    if (d) {
        d->setCurrentWriteChannel(QConsoleWidget::StandardOutput);
    }
    return s;
}

QTextStream &errChannel(QTextStream &s) {
    QConsoleIODevice *d = qobject_cast<QConsoleIODevice *>(s.device());
    if (d) {
        d->setCurrentWriteChannel(QConsoleWidget::StandardError);
    }
    return s;
}

void QConsoleWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        copy();
        QTextCursor cursor = cursorForPosition(event->pos());
        setTextCursor(cursor);
        paste();
        return;
    }
    WingCodeEdit::mousePressEvent(event);
}

void QConsoleWidget::insertFromMimeData(const QMimeData *source) {
    if (mode() == Input && canPaste()) {
        m_history.deactivate();
        WingCodeEdit::insertFromMimeData(source);
    }
}
