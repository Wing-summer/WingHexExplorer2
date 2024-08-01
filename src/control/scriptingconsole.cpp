#include "scriptingconsole.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QCompleter>
#include <QDebug>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>
#include <QStringListModel>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocumentFragment>

// note: this class is modified from PythonQt

//-----------------------------------------------------------------------------

ScriptingConsole::ScriptingConsole(QWidget *parent, Qt::WindowFlags windowFlags)
    : QTextEdit(parent) {

    setWindowFlags(windowFlags);

    _defaultTextCharacterFormat = currentCharFormat();

    _historyPosition = 0;
    _hadError = false;

    _completer = new QCompleter(this);
    _completer->setWidget(this);
    QObject::connect(_completer, SIGNAL(activated(const QString &)), this,
                     SLOT(insertCompletion(const QString &)));

    clear();
    setUndoRedoEnabled(false);
    setAcceptRichText(false);

    _getInputFn = std::bind(&ScriptingConsole::getInput, this);
    _sp = new ScriptConsoleMachine(_getInputFn, this);
    connect(_sp, &ScriptConsoleMachine::onOutput, this,
            [=](ScriptConsoleMachine::MessageType type,
                const ScriptConsoleMachine::MessageInfo &message) {
                switch (type) {
                case ScriptMachine::MessageType::Info:
                    stdOut(message.message);
                    break;
                case ScriptMachine::MessageType::Warn:
                    break;
                case ScriptMachine::MessageType::Error:
                    stdErr(message.message);
                    break;
                }
            });
}

//-----------------------------------------------------------------------------

void ScriptingConsole::stdOut(const QString &s) {
    _stdOut += s;
    int idx;
    while ((idx = _stdOut.indexOf('\n')) != -1) {
        auto msg = _stdOut.left(idx);
        consoleMessage(QString("<font color=\"green\">%1</font>")
                           .arg(msg)); // modified by wingsummer
        _stdOut = _stdOut.mid(idx + 1);
    }
}

void ScriptingConsole::stdErr(const QString &s) {
    _hadError = true;
    _stdErr += s;
    int idx;
    while ((idx = _stdErr.indexOf('\n')) != -1) {
        auto msg = _stdErr.left(idx);
        consoleMessage(QStringLiteral("<font color=\"red\">%1</font>")
                           .arg(msg)); // modified by wingsummer
        _stdErr = _stdErr.mid(idx + 1);
    }
}

void ScriptingConsole::flushStdOut() {
    if (!_stdOut.isEmpty()) {
        stdOut(QStringLiteral("\n"));
    }
    if (!_stdErr.isEmpty()) {
        stdErr(QStringLiteral("\n"));
    }
}

//-----------------------------------------------------------------------------

ScriptingConsole::~ScriptingConsole() {}

//-----------------------------------------------------------------------------

void ScriptingConsole::clear() {
    QTextEdit::clear();
    appendCommandPrompt();
}

//-----------------------------------------------------------------------------

void ScriptingConsole::executeLine(bool storeOnly) {
    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);

    // Select the text from the command prompt until the end of the block
    // and get the selected text.
    textCursor.setPosition(commandPromptPosition());
    textCursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QString code = textCursor.selectedText();

    // i don't know where this trailing space is coming from, blast it!
    if (code.endsWith(" ")) {
        code.truncate(code.length() - 1);
    }

    if (!code.isEmpty()) {
        // Update the history
        _history << code;
        _historyPosition = _history.count();
        _currentMultiLineCode += code + "\n";

        if (!storeOnly) {
            executeCode(_currentMultiLineCode);
            _currentMultiLineCode = "";
        }
    }
    // Insert a new command prompt
    appendCommandPrompt(storeOnly);
}

void ScriptingConsole::executeCode(const QString &code) {
    // put visible cursor to the end of the line
    QTextCursor cursor = QTextEdit::textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);

    int cursorPosition = this->textCursor().position();

    // evaluate the code
    _stdOut.clear();
    _stdErr.clear();

    auto ret = _sp->executeScript(code);

    flushStdOut();

    bool messageInserted = (this->textCursor().position() != cursorPosition);
    // If a message was inserted, then put another empty line before the command
    // prompt to improve readability.
    if (messageInserted) {
        append(QString());
    }
}

std::string ScriptingConsole::getInput() {
    // TODO
    return "";
}

//-----------------------------------------------------------------------------

