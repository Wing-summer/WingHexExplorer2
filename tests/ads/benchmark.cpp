#include "DockAreaWidget.h"
#include "DockManager.h"
#include "DockWidget.h"
#ifndef ADS_LEGACY_STATE
#include "DockingState.h"
#endif
#include <QApplication>
#include <QElapsedTimer>
#include <QLabel>
#include <QMainWindow>
#include <algorithm>
#include <cstdio>

static void drain() {
    QApplication::processEvents();
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QApplication::processEvents();
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    ads::CDockManager::setAutoHideConfigFlags(
        ads::CDockManager::DefaultAutoHideConfig);
    for (const int count : {10, 50, 200}) {
        for (const bool mixed : {false, true}) {
            QMainWindow window;
            window.resize(1280, 900);
            auto manager = new ads::CDockManager(&window);
            ads::CDockAreaWidget *area = nullptr;
            for (int i = 0; i < count; ++i) {
                auto dock = new ads::CDockWidget(
                    manager, QStringLiteral("Dock %1").arg(i));
                dock->setWidget(
                    new QLabel(QStringLiteral("Content %1").arg(i)));
                if (mixed && i % 19 == 18) {
                    manager->addDockWidgetFloating(dock);
                } else if (mixed && i % 17 == 16) {
                    manager->addAutoHideDockWidget(ads::SideBarLeft, dock);
                } else if (!area || i % 8 == 0) {
                    area = manager->addDockWidget(
                        i % 16 == 0 ? ads::RightDockWidgetArea
                                    : ads::BottomDockWidgetArea,
                        dock, area);
                } else {
                    manager->addDockWidget(ads::CenterDockWidgetArea, dock,
                                           area);
                }
                if (i % 7 == 6) {
                    dock->toggleView(false);
                }
            }
            window.show();
            drain();
            const auto state = manager->saveState();
            QList<qint64> times, applyTimes, decodeTimes;
            QElapsedTimer applyTimer;
            qint64 applied = 0;
            QObject::connect(manager, &ads::CDockManager::restoringState,
                             manager, [&] { applyTimer.start(); });
            QObject::connect(manager, &ads::CDockManager::stateRestored,
                             manager,
                             [&] { applied = applyTimer.nsecsElapsed(); });
            for (int i = 0; i < 14; ++i) {
                QElapsedTimer timer;
                timer.start();
                if (!manager->restoreState(state)) {
                    return 1;
                }
                drain();
                if (i >= 3) {
                    times.append(timer.nsecsElapsed());
                    applyTimes.append(applied);
#ifndef ADS_LEGACY_STATE
                    ads::state::Layout decoded;
                    timer.restart();
                    if (!ads::state::decode(state, 0, decoded)) {
                        return 2;
                    }
                    decodeTimes.append(timer.nsecsElapsed());
#endif
                }
            }
            std::sort(times.begin(), times.end());
            std::sort(applyTimes.begin(), applyTimes.end());
            std::sort(decodeTimes.begin(), decodeTimes.end());
            std::printf("count=%d mixed=%d bytes=%lld total_median_ms=%.3f "
                        "restore_signal_span_ms=%.3f",
                        count, mixed, static_cast<long long>(state.size()),
                        times[times.size() / 2] / 1e6,
                        applyTimes[applyTimes.size() / 2] / 1e6);
#ifndef ADS_LEGACY_STATE
            std::printf(" decode_median_ms=%.3f",
                        decodeTimes[decodeTimes.size() / 2] / 1e6);
#endif
            std::printf("\n");
            std::fflush(stdout);
        }
    }
}
