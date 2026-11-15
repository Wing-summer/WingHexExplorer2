/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#ifndef BACKGROUNDSCRIPTOUTPUT_H
#define BACKGROUNDSCRIPTOUTPUT_H

#include <memory>
#include <vector>

#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QStackedWidget>

#include "luau/runtime/scriptmachine.h"

class QAction;

class BackgroundScriptOutput final : public QWidget {
    Q_OBJECT
public:
    explicit BackgroundScriptOutput(QWidget *parent = nullptr);
    virtual ~BackgroundScriptOutput() override;
    void runScript(const QString &path);
    void appendOutput(const ScriptMachine::MessageInfo &message);
    QString requestInput(quint64 taskId);

signals:
    void sigOnUnreadOutputReceived();

protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    struct Page {
        QString path;
        quint64 taskId = 0;
        quint64 lastUsed = 0;
        bool unread = false;
        ScriptExecution::State state = ScriptExecution::State::Pending;
        ScriptMachine::MessageType lastOutputType =
            ScriptMachine::MessageType::Unknown;
        QListWidgetItem *item = nullptr;
        QPlainTextEdit *output = nullptr;

        inline bool active() const {
            return state != ScriptExecution::State::Success &&
                   state != ScriptExecution::State::Failed &&
                   state != ScriptExecution::State::Aborted;
        }
    };

    Page *currentPage() const;
    Page *findPage(quint64 taskId) const;
    void selectPage();
    void refreshPage(Page &page);
    void refreshActions();
    void refreshCount();
    void clearCurrent();
    void stopCurrent();
    void closeCurrent();
    void removePage(Page *page);
    void showOutputMenu(const QPoint &position);
    void taskStateChanged(quint64 taskId, ScriptExecution::State state);
    static QString stateText(ScriptExecution::State state);

private:
    std::vector<std::unique_ptr<Page>> m_pages;
    quint64 m_useCounter = 0;
    QListWidget *m_list = nullptr;
    QLabel *m_count = nullptr;
    QLabel *m_title = nullptr;
    QLabel *m_status = nullptr;
    QSplitter *m_splitter = nullptr;
    QWidget *m_sidebar = nullptr;
    QStackedWidget *m_stack = nullptr;
    QAction *m_clear = nullptr;
    QAction *m_stop = nullptr;
    QAction *m_close = nullptr;
};
#endif // BACKGROUNDSCRIPTOUTPUT_H