void ScriptingConsole::appendCommandPrompt(bool storeOnly) {
    if (storeOnly) {
        _commandPrompt = QStringLiteral("... > ");
    } else {
        _commandPrompt = QStringLiteral("as > ");
    }
    append(_commandPrompt);

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
}

//-----------------------------------------------------------------------------

void ScriptingConsole::setCurrentFont(const QColor &color, bool bold) {

    QTextCharFormat charFormat(_defaultTextCharacterFormat);

    QFont font(charFormat.font());
    font.setBold(bold);
    charFormat.setFont(font);

    QBrush brush(charFormat.foreground());
    brush.setColor(color);
    charFormat.setForeground(brush);

    setCurrentCharFormat(charFormat);
}

//-----------------------------------------------------------------------------

int ScriptingConsole::commandPromptPosition() {

    QTextCursor textCursor(this->textCursor());
    textCursor.movePosition(QTextCursor::End);

    return textCursor.block().position() + _commandPrompt.length();
}

//-----------------------------------------------------------------------------

void ScriptingConsole::insertCompletion(const QString &completion) {
    QTextCursor tc = textCursor();
    tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    if (tc.selectedText() == ".") {
        tc.insertText(QString(".") + completion);
    } else {
        tc = textCursor();
        tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
        tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        tc.insertText(completion);
        setTextCursor(tc);
    }
}

//-----------------------------------------------------------------------------
void ScriptingConsole::handleTabCompletion() {
    QTextCursor textCursor = this->textCursor();
    int pos = textCursor.position();
    textCursor.setPosition(commandPromptPosition());
    textCursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    int startPos = textCursor.selectionStart();

    int offset = pos - startPos;
    QString text = textCursor.selectedText();

    QString textToComplete;
    int cur = offset;
    while (cur--) {
        QChar c = text.at(cur);
        if (c.isLetterOrNumber() || c == '.' || c == '_') {
            textToComplete.prepend(c);
        } else {
            break;
        }
    }

    QString lookup;
    QString compareText = textToComplete;
    int dot = compareText.lastIndexOf('.');
    if (dot != -1) {
        lookup = compareText.mid(0, dot);
        compareText = compareText.mid(dot + 1, offset);
    }
    if (!lookup.isEmpty() || !compareText.isEmpty()) {
        compareText = compareText.toLower();
        QStringList found;

        // TODO
        // QStringList l = PythonQt::self()->introspection(_context, lookup,
        //                                                 PythonQt::Anything);
        // Q_FOREACH (QString n, l) {
        //     if (n.toLower().startsWith(compareText)) {
        //         found << n;
        //     }
        // }

        if (!found.isEmpty()) {
            _completer->setCompletionPrefix(compareText);
            _completer->setCompletionMode(QCompleter::PopupCompletion);
            _completer->setModel(new QStringListModel(found, _completer));
            _completer->setCaseSensitivity(Qt::CaseInsensitive);
            QTextCursor c = this->textCursor();
            c.movePosition(QTextCursor::StartOfWord);
            QRect cr = cursorRect(c);
            cr.setWidth(
                _completer->popup()->sizeHintForColumn(0) +
                _completer->popup()->verticalScrollBar()->sizeHint().width());
            cr.translate(0, 8);
            _completer->complete(cr);
        } else {
            _completer->popup()->hide();
        }
    } else {
        _completer->popup()->hide();
    }
}

