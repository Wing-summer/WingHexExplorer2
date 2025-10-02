#include "class/appmanager.h"
#include "class/languagemanager.h"
#include "class/settingmanager.h"
#include "class/wingmessagebox.h"

#include "define.h"

#include <QDebug>
#include <QProcessEnvironment>
#include <QSettings>

void loadEnvConfig(int argc, char *argv[]) {
    QFileInfo info(argv[0]);
    QDir appDir(info.absoluteDir());

    if (!appDir.exists(QStringLiteral("config.ini"))) {
        return;
    }

    auto path = appDir.absoluteFilePath(QStringLiteral("config.ini"));
    QSettings set(path, QSettings::IniFormat);

    // General
    for (auto &kv : set.childKeys()) {
        qputenv(qPrintable(kv), set.value(kv).toByteArray());
    }

    auto groups = set.childGroups();
    auto evaluate = [](const QProcessEnvironment &env,
                       const QString &statement) {
        // Parse and evaluate statements:
        // $NAME               -> check existence
        // $NAME=VALUE         -> ignore-case full match
        // $NAME==VALUE        -> case-sensitive full match
        // $NAME:=VALUE        -> ignore-case contains
        // $NAME::=VALUE       -> case-sensitive contains
        // VALUE: unless pure digits, must be enclosed in "" or ''

        static const QRegularExpression re(
            R"(^\$([A-Za-z_][A-Za-z0-9_]*)(?:\s*(==|::=|:=|=)\s*(\d+|"[^"]*"|'[^']*'))?$)");
        auto match = re.match(statement);
        if (!match.hasMatch()) {
            qWarning("[main::loadEnvConfig] Invalid syntax: %s",
                     qUtf8Printable(statement));
            return false;
        }

        auto name = match.captured(1);
        auto op = match.captured(2);
        auto value = match.captured(3);

        // Existence check: no operator provided
        if (op.isEmpty()) {
            return env.contains(name);
        }

        if (!value.isEmpty() &&
            ((value.startsWith('"') && value.endsWith('"')) ||
             (value.startsWith('\'') && value.endsWith('\'')))) {
            value.removeFirst().removeLast();
        }

        auto var = env.value(name);

        // Evaluate based on operator
        if (op == QStringLiteral(":=") || op == QStringLiteral("::=")) {
            const QStringList items = var.split(QDir::listSeparator());
            for (const QString &item : items) {
                if (op == QStringLiteral(":=")) {
                    if (item.contains(value, Qt::CaseInsensitive)) {
                        return true;
                    }
                } else {
                    if (item.contains(value, Qt::CaseSensitive)) {
                        return true;
                    }
                }
            }
            return false;
        }

        if (op == "=") {
            return QString::compare(var, value, Qt::CaseInsensitive) == 0;
        } else if (op == "==") {
            return var == value;
        } else {
            qWarning("[main::loadEnvConfig] Unknown operator: %s",
                     qUtf8Printable(op));
        }

        return false;
    };

    auto env = QProcessEnvironment::systemEnvironment();

    constexpr auto syslen = std::char_traits<char>::length(WING_SYSTEM_NAME);
    if (syslen) {
        set.beginGroup(WING_SYSTEM_NAME);
        for (auto &kv : set.childKeys()) {
            qputenv(qPrintable(kv), set.value(kv).toByteArray());
        }
        set.endGroup();
    }

    for (auto &g : groups) {
        if (evaluate(env, g)) {
            set.beginGroup(g);
            for (auto &kv : set.childKeys()) {
                qputenv(qPrintable(kv), set.value(kv).toByteArray());
            }
            set.endGroup();
        }
    }
}

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

    loadEnvConfig(argc, argv);

    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QApplication::setApplicationName(QStringLiteral(APP_NAME));
    QApplication::setOrganizationName(QStringLiteral(APP_ORG));
    QApplication::setApplicationVersion(QStringLiteral(WINGHEX_VERSION));

    static_assert(CHAR_BIT == 8, APP_NAME "is only supported with 8-bit char");

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
    } catch (const std::bad_alloc &) {
        auto &lang = LanguageManager::instance();
        auto df = lang.defaultLocale();

        // this exception can only occur when your memory are too limit or
        // you are writing more than 2GB with QByteArray on 32-bit operating
        // system or with Qt5.
        if (QLocale::China == df.territory()) {
            WingMessageBox::critical(
                nullptr, QStringLiteral(APP_NAME),
                QStringLiteral("崩溃啦！发生内存溢出异常！"));
        } else {
            WingMessageBox::critical(
                nullptr, QStringLiteral(APP_NAME),
                QStringLiteral("WingHexExplorer2 is out of memory. Crashed!"));
        }
        return int(CrashCode::OutofMemory);
    }
}
