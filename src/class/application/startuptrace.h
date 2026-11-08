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

#ifndef STARTUPTRACE_H
#define STARTUPTRACE_H

#include <QtGlobal>

#ifdef WINGHEX_ENABLE_STARTUP_PROFILE
#include <QElapsedTimer>
#endif

class QWidget;

// Opt-in diagnostics; normal startup never schedules profiling callbacks.
namespace StartupTrace {
#ifdef WINGHEX_ENABLE_STARTUP_PROFILE
void start();
bool enabled();
void mark(const char *phase, qint64 durationNs = -1);
void watchWindow(QWidget *window);

class PhaseTimer {
public:
    PhaseTimer();
    void checkpoint(const char *phase);

private:
    QElapsedTimer m_timer;
};
#else
inline void start() {}

inline bool enabled() { return false; }

inline void mark(const char *, qint64 = -1) {}

inline void watchWindow(QWidget *) {}

class PhaseTimer {
public:
    void checkpoint(const char *) {}
};
#endif
} // namespace StartupTrace

#endif // STARTUPTRACE_H
