// Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
// SPDX-License-Identifier: BSD-3-Clause

#include "qtsingleapplication.h"
#include "qtlocalpeer.h"
#include <QWidget>

#include <QCryptographicHash>

#ifdef Q_OS_UNIX
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <lmcons.h>
#ifdef PASCAL
#undef PASCAL
#endif
#include <windows.h>
#endif

/*!
    \class QtSingleApplication qtsingleapplication.h
    \brief The QtSingleApplication class provides an API to detect and
    communicate with running instances of an application.

    This class allows you to create applications where only one
    instance should be running at a time. I.e., if the user tries to
    launch another instance, the already running instance will be
    activated instead. Another usecase is a client-server system,
    where the first started instance will assume the role of server,
    and the later instances will act as clients of that server.

    By default, the full path of the executable file is used to
    determine whether two processes are instances of the same
    application. You can also provide an explicit identifier string
    that will be compared instead.

    The application should create the QtSingleApplication object early
    in the startup phase, and call isRunning() to find out if another
    instance of this application is already running. If isRunning()
    returns false, it means that no other instance is running, and
    this instance has assumed the role as the running instance. In
    this case, the application should continue with the initialization
    of the application user interface before entering the event loop
    with exec(), as normal.

    The messageReceived() signal will be emitted when the running
    application receives messages from another instance of the same
    application. When a message is received it might be helpful to the
    user to raise the application so that it becomes visible. To
    facilitate this, QtSingleApplication provides the
    setActivationWindow() function and the activateWindow() slot.

    If isRunning() returns true, another instance is already
    running. It may be alerted to the fact that another instance has
    started by using the sendMessage() function. Also data such as
    startup parameters (e.g. the name of the file the user wanted this
    new instance to open) can be passed to the running instance with
    this function. Then, the application should terminate (or enter
    client mode).

    If isRunning() returns true, but sendMessage() fails, that is an
    indication that the running instance is frozen.

    Here's an example that shows how to convert an existing
    application to use QtSingleApplication. It is very simple and does
    not make use of all QtSingleApplication's functionality (see the
    examples for that).

    \code
    // Original
    int main(int argc, char **argv)
    {
        QApplication app(argc, argv);

        MyMainWidget mmw;
        mmw.show();
        return app.exec();
    }

    // Single instance
    int main(int argc, char **argv)
    {
        QtSingleApplication app(argc, argv);

        if (app.isRunning())
            return !app.sendMessage(someDataString);

        MyMainWidget mmw;
        app.setActivationWindow(&mmw);
        mmw.show();
        return app.exec();
    }
    \endcode

    Once this QtSingleApplication instance is destroyed (normally when
    the process exits or crashes), when the user next attempts to run the
    application this instance will not, of course, be encountered. The
    next instance to call isRunning() or sendMessage() will assume the
    role as the new running instance.

    For console (non-GUI) applications, QtSingleCoreApplication may be
    used instead of this class, to avoid the dependency on the QtGui
    library.

    \sa QtSingleCoreApplication
*/

void QtSingleApplication::sysInit(const QString &appId) {
    actWin = nullptr;

    peer = new QtLocalPeer(this, appId);
    connect(peer, &QtLocalPeer::messageReceived, this,
            &QtSingleApplication::messageReceived);
}

/*!
    Creates a QtSingleApplication object.
*/

QtSingleApplication::QtSingleApplication(int &argc, char **argv)
    : QApplication(argc, argv) {
    sysInit(genBlockServerName());
}

/*!
    Returns true if another instance of this application is running;
    otherwise false.

    This function does not find instances of this application that are
    being run by a different user (on Windows: that are running in
    another session).

    \sa sendMessage()
*/

bool QtSingleApplication::isRunning() { return peer->isClient(); }

/*!
    Tries to send the text \a message to the currently running
    instance. The QtSingleApplication object in the running instance
    will emit the messageReceived() signal when it receives the
    message.

    This function returns true if the message has been sent to, and
    processed by, the current instance. If there is no instance
    currently running, or if the running instance fails to process the
    message within \a timeout milliseconds, this function return false.

    \sa isRunning(), messageReceived()
*/
bool QtSingleApplication::sendMessage(const QByteArray &message, int timeout) {
    return peer->sendMessage(message, timeout);
}

