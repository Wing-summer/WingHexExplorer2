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

#include "resettabletimer.h"

#include <QDebug>

ResettableTimer::ResettableTimer(QObject *parent) : QObject(parent) {
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &ResettableTimer::onTimeout);
}

void ResettableTimer::start(int intervalMs) {
    if (intervalMs <= 0) {
        qWarning()
            << "ResettableTimer::start called with non-positive interval:"
            << intervalMs;
        return;
    }
    m_intervalMs = intervalMs;
    m_timer.start(m_intervalMs);
}

void ResettableTimer::reset(int newIntervalMs) {
    if (newIntervalMs > 0) {
        m_intervalMs = newIntervalMs;
    }

    if (m_intervalMs <= 0) {
        // nothing to start/restart
        qWarning() << "ResettableTimer::reset called but interval not set.";
        return;
    }

    // restart works even if timer was about to fire
    m_timer.start(m_intervalMs);
}

void ResettableTimer::stop() { m_timer.stop(); }

void ResettableTimer::triggerNow() {
    // Stop current timer (if running) then immediately call handler
    m_timer.stop();
    onTimeout();
}

bool ResettableTimer::isActive() const { return m_timer.isActive(); }

int ResettableTimer::interval() const { return m_intervalMs; }

void ResettableTimer::setInterval(int intervalMs) {
    if (intervalMs <= 0) {
        qWarning() << "ResettableTimer::setInterval non-positive:"
                   << intervalMs;
        return;
    }
    m_intervalMs = intervalMs;
    if (m_timer.isActive()) {
        // restart with new interval
        m_timer.start(m_intervalMs);
    }
}

void ResettableTimer::onTimeout() {
    // Timer single-shot -> it's stopped now.
    Q_EMIT timeoutTriggered();
}
