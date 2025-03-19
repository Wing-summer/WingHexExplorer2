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
#include "class/wingconsolehighligher.h"
#include "model/codecompletionmodel.h"

#include <QApplication>
#include <QColor>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QTextBlock>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

ScriptingConsole::ScriptingConsole(QWidget *parent) : QConsoleWidget(parent) {
    _warnCharFmt.setForeground(QColorConstants::Svg::gold);
    setHighlighter(new WingConsoleHighligher);
    setSyntax(syntaxRepo().definitionForName("AngelScript"));
}

ScriptingConsole::~ScriptingConsole() {}

void ScriptingConsole::stdOut(const QString &str) {
    writeStdOut(str);
    dontHighlightLastLine();
}

void ScriptingConsole::stdErr(const QString &str) {
    writeStdErr(str);
    dontHighlightLastLine();
}

void ScriptingConsole::stdWarn(const QString &str) {
    write(str, _warnCharFmt);
    dontHighlightLastLine();
}

void ScriptingConsole::newLine() { _s << Qt::endl; }

void ScriptingConsole::dontHighlightLastLine() { dontHighlightLastOffset(-1); }

void ScriptingConsole::dontHighlightLastOffset(int offset) {
    auto blk = document()->lastBlock();
    auto hl = highlighter();
    hl->setProperty(blk, "cmdoff", offset);
    hl->rehighlightBlock(blk);
}

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
    _s.setDevice(this->device());

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
                        _s << Qt::endl;
                    }
                    stdOut(tr("[Info]") + message.message);
                    _s << Qt::flush;
                    newLine();
                    break;
                case ScriptMachine::MessageType::Warn:
                    if (isNotBlockStart) {
                        _s << Qt::endl;
                    }
                    stdWarn(tr("[Warn]") + message.message);
                    _s << Qt::flush;
                    newLine();
                    break;
                case ScriptMachine::MessageType::Error:
                    if (isNotBlockStart) {
                        _s << Qt::endl;
                    }
                    stdErr(tr("[Error]") + message.message);
                    _s << Qt::flush;
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

void ScriptingConsole::initOutput() {
    stdWarn(tr("Scripting console for WingHexExplorer"));
    _s << Qt::endl;
    stdWarn(tr(">>>> Powered by AngelScript <<<<"));
    _s << Qt::endl << Qt::endl;
    appendCommandPrompt();
    setMode(Input);
}

void ScriptingConsole::clearConsole() {
    setMode(Output);
    clear();
    appendCommandPrompt(_lastCommandPrompt);
    setMode(Input);
}

void ScriptingConsole::processKeyEvent(QKeyEvent *e) { keyPressEvent(e); }

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
    _s.device()->waitForReadyRead(-1);
    QString instr;
    _s >> instr;
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

void ScriptingConsole::appendCommandPrompt(bool storeOnly) {
    QString commandPrompt;

    if (storeOnly) {
        commandPrompt += QStringLiteral("... > ");
    } else {
        auto cursor = this->textCursor();
        if (!cursor.atBlockStart()) {
            newLine();
        }
        if (_sp && _sp->isDebugMode()) {
            commandPrompt += QStringLiteral("[dbg] > ");
        } else {
            commandPrompt += QStringLiteral("as > ");
        }
    }

    _lastCommandPrompt = storeOnly;

    writeStdOut(commandPrompt);
    dontHighlightLastOffset(commandPrompt.length());
}

ScriptMachine *ScriptingConsole::machine() const { return _sp; }

ScriptConsoleMachine *ScriptingConsole::consoleMachine() const { return _sp; }
