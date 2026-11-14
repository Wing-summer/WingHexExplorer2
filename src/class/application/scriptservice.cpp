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

#include "scriptservice.h"

#include <utility>

#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QHostAddress>
#include <QSslError>
#include <QSslSocket>
#include <QTimer>

#include "class/security/wingcert.h"
#include "luau/runtime/scriptexecution.h"
#include "luau/runtime/scriptmachine.h"
#include "scriptrpcconnection.h"
#include "scriptrpcerror.h"

namespace {
constexpr int MaxActiveTasks = 64;
constexpr int MaxTaskRecords = 1024;
constexpr int MaxConnections = 64;

QString timestamp() {
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
}

ScriptRpcResult rpcFailure(ScriptRpcError code, const QString &message) {
    return ScriptRpcFailure{code, message};
}
} // namespace

ScriptService::ScriptService(const StartupOptions &options, QObject *parent)
    : QTcpServer(parent), m_options(options) {}

ScriptService::~ScriptService() {
    close();
    for (const auto &task : std::as_const(m_tasks)) {
        delete task->execution.data();
    }
    const auto connections = m_connections;
    for (auto connection : connections) {
        delete connection;
    }
}

ScriptService::StartResult ScriptService::start() {
    if (!m_options.certificate().isEmpty()) {
        const auto result = WingCert::instance().serverConfiguration(
            m_options.certificate(), m_options.privateKey());
        if (result.error != WingCert::ConfigurationError::None) {
            return {StartError::TlsConfiguration, result.message};
        }
        m_tlsConfiguration = result.configuration;
    }
    if (!listen(QHostAddress(m_options.address()), m_options.port())) {
        return {StartError::Listen,
                QStringLiteral("Cannot listen for script requests: ") +
                    errorString()};
    }
    return {};
}

void ScriptService::incomingConnection(qintptr descriptor) {
    auto socket = new QSslSocket(this);
    if (!socket->setSocketDescriptor(descriptor)) {
        delete socket;
        return;
    }
    if (m_stopping || m_connections.size() >= MaxConnections) {
        delete socket;
        return;
    }
    auto connection = new ScriptRpcConnection(socket, *this);
    m_connections.insert(connection);
    connect(connection, &QObject::destroyed, this,
            [this, connection] { m_connections.remove(connection); });
    if (m_options.certificate().isEmpty()) {
        connection->setReady();
        return;
    }
    socket->setSslConfiguration(m_tlsConfiguration);
    connect(socket, &QSslSocket::sslErrors, connection,
            [socket](const QList<QSslError> &errors) {
                if (WingCert::instance().verifyPeer(socket->peerCertificate(),
                                                    errors)) {
                    socket->ignoreSslErrors(errors);
                }
            });
    connect(socket, &QSslSocket::encrypted, connection, [socket, connection] {
        if (WingCert::instance().isTrustedCertificate(
                socket->peerCertificate())) {
            connection->setReady();
        } else {
            connection->close();
        }
    });
    socket->startServerEncryption();
}

QString ScriptService::endpoint() const {
    const auto host = serverAddress().toString();
    return QStringLiteral("%1://%2:%3")
        .arg(m_options.certificate().isEmpty() ? QStringLiteral("tcp")
                                               : QStringLiteral("tls"),
             serverAddress().protocol() == QAbstractSocket::IPv6Protocol
                 ? QStringLiteral("[%1]").arg(host)
                 : host)
        .arg(serverPort());
}

QString ScriptService::stateName(TaskState state) {
    switch (state) {
    case TaskState::Queued:
        return QStringLiteral("queued");
    case TaskState::Running:
        return QStringLiteral("running");
    case TaskState::Resident:
        return QStringLiteral("resident");
    case TaskState::Stopping:
        return QStringLiteral("stopping");
    case TaskState::Stopped:
        return QStringLiteral("stopped");
    case TaskState::Failed:
        return QStringLiteral("failed");
    }
    Q_UNREACHABLE();
}

bool ScriptService::Task::active() const {
    return state != TaskState::Stopped && state != TaskState::Failed;
}

