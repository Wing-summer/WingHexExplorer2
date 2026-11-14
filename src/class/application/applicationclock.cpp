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

#include "applicationclock.h"

ApplicationClock::ApplicationClock() { m_timer.start(); }

const ApplicationClock &ApplicationClock::instance() {
    static const ApplicationClock clock;
    return clock;
}

void ApplicationClock::start() { instance(); }

quint64 ApplicationClock::elapsed() {
    return quint64(instance().m_timer.elapsed());
}
