#ifndef _QCONSOLEWIDGET_H_
#define _QCONSOLEWIDGET_H_

#include <QCompleter>
#include <QPlainTextEdit>
#include <QTextStream>

class QConsoleIODevice;
class QConsoleWidgetCompleter;

class QConsoleWidget : public QPlainTextEdit {
    Q_OBJECT

public:
    struct History {
        QStringList strings_;
        int pos_;
        QString token_;
        bool active_;
        int maxsize_;
        History(void);
        ~History(void);
        void add(const QString &str);
        const QString &currentValue() const {
            return pos_ == -1 ? token_ : strings_.at(pos_);
        }
        void activate(const QString &tk = QString());
        void deactivate() { active_ = false; }
        bool isActive() const { return active_; }
        bool move(bool dir);
        int indexOf(bool dir, int from) const;
    };

public:
    enum ConsoleMode { Input, Output };
    Q_ENUM(ConsoleMode)

    enum ConsoleChannel {
        StandardInput,
        StandardOutput,
        StandardError,
        nConsoleChannels
    };
    Q_ENUM(ConsoleChannel)

public:
    explicit QConsoleWidget(QWidget *parent = nullptr);
    virtual ~QConsoleWidget();

public:
    ConsoleMode mode() const;
    void setMode(ConsoleMode m);
    QIODevice *device() const;
    QTextCharFormat channelCharFormat(ConsoleChannel ch) const;
    void setChannelCharFormat(ConsoleChannel ch, const QTextCharFormat &fmt);
    const QStringList &completionTriggers() const;
    void setCompletionTriggers(const QStringList &l);
    virtual QSize sizeHint() const override;
    // write a formatted message to the console
    void write(const QString &message, const QTextCharFormat &fmt);
    static History &history();
    void setCompleter(QConsoleWidgetCompleter *c);
    // get the current command line
    QString getCommandLine();

public slots:

    // write to StandardOutput
    void writeStdOut(const QString &s);
    // write to StandardError
    void writeStdErr(const QString &s);

signals:

    // fired when user hits the return key
    void consoleCommand(const QString &code);
    // fired when user enters a Ctrl-Q
    void abortEvaluation();

protected:
    bool canPaste() const;
    bool canCut() const { return isSelectionInEditZone(); }
    void handleReturnKey();
    void handleTabKey();
    void updateCompleter();
    void checkCompletionTriggers(const QString &txt);
    // reimp QPlainTextEdit functions
    void keyPressEvent(QKeyEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    // Returns true if there is a selection in edit zone
    bool isSelectionInEditZone() const;
    // Returns true if cursor is in edit zone
    bool isCursorInEditZone() const;
    // replace the command line
    void replaceCommandLine(const QString &str);

protected slots:

    // insert the completion from completer
    void insertCompletion(const QString &completion);

private:
    static History history_;
    ConsoleMode mode_;
    int inpos_, completion_pos_;
    QStringList completion_triggers_;
    QString currentMultiLineCode_;
    QConsoleIODevice *iodevice_;
    QTextCharFormat chanFormat_[nConsoleChannels];
    QConsoleWidgetCompleter *completer_;
};

QTextStream &waitForInput(QTextStream &s);
QTextStream &inputMode(QTextStream &s);
QTextStream &outChannel(QTextStream &s);
QTextStream &errChannel(QTextStream &s);

class QConsoleWidgetCompleter : public QCompleter {
public:
    /*
     * Update the completion model given a string.  The given string
     * is the current console text between the cursor and the start of
     * the line.
     *
     * Return the completion count
     */
    virtual int updateCompletionModel(const QString &str) = 0;

    /*
     * Return the position in the command line where the completion
     * should be inserted
     */
    virtual int insertPos() = 0;
};

#endif
