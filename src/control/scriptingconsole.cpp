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

#include "scriptingconsole.h"
#include "QConsoleWidget/QConsoleIODevice.h"
#include "class/scriptmachine.h"
#include "class/scriptsettings.h"
#include "class/skinmanager.h"
#include "class/wingmessagebox.h"
#include "model/codecompletionmodel.h"
#include "utilities.h"

#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QIcon>
#include <QKeyEvent>
#include <QMenu>
#include <QMimeData>
#include <QRegularExpression>
#include <QTextBlock>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

ScriptingConsole::ScriptingConsole(QWidget *parent)
    : ScriptingConsoleBase(parent) {
    connect(&ScriptSettings::instance(), &ScriptSettings::consoleSettingUpdate,
            this, &ScriptingConsole::applyScriptSettings);
    applyScriptSettings();
}

ScriptingConsole::~ScriptingConsole() {}

void ScriptingConsole::handleReturnKey() {
    auto cursor = this->textCursor();
    cursor.movePosition(QTextCursor::PreviousCharacter,
                        QTextCursor::KeepAnchor);
    // TODO whether '\' is in a string ?
    if (cursor.selectedText() == QStringLiteral("\\")) {
        cursor = this->textCursor();
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        auto restCode = cursor.selectedText();
        cursor.removeSelectedText();
        QString code = getCommandLine();
        setMode(Output);
        _codes += code.sliced(0, code.length() - 1);
        appendCommandPrompt(true);
        cursor = this->textCursor();
        setMode(Input);
        cursor.insertText(restCode);
        cursor.movePosition(QTextCursor::PreviousCharacter);
        setTextCursor(cursor);
    } else {
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

        if (!_isWaitingRead) {
            Q_EMIT consoleCommand(code);
        }
    }
}

void ScriptingConsole::init() {
    _getInputFn = std::bind(&ScriptingConsole::getInput, this);

    connect(this, &QConsoleWidget::consoleCommand, this,
            &ScriptingConsole::runConsoleCommand);

    auto cm = new AsConsoleCompletion(this);
    connect(cm, &AsCompletion::onFunctionTip, this,
            &ScriptingConsole::onFunctionTip);
}

void ScriptingConsole::clearConsole() {
    setMode(Output);

    auto cur = this->textCursor();
    auto off = cur.position() - this->currentHeaderPos();
    auto lastCmd = this->currentCommandLine();
    auto dis = lastCmd.length() - off;

    clear();

    if (lastCommandPrompt()) {
        auto lines = _codes.split('\n');
        auto pl = lines.begin();
        appendCommandPrompt(false);
        write(*pl);

        pl++;
        for (; pl != lines.end(); pl++) {
            appendCommandPrompt(true);
            write(*pl);
        }
        appendCommandPrompt(true);
    } else {
        appendCommandPrompt(false);
    }

    setMode(Input);
    replaceCommandLine(lastCmd);
    cur = this->textCursor();
    cur.movePosition(QTextCursor::EndOfBlock);
    cur.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, dis);
    setTextCursor(cur);
}

void ScriptingConsole::processKeyEvent(QKeyEvent *e) { keyPressEvent(e); }

void ScriptingConsole::onOutput(const ScriptMachine::MessageInfo &message) {
    // <type, <row, col>>
    static QPair<ScriptMachine::MessageType, QPair<int, int>> lastInfo{
        ScriptMachine::MessageType::Print, {-1, -1}};

    auto doc = this->document();
    auto lastLine = doc->lastBlock();
    auto isNotBlockStart = !lastLine.text().isEmpty();

    auto fmtMsg = [](const ScriptMachine::MessageInfo &message) -> QString {
        if (message.row <= 0 || message.col <= 0) {
            return message.message;
        } else {
            return QStringLiteral("(") + QString::number(message.row) +
                   QStringLiteral(", ") + QString::number(message.col) +
                   QStringLiteral(")") + message.message;
        }
    };

    auto isMatchLast = [](const ScriptMachine::MessageInfo &message) -> bool {
        if (message.row < 0 || message.col < 0) {
            return false;
        }
        return lastInfo.first == message.type &&
               lastInfo.second.first == message.row &&
               lastInfo.second.second == message.col;
    };

    switch (message.type) {
    case ScriptMachine::MessageType::Info:
        if (isMatchLast(message)) {
            stdOutLine(message.message);
        } else {
            if (isNotBlockStart) {
                newLine();
            }
            stdOutLine(tr("[Info]") + fmtMsg(message));
        }
        flush();
        break;
    case ScriptMachine::MessageType::Warn:
        if (isMatchLast(message)) {
            stdWarnLine(message.message);
        } else {
            if (isNotBlockStart) {
                newLine();
            }
            stdWarnLine(tr("[Warn]") + fmtMsg(message));
        }
        flush();
        break;
    case ScriptMachine::MessageType::Error:
        if (isMatchLast(message)) {
            stdErrLine(message.message);
        } else {
            if (isNotBlockStart) {
                newLine();
            }
            stdErrLine(tr("[Error]") + fmtMsg(message));
        }
        flush();
        break;
    case ScriptMachine::MessageType::Print:
        if (lastInfo.first != message.type && isNotBlockStart) {
            newLine();
        }
        stdOutLine(message.message);
        break;
    }

    lastInfo.first = message.type;
    lastInfo.second = qMakePair(message.row, message.col);
}

