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

#include "scriptmachine.h"

LuauScheduler::LuauScheduler(ScriptMachine &machine, LuauThread *th,
                             QObject *parent)
    : QObject(parent), m_machine(machine), m_th(th) {
    Q_ASSERT(th);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &LuauScheduler::step);
}

LuauScheduler::~LuauScheduler() {}

void LuauScheduler::start(int arguments) {
    Q_ASSERT(!m_th->isRunning);
    m_executeError.clear();
    m_arguments = arguments;
    if (!lua_isfunction(m_th->state, -1 - arguments)) {
        m_executeError =
            QStringLiteral("thread stack does not contain a function");
        finish(LUA_ERRRUN);
        return;
    }
    m_th->isRunning = true;
    m_timer.start(0);
}

void LuauScheduler::stop() {
    m_timer.stop();
    m_th->isRunning = false;
}

void LuauScheduler::step() {
    if (m_machine.isExecuting()) {
        m_timer.start(10);
        return;
    }
    int status = LUA_OK;
    try {
        auto L = m_th->state;
        if (!m_th->requestStop) {
            status = m_machine.resumeThread(m_th, m_arguments);
            m_arguments = 0;
            if (status == LUA_YIELD) {
                m_timer.start(0);
                return;
            }
            if (m_th->requestStop) {
                status = LUA_OK;
            }
            if (status != LUA_OK) {
                size_t len;
                const char *str = lua_tolstring(L, -1, &len);
                if (str) {
                    m_executeError = QString::fromUtf8(str, len);
                } else {
                    m_executeError = QStringLiteral("Unknown error");
                }
                lua_pop(L, 1);
            }
        }
    } catch (const std::exception &ex) {
        m_executeError = QString::fromUtf8(ex.what());
        status = LUA_ERRRUN;
    }
    finish(status);
}

void LuauScheduler::finish(int status) {
    m_th->isRunning = false;
    // Completion handlers may destroy the owner and this scheduler.
    // Do not access members after emitting.
    Q_EMIT sigOnFinished(status);
}

QString LuauScheduler::executeError() const { return m_executeError; }