QJsonObject ScriptService::Task::json() const {
    return {{QStringLiteral("taskId"), id},
            {QStringLiteral("path"), path},
            {QStringLiteral("state"), stateName(state)},
            {QStringLiteral("createdAt"), createdAt},
            {QStringLiteral("updatedAt"), updatedAt},
            {QStringLiteral("finishedAt"), finishedAt},
            {QStringLiteral("error"), error.isEmpty()
                                          ? QJsonValue(QJsonValue::Null)
                                          : QJsonValue(error)}};
}

QJsonArray ScriptService::taskList() const {
    QJsonArray result;
    for (const auto &task : m_tasks) {
        result.append(task->json());
    }
    return result;
}

QList<ScriptService::TaskSummary> ScriptService::taskSummaries() const {
    QList<TaskSummary> result;
    result.reserve(m_tasks.size());
    for (const auto &task : m_tasks) {
        result.append({task->id, task->path, task->state});
    }
    return result;
}

int ScriptService::activeTasks() const {
    int count = 0;
    for (const auto &task : m_tasks) {
        count += task->active();
    }
    return count;
}

bool ScriptService::validScript(const QString &path) {
    const QFileInfo file(path);
    const auto suffix = file.suffix();
    return file.isFile() && file.isReadable() &&
           file.size() <= DEFAULT_SCRIPT_FILE_SIZE_LIMIT &&
           (suffix.compare(QLatin1String("lua"), Qt::CaseInsensitive) == 0 ||
            suffix.compare(QLatin1String("luau"), Qt::CaseInsensitive) == 0);
}

ScriptRpcResult ScriptService::invoke(const QString &method,
                                      const QJsonObject &params) {
    if (method == QLatin1String("service.status")) {
        return QJsonObject{
            {"endpoint", endpoint()},
            {"state",
             m_stopping ? QLatin1String("stopping") : QLatin1String("running")},
            {"activeTasks", activeTasks()},
            {"taskRecords", m_tasks.size()},
            {"limits", QJsonObject{{"activeTasks", MaxActiveTasks},
                                   {"taskRecords", MaxTaskRecords},
                                   {"connections", MaxConnections}}}};
    }
    if (method == QLatin1String("service.shutdown")) {
        QTimer::singleShot(0, this, &ScriptService::shutdown);
        return QJsonObject{{QLatin1String("accepted"), true}};
    }
    if (method == QLatin1String("task.list")) {
        return taskList();
    }
    if (method != QLatin1String("task.execute") &&
        method != QLatin1String("task.get") &&
        method != QLatin1String("task.stop") &&
        method != QLatin1String("task.remove")) {
        return rpcFailure(ScriptRpcError::MethodNotFound,
                          QStringLiteral("Method not found"));
    }
    const auto idValue = params.value(QLatin1String("taskId"));
    const auto id = idValue.toString();
    if (!idValue.isString() || id.isEmpty() || id.toUtf8().size() > 256) {
        return rpcFailure(
            ScriptRpcError::InvalidParams,
            QStringLiteral(
                "Expected nonempty taskId (at most 256 UTF-8 bytes)"));
    }
    if (method == QLatin1String("task.execute")) {
        if (m_stopping) {
            return rpcFailure(ScriptRpcError::ServiceStopping,
                              QStringLiteral("Service is shutting down"));
        }
        if (m_tasks.contains(id)) {
            return rpcFailure(ScriptRpcError::DuplicateTask,
                              QStringLiteral("Task ID already exists"));
        }
        if (activeTasks() >= MaxActiveTasks ||
            m_tasks.size() >= MaxTaskRecords) {
            return rpcFailure(ScriptRpcError::TaskCapacity,
                              QStringLiteral("Task capacity reached"));
        }
        const auto path = params.value(QStringLiteral("path")).toString();
        if (!validScript(path)) {
            return rpcFailure(ScriptRpcError::InvalidScript,
                              QStringLiteral("Invalid or unreadable script"));
        }
        auto task = std::make_shared<Task>();
        task->id = id;
        task->path = QFileInfo(path).canonicalFilePath();
        task->createdAt = timestamp();
        task->updatedAt = task->createdAt;
        task->execution = ScriptMachine::instance().createExecution(task->path);
        if (!task->execution) {
            return rpcFailure(ScriptRpcError::InternalError,
                              QStringLiteral("Script engine is unavailable"));
        }
        m_tasks.insert(id, task);
        connect(task->execution, &ScriptExecution::sigOnEntryCompleted, this,
                [this, task] {
                    if (task->state != TaskState::Stopping) {
                        updateTask(task, TaskState::Resident);
                    }
                });
        connect(task->execution, &ScriptExecution::sigOnExecutionFailed, this,
                [this, task](ScriptFailure failure, const QString &message) {
                    taskFailed(task, failure, message);
                });
        connect(task->execution, &ScriptExecution::sigOnExecutionStopped, this,
                [this, task] { finishTask(task); });
        updateTask(task, TaskState::Queued);
        QTimer::singleShot(0, this, [this, task] {
            if (task->state == TaskState::Queued) {
                updateTask(task, TaskState::Running);
                task->execution->start();
            }
        });
        return task->json();
    }
    const auto task = m_tasks.value(id);
    if (!task) {
        return rpcFailure(ScriptRpcError::UnknownTask,
                          QStringLiteral("Unknown task"));
    }
    if (method == QStringLiteral("task.get")) {
        return task->json();
    }
    if (method == QStringLiteral("task.stop")) {
        stopTask(id);
        return task->json();
    }
    if (task->active()) {
        return rpcFailure(ScriptRpcError::InvalidTaskState,
                          QStringLiteral("Cannot remove an active task"));
    }
    m_tasks.remove(id);
    publish(QStringLiteral("task.removed"),
            QJsonObject{{QStringLiteral("taskId"), id}});
    Q_EMIT sigOnTasksChanged();
    return QJsonObject{{QStringLiteral("removed"), true}};
}

