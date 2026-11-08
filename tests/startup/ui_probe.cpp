// SPDX-License-Identifier: AGPL-3.0-or-later
#include <cstdio>
#include <functional>

#include <QApplication>
#include <QDialog>
#include <QListWidget>
#include <QSplitter>
#include <QTimer>

#include "class/plugin/pluginsystem.h"
#include "class/settings/settingmanager.h"
#include "class/settings/thememanager.h"
#include "dialog/mainwindow.h"
#include "dialog/settingdialog.h"

// Reuse the existing host-test friend to exercise the production registration
// function during startup, without loading a shared plugin into the test.
class PluginDocumentHostProbe {
public:
    static void registerPages(WingHex::IWingPlugin *plugin) {
        PluginSystem::instance().registeredSettingPages(plugin);
    }
};

class FixturePage final : public WingHex::SettingPage {
public:
    explicit FixturePage(const QString &id) : m_id(id) {}

    QIcon categoryIcon() const override { return {}; }

    QString name() const override { return m_id; }

    QString id() const override { return m_id; }

    bool showInRibbon() const override { return true; }

    void restore() override {}

private:
    QString m_id;
};

class FixturePlugin final : public WingHex::IWingPlugin {
public:
    QList<WingHex::SettingPage *> pages;

    bool init(const std::unique_ptr<QSettings> &) override { return true; }

    void unload(std::unique_ptr<QSettings> &) override {}

    const QString pluginName() const override {
        return QStringLiteral("fixture");
    }

    const QString pluginComment() const override { return {}; }

    QList<WingHex::SettingPage *> registeredSettingPages() const override {
        return pages;
    }
};

int main(int argc, char **argv) {
    QApplication application(argc, argv);
    application.setOrganizationName(QStringLiteral("WingCloudStudio"));
    application.setApplicationName(QStringLiteral("WingHexExplorer2"));
    auto &settings = SettingManager::instance();
    settings.setEnablePlugin(true);
    settings.setEnabledExtPlugins({});
    settings.setEnabledDevPlugins({});
    settings.setScriptEnabled(false);
    settings.setCheckUpdate(false);
    auto &theme = ThemeManager::instance();
    theme.setStyleSheetDeferred(true);
    theme.setTheme(ThemeManager::Theme(settings.themeID()));
    FixturePlugin plugin;
    plugin.pages = {new FixturePage(QStringLiteral("Fixture A")),
                    new FixturePage(QStringLiteral("Fixture B"))};
    bool registered = false;
    std::function<void()> registerPages;
    registerPages = [&] {
        if (!PluginSystem::instance().mainWindow()) {
            QTimer::singleShot(0, &application, registerPages);
            return;
        }
        PluginDocumentHostProbe::registerPages(&plugin);
        registered = true;
    };
    QTimer::singleShot(0, &application, registerPages);
    auto splash = new SplashDialog;
    MainWindow window(splash);
    splash->finish();
    if (!registered) {
        return 3;
    }
    theme.setStyleSheetDeferred(false);
    if (!window.findChildren<SettingDialog *>().isEmpty()) {
        return 1;
    }
    window.show();
    bool passed = true;
    SettingDialog *first = nullptr;
    // Exercise the real modal entry points, including page selection and
    // reuse. Close from the nested event loop without modifying settings.
    for (const auto &entry :
         {qMakePair("on_settingGeneral", 0), qMakePair("on_settingPlugin", 2),
          qMakePair("Fixture B", 5)}) {
        QTimer::singleShot(0, &window, [&] {
            const auto dialogs = window.findChildren<SettingDialog *>();
            auto modal =
                qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (dialogs.size() != 1 || !modal) {
                passed = false;
                if (modal) {
                    modal->reject();
                }
                return;
            }
            const auto dialog = dialogs.first();
            auto splitter = dialog->findChild<QSplitter *>(
                QStringLiteral("splitter"), Qt::FindDirectChildrenOnly);
            auto list = splitter
                            ? splitter->widget(0)->findChild<QListWidget *>(
                                  QStringLiteral("listWidget"),
                                  Qt::FindDirectChildrenOnly)
                            : nullptr;
            std::fprintf(stderr, "entry=%s pages=%d current=%d expected=%d\n",
                         entry.first, list ? list->count() : -1,
                         list ? list->currentRow() : -1, entry.second);
            passed &= list && list->count() == 7 &&
                      list->currentRow() == entry.second;
            passed &= !first || first == dialog;
            first = dialog;
            modal->close();
        });
        if (entry.second == 5) {
            QToolButton *button = nullptr;
            for (auto candidate : window.findChildren<QToolButton *>()) {
                if (candidate->text() == QString::fromLatin1(entry.first)) {
                    button = candidate;
                    break;
                }
            }
            if (!button) {
                return 4;
            }
            button->click();
        } else {
            const auto invoked = QMetaObject::invokeMethod(
                &window, entry.first, Qt::DirectConnection);
            passed = passed && invoked;
        }
    }
    if (!passed) {
        return 2;
    }
    std::puts("PASS: lazy host settings, plugin registration/Ribbon order and "
              "repeated entry");
    return 0;
}
