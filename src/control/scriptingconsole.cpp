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
#include "class/logger.h"
#include "class/scriptmachine.h"
#include "class/scriptsettings.h"
#include "class/skinmanager.h"
#include "model/codecompletionmodel.h"

#include <QApplication>
#include <QColor>
#include <QIcon>
#include <QKeyEvent>
#include <QMenu>
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
        dontHighlightLastLine(true);
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

        emit consoleCommand(code);
    }
}

void ScriptingConsole::init() {
    _getInputFn = std::bind(&ScriptingConsole::getInput, this);

    // _sp = new ScriptConsoleMachine(_getInputFn, this);
    // connect(_sp, &ScriptConsoleMachine::onClearConsole, this,
    //         &ScriptingConsole::clear);
    // connect(this, &ScriptingConsole::abortEvaluation, _sp,
    //         &ScriptConsoleMachine::abortScript);

    connect(this, &QConsoleWidget::consoleCommand, this,
            &ScriptingConsole::runConsoleCommand);

    auto cm = new AsConsoleCompletion(this);
    connect(cm, &AsCompletion::onFunctionTip, this,
            &ScriptingConsole::onFunctionTip);
}

void ScriptingConsole::clearConsole() {
    setMode(Output);
    clear();
    appendCommandPrompt(lastCommandPrompt());
    setMode(Input);
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
            stdOut(message.message);
        } else {
            if (isNotBlockStart) {
                newLine();
            }
            stdOut(tr("[Info]") + fmtMsg(message));
        }
        flush();
        break;
    case ScriptMachine::MessageType::Warn:
        if (isMatchLast(message)) {
            stdWarn(message.message);
        } else {
            if (isNotBlockStart) {
                newLine();
            }
            stdWarn(tr("[Warn]") + fmtMsg(message));
        }
        flush();
        break;
    case ScriptMachine::MessageType::Error:
        if (isMatchLast(message)) {
            stdErr(message.message);
        } else {
            if (isNotBlockStart) {
                newLine();
            }
            stdErr(tr("[Error]") + fmtMsg(message));
        }
        flush();
        break;
    case ScriptMachine::MessageType::Print:
        if (lastInfo.first != message.type) {
            newLine();
        }
        stdOut(message.message);
        break;
    }

    lastInfo.first = message.type;
    lastInfo.second = qMakePair(message.row, message.col);
}

void ScriptingConsole::applyScriptSettings() {
    auto &set = ScriptSettings::instance();
    auto dfont = QFont(set.consoleFontFamily());
    dfont.setPointSize(set.consoleFontSize());

    auto thname = set.editorTheme();
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
    if (exec.endsWith('\\')) {
        static QRegularExpression ex(QStringLiteral("[\\\\\\s]+$"));
        _codes += exec.remove(ex);
        setMode(Output);
        appendCommandPrompt(true);
        setMode(Input);
    } else {
        setMode(Output);
        _codes += exec;
        if (!ScriptMachine::instance().executeCode(ScriptMachine::Interactive,
                                                   _codes)) {
            // WingMessageBox::
        }
        _codes.clear();
        appendCommandPrompt();
        setMode(Input);
    }
}

QString ScriptingConsole::getInput() {
    appendCommandPrompt(true);
    setMode(Input);
    consoleStream().device()->waitForReadyRead(-1);
    QString instr;
    consoleStream() >> instr;
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
        auto cursor = textCursor();
        cursor.insertText(QStringLiteral("()"));
        if (!args.isEmpty()) {
            cursor.movePosition(QTextCursor::Left);
            setTextCursor(cursor);
        }
    }
}

QString ScriptingConsole::currentCodes() const {
    return _codes + currentCommandLine();
}

void ScriptingConsole::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);

    menu.addAction(QIcon(QStringLiteral(":/qeditor/copy.png")), tr("Copy"),
                   QKeySequence(QKeySequence::Copy), this,
                   &ScriptingConsole::copy);
    menu.addAction(QIcon(QStringLiteral(":/qeditor/cut.png")), tr("Cut"),
                   QKeySequence(QKeySequence::Cut), this,
                   &ScriptingConsole::cut);
    menu.addAction(QIcon(QStringLiteral(":/qeditor/paste.png")), tr("Paste"),
                   QKeySequence(QKeySequence::Paste), this,
                   &ScriptingConsole::paste);

    menu.exec(event->globalPos());
}
