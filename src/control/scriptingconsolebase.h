#ifndef SCRIPTINGCONSOLEBASE_H
#define SCRIPTINGCONSOLEBASE_H

#include "QConsoleWidget/QConsoleWidget.h"

class ScriptingConsoleBase : public QConsoleWidget {
    Q_OBJECT
public:
    explicit ScriptingConsoleBase(QWidget *parent = nullptr);

public:
    QTextStream &consoleStream();

    bool lastCommandPrompt() const;

public slots:
    void stdOut(const QString &str);
    void stdErr(const QString &str);
    void stdWarn(const QString &str);
    void newLine();
    void flush();

    void initOutput();

    //! Appends a newline and command prompt at the end of the document.
    void appendCommandPrompt(bool storeOnly = false);

protected:
    void dontHighlightLastLine(bool followTheme);

    void dontHighlightLastOffset(int offset, bool followTheme = true);

private:
    QTextStream _s;
    QTextCharFormat _warnCharFmt;

    bool _lastCommandPrompt = false;
};

#endif // SCRIPTINGCONSOLEBASE_H
