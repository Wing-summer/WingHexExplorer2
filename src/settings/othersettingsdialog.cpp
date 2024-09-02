#include "othersettingsdialog.h"
#include "class/settingmanager.h"
#include "ui_othersettingsdialog.h"
#include "utilities.h"

OtherSettingsDialog::OtherSettingsDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::OtherSettingsDialog) {
    ui->setupUi(this);
    reload();
}

OtherSettingsDialog::~OtherSettingsDialog() { delete ui; }

void OtherSettingsDialog::reload() {
    auto &set = SettingManager::instance();
    ui->cbNativeFileDialog->setChecked(set.useNativeFileDialog());
    ui->cbNativeTitile->setChecked(set.useNativeTitleBar());
}

QIcon OtherSettingsDialog::categoryIcon() const { return ICONRES("other"); }

QString OtherSettingsDialog::name() const { return tr("Others"); }

bool OtherSettingsDialog::isInPluginPage() const { return false; }

void OtherSettingsDialog::apply() {
    auto &set = SettingManager::instance();
    set.setUseNativeFileDialog(ui->cbNativeFileDialog->isChecked());
    set.setUseNativeTitleBar(ui->cbNativeTitile->isChecked());
    set.save(SettingManager::OTHER);
}

void OtherSettingsDialog::reset() {
    SettingManager::instance().reset(SettingManager::SETTING::APP);
    reload();
}

void OtherSettingsDialog::cancel() { reload(); }
