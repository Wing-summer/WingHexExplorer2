#ifndef _QCONSOLEWIDGET_H_
#define _QCONSOLEWIDGET_H_

#include "WingCodeEdit/wingcodeedit.h"
#include <QTextStream>

class QConsoleIODevice;

class QConsoleWidget : public WingCodeEdit {
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

    QConsoleWidget(QWidget *parent = 0);
    ~QConsoleWidget();

    ConsoleMode mode() const { return mode_; }
    void setMode(ConsoleMode m);
    QIODevice *device() const { return (QIODevice *)iodevice_; }

    virtual QSize sizeHint() const override { return QSize(600, 400); }
    // write a formatted message to the console
    void write(const QString &message);

    // get the current command line
    QString getCommandLine();

    static History &history();

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

    // reimp QPlainTextEdit functions
    void keyPressEvent(QKeyEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    // Returns true if there is a selection in edit zone
    bool isSelectionInEditZone() const;
    // Returns true if cursor is in edit zone
    bool isCursorInEditZone() const;
    // replace the command line
    void replaceCommandLine(const QString &str);

private:
    ConsoleMode mode_;
    int inpos_;
    QString currentMultiLineCode_;
    QConsoleIODevice *iodevice_;

    static History history_;
};

QTextStream &waitForInput(QTextStream &s);
QTextStream &inputMode(QTextStream &s);
QTextStream &outChannel(QTextStream &s);

#endif
