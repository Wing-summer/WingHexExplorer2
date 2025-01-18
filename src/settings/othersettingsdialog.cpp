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
    ui->cbLogLevel->setCurrentIndex(Logger::instance().logLevel());

#ifndef WINGHEX_USE_FRAMELESS
    ui->cbNativeTitile->setEnabled(false);
#endif

    Utilities::addSpecialMark(ui->cbDontShowSplash);
    Utilities::addSpecialMark(ui->cbNativeTitile);
    Utilities::addSpecialMark(ui->lblLevel);
    Utilities::addSpecialMark(ui->lblCount);
    Utilities::addSpecialMark(ui->cbCheckWhenStartup);

    connect(ui->cbNativeTitile,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this, &OtherSettingsDialog::optionNeedRestartChanged);

    connect(ui->cbCheckWhenStartup,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this, &OtherSettingsDialog::optionNeedRestartChanged);

    connect(ui->cbLogLevel, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OtherSettingsDialog::optionNeedRestartChanged);

    reload();
}

OtherSettingsDialog::~OtherSettingsDialog() { delete ui; }

void OtherSettingsDialog::reload() {
    this->blockSignals(true);
    auto &set = SettingManager::instance();
    ui->cbDontShowSplash->setChecked(set.dontUseSplash());
    ui->cbNativeFileDialog->setChecked(set.useNativeFileDialog());
    ui->cbCheckWhenStartup->setChecked(set.checkUpdate());
#ifdef WINGHEX_USE_FRAMELESS
    ui->cbNativeTitile->setChecked(set.useNativeTitleBar());
#endif
    this->blockSignals(false);
}

QIcon OtherSettingsDialog::categoryIcon() const { return ICONRES("other"); }

QString OtherSettingsDialog::name() const { return tr("Others"); }

QString OtherSettingsDialog::id() const { return QStringLiteral("Others"); }

void OtherSettingsDialog::apply() {
    auto &set = SettingManager::instance();
    set.setDontUseSplash(ui->cbDontShowSplash->isChecked());
    set.setUseNativeFileDialog(ui->cbNativeFileDialog->isChecked());
#ifdef WINGHEX_USE_FRAMELESS
    set.setUseNativeTitleBar(ui->cbNativeTitile->isChecked());
#endif
    set.setCheckUpdate(ui->cbCheckWhenStartup->isChecked());
    set.setLogLevel(ui->cbLogLevel->currentIndex());
    set.save(SettingManager::OTHER);
}

void OtherSettingsDialog::reset() {
    SettingManager::instance().reset(SettingManager::SETTING::OTHER);
    reload();
}

void OtherSettingsDialog::cancel() { reload(); }
