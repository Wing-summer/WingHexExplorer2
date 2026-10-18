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
#include "QConsoleIODevice.h"

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

QConsoleWidget::QConsoleWidget(QWidget *parent)
    : WingCodeEdit(parent), mode_(Output), inpos_(0) {
    iodevice_ = new QConsoleIODevice(this, this);

    setTextInteractionFlags(Qt::TextEditorInteraction);
    setUndoRedoEnabled(false);

    setAutoCloseChar(true);
    setMatchBraces(true);

    setDefaultTheme();
}

QConsoleWidget::~QConsoleWidget() {}

QConsoleWidget::ConsoleMode QConsoleWidget::mode() const { return mode_; }

void QConsoleWidget::setMode(ConsoleMode m) {
    if (m == mode_) {
        return;
    }

    if (m == Input) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
        inpos_ = cursor.position();
        mode_ = Input;
    }

    if (m == Output) {
        history_.deactivate();
        mode_ = Output;
    }
}

QIODevice *QConsoleWidget::device() const { return iodevice_; }

QString QConsoleWidget::getCommandLine() {
    if (mode_ == Output) {
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
        history_.deactivate();
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
        history_.add(code);
    }

    // append the newline char and
    // send signal / update iodevice
    if (iodevice_->isOpen()) {
        iodevice_->consoleWidgetInput(code);
    }

    Q_EMIT consoleCommand(code);
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
    const auto first = doc->findBlock(inpos_);
    const auto last = doc->lastBlock();
    auto block = cursor.block();
    doc->documentLayout()->blockBoundingRect(block);
    const auto line =
        block.layout()->lineForTextPosition(cursor.positionInBlock());
    const auto firstLine =
        first.layout()->lineForTextPosition(inpos_ - first.position());
    const auto atBoundary =
        up ? block == first && (!line.isValid() || !firstLine.isValid() ||
                                line.lineNumber() <= firstLine.lineNumber())
           : block == last &&
                 (!line.isValid() ||
                  line.lineNumber() == last.layout()->lineCount() - 1);

    if (modifiers != Qt::NoModifier || !atBoundary) {
        if (cursor.anchor() < inpos_) {
            cursor.setPosition(inpos_);
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

    if (!history_.isActive()) {
        if (!up) {
            return;
        }
        auto code = currentCommandLine();
        code.replace(QChar::ParagraphSeparator, QChar::LineFeed);
        history_.activate(code);
    }
    if (history_.move(up)) {
        replaceCommandLine(history_.currentValue());
        cursor = textCursor();
        if (up) {
            cursor.setPosition(inpos_);
        } else {
            cursor.movePosition(QTextCursor::End);
        }
        setTextCursor(cursor);
        if (history_.pos_ == -1) {
            history_.deactivate();
        }
    } else {
        if (history_.pos_ == -1) {
            history_.deactivate();
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
            Q_EMIT abortEvaluation();
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
                history_.deactivate();
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
            history_.deactivate();
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
    if (history_.isActive() && !navigationKey) {
        history_.deactivate();
    }

    // Force the cursor back to the interactive area
    // for all keys except modifiers
    if (!isCursorInEditZone() && key != Qt::Key_Control &&
        key != Qt::Key_Shift && key != Qt::Key_Alt) {
        auto block = textCursor.block();
        if (block.position() >= document()->findBlock(inpos_).position()) {
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
            pos == inputStartForBlock(block) && pos > inpos_) {
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
                textCursor.selectionStart() < inpos_) {
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
                                   ? inpos_
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
    textCursor.setPosition(inpos_, QTextCursor::KeepAnchor);

    // ... and replace it with new string.
    textCursor.insertText(str);

    // move to the end of the document
    textCursor.movePosition(QTextCursor::End);
    setTextCursor(textCursor);
}

QString QConsoleWidget::currentCommandLine() const {
    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);
    textCursor.setPosition(inpos_, QTextCursor::KeepAnchor);
    return textCursor.selectedText();
}

int QConsoleWidget::inputStartForBlock(const QTextBlock &block) const {
    return qMax(inpos_, block.position());
}

int QConsoleWidget::blockTextStart(const QTextBlock &block) const {
    if (block.position() < document()->findBlock(inpos_).position()) {
        return -1;
    }
    return inputStartForBlock(block) - block.position();
}

void QConsoleWidget::dragMoveEvent(QDragMoveEvent *e) { e->ignore(); }

int QConsoleWidget::currentHeaderPos() const { return inpos_; }

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
        inpos_ = tc.position() - inpos_;

        // insert block
        tc.movePosition(QTextCursor::StartOfBlock);
        tc.insertBlock();
        auto id = tc.blockNumber();
        tc.movePosition(QTextCursor::PreviousBlock);

        tc.insertText(message);
        tc.movePosition(QTextCursor::End);
        // restore input pos
        inpos_ = tc.position() - inpos_;
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

QConsoleWidget::History &QConsoleWidget::history() { return history_; }

/////////////////// QConsoleWidget::History /////////////////////

QConsoleWidget::History QConsoleWidget::history_;

QConsoleWidget::History::History(void)
    : pos_(0), active_(false), maxsize_(100) {}
QConsoleWidget::History::~History(void) {}

void QConsoleWidget::History::add(const QString &str) {
    active_ = false;
    auto idx = strings_.indexOf(str);
    if (idx >= 0) {
        strings_.move(idx, 0);
        return;
    }
    if (strings_.size() == maxsize_) {
        strings_.pop_back();
    }
    strings_.push_front(str);
}

void QConsoleWidget::History::activate(const QString &tk) {
    active_ = true;
    token_ = tk;
    pos_ = -1;
}

bool QConsoleWidget::History::move(bool dir) {
    if (active_) {
        int next = indexOf(dir, pos_);
        if (pos_ != next) {
            pos_ = next;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

int QConsoleWidget::History::indexOf(bool dir, int from) const {
    return qBound(-1, from + (dir ? 1 : -1), int(strings_.size()) - 1);
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
        history_.deactivate();
        WingCodeEdit::insertFromMimeData(source);
    }
}
