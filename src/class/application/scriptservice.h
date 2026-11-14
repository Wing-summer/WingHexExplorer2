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

#ifndef SCRIPTSERVICE_H
#define SCRIPTSERVICE_H

#include <memory>

#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QPointer>
#include <QSet>
#include <QSslConfiguration>
#include <QTcpServer>

#include "applicationmode.h"
#include "class/application/scriptrpcconnection.h"
#include "luau/runtime/scriptexecution.h"
#include "scriptrpcerror.h"

class ScriptService final : public QTcpServer {
    Q_OBJECT
public:
    explicit ScriptService(const StartupOptions &options,
                           QObject *parent = nullptr);
    virtual ~ScriptService() override;

public:
    enum class StartError { None, TlsConfiguration, Listen };

    struct StartResult {
        StartError error = StartError::None;
        QString message;
    };

    enum class TaskState {
        Queued,
        Running,
        Resident,
        Stopping,
        Stopped,
        Failed
    };

    struct TaskSummary {
        QString id;
        QString path;
        TaskState state;
    };

public:
    StartResult start();
    QString endpoint() const;
    QJsonArray taskList() const;
    QList<TaskSummary> taskSummaries() const;
    int activeTasks() const;
    ScriptRpcResult invoke(const QString &method, const QJsonObject &params);
    bool stopTask(const QString &taskId);
    void shutdown();
    static bool validScript(const QString &path);

signals:
    void sigOnTasksChanged();
    void sigOnServiceStopped();

protected:
    virtual void incomingConnection(qintptr descriptor) override;

private:
    struct Task {
        QString id;
        QString path;
        QString createdAt;
        QString updatedAt;
        QString finishedAt;
        TaskState state = TaskState::Queued;
        QJsonObject error;
        QPointer<ScriptExecution> execution;
        QJsonObject json() const;
        bool active() const;
    };

private:
    static QString stateName(TaskState state);
    void publish(const QString &method, const QJsonValue &params);
    void updateTask(const std::shared_ptr<Task> &task, TaskState state);
    void taskFailed(const std::shared_ptr<Task> &task, ScriptFailure failure,
                    const QString &message);
    void finishTask(const std::shared_ptr<Task> &task);
    void checkStopped();

private:
    const StartupOptions m_options;
    QSslConfiguration m_tlsConfiguration;
    QMap<QString, std::shared_ptr<Task>> m_tasks;
    QSet<ScriptRpcConnection *> m_connections;
    bool m_stopping = false;
    bool m_stopScheduled = false;
};
#endif // SCRIPTSERVICE_H
