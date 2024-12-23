// Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
// SPDX-License-Identifier: BSD-3-Clause

#ifndef QTLOCALPEER_H
#define QTLOCALPEER_H

#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>

#include "qtlockedfile.h"

class QtLocalPeer : public QObject {
    Q_OBJECT

public:
    QtLocalPeer(QObject *parent = nullptr, const QString &appId = QString());
    bool isClient();
    bool sendMessage(const QByteArray &uMsg, int timeout);
    QString applicationId() const { return id; }

Q_SIGNALS:
    void messageReceived(const QByteArray &message);

protected Q_SLOTS:
    void receiveConnection();

protected:
    QString id;
    QString socketName;
    QLocalServer *server;
    QtLP_Private::QtLockedFile lockFile;

private:
    static const char *ack;
};

#endif // QTLOCALPEER_H
