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

#include "QConsoleIODevice.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>

#include "QConsoleWidget.h"

QConsoleIODevice::QConsoleIODevice(QConsoleWidget *w, QObject *parent)
    : QIODevice(parent), m_widget(w), m_readpos(0), m_writtenSinceLastEmit(0),
      m_readSinceLastEmit(0) {
    setCurrentWriteChannel(QConsoleWidget::StandardOutput);

    open(QIODevice::ReadWrite | QIODevice::Unbuffered);
}

QConsoleIODevice::~QConsoleIODevice() {}

qint64 QConsoleIODevice::bytesAvailable() const {
    return (qint64)(m_readbuff.size() - m_readpos);
}

bool QConsoleIODevice::waitForReadyRead(int msecs) {
    if (!m_widget->device()->isOpen() ||
        m_widget->mode() != QConsoleWidget::Input) {
        return false;
    }

    if (bytesAvailable()) {
        return true;
    }

    QEventLoop loop;

    connect(this, SIGNAL(readyRead()), &loop, SLOT(quit()));
    connect(this, SIGNAL(aboutToClose()), &loop, SLOT(quit()));
    if (msecs > 0) {
        QTimer::singleShot(msecs, &loop, SLOT(quit()));
    }

    m_readyReadEmmited = false;
    loop.exec();
    return m_readyReadEmmited;
}

QConsoleWidget *QConsoleIODevice::widget() const { return m_widget; }

qint64 QConsoleIODevice::readData(char *data, qint64 len) {
    int b = bytesAvailable();
    if (b) {
        b = qMin(len, b);
        memcpy(data, m_readbuff.constData() + m_readpos, b);
        m_readpos += b;
    }
    return (qint64)b;
}

qint64 QConsoleIODevice::writeData(const char *data, qint64 len) {
    m_widget->write(QString::fromLatin1(data, len));
    m_writtenSinceLastEmit += len;
    if (!signalsBlocked()) {
        Q_EMIT bytesWritten(m_writtenSinceLastEmit);
        m_writtenSinceLastEmit = 0;
    }
    return len;
}

void QConsoleIODevice::consoleWidgetInput(const QString &in) {
    QByteArray ba = in.toLatin1();
    int sz = ba.size();
    if (bytesAvailable()) {
        if (m_readpos) {
            m_readbuff = m_readbuff.mid(m_readpos);
        }
        m_readbuff.append(ba);
    } else {
        m_readbuff = ba;
    }
    m_readpos = 0;

    m_readSinceLastEmit += sz;
    if (!signalsBlocked()) {
        Q_EMIT readyRead();
        m_readyReadEmmited = true;
        m_readSinceLastEmit = 0;
    }
}
