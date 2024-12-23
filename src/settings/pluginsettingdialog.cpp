/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
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

#include "pluginsettingdialog.h"
#include "../class/settingmanager.h"
#include "dbghelper.h"
#include "plugin/pluginsystem.h"
#include "ui_pluginsettingdialog.h"
#include "utilities.h"

PluginSettingDialog::PluginSettingDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::PluginSettingDialog) {
    ui->setupUi(this);

    Utilities::addSpecialMark(ui->cbEnablePlugin);
    Utilities::addSpecialMark(ui->cbEnablePluginRoot);
    connect(ui->cbEnablePlugin,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this, &PluginSettingDialog::optionNeedRestartChanged);
    connect(ui->cbEnablePluginRoot,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this, &PluginSettingDialog::optionNeedRestartChanged);

    reload();

    auto &plgsys = PluginSystem::instance();
    auto pico = ICONRES("plugin");
    ui->plglist->clear();
    for (auto &p : plgsys.plugins()) {
        ui->plglist->addItem(new QListWidgetItem(pico, p->pluginName()));
    }

    ui->txtc->clear();
}

PluginSettingDialog::~PluginSettingDialog() { delete ui; }

void PluginSettingDialog::buildUp(const QList<PluginPage *> &pages) {
    ASSERT_SINGLETON;
    for (auto &page : pages) {
        ui->tabWidget->addTab(page, page->categoryIcon(), page->name());
    }
}

void PluginSettingDialog::reload() {
    this->blockSignals(true);
    auto &set = SettingManager::instance();
    ui->cbEnablePlugin->setChecked(set.enablePlugin());
    ui->cbEnablePluginRoot->setChecked(set.enablePlgInRoot());
    this->blockSignals(false);
}

QIcon PluginSettingDialog::categoryIcon() const { return ICONRES("plugin"); }

QString PluginSettingDialog::name() const { return tr("Plugin"); }

QString PluginSettingDialog::id() const { return QStringLiteral("Plugin"); }

void PluginSettingDialog::apply() {
    auto &set = SettingManager::instance();
    set.setEnablePlugin(ui->cbEnablePlugin->isChecked());
    set.setEnablePlgInRoot(ui->cbEnablePluginRoot->isChecked());
    set.save(SettingManager::SETTING::PLUGIN);
}

void PluginSettingDialog::reset() {
    SettingManager::instance().reset(SettingManager::SETTING::PLUGIN);
    reload();
}

void PluginSettingDialog::cancel() { reload(); }

void PluginSettingDialog::on_plglist_itemSelectionChanged() {
    auto &plgsys = PluginSystem::instance();

    auto plg = plgsys.plugin(ui->plglist->currentRow());

    ui->txtc->append(tr("pluginName") + " : " + plg->pluginName());
    ui->txtc->append(tr("pluginAuthor") + " : " + plg->pluginAuthor());
    ui->txtc->append(tr("pluginVersion") + " : " +
                     QString::number(plg->pluginVersion()));
    ui->txtc->append(tr("pluginComment") + " : " + plg->pluginComment());
}
