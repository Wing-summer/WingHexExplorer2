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

#include "QConsoleWidget.h"
#include "QConsoleIODevice.h"
#include "wingsyntaxhighlighter.h"

#include <QAbstractItemView>
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

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

QConsoleWidget::QConsoleWidget(QWidget *parent)
    : WingCodeEdit(parent), mode_(Output) {
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
    if (m == mode_)
        return;

    if (m == Input) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
        inpos_ = cursor.position();
        mode_ = Input;
    }

    if (m == Output) {
        mode_ = Output;
    }
}

QIODevice *QConsoleWidget::device() const { return iodevice_; }

QString QConsoleWidget::getCommandLine() {
    if (mode_ == Output)
        return QString();
    // select text in edit zone (from the input pos to the end)
    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);
    textCursor.setPosition(inpos_, QTextCursor::KeepAnchor);
    QString code = textCursor.selectedText();
    code.replace(QChar::ParagraphSeparator, QChar::LineFeed);
    return code;
}

void QConsoleWidget::paste() {
    QTextCursor textCursor = this->textCursor();
    const QMimeData *const clipboard = QApplication::clipboard()->mimeData();
    const QString text = clipboard->text();
    if (!text.isNull()) {
        textCursor.insertText(text);
    }
}

void QConsoleWidget::handleReturnKey() {
    QString code = getCommandLine();

    // start new block
    appendPlainText(QString());
    setMode(Output);

    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);
    setTextCursor(textCursor);

    // Update the history
    if (!code.isEmpty())
        history_.add(code);

    // append the newline char and
    // send signal / update iodevice
    if (iodevice_->isOpen())
        iodevice_->consoleWidgetInput(code);

    Q_EMIT consoleCommand(code);
}

void QConsoleWidget::handleTabKey() {
    QTextCursor tc = this->textCursor();
    int anchor = tc.anchor();
    int position = tc.position();
    tc.setPosition(inpos_);
    tc.setPosition(position, QTextCursor::KeepAnchor);
    QString text = tc.selectedText().trimmed();
    tc.setPosition(anchor, QTextCursor::MoveAnchor);
    tc.setPosition(position, QTextCursor::KeepAnchor);
    if (text.isEmpty()) {
        tc.insertText(QString(4, ' '));
    }
}

