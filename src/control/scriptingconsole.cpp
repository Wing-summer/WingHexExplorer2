#include "scriptingconsole.h"

#include <QColor>

ScriptingConsole::ScriptingConsole(QWidget *parent) : QConsoleWidget(parent) {
    m_stdoutFmtTitle = this->currentCharFormat();

    m_stdoutFmtWarn = m_stdoutFmtContent =
        channelCharFormat(ConsoleChannel::StandardOutput);

    m_stdoutFmtContent.setForeground(Qt::green);
    m_stdoutFmtWarn.setForeground(QColorConstants::Svg::gold);

    setChannelCharFormat(ConsoleChannel::StandardOutput, m_stdoutFmtContent);

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
                    stdWarn(message.message);
                    break;
                case ScriptMachine::MessageType::Error:
                    stdErr(message.message);
                    break;
                }
                appendCommandPrompt();
            });

    connect(this, &QConsoleWidget::consoleCommand, this,
            &ScriptingConsole::executeCode);

    _s.setDevice(this->device());
    appendCommandPrompt();
    setMode(Input);
}

ScriptingConsole::~ScriptingConsole() {}

void ScriptingConsole::stdOut(const QString &str) {
    writeStdOut(str);
    _s << Qt::endl;
}

void ScriptingConsole::stdErr(const QString &str) {
    writeStdErr(str);
    _s << Qt::endl;
}

void ScriptingConsole::stdWarn(const QString &str) {
    write(str, m_stdoutFmtWarn);
    _s << Qt::endl;
}

void ScriptingConsole::executeCode(const QString &code) {
    setMode(Output);
    _sp->executeScript(code);
    setMode(Input);
}

std::string ScriptingConsole::getInput() {
    _s.device()->waitForReadyRead(-1);
    return _s.readAll().toStdString();
}

void ScriptingConsole::appendCommandPrompt(bool storeOnly) {
    QString commandPrompt;
    if (storeOnly) {
        commandPrompt = QStringLiteral("... > ");
    } else {
        commandPrompt = QStringLiteral("as > ");
    }
    write(commandPrompt, m_stdoutFmtTitle);
}
