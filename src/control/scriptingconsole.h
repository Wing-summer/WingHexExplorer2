#ifndef ScriptingConsole_H
#define ScriptingConsole_H

#include "QConsoleWidget/src/QConsoleWidget.h"
#include "class/scriptconsolemachine.h"

#include <QMutex>

class ScriptingConsole : public QConsoleWidget {
    Q_OBJECT

public:
    explicit ScriptingConsole(QWidget *parent = nullptr);

    virtual ~ScriptingConsole();

    ScriptMachine *machine() const;
    ScriptConsoleMachine *consoleMachine() const;

    //! Appends a newline and command prompt at the end of the document.
    void appendCommandPrompt(bool storeOnly = false);

public slots:
    void stdOut(const QString &str);
    void stdErr(const QString &str);
    void stdWarn(const QString &str);
    void newLine();

    void init();

    void clearConsole();

    void pushInputCmd(const QString &cmd);

private:
    void consoleCommand(const QString &code);

    QString getInput();

private:
    ScriptConsoleMachine *_sp = nullptr;
    QTextStream _s;

    bool _lastCommandPrompt = false;

    QStringList _cmdQueue;
    QMutex _queueLocker;
    bool _waitforRead = false;

    QTextCharFormat m_stdoutFmtTitle;
    QTextCharFormat m_stdoutFmtContent;
    QTextCharFormat m_stdoutFmtWarn;

    std::function<QString(void)> _getInputFn;
};

#endif