void QConsoleWidget::keyPressEvent(QKeyEvent *e) {
    if (processCompletionBegin(e)) {
        return;
    }

    QTextCursor textCursor = this->textCursor();
    bool selectionInEditZone = isSelectionInEditZone();

    // check for user abort request
    if (e->modifiers() & Qt::ControlModifier) {
        if (e->key() == Qt::Key_Q) // Ctrl-Q aborts
        {
            Q_EMIT abortEvaluation();
            e->accept();
            return;
        }
    }

    // Allow copying anywhere in the console ...
    if (e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier) {
        if (textCursor.hasSelection())
            copy();
        e->accept();
        return;
    }

    // the rest of key events are ignored during output mode
    if (mode() != Input) {
        e->ignore();
        return;
    }

    // Allow cut only if the selection is limited to the interactive area ...
    if (e->key() == Qt::Key_X && e->modifiers() == Qt::ControlModifier) {
        if (selectionInEditZone)
            cut();
        e->accept();
        return;
    }

    // Allow paste only if the selection is in the interactive area ...
    if (e->key() == Qt::Key_V && e->modifiers() == Qt::ControlModifier) {
        if (selectionInEditZone || isCursorInEditZone()) {
            paste();
        }

        e->accept();
        return;
    }

    int key = e->key();
    int shiftMod = e->modifiers() == Qt::ShiftModifier;

    if (history_.isActive() && key != Qt::Key_Up && key != Qt::Key_Down)
        history_.deactivate();

    // Force the cursor back to the interactive area
    // for all keys except modifiers
    if (!isCursorInEditZone() && key != Qt::Key_Control &&
        key != Qt::Key_Shift && key != Qt::Key_Alt) {
        textCursor.movePosition(QTextCursor::End);
        setTextCursor(textCursor);
    }

    switch (key) {
    case Qt::Key_Up:
        // Activate the history and move to the 1st matching history item
        if (!history_.isActive())
            history_.activate(getCommandLine());
        if (history_.move(true))
            replaceCommandLine(history_.currentValue());
        else
            QApplication::beep();
        e->accept();
        break;

    case Qt::Key_Down:
        if (history_.move(false))
            replaceCommandLine(history_.currentValue());
        else
            QApplication::beep();
        e->accept();

    case Qt::Key_Left:
        if (textCursor.position() > inpos_)
            processDefaultKeyPressEvent(e);
        else {
            QApplication::beep();
            e->accept();
        }
        break;

    case Qt::Key_Delete:
        e->accept();
        if (selectionInEditZone)
            cut();
        else {
            // cursor must be in edit zone
            if (textCursor.position() < inpos_)
                QApplication::beep();
            else
                processDefaultKeyPressEvent(e);
        }
        break;

    case Qt::Key_Backspace:
        e->accept();
        if (selectionInEditZone)
            cut();
        else {
            // cursor must be in edit zone
            if (textCursor.position() <= inpos_)
                QApplication::beep();
            else
                processDefaultKeyPressEvent(e);
        }
        break;

    case Qt::Key_Tab:
        e->accept();
        handleTabKey();
        return;

    case Qt::Key_Home:
        e->accept();
        textCursor.setPosition(inpos_, shiftMod ? QTextCursor::KeepAnchor
                                                : QTextCursor::MoveAnchor);
        setTextCursor(textCursor);
        break;

    case Qt::Key_Enter:
    case Qt::Key_Return:
        e->accept();
        handleReturnKey();
        break;

    case Qt::Key_Escape:
        e->accept();
        replaceCommandLine(QString());
        break;

    default:
        e->accept();
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
    if ((a = menu->findChild<QAction *>("edit-cut")))
        a->setEnabled(canCut());
    if ((a = menu->findChild<QAction *>("edit-delete")))
        a->setEnabled(canCut());
    if ((a = menu->findChild<QAction *>("edit-paste")))
        a->setEnabled(canPaste());

    menu->exec(event->globalPos());
    delete menu;
}

bool QConsoleWidget::isSelectionInEditZone() const {
    QTextCursor textCursor = this->textCursor();
    if (!textCursor.hasSelection())
        return false;

    int selectionStart = textCursor.selectionStart();
    int selectionEnd = textCursor.selectionEnd();

    return selectionStart >= inpos_ && selectionEnd >= inpos_;
}

bool QConsoleWidget::isCursorInEditZone() const {
    return textCursor().position() >= inpos_;
}

bool QConsoleWidget::canPaste() const {
    QTextCursor textCursor = this->textCursor();
    if (textCursor.position() < inpos_)
        return false;
    if (textCursor.anchor() < inpos_)
        return false;
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
        if (needsRestore)
            setTextCursor(tc);

        return id;
    }
}

QConsoleWidget::History &QConsoleWidget::history() { return history_; }

/////////////////// QConsoleWidget::History /////////////////////

QConsoleWidget::History QConsoleWidget::history_;

QConsoleWidget::History::History(void)
    : pos_(0), active_(false), maxsize_(10000) {}
QConsoleWidget::History::~History(void) {}

void QConsoleWidget::History::add(const QString &str) {
    active_ = false;
    // if (strings_.contains(str)) return;
    if (strings_.size() == maxsize_)
        strings_.pop_back();
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
        } else
            return false;
    } else
        return false;
}

int QConsoleWidget::History::indexOf(bool dir, int from) const {
    int i = from, to = from;
    if (dir) {
        while (i < strings_.size() - 1) {
            const QString &si = strings_.at(++i);
            if (si.startsWith(token_)) {
                return i;
            }
        }
    } else {
        while (i > 0) {
            const QString &si = strings_.at(--i);
            if (si.startsWith(token_)) {
                return i;
            }
        }
        return -1;
    }
    return to;
}

/////////////////// Stream manipulators /////////////////////

QTextStream &waitForInput(QTextStream &s) {
    QConsoleIODevice *d = qobject_cast<QConsoleIODevice *>(s.device());
    if (d)
        d->waitForReadyRead(-1);
    return s;
}

QTextStream &inputMode(QTextStream &s) {
    QConsoleIODevice *d = qobject_cast<QConsoleIODevice *>(s.device());
    if (d && d->widget())
        d->widget()->setMode(QConsoleWidget::Input);
    return s;
}
QTextStream &outChannel(QTextStream &s) {
    QConsoleIODevice *d = qobject_cast<QConsoleIODevice *>(s.device());
    if (d)
        d->setCurrentWriteChannel(QConsoleWidget::StandardOutput);
    return s;
}
QTextStream &errChannel(QTextStream &s) {
    QConsoleIODevice *d = qobject_cast<QConsoleIODevice *>(s.device());
    if (d)
        d->setCurrentWriteChannel(QConsoleWidget::StandardError);
    return s;
}
