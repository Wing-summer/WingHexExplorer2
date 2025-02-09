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
#include "class/logger.h"
#include "class/scriptconsolemachine.h"
#include "qregularexpression.h"

#include <QApplication>
#include <QColor>
#include <QKeyEvent>

ScriptingConsole::ScriptingConsole(QWidget *parent) : QConsoleWidget(parent) {}

ScriptingConsole::~ScriptingConsole() {}

void ScriptingConsole::stdOut(const QString &str) { writeStdOut(str); }

void ScriptingConsole::stdErr(const QString &str) { writeStdErr(str); }

void ScriptingConsole::stdWarn(const QString &str) {
    write(str, QStringLiteral("stdwarn"));
}

void ScriptingConsole::newLine() { _s << Qt::endl; }

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
                switch (type) {
                case ScriptMachine::MessageType::Info:
                    stdOut(tr("[Info]") + message.message);
                    _s << Qt::flush;
                    newLine();
                    break;
                case ScriptMachine::MessageType::Warn:
                    stdWarn(tr("[Warn]") + message.message);
                    _s << Qt::flush;
                    newLine();
                    break;
                case ScriptMachine::MessageType::Error:
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

void ScriptingConsole::pushInputCmd(const QString &cmd) {
    QMutexLocker locker(&_queueLocker);
    _cmdQueue.append(cmd);
}

void ScriptingConsole::processKeyEvent(QKeyEvent *e) { keyPressEvent(e); }

void ScriptingConsole::runConsoleCommand(const QString &code) {
    if (_waitforRead) {
        _waitforRead = false;
        return;
    }

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
    _waitforRead = true;
    QString instr;

    auto d = _s.device();
    d->skip(d->bytesAvailable());

    do {
        {
            QMutexLocker locker(&_queueLocker);
            if (!_cmdQueue.isEmpty()) {
                instr = _cmdQueue.takeFirst();
                setMode(Output);
                QConsoleWidget::write(instr);
                setMode(Input);
                break;
            }
        }
        qApp->processEvents();
    } while (!d->waitForReadyRead(100));

    instr = _s.readAll();

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

void ScriptingConsole::appendCommandPrompt(bool storeOnly) {
    QString commandPrompt;

    if (storeOnly) {
        commandPrompt += QStringLiteral("... > ");
    } else {
        auto cursor = this->cursor();
        if (!cursor.atBlockStart()) {
            commandPrompt = QStringLiteral("\n");
        }
        if (_sp && _sp->isDebugMode()) {
            commandPrompt += QStringLiteral("[dbg] > ");
        } else {
            commandPrompt += QStringLiteral("as > ");
        }
    }

    _lastCommandPrompt = storeOnly;

    QConsoleWidget::write(commandPrompt);
}

ScriptMachine *ScriptingConsole::machine() const { return _sp; }

ScriptConsoleMachine *ScriptingConsole::consoleMachine() const { return _sp; }
