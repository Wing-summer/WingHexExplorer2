#include "class/appmanager.h"
#include "class/settingmanager.h"

int main(int argc, char *argv[]) {
    /* 有关对在 QT5 的 Win 平台禁用高 dpi 支持
     * 的原因说明：
     * - 因为启用的话会导致样式表对组件边界绘制出现失效
     * - 但 QT6 支持的好，而 Linux 不会有问题
     */

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) &&                                \
     QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#ifndef Q_OS_WIN
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QGuiApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    AppManager a(argc, argv);
    auto w = a.mainWindow();

    auto &set = SettingManager::instance();
    switch (set.defaultWinState()) {
    case Qt::WindowNoState:
        w->show();
        Utilities::moveToCenter(w);
        break;
    case Qt::WindowMinimized:
        w->showMinimized();
        break;
    case Qt::WindowActive:
    case Qt::WindowMaximized:
        w->showMaximized();
        break;
    case Qt::WindowFullScreen:
        w->showFullScreen();
        break;
    }

    return a.exec();
}
