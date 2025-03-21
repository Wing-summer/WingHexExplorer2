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
#include "class/scriptconsolemachine.h"
#include "class/scriptsettings.h"
#include "model/codecompletionmodel.h"

#include <QApplication>
#include <QColor>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QTextBlock>

ScriptingConsole::ScriptingConsole(QWidget *parent)
    : ScriptingConsoleBase(parent) {
    connect(&ScriptSettings::instance(), &ScriptSettings::consoleSettingUpdate,
            this, &ScriptingConsole::applyScriptSettings);
    applyScriptSettings();
}

ScriptingConsole::~ScriptingConsole() {}

void ScriptingConsole::handleReturnKey() {
    QString code = getCommandLine();

    // start new block
    appendPlainText(QString());
    dontHighlightLastLine();
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

void ScriptingConsole::init() {
    _getInputFn = std::bind(&ScriptingConsole::getInput, this);

    _sp = new ScriptConsoleMachine(_getInputFn, this);
    connect(_sp, &ScriptConsoleMachine::onClearConsole, this,
            &ScriptingConsole::clear);
    connect(this, &ScriptingConsole::abortEvaluation, _sp,
            &ScriptConsoleMachine::abortScript);

    connect(_sp, &ScriptConsoleMachine::onOutput, this,
            [=](ScriptConsoleMachine::MessageType type,
                const ScriptConsoleMachine::MessageInfo &message) {
                auto doc = this->document();
                auto lastLine = doc->lastBlock();
                auto isNotBlockStart = !lastLine.text().isEmpty();
                switch (type) {
                case ScriptMachine::MessageType::Info:
                    if (isNotBlockStart) {
                        newLine();
                    }
                    stdOut(tr("[Info]") + message.message);
                    flush();
                    newLine();
                    break;
                case ScriptMachine::MessageType::Warn:
                    if (isNotBlockStart) {
                        newLine();
                    }
                    stdWarn(tr("[Warn]") + message.message);
                    flush();
                    newLine();
                    break;
                case ScriptMachine::MessageType::Error:
                    if (isNotBlockStart) {
                        newLine();
                    }
                    stdErr(tr("[Error]") + message.message);
                    flush();
                    newLine();
                    break;
                case ScriptMachine::MessageType::Print:
                    // If running ouput in the console,
                    // otherwise logging.
                    if (_sp->isRunning()) {
                        stdOut(message.message);
                    } else {
                        Logger::logPrint(Logger::packDebugStr(
                            packUpLoggingStr(message.message)));
                    }
                    break;
                }
            });

    connect(this, &QConsoleWidget::consoleCommand, this,
            &ScriptingConsole::runConsoleCommand);

    auto cm = new AsConsoleCompletion(_sp->engine(), this);
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

void ScriptingConsole::applyScriptSettings() {
    auto &set = ScriptSettings::instance();
    auto dfont = QFont(set.consoleFontFamily());
    dfont.setPointSize(set.consoleFontSize());
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
        if (!_sp->executeCode(_codes)) {
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

ScriptMachine *ScriptingConsole::machine() const { return _sp; }

ScriptConsoleMachine *ScriptingConsole::consoleMachine() const { return _sp; }
