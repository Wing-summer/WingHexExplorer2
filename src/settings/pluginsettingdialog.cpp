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
#include "class/settingmanager.h"
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
        auto pco = p->pluginIcon();
        ui->plglist->addItem(
            new QListWidgetItem(pco.isNull() ? pico : pco, p->pluginName()));
    }
    ui->txtc->clear();

    pico = ICONRES("devext");
    ui->devlist->clear();
    for (auto &d : plgsys.devices()) {
        auto pco = d->pluginIcon();
        ui->devlist->addItem(
            new QListWidgetItem(pco.isNull() ? pico : pco, d->pluginName()));
    }
    ui->txtd->clear();
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

void PluginSettingDialog::on_devlist_currentRowChanged(int currentRow) {
    if (currentRow < 0) {
        return;
    }

    auto &plgsys = PluginSystem::instance();
    auto plg = plgsys.device(currentRow);

    auto info = plgsys.getPluginInfo(plg);
    ui->txtd->clear();
    ui->txtd->append(tr("ID") + " : " + info.id);
    ui->txtd->append(tr("Name") + " : " + plg->pluginName());
    ui->txtd->append(tr("License") + " : " + info.license);
    ui->txtd->append(tr("Author") + " : " + info.author);
    ui->txtd->append(tr("Vendor") + " : " + info.vendor);
    ui->txtd->append(tr("Version") + " : " + info.version.toString());
    ui->txtd->append(tr("Comment") + " : " + plg->pluginComment());
    ui->txtd->append(tr("URL") + " : " + info.url);
}

void PluginSettingDialog::on_plglist_currentRowChanged(int currentRow) {
    if (currentRow < 0) {
        return;
    }

    auto &plgsys = PluginSystem::instance();
    auto plg = plgsys.plugin(currentRow);

    auto info = plgsys.getPluginInfo(plg);
    ui->txtc->clear();
    ui->txtc->append(tr("ID") + " : " + info.id);
    ui->txtc->append(tr("Name") + " : " + plg->pluginName());
    ui->txtc->append(tr("License") + " : " + info.license);
    ui->txtc->append(tr("Author") + " : " + info.author);
    ui->txtc->append(tr("Vendor") + " : " + info.vendor);
    ui->txtc->append(tr("Version") + " : " + info.version.toString());
    ui->txtc->append(tr("Comment") + " : " + plg->pluginComment());
    if (!info.dependencies.isEmpty()) {
        ui->txtc->append(tr("pluginDependencies:"));
        for (auto &d : info.dependencies) {
            ui->txtc->append(QString(4, ' ') + tr("PUID:") + d.puid);
            ui->txtc->append(QString(4, ' ') + tr("Version:") +
                             d.version.toString());
        }
    }
    ui->txtc->append(tr("URL") + " : " + info.url);
}
