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

#include "standalonemodes.h"

#include <QApplication>
#include <QPointer>
#include <QScopeGuard>

#include "class/application/scriptenvironment.h"
#include "class/plugin/pluginsystem.h"
#include "class/settings/settingmanager.h"
#include "class/settings/thememanager.h"
#include "define.h"
#include "dialog/common/wingmessagebox.h"
#include "dialog/diffdialog.h"
#include "dialog/scriptingdialog.h"
#include "dialog/settingdialog.h"
#include "settings/diffsettingdialog.h"
#include "uiservices.h"

int runStandaloneMode(const StartupOptions &options) {
    const bool scripting = options.mode() == StartupMode::Script;
    initializeUiServices(scripting);
    if (!scripting) {
        QPointer<DiffDialog> window = new DiffDialog;
        auto release = qScopeGuard([&] { delete window.data(); });
        QObject::connect(
            window, &DiffDialog::sigOnSettingsRequested, window,
            [window, settings = QPointer<SettingDialog>{}]() mutable {
                if (!settings) {
                    settings = new SettingDialog(
                        ThemeManager::instance().standardIcon(
                            ThemeManager::StandardIcon::Settings),
                        DiffDialog::tr("Settings"), window);
                    settings->addPage(new DiffSettingDialog(settings));
                    settings->build();
                }
                settings->showConfig(QStringLiteral("Diff"));
            });
        for (int index = 0; index < options.files().size(); ++index) {
            window->openFile(options.files()[index],
                             index == 1 ? DiffDialog::Side::Right
                                        : DiffDialog::Side::Left);
        }
        window->show();
        return qApp->exec();
    }
    if (!SettingManager::instance().scriptEnabled()) {
        WingMessageBox::critical(
            nullptr, qAppName(),
            ScriptingDialog::tr("Scripting is disabled in settings."));
        return int(CrashCode::ScriptDisabled);
    }
    ScriptEnvironment environment;
    environment.loadPlugins();
    if (!ScriptEnvironment::initializeMachine() ||
        !ScriptEnvironment::finishRegistration()) {
        WingMessageBox::critical(
            nullptr, qAppName(),
            ScriptingDialog::tr("The script engine failed to start."));
        return int(CrashCode::ScriptInitFailed);
    }
    QPointer<SettingDialog> settings = createScriptSettingsDialog(nullptr);
    QPointer<ScriptingDialog> window;
    auto release = qScopeGuard([&] {
        delete window.data();
        delete settings.data();
    });
    window = new ScriptingDialog(settings,
                                 ScriptingDialog::CloseBehavior::CloseEditors);
    window->initConsole();
    QStringList failed;
    for (const auto &path : options.files()) {
        if (!window->openFile(path)) {
            failed.append(path);
        }
    }
    window->show();
    PluginSystem::instance().dispatchAppReadyEvent();
    if (!failed.isEmpty()) {
        WingMessageBox::critical(
            window, ScriptingDialog::tr("Error"),
            ScriptingDialog::tr("Could not open the following scripts:\n%1")
                .arg(failed.join(QLatin1Char('\n'))));
    }
    return qApp->exec();
}
