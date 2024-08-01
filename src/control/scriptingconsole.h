#ifndef ScriptingConsole_H
#define ScriptingConsole_H

#include <QTextEdit>
#include <QVariant>

#include "src/class/scriptconsolemachine.h"

class QCompleter;

//-------------------------------------------------------------------------------
//! A simple console for scripting
class ScriptingConsole : public QTextEdit {
    Q_OBJECT

public:
    ScriptingConsole(QWidget *parent, Qt::WindowFlags i = Qt::Widget);

    ~ScriptingConsole();

public Q_SLOTS:
    //! execute current line
    void executeLine(bool storeOnly);

    //! derived key press event
    void keyPressEvent(QKeyEvent *e);

    //! output from console
    void consoleMessage(const QString &message);

    //! get history
    QStringList history() { return _history; }

    //! set history
    void setHistory(const QStringList &h) {
        _history = h;
        _historyPosition = 0;
    }

    //! clear the console
    void clear();

    //! overridden to control which characters a user may delete
    virtual void cut();

    //! output redirection
    void stdOut(const QString &s);
    //! output redirection
    void stdErr(const QString &s);

    void insertCompletion(const QString &);

    //! Appends a newline and command prompt at the end of the document.
    void appendCommandPrompt(bool storeOnly = false);

public:
    //! returns true if python cerr had an error
    bool hadError() { return _hadError; }

    //! returns true if python cerr had an error
    void clearError() { _hadError = false; }

protected:
    //! handle the pressing of tab
    void handleTabCompletion();

    //! Returns the position of the command prompt
    int commandPromptPosition();

    //! Returns if deletion is allowed at the current cursor
    //! (with and without selected text)
    bool verifySelectionBeforeDeletion();

    //! Sets the current font
    void setCurrentFont(const QColor &color = QColor(0, 0, 0),
                        bool bold = false);

    //! change the history according to _historyPos
    void changeHistory();

    //! flush output that was not yet printed
    void flushStdOut();

private:
    void executeCode(const QString &code);

    std::string getInput();

private:
    ScriptConsoleMachine *_sp;

    QStringList _history;
    int _historyPosition;

    QString _clickedAnchor;
    QString _storageKey;
    QString _commandPrompt;

    QString _currentMultiLineCode;

    QString _stdOut;
    QString _stdErr;

    QTextCharFormat _defaultTextCharacterFormat;
    QCompleter *_completer;

    std::function<std::string(void)> _getInputFn;
    bool _hadError;
};

#endif