/*!
    Returns the application identifier. Two processes with the same
    identifier will be regarded as instances of the same application.
*/
QString QtSingleApplication::id() const { return peer->applicationId(); }

/*!
  Sets the activation window of this application to \a aw. The
  activation window is the widget that will be activated by
  activateWindow(). This is typically the application's main window.

  If \a activateOnMessage is true (the default), the window will be
  activated automatically every time a message is received, just prior
  to the messageReceived() signal being emitted.

  \sa activateWindow(), messageReceived()
*/

void QtSingleApplication::setActivationWindow(QWidget *aw,
                                              bool activateOnMessage) {
    actWin = aw;
    if (activateOnMessage)
        connect(peer, &QtLocalPeer::messageReceived, this,
                &QtSingleApplication::activateWindow);
    else
        disconnect(peer, &QtLocalPeer::messageReceived, this,
                   &QtSingleApplication::activateWindow);
}

/*!
    Returns the applications activation window if one has been set by
    calling setActivationWindow(), otherwise returns 0.

    \sa setActivationWindow()
*/
QWidget *QtSingleApplication::activationWindow() const { return actWin; }

/*!
  De-minimizes, raises, and activates this application's activation window.
  This function does nothing if no activation window has been set.

  This is a convenience function to show the user that this
  application instance has been activated when he has tried to start
  another instance.

  This function should typically be called in response to the
  messageReceived() signal. By default, that will happen
  automatically, if an activation window has been set.

  \sa setActivationWindow(), messageReceived(), initialize()
*/
void QtSingleApplication::activateWindow() {
    if (actWin) {
        actWin->setWindowState(actWin->windowState() & ~Qt::WindowMinimized);
        actWin->raise();
        actWin->activateWindow();
    }
}

QString QtSingleApplication::genBlockServerName() const {
#ifdef Q_OS_MACOS
    // Maximum key size on macOS is PSHMNAMLEN (31).
    QCryptographicHash appData(QCryptographicHash::Md5);
#else
    QCryptographicHash appData(QCryptographicHash::Sha256);
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
    appData.addData("SingleApplication", 17);
#else
    appData.addData(QByteArrayView{"SingleApplication"});
#endif
    appData.addData(applicationName().toUtf8());
    appData.addData(organizationName().toUtf8());
    appData.addData(organizationDomain().toUtf8());

    // User level block requires a user specific data in the hash
    appData.addData(getUsername().toUtf8());

    // Replace the backslash in RFC 2045 Base64 [a-zA-Z0-9+/=] to comply with
    // server naming requirements.
    return QString::fromUtf8(appData.result().toBase64().replace("/", "_"));
}

/*!
    \fn void QtSingleApplication::messageReceived(const QString& message)

    This signal is emitted when the current instance receives a \a
    message from another instance of this application.

    \sa sendMessage(), setActivationWindow(), activateWindow()
*/

/*!
    \fn void QtSingleApplication::initialize(bool dummy = true)

    \obsolete
*/
QString QtSingleApplication::getUsername() const {
#ifdef Q_OS_WIN
    wchar_t username[UNLEN + 1];
    // Specifies size of the buffer on input
    DWORD usernameLength = UNLEN + 1;
    if (GetUserNameW(username, &usernameLength))
        return QString::fromWCharArray(username);
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    return QString::fromLocal8Bit(qgetenv("USERNAME"));
#else
    return qEnvironmentVariable("USERNAME");
#endif
#endif
#ifdef Q_OS_UNIX
    QString username;
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (pw)
        username = QString::fromLocal8Bit(pw->pw_name);
    if (username.isEmpty()) {
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
        username = QString::fromLocal8Bit(qgetenv("USER"));
#else
        username = qEnvironmentVariable("USER");
#endif
    }
    return username;
#endif
}
