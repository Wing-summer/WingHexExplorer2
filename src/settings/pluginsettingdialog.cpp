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
#include "class/pluginsystem.h"
#include "class/settingmanager.h"
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

    auto minfo = plgsys.monitorManagerInfo();
    if (minfo) {
        auto sep = QStringLiteral(" : ");

        ui->txtm->append(getWrappedText(tr("ID") + sep + minfo->id));
        ui->txtm->append(getWrappedText(tr("License") + sep + minfo->license));
        ui->txtm->append(getWrappedText(tr("Author") + sep + minfo->author));
        ui->txtm->append(getWrappedText(tr("Vendor") + sep + minfo->vendor));
        ui->txtm->append(
            getWrappedText(tr("Version") + sep + minfo->version.toString()));
        ui->txtm->append(getWrappedText(
            tr("URL") + sep + QStringLiteral("<a href=\"") + minfo->url +
            QStringLiteral("\">") + minfo->url + QStringLiteral("</a>")));
        ui->txtm->append(getWrappedText(tr("Comment") + sep));
        auto p = plgsys.monitorManager();
        if (p) {
            ui->txtm->append(getWrappedText(p->comment()));
        }
    } else {
        ui->txtm->setText(tr("NoMonitorPlugin"));
    }

    auto set = &SettingManager::instance();
    connect(ui->cbEnablePlugin, &QCheckBox::toggled, set,
            &SettingManager::setEnablePlugin);
    connect(ui->cbEnablePluginRoot, &QCheckBox::toggled, set,
            &SettingManager::setEnablePlgInRoot);
}

PluginSettingDialog::~PluginSettingDialog() { delete ui; }

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

void PluginSettingDialog::restore() {
    SettingManager::instance().reset(SettingManager::SETTING::PLUGIN);
    reload();
}

void PluginSettingDialog::on_devlist_currentRowChanged(int currentRow) {
    if (currentRow < 0) {
        return;
    }

    auto &plgsys = PluginSystem::instance();
    auto plg = plgsys.device(currentRow);

    auto info = plgsys.getPluginInfo(plg);
    ui->txtd->clear();
    static auto sep = QStringLiteral(" : ");
    ui->txtd->append(getWrappedText(tr("ID") + sep + info.id));
    ui->txtd->append(getWrappedText(tr("Name") + sep + plg->pluginName()));
    ui->txtd->append(getWrappedText(tr("License") + sep + info.license));
    ui->txtd->append(getWrappedText(tr("Author") + sep + info.author));
    ui->txtd->append(getWrappedText(tr("Vendor") + sep + info.vendor));
    ui->txtd->append(
        getWrappedText(tr("Version") + sep + info.version.toString()));
    ui->txtd->append(
        getWrappedText(tr("Comment") + sep + plg->pluginComment()));
    ui->txtd->append(getWrappedText(
        tr("URL") + sep + QStringLiteral("<a href=\"") + info.url +
        QStringLiteral("\">") + info.url + QStringLiteral("</a>")));
}

void PluginSettingDialog::on_plglist_currentRowChanged(int currentRow) {
    if (currentRow < 0) {
        return;
    }

    auto &plgsys = PluginSystem::instance();
    auto plg = plgsys.plugin(currentRow);

    auto info = plgsys.getPluginInfo(plg);
    ui->txtc->clear();
    static auto sep = QStringLiteral(" : ");
    ui->txtc->append(getWrappedText(tr("ID") + sep + info.id));
    ui->txtc->append(getWrappedText(tr("Name") + sep + plg->pluginName()));
    ui->txtc->append(getWrappedText(tr("License") + sep + info.license));
    ui->txtc->append(getWrappedText(tr("Author") + sep + info.author));
    ui->txtc->append(getWrappedText(tr("Vendor") + sep + info.vendor));
    ui->txtc->append(
        getWrappedText(tr("Version") + sep + info.version.toString()));
    ui->txtc->append(
        getWrappedText(tr("Comment") + sep + plg->pluginComment()));
    if (!info.dependencies.isEmpty()) {
        ui->txtc->append(getWrappedText(tr("pluginDependencies:")));
        for (auto &d : info.dependencies) {
            ui->txtc->append(
                getWrappedText(QString(4, ' ') + tr("PUID:") + d.puid));
            ui->txtc->append(getWrappedText(QString(4, ' ') + tr("Version:") +
                                            d.version.toString()));
        }
    }
    ui->txtc->append(getWrappedText(
        tr("URL") + sep + QStringLiteral("<a href=\"") + info.url +
        QStringLiteral("\">") + info.url + QStringLiteral("</a> ")));
}

QString PluginSettingDialog::getWrappedText(const QString &str) {
    return QStringLiteral("<a>") + str + QStringLiteral("</a>");
}
