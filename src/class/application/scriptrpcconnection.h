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

#ifndef SCRIPTRPCCONNECTION_H
#define SCRIPTRPCCONNECTION_H

#include <QJsonValue>
#include <QObject>

#include "scriptrpcerror.h"

class QTcpSocket;
class ScriptService;

class ScriptRpcConnection final : public QObject {
    Q_OBJECT
public:
    explicit ScriptRpcConnection(QTcpSocket *socket, ScriptService &service);

public:
    void setReady();
    void notify(const QString &method, const QJsonValue &params);
    void close();

    inline bool subscribed() const { return m_subscribed; }

private:
    void readMessages();
    QJsonValue dispatch(const QJsonValue &value);
    QJsonValue request(const QJsonValue &value);
    void send(const QJsonValue &value);
    static QJsonObject error(ScriptRpcError code, const QString &message,
                             const QJsonValue &id = QJsonValue::Null);

private:
    QTcpSocket *m_socket;
    ScriptService &m_service;
    QByteArray m_buffer;
    qint64 m_bodySize = -1;
    bool m_ready = false;
    bool m_subscribed = false;
};
#endif // SCRIPTRPCCONNECTION_H
