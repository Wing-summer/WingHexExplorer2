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

#include "uiservices.h"

#include <QApplication>
#include <QFont>

#include "WingCodeEdit/wingcodeedit.h"
#include <KSyntaxHighlighting/Repository>

#include "class/application/logger.h"
#include "class/settings/languagemanager.h"
#include "class/settings/settingmanager.h"
#include "class/settings/thememanager.h"
#include "dialog/settingdialog.h"
#include "settings/lspsettingdialog.h"
#include "settings/qeditconfig.h"

void initializeUiServices(bool scripting) {
    LanguageManager::instance();
    auto &settings = SettingManager::instance();
    Logger::instance().setLogLevel(Logger::Level(settings.logLevel()));
    qApp->setFont(QFont(settings.appFontFamily(), settings.appfontSize()));
    auto &theme = ThemeManager::instance();
    theme.setTheme(ThemeManager::Theme(settings.themeID()));
    if (scripting) {
        WingCodeEdit::syntaxRepo().addCustomSearchPath(
            QStringLiteral(":/WingScript/Luau"));
    }
}

SettingDialog *createScriptSettingsDialog(QWidget *parent) {
    auto &set = SettingManager::instance();
    // script settings
    auto dialog = new SettingDialog(
        ThemeManager::instance().standardIcon(
            ThemeManager::StandardIcon::ScriptSettings),
        QCoreApplication::translate("MainWindow", "Scripting environment"),
        parent);
    auto edit = new QEditConfig(false, dialog);
    dialog->addPage(edit);
    QObject::connect(edit, &WingHex::SettingPage::optionNeedRestartChanged,
                     dialog,
                     QOverload<>::of(&SettingDialog::toastTakeEffectReboot));
    edit = new QEditConfig(true, dialog);
    dialog->addPage(edit);
    QObject::connect(edit, &WingHex::SettingPage::optionNeedRestartChanged,
                     dialog,
                     QOverload<>::of(&SettingDialog::toastTakeEffectReboot));
    auto lspset = new LspSettingDialog(dialog);
    dialog->addPage(lspset);
    QObject::connect(lspset, &WingHex::SettingPage::optionNeedRestartChanged,
                     dialog,
                     QOverload<>::of(&SettingDialog::toastTakeEffectReboot));
    dialog->build();

    const auto ge = set.settingsScriptLayout();
    if (!ge.isEmpty()) {
        dialog->restoreLayout(ge);
    }
    QObject::connect(dialog, &SettingDialog::sigOnClosing, dialog, [dialog]() {
        SettingManager::instance().setSettingsScriptLayout(
            dialog->saveLayout());
    });
    return dialog;
}
