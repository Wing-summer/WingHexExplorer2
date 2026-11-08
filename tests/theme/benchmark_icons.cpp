#include <algorithm>
#include <cstdio>

#include <QApplication>
#include <QElapsedTimer>
#include <QPixmapCache>

#include "class/settings/thememanager.h"

int main(int argc, char **argv) {
    QApplication application(argc, argv);
    auto &theme = ThemeManager::instance();
    theme.setTheme(ThemeManager::Theme::Light);
    QList<QIcon> icons;
    for (const auto id : theme.standardIcons()) {
        auto icon = theme.standardIcon(id);
        if (!icon.isNull()) {
            icons.append(icon);
        }
    }
    QList<qint64> samples;
    QPixmapCache::clear();
    for (int run = 0; run < 12; ++run) {
        QElapsedTimer timer;
        timer.start();
        for (int repeat = 0; repeat < 10; ++repeat) {
            for (const auto &icon : icons) {
                for (const int size : {16, 24, 32}) {
                    if (icon.pixmap(size, size).isNull()) {
                        return 1;
                    }
                }
            }
        }
        if (run >= 1) {
            samples.append(timer.nsecsElapsed());
        }
    }
    std::sort(samples.begin(), samples.end());
    std::printf("icons=%lld repeated_render_median_ms=%.3f\n",
                static_cast<long long>(icons.size()),
                samples[samples.size() / 2] / 1e6);
}
