#ifndef _QCONSOLEWIDGET_H_
#define _QCONSOLEWIDGET_H_

#include <QIODevice>
#include <QTextCharFormat>
#include <QTextStream>

#include "qeditor.h"

class QConsoleIODevice;

class QConsoleWidget : public QEditor {
    Q_OBJECT

public:
    enum ConsoleMode { Input, Output };
    Q_ENUM(ConsoleMode)

    explicit QConsoleWidget(QWidget *parent = nullptr);
    virtual ~QConsoleWidget();

    ConsoleMode mode() const { return mode_; }
    void setMode(ConsoleMode m);
    QIODevice *device() const { return (QIODevice *)iodevice_; }

    virtual QSize sizeHint() const override { return QSize(600, 400); }

    // write a formatted message to the console
    void write(const QString &message, const QString &sfmtID = {}) override;

    static const QStringList &history() { return history_.strings_; }

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

    void keyPressEvent(QKeyEvent *e) override;

    // Returns true if there is a selection in edit zone
    bool isSelectionInEditZone() const;
    // Returns true if cursor is in edit zone
    bool isCursorInEditZone() const;
    // replace the command line
    void replaceCommandLine(const QString &str);

    static QString getHistoryPath();

    // QEditor interface
public slots:
    virtual void cut() override;

private:
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

    static History history_;
    ConsoleMode mode_;
    QDocumentCursor inpos_;
    QString currentMultiLineCode_;
    QConsoleIODevice *iodevice_;
};

QTextStream &waitForInput(QTextStream &s);
QTextStream &inputMode(QTextStream &s);
QTextStream &outChannel(QTextStream &s);
QTextStream &errChannel(QTextStream &s);

#endif
