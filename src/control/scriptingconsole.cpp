#include "scriptingconsole.h"
#include "class/scriptconsolemachine.h"

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
            [=]() { executeCode(QStringLiteral("clear()")); });
}

ScriptingConsole::~ScriptingConsole() {}

void ScriptingConsole::stdOut(const QString &str) { writeStdOut(str); }

void ScriptingConsole::stdErr(const QString &str) { writeStdErr(str); }

void ScriptingConsole::stdWarn(const QString &str) {
    write(str, m_stdoutFmtWarn);
}

void ScriptingConsole::init(bool consoleMode) {
    _getInputFn = std::bind(&ScriptingConsole::getInput, this);

    if (consoleMode) {
        auto sp = new ScriptConsoleMachine(_getInputFn, this);
        connect(sp, &ScriptConsoleMachine::onClearConsole, this,
                &ScriptingConsole::clear);
        _sp = sp;
    } else {
        _sp = new ScriptMachine(_getInputFn, this);
    }

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
        commandPrompt += QStringLiteral("... > ");
    } else {
        auto cursor = this->textCursor();
        if (!cursor.atBlockStart()) {
            commandPrompt = QStringLiteral("\n");
        }
        commandPrompt += QStringLiteral("as > ");
    }

    write(commandPrompt, m_stdoutFmtTitle);
}

ScriptMachine *ScriptingConsole::machine() const { return _sp; }

ScriptConsoleMachine *ScriptingConsole::consoleMachine() const {
    return qobject_cast<ScriptConsoleMachine *>(_sp);
}
