/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#ifndef _QCONSOLEWIDGET_H_
#define _QCONSOLEWIDGET_H_

#include <QCompleter>
#include <QTextStream>

#include "WingCodeEdit/wingcodeedit.h"

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
    // write a formatted message to the console
    void write(const QString &message, const QTextCharFormat &fmt);
    static History &history();

    // get the current command line
    QString getCommandLine();

    virtual void paste();

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
    bool canCut() const;

    virtual void handleReturnKey();
    virtual void handleTabKey();
    // reimp QPlainTextEdit functions
    void keyPressEvent(QKeyEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    // Returns true if there is a selection in edit zone
    bool isSelectionInEditZone() const;
    // Returns true if cursor is in edit zone
    bool isCursorInEditZone() const;
    // replace the command line
    void replaceCommandLine(const QString &str);

    QString currentCommandLine() const;

protected:
    static History history_;
    ConsoleMode mode_;
    int inpos_;
    QString currentMultiLineCode_;
    QConsoleIODevice *iodevice_;
    QTextCharFormat chanFormat_[nConsoleChannels];
};

QTextStream &waitForInput(QTextStream &s);
QTextStream &inputMode(QTextStream &s);
QTextStream &outChannel(QTextStream &s);
QTextStream &errChannel(QTextStream &s);

#endif
