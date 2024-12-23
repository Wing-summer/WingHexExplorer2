// Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
// SPDX-License-Identifier: BSD-3-Clause

#ifndef QTSINGLEAPPLICATION_H
#define QTSINGLEAPPLICATION_H

#include <QApplication>

class QtLocalPeer;

class QtSingleApplication : public QApplication {
    Q_OBJECT

public:
    explicit QtSingleApplication(int &argc, char **argv,
                                 bool GUIenabled = true);
    explicit QtSingleApplication(const QString &id, int &argc, char **argv);

public:
    bool isRunning();
    QString id() const;

    void setActivationWindow(QWidget *aw, bool activateOnMessage = true);
    QWidget *activationWindow() const;

    // Obsolete:
    void initialize(bool dummy = true) {
        isRunning();
        Q_UNUSED(dummy)
    }

public Q_SLOTS:
    bool sendMessage(const QByteArray &message, int timeout = 5000);
    void activateWindow();

Q_SIGNALS:
    void messageReceived(const QByteArray &message);

private:
    QString genBlockServerName() const;
    QString getUsername() const;

private:
    void sysInit(const QString &appId);
    QtLocalPeer *peer;
    QWidget *actWin;
};

#endif // QTSINGLEAPPLICATION_H