void ScriptingConsole::abortCurrentCode() {
    setMode(Output);
    _codes.clear();
    appendCommandPrompt();
    setMode(Input);
}

void ScriptingConsole::applyScriptSettings() {
    auto &set = ScriptSettings::instance();
    auto dfont = QFont(set.consoleFontFamily());
    dfont.setPointSize(set.consoleFontSize());

    auto thname = set.consoleTheme();
    if (thname.isEmpty()) {
        switch (SkinManager::instance().currentTheme()) {
        case SkinManager::Theme::Dark:
            setTheme(syntaxRepo().defaultTheme(
                KSyntaxHighlighting::Repository::DarkTheme));
            break;
        case SkinManager::Theme::Light:
            setTheme(syntaxRepo().defaultTheme(
                KSyntaxHighlighting::Repository::LightTheme));
            break;
        }
    } else {
        setTheme(syntaxRepo().theme(thname));
    }

    this->setFont(dfont);
    this->setTabWidth(set.consoleTabWidth());
    this->setIndentationMode(WingCodeEdit::IndentationMode(set.consoleInden()));
    this->setMatchBraces(set.consoleMatchBraces());
    this->setShowWhitespace(set.consoleShowWhiteSpace());
    this->setAutoCloseChar(set.consoleAutoCloseChar());
}

void ScriptingConsole::runConsoleCommand(const QString &code) {
    auto exec = code.trimmed();
    if (exec == QStringLiteral("#ls")) {
        auto &ins = ScriptMachine::instance();
        auto mod = ins.module(ScriptMachine::Interactive);
        if (mod) {
            auto total = mod->GetGlobalVarCount();

            setMode(Output);

            if (total == 0) {
                stdOutLine("<none>");
            } else {
                auto &sm = ScriptMachine::instance();
                for (asUINT i = 0; i < total; ++i) {
                    const char *name;
                    int typeID;
                    auto decl = mod->GetGlobalVarDeclaration(i);
                    if (decl && mod->GetGlobalVar(i, &name, nullptr, &typeID) ==
                                    asSUCCESS) {
                        auto value = sm.debugger()->toString(
                            mod->GetAddressOfGlobalVar(i), typeID, sm.engine(),
                            1);
                        stdOutLine(decl + QStringLiteral(" = ") + value);
                    }
                }
            }

            _codes.clear();
            appendCommandPrompt();
            setMode(Input);
        }
    } else if (exec.startsWith(QStringLiteral("#del"))) {
        // this is special command
        auto &ins = ScriptMachine::instance();
        auto mod = ins.module(ScriptMachine::Interactive);
        if (mod) {
            // first check whether contains \n
            auto idx = exec.indexOf('\n');
            if (idx >= 0) {
                setMode(Output);
                stdErrLine(tr("InvalidDelCmd"));
            } else {
                // ok, then tokens should be devided by the space
                exec.remove(0, 4);
                auto vars = exec.split(' ', Qt::SkipEmptyParts);

                QList<asUINT> indices;

                // then check
                setMode(Output);
                for (auto &v : vars) {
                    auto idx = mod->GetGlobalVarIndexByName(v.toUtf8());
                    if (idx >= 0) {
                        indices.append(idx);
                    } else {
                        stdWarnLine(tr("NotFoundIgnore:") + v);
                    }
                }

                std::sort(indices.begin(), indices.end(), std::greater<int>());

                // ok, remove
                for (auto &i : indices) {
                    mod->RemoveGlobalVar(i);
                }
            }
        }
        _codes.clear();
        appendCommandPrompt();
        setMode(Input);
    } else if (exec == QStringLiteral("#cls")) {
        auto &ins = ScriptMachine::instance();
        auto mod = ins.module(ScriptMachine::Interactive);
        if (mod) {
            auto total = mod->GetGlobalVarCount();
            if (total) {
                asUINT i = total;
                do {
                    --i;
                    mod->RemoveGlobalVar(i);
                } while (i);
            }
        }
        _codes.clear();
        appendCommandPrompt();
        setMode(Input);
    } else if (exec.endsWith('\\')) {
        static QRegularExpression ex(QStringLiteral("[\\\\\\s]+$"));
        _codes.append('\n');
        _codes += exec.remove(ex);
        setMode(Output);
        appendCommandPrompt(true);
        setMode(Input);
    } else {
        setMode(Output);
        _codes += exec;
        ScriptMachine::instance().executeCode(ScriptMachine::Interactive,
                                              _codes);
        _codes.clear();
        appendCommandPrompt();
        setMode(Input);
    }
}

