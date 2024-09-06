#ifndef ScriptingConsole_H
#define ScriptingConsole_H

#include "QConsoleWidget/src/QConsoleWidget.h"
#include "class/scriptmachine.h"

class ScriptConsoleMachine;

class ScriptingConsole : public QConsoleWidget {
    Q_OBJECT

public:
    explicit ScriptingConsole(QWidget *parent = nullptr);

    virtual ~ScriptingConsole();

    ScriptMachine *machine() const;
    ScriptConsoleMachine *consoleMachine() const;

public slots:
    void stdOut(const QString &str);
    void stdErr(const QString &str);
    void stdWarn(const QString &str);

    void init(bool consoleMode);

private:
    void executeCode(const QString &code);

    QString getInput();

    //! Appends a newline and command prompt at the end of the document.
    void appendCommandPrompt(bool storeOnly = false);

private:
    ScriptMachine *_sp = nullptr;
    QTextStream _s;

    QTextCharFormat m_stdoutFmtTitle;
    QTextCharFormat m_stdoutFmtContent;
    QTextCharFormat m_stdoutFmtWarn;

    std::function<QString(void)> _getInputFn;
};

#endif
