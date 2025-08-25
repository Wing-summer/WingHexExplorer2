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

#ifndef QCONSOLEIODEVICE_H
#define QCONSOLEIODEVICE_H

#include <QByteArray>
#include <QIODevice>

class QConsoleWidget;

class QConsoleIODevice : public QIODevice {
    Q_OBJECT

public:
    explicit QConsoleIODevice(QConsoleWidget *w, QObject *parent = nullptr);
    ~QConsoleIODevice();
    qint64 bytesAvailable() const override;
    bool waitForReadyRead(int msecs) override;
    QConsoleWidget *widget() const;

    void consoleWidgetInput(const QString &in);

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    friend class QConsoleWidget;
    QConsoleWidget *widget_;
    QByteArray readbuff_;
    int readpos_;
    qint64 writtenSinceLastEmit_, readSinceLastEmit_;
    bool readyReadEmmited_;
};

#endif