QString ScriptingConsole::getInput() {
    auto &s = consoleStream();
    appendCommandPrompt(true);
    setMode(Input);
    s.status();
    auto d = s.device();
    auto ba = d->bytesAvailable();
    d->skip(ba);
    _isWaitingRead = true;
    d->waitForReadyRead(-1);
    QString instr;
    s >> instr;
    _isWaitingRead = false;
    setMode(Output);
    return instr;
}

QString ScriptingConsole::packUpLoggingStr(const QString &message) {
    return tr("[Console]") + message;
}

void ScriptingConsole::keyPressEvent(QKeyEvent *e) {
    if (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_L) {
        clearConsole();
    } else {
        QConsoleWidget::keyPressEvent(e);
    }
}

void ScriptingConsole::onCompletion(const QModelIndex &index) {
    WingCodeEdit::onCompletion(index);
    auto selfdata = index.data(Qt::SelfDataRole).value<CodeInfoTip>();
    if (selfdata.type == CodeInfoTip::Type::Function ||
        selfdata.type == CodeInfoTip::Type::ClsFunction) {
        auto args = selfdata.addinfo.value(CodeInfoTip::Args);

        auto cur = textCursor();
        cur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        auto ch = cur.selectedText();
        if (ch.isEmpty() || ch.front().isSpace()) {
            auto cursor = textCursor();
            cursor.insertText(QStringLiteral("()"));
            if (!args.isEmpty()) {
                cursor.movePosition(QTextCursor::Left);
                setTextCursor(cursor);
            }
        } else {
            auto cursor = textCursor();
            cursor.insertText(QStringLiteral("("));
        }
    }
}

void ScriptingConsole::paste() {
    if (ScriptMachine::instance().isRunning(ScriptMachine::Interactive)) {
        return;
    }

    const QMimeData *const clipboard = QApplication::clipboard()->mimeData();
    const QString text = clipboard->text();
    if (!text.isEmpty()) {
        if (text.indexOf('\n') < 0) {
            if (isCursorInEditZone()) {
                auto cursor = this->textCursor();
                cursor.insertText(text);
            } else {
                replaceCommandLine(text);
            }
        } else {
            auto ret = WingMessageBox::question(
                nullptr, tr("MultiCodeCanNotUndo"), text);
            if (ret == QMessageBox::No) {
                return;
            }
            auto lines = text.split('\n');
            if (lines.isEmpty()) {
                return;
            }

            setMode(Output);
            auto pl = lines.begin();
            auto pend = std::prev(lines.end());
            write(*pl);
            pl++;
            for (; pl != pend; pl++) {
                appendCommandPrompt(true);
                write(*pl);
            }
            appendCommandPrompt(true);
            setMode(Input);
            replaceCommandLine(*pl);
            lines.removeLast();
            _codes = lines.join('\n');
        }
    }
}

bool ScriptingConsole::isTerminal() const { return _isTerminal; }

void ScriptingConsole::setIsTerminal(bool newIsTerminal) {
    _isTerminal = newIsTerminal;
}

QString ScriptingConsole::currentCodes() const {
    QTextCursor textCursor = this->textCursor();
    textCursor.setPosition(inpos_, QTextCursor::KeepAnchor);
    return _codes + textCursor.selectedText();
}

void ScriptingConsole::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);

    auto a = menu.addAction(QIcon(QStringLiteral(":/qeditor/copy.png")),
                            tr("Copy"), QKeySequence(QKeySequence::Copy), this,
                            &ScriptingConsole::copy);
    a->setShortcutContext(Qt::WidgetShortcut);
    a = menu.addAction(QIcon(QStringLiteral(":/qeditor/cut.png")), tr("Cut"),
                       QKeySequence(QKeySequence::Cut), this,
                       &ScriptingConsole::cut);
    a->setShortcutContext(Qt::WidgetShortcut);
    a = menu.addAction(QIcon(QStringLiteral(":/qeditor/paste.png")),
                       tr("Paste"), QKeySequence(QKeySequence::Paste), this,
                       &ScriptingConsole::paste);
    a->setShortcutContext(Qt::WidgetShortcut);

    if (_isTerminal) {
        a = menu.addAction(ICONRES(QStringLiteral("del")), tr("Clear"),
                           QKeySequence(Qt::ControlModifier | Qt::Key_L), this,
                           &ScriptingConsole::clearConsole);
        a->setShortcutContext(Qt::WidgetShortcut);
        menu.addSeparator();
        a = menu.addAction(ICONRES(QStringLiteral("dbgstop")),
                           tr("AbortScript"),
                           QKeySequence(Qt::ControlModifier | Qt::Key_Q), []() {
                               ScriptMachine::instance().abortScript(
                                   ScriptMachine::Background);
                           });
        a->setShortcutContext(Qt::WidgetShortcut);
    } else {
        a = menu.addAction(ICONRES(QStringLiteral("del")), tr("Clear"),
                           QKeySequence(Qt::ControlModifier | Qt::Key_L), this,
                           &ScriptingConsole::clear);
        a->setShortcutContext(Qt::WidgetShortcut);
    }

    menu.exec(event->globalPos());
}
