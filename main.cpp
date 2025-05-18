#include "class/appmanager.h"
#include "class/settingmanager.h"

#include "define.h"

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

    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

#ifdef Q_OS_LINUX
    // fix wayland issue (a workaround): floating dock not work
    // reference:
    // https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System/
    //         issues/714#issuecomment-2802752677
    auto denv = qgetenv("XDG_SESSION_TYPE");
    if (denv.isEmpty() ||
        denv.compare(QByteArrayLiteral("wayland"), Qt::CaseInsensitive) == 0) {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }
#endif

    QApplication::setApplicationName(APP_NAME);
    QApplication::setOrganizationName(APP_ORG);
    QApplication::setApplicationVersion(WINGHEX_VERSION);

    try {
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
    } catch (CrashCode errCode) {
        return int(errCode);
    }
}
