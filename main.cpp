#include "SingleApplication/singleapplication.h"
#include "define.h"
#include "src/class/languagemanager.h"
#include "src/class/settingmanager.h"
#include "src/class/skinmanager.h"
#include "src/dialog/mainwindow.h"
#include "src/utilities.h"

#include <QApplication>
#include <QByteArray>
#include <QDataStream>

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
    QGuiApplication::setAttribute(Qt::AA_DontUseNativeDialogs);

    SingleApplication a(argc, argv);
    a.setApplicationName(APP_NAME);
    a.setOrganizationName(APP_ORG);
    a.setApplicationVersion(WINGHEX_VERSION);

    auto args = a.arguments();
    if (a.isSecondary()) {
        QByteArray buffer;
        QDataStream out(&buffer, QIODevice::WriteOnly);
        if (args.size() > 1) {
            for (auto var = args.begin() + 1; var != args.end(); ++var) {
                out << *var;
            }
        }
        a.sendMessage(buffer);
    }

    auto &set = SettingManager::instance();
    QFont font(set.appFontFamily(), set.appfontSize());
    qApp->setFont(font);

    QObject::connect(&set, &SettingManager::sigAppFontFamilyChanged, &a,
                     [](const QString &fontfam) {
                         QFont font(fontfam);
                         qApp->setFont(font);
                     });
    QObject::connect(&set, &SettingManager::sigAppfontSizeChanged, &a,
                     [](int v) {
                         auto font = qApp->font();
                         font.setPointSize(v);
                         qApp->setFont(font);
                     });

    SkinManager::instance();

    LanguageManager::instance();

    MainWindow w;

    QObject::connect(&a, &SingleApplication::instanceStarted, &a, [&w] {
        if (!w.isEnabled()) {
            return;
        }
        w.show();
        w.raise();
        w.activateWindow();
    });

    QObject::connect(&a, &SingleApplication::receivedMessage, &a,
                     [&w](quint32 instanceId, QByteArray message) {
                         Q_UNUSED(instanceId);
                         QDataStream out(&message, QIODevice::WriteOnly);
                         while (!out.atEnd()) {
                             QString param;
                             out >> param;
                             if (QFile::exists(param)) {
                                 w.openFile(param, nullptr);
                             }
                         }
                     });

    if (args.size() > 1) {
        for (auto var = args.begin() + 1; var != args.end(); ++var) {
            // TODO more functions support
            w.openFile(*var, nullptr);
        }
    }

    w.show();
    Utilities::moveToCenter(&w);

    return a.exec();
}