void ScriptingConsole::keyPressEvent(QKeyEvent *event) {

    if (_completer && _completer->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
        switch (event->key()) {
        case Qt::Key_Return:
            if (!_completer->popup()->currentIndex().isValid()) {
                insertCompletion(_completer->currentCompletion());
                _completer->popup()->hide();
                event->accept();
            }
            event->ignore();
            return;
            break;
        case Qt::Key_Enter:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:

            event->ignore();
            return; // let the completer do default behavior
        default:
            break;
        }
    }
    bool eventHandled = false;
    QTextCursor textCursor = this->textCursor();

    int key = event->key();
    switch (key) {

    case Qt::Key_Left:

        // Moving the cursor left is limited to the position
        // of the command prompt.

        if (textCursor.position() <= commandPromptPosition()) {

            QApplication::beep();
            eventHandled = true;
        }
        break;

    case Qt::Key_Up:

        // Display the previous command in the history
        if (_historyPosition > 0) {
            _historyPosition--;
            changeHistory();
        }

        eventHandled = true;
        break;

    case Qt::Key_Down:

        // Display the next command in the history
        if (_historyPosition + 1 < _history.count()) {
            _historyPosition++;
            changeHistory();
        }

        eventHandled = true;
        break;

    case Qt::Key_Return:

        executeLine(event->modifiers() & Qt::ShiftModifier);
        eventHandled = true;
        break;

    case Qt::Key_Backspace:

        if (textCursor.hasSelection()) {

            cut();
            eventHandled = true;

        } else {

            // Intercept backspace key event to check if
            // deleting a character is allowed. It is not
            // allowed, if the user wants to delete the
            // command prompt.

            if (textCursor.position() <= commandPromptPosition()) {

                QApplication::beep();
                eventHandled = true;
            }
        }
        break;

    case Qt::Key_Delete:

        cut();
        eventHandled = true;
        break;

    default:

        if (key >= Qt::Key_Space && key <= Qt::Key_division) {

            if (textCursor.hasSelection() && !verifySelectionBeforeDeletion()) {

                // The selection must not be deleted.
                eventHandled = true;

            } else {

                // The key is an input character, check if the cursor is
                // behind the last command prompt, else inserting the
                // character is not allowed.

                int commandPromptPosition = this->commandPromptPosition();
                if (textCursor.position() < commandPromptPosition) {

                    textCursor.setPosition(commandPromptPosition);
                    setTextCursor(textCursor);
                }
            }
        }
    }

    if (eventHandled) {

        _completer->popup()->hide();
        event->accept();

    } else {

        QTextEdit::keyPressEvent(event);
        QString text = event->text();
        if (!text.isEmpty()) {
            handleTabCompletion();
        } else {
            _completer->popup()->hide();
        }
        eventHandled = true;
    }
}

//-----------------------------------------------------------------------------

void ScriptingConsole::cut() {

    bool deletionAllowed = verifySelectionBeforeDeletion();
    if (deletionAllowed) {
        QTextEdit::cut();
    }
}

//-----------------------------------------------------------------------------

bool ScriptingConsole::verifySelectionBeforeDeletion() {

    bool deletionAllowed = true;

    QTextCursor textCursor = this->textCursor();

    int commandPromptPosition = this->commandPromptPosition();
    int selectionStart = textCursor.selectionStart();
    int selectionEnd = textCursor.selectionEnd();

    if (textCursor.hasSelection()) {

        // Selected text may only be deleted after the last command prompt.
        // If the selection is partly after the command prompt set the selection
        // to the part and deletion is allowed. If the selection occurs before
        // the last command prompt, then deletion is not allowed.

        if (selectionStart < commandPromptPosition ||
            selectionEnd < commandPromptPosition) {

            // Assure selectionEnd is bigger than selection start
            if (selectionStart > selectionEnd) {
                int tmp = selectionEnd;
                selectionEnd = selectionStart;
                selectionStart = tmp;
            }

            if (selectionEnd < commandPromptPosition) {

                // Selection is completely before command prompt,
                // so deletion is not allowed.
                QApplication::beep();
                deletionAllowed = false;

            } else {

                // The selectionEnd is after the command prompt, so set
                // the selection start to the commandPromptPosition.
                selectionStart = commandPromptPosition;
                textCursor.setPosition(selectionStart);
                textCursor.setPosition(selectionStart, QTextCursor::KeepAnchor);
                setTextCursor(textCursor);
            }
        }

    } else { // if (hasSelectedText())

        // When there is no selected text, deletion is not allowed before the
        // command prompt.
        if (textCursor.position() < commandPromptPosition) {

            QApplication::beep();
            deletionAllowed = false;
        }
    }

    return deletionAllowed;
}

//-----------------------------------------------------------------------------

void ScriptingConsole::changeHistory() {

    // Select the text after the last command prompt ...
    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);
    textCursor.setPosition(commandPromptPosition(), QTextCursor::KeepAnchor);

    // ... and replace it with the history text.
    textCursor.insertText(_history.value(_historyPosition));

    textCursor.movePosition(QTextCursor::End);
    setTextCursor(textCursor);
}

//-----------------------------------------------------------------------------

void ScriptingConsole::consoleMessage(const QString &message) {
    append(QString());
    insertHtml(message); // modified by wingsummer

    // Reset all font modifications done by the html string
    setCurrentCharFormat(_defaultTextCharacterFormat);
}
