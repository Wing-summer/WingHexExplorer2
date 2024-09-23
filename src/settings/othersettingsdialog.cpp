#include "othersettingsdialog.h"
#include "class/logger.h"
#include "class/settingmanager.h"
#include "ui_othersettingsdialog.h"
#include "utilities.h"

OtherSettingsDialog::OtherSettingsDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::OtherSettingsDialog) {
    ui->setupUi(this);

    QStringList levels;
    for (auto i = Logger::LEVEL_BEGIN; i != Logger::LEVEL_END;
         i = Logger::Level(i + 1)) {
        levels << Logger::getString(i);
    }

    ui->cbLogLevel->addItems(levels);
    ui->cbLogLevel->setCurrentIndex(Logger::instance()->logLevel());

#ifndef WINGHEX_USE_FRAMELESS
    ui->cbNativeTitile->setEnabled(false);
    ui->cbNativeTitile->setChecked(false);
#endif

    reload();
}

OtherSettingsDialog::~OtherSettingsDialog() { delete ui; }

void OtherSettingsDialog::reload() {
    auto &set = SettingManager::instance();
    ui->cbNativeFileDialog->setChecked(set.useNativeFileDialog());
#ifdef WINGHEX_USE_FRAMELESS
    ui->cbNativeTitile->setChecked(set.useNativeTitleBar());
#endif
}

QIcon OtherSettingsDialog::categoryIcon() const { return ICONRES("other"); }

QString OtherSettingsDialog::name() const { return tr("Others"); }

bool OtherSettingsDialog::isInPluginPage() const { return false; }

void OtherSettingsDialog::apply() {
    auto &set = SettingManager::instance();
    set.setUseNativeFileDialog(ui->cbNativeFileDialog->isChecked());
#ifdef WINGHEX_USE_FRAMELESS
    set.setUseNativeTitleBar(ui->cbNativeTitile->isChecked());
#endif
    set.setLogLevel(ui->cbLogLevel->currentIndex());
    set.save(SettingManager::OTHER);
}

void OtherSettingsDialog::reset() {
    SettingManager::instance().reset(SettingManager::SETTING::APP);
    reload();
}

void OtherSettingsDialog::cancel() { reload(); }
