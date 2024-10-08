#include "class/appmanager.h"

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
    w->show();
    Utilities::moveToCenter(w);

    return a.exec();
}
