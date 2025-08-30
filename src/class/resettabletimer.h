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

#ifndef RESETTABLETIMER_H
#define RESETTABLETIMER_H

#include <QObject>

#include <QObject>
#include <QTimer>

class ResettableTimer : public QObject {
    Q_OBJECT
public:
    explicit ResettableTimer(QObject *parent = nullptr);

public slots:
    // Start with specified interval (ms). Replaces saved interval.
    void start(int intervalMs);

    // Reset (restart) with previously set interval.
    // If newIntervalMs >= 0, update interval and restart with it.
    void reset(int newIntervalMs = -1);

    // Stop the timer (no callback until start/reset again).
    void stop();

    // Force immediate trigger (calls callback / emits signal).
    void triggerNow();

    void setInterval(int intervalMs);

public:
    bool isActive() const;
    int interval() const;

signals:
    // Emitted when timeout happens.
    void timeoutTriggered();

private slots:
    void onTimeout();

private:
    QTimer m_timer;
    int m_intervalMs = 0; // last used interval in ms
};

#endif // RESETTABLETIMER_H
