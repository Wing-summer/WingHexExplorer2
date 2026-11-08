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

#include "startuptrace.h"

#include <QEvent>
#include <QTimer>
#include <QWidget>

namespace {
bool tracing = false;

QElapsedTimer &processTimer() {
    static QElapsedTimer timer;
    return timer;
}

class FirstPaintObserver : public QObject {
public:
    explicit FirstPaintObserver(QWidget *window) : QObject(window) {
        window->installEventFilter(this);
    }

protected:
    bool eventFilter(QObject *object, QEvent *event) override {
        if (event->type() == QEvent::Paint && !m_scheduled) {
            m_scheduled = true;
            object->removeEventFilter(this);
            // Run after the complete first paint, including child widgets.
            QTimer::singleShot(0, this, [this]() {
                StartupTrace::mark("process.first_paint_ready");
                if (qEnvironmentVariableIntValue(
                        "WINGHEX_STARTUP_EXIT_AFTER_READY") == 1) {
                    static_cast<QWidget *>(parent())->close();
                }
                deleteLater();
            });
        }
        return false;
    }

private:
    bool m_scheduled = false;
};
} // namespace

void StartupTrace::start() {
    tracing = !qEnvironmentVariableIsSet("WINGHEX_STARTUP_TRACE") ||
              qEnvironmentVariableIntValue("WINGHEX_STARTUP_TRACE") == 1;
    if (tracing) {
        processTimer().start();
    }
}

bool StartupTrace::enabled() { return tracing; }

void StartupTrace::mark(const char *phase, qint64 durationNs) {
    if (!tracing) {
        return;
    }
    // Bypass the GUI log sink: profiling must not append text to startup docks.
    std::fprintf(stderr,
                 "[startup] {\"phase\":\"%s\",\"at_ms\":%.3f,"
                 "\"duration_ms\":%.3f}\n",
                 phase, processTimer().nsecsElapsed() / 1e6,
                 durationNs < 0 ? 0.0 : durationNs / 1e6);
    std::fflush(stderr);
}

void StartupTrace::watchWindow(QWidget *window) {
    if (tracing) {
        new FirstPaintObserver(window);
    }
}

StartupTrace::PhaseTimer::PhaseTimer() {
    if (tracing) {
        m_timer.start();
    }
}

void StartupTrace::PhaseTimer::checkpoint(const char *phase) {
    if (tracing) {
        mark(phase, m_timer.nsecsElapsed());
        m_timer.restart();
    }
}
