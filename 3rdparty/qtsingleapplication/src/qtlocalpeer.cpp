// Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
// SPDX-License-Identifier: BSD-3-Clause

#include "qtlocalpeer.h"
#include <QCoreApplication>
#include <QDataStream>
#include <QRegularExpression>
#include <QTime>

#if defined(Q_OS_WIN)
#include <QLibrary>
#include <qt_windows.h>
typedef BOOL(WINAPI *PProcessIdToSessionId)(DWORD, DWORD *);
static PProcessIdToSessionId pProcessIdToSessionId = 0;
#endif
#if defined(Q_OS_UNIX)
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#endif

#include "qtlockedfile.h"

const char *QtLocalPeer::ack = "ack";

QtLocalPeer::QtLocalPeer(QObject *parent, const QString &appId)
    : QObject(parent), id(appId) {
    QString prefix = id;
    if (id.isEmpty()) {
        id = QCoreApplication::applicationFilePath();
#if defined(Q_OS_WIN)
        id = id.toLower();
#endif
        prefix = id.section(QLatin1Char('/'), -1);
    }
    static QRegularExpression regex("[^a-zA-Z]");
    prefix.remove(regex);
    prefix.truncate(6);

    QByteArray idc = id.toUtf8();
    quint16 idNum =
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        qChecksum(idc);
#else
        qChecksum(idc.constData(), idc.size());
#endif
    socketName = QLatin1String("qtsingleapp-") + prefix + QLatin1Char('-') +
                 QString::number(idNum, 16);

#if defined(Q_OS_WIN)
    if (!pProcessIdToSessionId) {
        QLibrary lib("kernel32");
        pProcessIdToSessionId =
            (PProcessIdToSessionId)lib.resolve("ProcessIdToSessionId");
    }
    if (pProcessIdToSessionId) {
        DWORD sessionId = 0;
        pProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
        socketName += QLatin1Char('-') + QString::number(sessionId, 16);
    }
#else
    socketName += QLatin1Char('-') + QString::number(::getuid(), 16);
#endif

    server = new QLocalServer(this);
    QString lockName = QDir(QDir::tempPath()).absolutePath() +
                       QLatin1Char('/') + socketName +
                       QLatin1String("-lockfile");
    lockFile.setFileName(lockName);
    lockFile.open(QIODevice::ReadWrite);
}

QtLocalPeer::~QtLocalPeer() { server->close(); }

bool QtLocalPeer::isClient() {
    if (lockFile.isLocked())
        return false;

    if (!lockFile.lock(QtLP_Private::QtLockedFile::WriteLock, false))
        return true;

    bool res = server->listen(socketName);
#if defined(Q_OS_UNIX) && (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
    // ### Workaround
    if (!res && server->serverError() == QAbstractSocket::AddressInUseError) {
        QFile::remove(QDir::cleanPath(QDir::tempPath()) + QLatin1Char('/') +
                      socketName);
        res = server->listen(socketName);
    }
#endif
    if (!res)
        qWarning("QtSingleCoreApplication: listen on local socket failed, %s",
                 qPrintable(server->errorString()));
    QObject::connect(server, SIGNAL(newConnection()),
                     SLOT(receiveConnection()));
    return false;
}

bool QtLocalPeer::sendMessage(const QByteArray &uMsg, int timeout) {
    if (!isClient())
        return false;

    QLocalSocket socket;
    bool connOk = false;
    for (int i = 0; i < 2; i++) {
        // Try twice, in case the other instance is just starting up
        socket.connectToServer(socketName);
        connOk = socket.waitForConnected(timeout / 2);
        if (connOk || i)
            break;
        int ms = 250;
#if defined(Q_OS_WIN)
        Sleep(DWORD(ms));
#else
        struct timespec ts = {ms / 1000, (ms % 1000) * 1000 * 1000};
        nanosleep(&ts, NULL);
#endif
    }
    if (!connOk)
        return false;

    QDataStream ds(&socket);
    ds.writeBytes(uMsg.constData(), uMsg.size());
    bool res = socket.waitForBytesWritten(timeout);
    if (res) {
        res &= socket.waitForReadyRead(timeout); // wait for ack
        if (res)
            res &= (socket.read(qstrlen(ack)) == ack);
    }
    return res;
}

void QtLocalPeer::receiveConnection() {
    QLocalSocket *socket = server->nextPendingConnection();
    if (!socket)
        return;

    while (true) {
        if (socket->state() == QLocalSocket::UnconnectedState) {
            qWarning("QtLocalPeer: Peer disconnected");
            delete socket;
            return;
        }
        if (socket->bytesAvailable() >= qint64(sizeof(quint32)))
            break;
        socket->waitForReadyRead();
    }

    QDataStream ds(socket);
    QByteArray uMsg;
    quint32 remaining;
    ds >> remaining;
    uMsg.resize(remaining);
    int got = 0;
    char *uMsgBuf = uMsg.data();
    do {
        got = ds.readRawData(uMsgBuf, remaining);
        remaining -= got;
        uMsgBuf += got;
    } while (remaining && got >= 0 && socket->waitForReadyRead(2000));
    if (got < 0) {
        qWarning("QtLocalPeer: Message reception failed %s",
                 socket->errorString().toLatin1().constData());
        delete socket;
        return;
    }
    socket->write(ack, qstrlen(ack));
    socket->waitForBytesWritten(1000);
    socket->waitForDisconnected(1000); // make sure client reads ack
    delete socket;
    Q_EMIT messageReceived(uMsg); // ### (might take a long time to return)
}
