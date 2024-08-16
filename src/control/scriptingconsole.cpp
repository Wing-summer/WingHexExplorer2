#include "scriptingconsole.h"

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
    _s << Qt::endl;
    appendCommandPrompt();
    setMode(Input);

    auto shortCut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_L), this);
    connect(shortCut, &QShortcut::activated, this,
            [=]() { executeCode(QStringLiteral("clear()")); });
}

ScriptingConsole::~ScriptingConsole() {}

void ScriptingConsole::stdOut(const QString &str) { writeStdOut(str); }

void ScriptingConsole::stdErr(const QString &str) { writeStdErr(str); }

void ScriptingConsole::stdWarn(const QString &str) {
    write(str, m_stdoutFmtWarn);
}

void ScriptingConsole::init() {
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
            });
    connect(_sp, &ScriptConsoleMachine::onClearConsole, this,
            &ScriptingConsole::clear);

    connect(this, &QConsoleWidget::consoleCommand, this,
            &ScriptingConsole::executeCode);
}

void ScriptingConsole::executeCode(const QString &code) {
    setMode(Output);
    if (!_sp->executeScript(code)) {
    }
    appendCommandPrompt();
    setMode(Input);
}

QString ScriptingConsole::getInput() {
    _s.device()->waitForReadyRead(-1);
    return _s.readAll();
}

void ScriptingConsole::appendCommandPrompt(bool storeOnly) {
    QString commandPrompt;
    if (storeOnly) {
        commandPrompt = QStringLiteral("\n... > ");
    } else {
        commandPrompt = QStringLiteral("\nas > ");
    }
    write(commandPrompt, m_stdoutFmtTitle);
}

ScriptConsoleMachine *ScriptingConsole::machine() const { return _sp; }
