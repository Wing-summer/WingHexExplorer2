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
    connect(ui->cbEnablePlugin, &QCheckBox::stateChanged, this,
            &PluginSettingDialog::optionNeedRestartChanged);
    connect(ui->cbEnablePluginRoot, &QCheckBox::stateChanged, this,
            &PluginSettingDialog::optionNeedRestartChanged);

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

void PluginSettingDialog::buildUp(const QList<SettingPage *> &pages) {
    ASSERT_SINGLETON;
    for (auto &page : pages) {
        if (page->isInPluginPage()) {
            ui->tabWidget->addTab(page, page->categoryIcon(), page->name());
            _pages << page;
        }
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

bool PluginSettingDialog::isInPluginPage() const { return false; }

void PluginSettingDialog::apply() {
    auto &set = SettingManager::instance();
    set.setEnablePlugin(ui->cbEnablePlugin->isChecked());
    set.setEnablePlgInRoot(ui->cbEnablePluginRoot->isChecked());
    set.save(SettingManager::SETTING::PLUGIN);

    for (auto &page : _pages) {
        page->apply();
    }
}

void PluginSettingDialog::reset() {
    SettingManager::instance().reset(SettingManager::SETTING::PLUGIN);
    for (auto &page : _pages) {
        page->reset();
    }
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
