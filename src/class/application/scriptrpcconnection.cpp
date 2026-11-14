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

#include "scriptrpcconnection.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QTimer>

#include "scriptservice.h"

namespace {
constexpr qint64 MaxHeader = 8192;
constexpr qint64 MaxBody = 1024 * 1024;
constexpr qint64 MaxOutput = 4 * MaxBody;
} // namespace

ScriptRpcConnection::ScriptRpcConnection(QTcpSocket *socket,
                                         ScriptService &service)
    : QObject(&service), m_socket(socket), m_service(service) {
    socket->setParent(this);
    socket->setReadBufferSize(MaxBody + MaxHeader);
    connect(socket, &QTcpSocket::readyRead, this,
            &ScriptRpcConnection::readMessages);
    connect(socket, &QTcpSocket::disconnected, this, &QObject::deleteLater);
    QTimer::singleShot(10000, this, [this] {
        if (!m_ready) {
            close();
        }
    });
}

void ScriptRpcConnection::setReady() {
    m_ready = true;
    readMessages();
}

void ScriptRpcConnection::close() {
    m_socket->abort();
    deleteLater();
}

QJsonObject ScriptRpcConnection::error(ScriptRpcError code,
                                       const QString &message,
                                       const QJsonValue &id) {
    return {{QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
            {QStringLiteral("id"), id},
            {QStringLiteral("error"),
             QJsonObject{{QStringLiteral("code"), static_cast<int>(code)},
                         {QStringLiteral("message"), message}}}};
}

void ScriptRpcConnection::readMessages() {
    if (!m_ready) {
        return;
    }
    m_buffer += m_socket->read(MaxBody + MaxHeader - m_buffer.size());
    while (true) {
        if (m_bodySize < 0) {
            const auto end = m_buffer.indexOf("\r\n\r\n");
            if (end < 0) {
                if (m_buffer.size() > MaxHeader) {
                    close();
                }
                break;
            }
            if (end + 4 > MaxHeader) {
                close();
                return;
            }
            int lengths = 0;
            for (auto &line : m_buffer.first(end).split('\n')) {
                const auto colon = line.indexOf(':');
                if (colon < 1) {
                    close();
                    return;
                }
                if (line.first(colon).trimmed().compare(
                        "Content-Length", Qt::CaseInsensitive) != 0) {
                    continue;
                }
                const auto value = line.sliced(colon + 1).trimmed();
                if (value.isEmpty()) {
                    close();
                    return;
                }
                for (const auto byte : value) {
                    if (byte < '0' || byte > '9') {
                        close();
                        return;
                    }
                }
                bool ok = false;
                m_bodySize = value.toLongLong(&ok);
                if (!ok || ++lengths != 1 || m_bodySize > MaxBody) {
                    close();
                    return;
                }
            }
            if (lengths != 1) {
                close();
                return;
            }
            m_buffer.remove(0, end + 4);
        }
        if (m_buffer.size() < m_bodySize) {
            break;
        }
        const auto body = m_buffer.first(m_bodySize);
        m_buffer.remove(0, m_bodySize);
        m_bodySize = -1;
        QJsonParseError parseError;
        const auto document = QJsonDocument::fromJson(body, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            const auto wrapped =
                QJsonDocument::fromJson(QByteArray("[") + body + "]");
            if (wrapped.isArray() && wrapped.array().size() == 1) {
                send(dispatch(wrapped.array().first()));
            } else {
                send(error(ScriptRpcError::ParseError,
                           QStringLiteral("Parse error")));
            }
        } else {
            const auto response =
                dispatch(document.isArray() ? QJsonValue(document.array())
                                            : QJsonValue(document.object()));
            if (!response.isUndefined()) {
                send(response);
            }
        }
        if (m_socket->state() == QAbstractSocket::UnconnectedState) {
            return;
        }
    }
    if (m_socket->bytesAvailable()) {
        QTimer::singleShot(0, this, &ScriptRpcConnection::readMessages);
    }
}

QJsonValue ScriptRpcConnection::dispatch(const QJsonValue &value) {
    if (!value.isArray()) {
        return request(value);
    }
    const auto batch = value.toArray();
    if (batch.isEmpty()) {
        return error(ScriptRpcError::InvalidRequest,
                     QStringLiteral("Invalid request"));
    }
    QJsonArray responses;
    for (const auto &item : batch) {
        const auto response = request(item);
        if (!response.isUndefined()) {
            responses.append(response);
        }
    }
    return responses.isEmpty() ? QJsonValue(QJsonValue::Undefined)
                               : QJsonValue(responses);
}

QJsonValue ScriptRpcConnection::request(const QJsonValue &value) {
    const auto object = value.toObject();
    const auto id = object.value(QStringLiteral("id"));
    const auto method = object.value(QStringLiteral("method"));
    if (!value.isObject() ||
        object.value(QStringLiteral("jsonrpc")) != QStringLiteral("2.0") ||
        !method.isString() ||
        (!id.isUndefined() && !id.isString() && !id.isDouble() &&
         !id.isNull())) {
        return error(ScriptRpcError::InvalidRequest,
                     QStringLiteral("Invalid request"));
    }
    QJsonObject response;
    const auto params = object.value(QStringLiteral("params"));
    if (!params.isUndefined() && !params.isObject()) {
        response = error(ScriptRpcError::InvalidParams,
                         QStringLiteral("Expected named parameters"), id);
    } else if (method.toString() == QStringLiteral("service.subscribe")) {
        const auto enabled = params.toObject().value(QStringLiteral("enabled"));
        if (!enabled.isBool()) {
            response = error(ScriptRpcError::InvalidParams,
                             QStringLiteral("Expected enabled boolean"), id);
        } else {
            m_subscribed = enabled.toBool();
            response.insert(QStringLiteral("result"), m_service.taskList());
        }
    } else {
        const auto result =
            m_service.invoke(method.toString(), params.toObject());
        if (const auto failure = std::get_if<ScriptRpcFailure>(&result)) {
            response = error(failure->code, failure->message, id);
        } else {
            response.insert(QStringLiteral("result"),
                            std::get<QJsonValue>(result));
        }
    }
    if (id.isUndefined()) {
        return QJsonValue(QJsonValue::Undefined);
    }
    response.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
    response.insert(QStringLiteral("id"), id);
    return response;
}

void ScriptRpcConnection::send(const QJsonValue &value) {
    const auto body =
        value.isArray()
            ? QJsonDocument(value.toArray()).toJson(QJsonDocument::Compact)
            : QJsonDocument(value.toObject()).toJson(QJsonDocument::Compact);
    const QByteArray packet = QByteArray("Content-Length: ") +
                              QByteArray::number(body.size()) + "\r\n\r\n" +
                              body;
    if (body.size() > MaxBody ||
        m_socket->bytesToWrite() + packet.size() > MaxOutput) {
        close();
        return;
    }
    m_socket->write(packet);
}

void ScriptRpcConnection::notify(const QString &method,
                                 const QJsonValue &params) {
    if (m_ready && m_subscribed) {
        send(QJsonObject{{QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
                         {QStringLiteral("method"), method},
                         {QStringLiteral("params"), params}});
    }
}
