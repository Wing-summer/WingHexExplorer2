#include "scriptingconsole.h"
#include "class/scriptconsolemachine.h"

#include <QApplication>
#include <QColor>
#include <QShortcut>

ScriptingConsole::ScriptingConsole(QWidget *parent) : QConsoleWidget(parent) {
    m_stdoutFmtTitle = this->currentCharFormat();

    m_stdoutFmtWarn = m_stdoutFmtContent =
        channelCharFormat(ConsoleChannel::StandardOutput);

    m_stdoutFmtContent.setForeground(Qt::green);
    m_stdoutFmtWarn.setForeground(QColorConstants::Svg::gold);

    setChannelCharFormat(ConsoleChannel::StandardOutput, m_stdoutFmtContent);

    _s.setDevice(this->device());
    stdWarn(tr("Scripting console for WingHexExplorer"));
    _s << Qt::endl;
    stdWarn(tr(">>>> Powered by AngelScript <<<<"));
    _s << Qt::endl << Qt::endl;
    appendCommandPrompt();
    setMode(Input);

    auto shortCut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_L), this);
    connect(shortCut, &QShortcut::activated, this,
            &ScriptingConsole::clearConsole);
}

ScriptingConsole::~ScriptingConsole() {}

void ScriptingConsole::stdOut(const QString &str) { writeStdOut(str); }

void ScriptingConsole::stdErr(const QString &str) { writeStdErr(str); }

void ScriptingConsole::stdWarn(const QString &str) {
    write(str, m_stdoutFmtWarn);
}

void ScriptingConsole::newLine() { _s << Qt::endl; }

void ScriptingConsole::init() {
    _getInputFn = std::bind(&ScriptingConsole::getInput, this);

    _sp = new ScriptConsoleMachine(_getInputFn, this);
    connect(_sp, &ScriptConsoleMachine::onClearConsole, this,
            &ScriptingConsole::clear);

    connect(_sp, &ScriptConsoleMachine::onOutput, this,
            [=](ScriptConsoleMachine::MessageType type,
                const ScriptConsoleMachine::MessageInfo &message) {
                switch (type) {
                case ScriptMachine::MessageType::Info:
                    stdOut(tr("[Info]") + message.message);
                    newLine();
                    break;
                case ScriptMachine::MessageType::Warn:
                    stdWarn(tr("[Warn]") + message.message);
                    newLine();
                    break;
                case ScriptMachine::MessageType::Error:
                    stdErr(tr("[Error]") + message.message);
                    newLine();
                    break;
                case ScriptMachine::MessageType::Print:
                    stdOut(message.message);
                    break;
                }
            });

    connect(this, &QConsoleWidget::consoleCommand, this,
            &ScriptingConsole::consoleCommand);
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

void ScriptingConsole::consoleCommand(const QString &code) {
    if (_waitforRead) {
        _waitforRead = false;
        return;
    }

    setMode(Output);
    if (!_sp->executeCode(code)) {
    }
    appendCommandPrompt();
    setMode(Input);
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
                write(instr, QTextCharFormat());
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

void ScriptingConsole::appendCommandPrompt(bool storeOnly) {
    QString commandPrompt;

    if (storeOnly) {
        commandPrompt += QStringLiteral("... > ");
    } else {
        auto cursor = this->textCursor();
        if (!cursor.atBlockStart()) {
            commandPrompt = QStringLiteral("\n");
        }
        if (_sp && _sp->isInDebugMode()) {
            commandPrompt += QStringLiteral("[dbg] > ");
        } else {
            commandPrompt += QStringLiteral("as > ");
        }
    }

    _lastCommandPrompt = storeOnly;

    write(commandPrompt, m_stdoutFmtTitle);
}

ScriptMachine *ScriptingConsole::machine() const { return _sp; }

ScriptConsoleMachine *ScriptingConsole::consoleMachine() const { return _sp; }