void ScriptService::publish(const QString &method, const QJsonValue &params) {
    QTimer::singleShot(0, this, [this, method, params] {
        const auto connections = m_connections;
        for (auto connection : connections) {
            connection->notify(method, params);
        }
    });
}

void ScriptService::updateTask(const std::shared_ptr<Task> &task,
                               TaskState state) {
    task->state = state;
    task->updatedAt = timestamp();
    if (!task->active()) {
        task->finishedAt = task->updatedAt;
    }
    publish(QStringLiteral("task.changed"), task->json());
    Q_EMIT sigOnTasksChanged();
}

bool ScriptService::stopTask(const QString &taskId) {
    const auto task = m_tasks.value(taskId);
    if (!task || !task->active() || task->state == TaskState::Stopping) {
        return false;
    }
    updateTask(task, TaskState::Stopping);
    task->execution->stop();
    return true;
}

void ScriptService::taskFailed(const std::shared_ptr<Task> &task,
                               ScriptFailure failure, const QString &message) {
    if (task->state != TaskState::Stopping) {
        QString code;
        switch (failure) {
        case ScriptFailure::FileError:
            code = QStringLiteral("file-error");
            break;
        case ScriptFailure::CompilationError:
            code = QStringLiteral("compilation-error");
            break;
        case ScriptFailure::ExecutionError:
            code = QStringLiteral("execution-error");
            break;
        case ScriptFailure::CallbackError:
            code = QStringLiteral("callback-error");
            break;
        case ScriptFailure::Timeout:
            code = QStringLiteral("timeout");
            break;
        }
        task->error = {{QStringLiteral("code"), code},
                       {QStringLiteral("message"), message}};
        qWarning("Script task %s failed: %s", qUtf8Printable(task->id),
                 qUtf8Printable(message));
    }
    finishTask(task);
}

void ScriptService::finishTask(const std::shared_ptr<Task> &task) {
    if (task->error.isEmpty() && task->state != TaskState::Stopping) {
        updateTask(task, TaskState::Stopping);
    }
    updateTask(task,
               task->error.isEmpty() ? TaskState::Stopped : TaskState::Failed);
    if (task->execution) {
        task->execution->deleteLater();
        task->execution = nullptr;
    }
    checkStopped();
}

void ScriptService::shutdown() {
    if (m_stopping) {
        return;
    }
    m_stopping = true;
    close();
    for (const auto &task : std::as_const(m_tasks)) {
        stopTask(task->id);
    }
    checkStopped();
}

void ScriptService::checkStopped() {
    if (m_stopping && !activeTasks() && !m_stopScheduled) {
        m_stopScheduled = true;
        QTimer::singleShot(250, this, &ScriptService::sigOnServiceStopped);
    }
}
