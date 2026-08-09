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

#include "luauscheduler.h"

#include <QTimer>

LuauScheduler::LuauScheduler(LuauThread *th) : QObject(), m_th(th) {
    Q_ASSERT(th);
}

LuauScheduler::~LuauScheduler() {}

void LuauScheduler::start() {
    if (!lua_isfunction(m_th->state, -1)) {
        return;
    }
    m_th->isRunning = true;
    QTimer::singleShot(0, this, &LuauScheduler::step);
}

void LuauScheduler::requestStop() { m_th->requestStop = true; }

void LuauScheduler::step() {
    try {
        int status = lua_resume(m_th->state, nullptr, 0);
        if (status == LUA_YIELD) {
            QTimer::singleShot(0, this, &LuauScheduler::step);
        } else {
            m_th->isRunning = false;
            Q_EMIT finished(status);
            deleteLater();
        }
    } catch (const std::exception &ex) {
        m_executeError = QString::fromUtf8(ex.what());
        m_th->isRunning = false;
        Q_EMIT finished(LUA_ERRRUN);
        deleteLater();
    }
}

QString LuauScheduler::executeError() const { return m_executeError; }
