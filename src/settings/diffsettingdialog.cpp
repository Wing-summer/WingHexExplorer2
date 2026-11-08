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

#include "diffsettingdialog.h"
#include "ui_diffsettingdialog.h"

#include "class/settings/settingmanager.h"
#include "class/settings/thememanager.h"

DiffSettingDialog::DiffSettingDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::DiffSettingDialog) {
    ui->setupUi(this);
    ui->sbFileLimit->setRange(
        int(WingDiff::Limits::MinimumFileSize / WingDiff::MiB),
        int(WingDiff::Limits::MaximumFileSize / WingDiff::MiB));
    ui->sbDetails->setRange(int(WingDiff::Limits::MinimumDetailedRanges),
                            int(WingDiff::Limits::MaximumDetailedRanges));
    for (const auto bytes : WingDiff::Limits::MemoryBudgets) {
        ui->cbMemory->addItem(
            QStringLiteral("%1 MiB").arg(bytes / WingDiff::MiB),
            QVariant::fromValue(bytes));
    }
    reload();

    connect(ui->sbFileLimit, &QSpinBox::valueChanged, this,
            &DiffSettingDialog::setFileLimit);
    connect(ui->cbMemory, &QComboBox::currentIndexChanged, this,
            &DiffSettingDialog::setMemoryBudget);
    connect(ui->sbDetails, &QSpinBox::valueChanged, this,
            &DiffSettingDialog::setDetailedLimit);
    connect(ui->cbPrecise, &QCheckBox::toggled, &SettingManager::instance(),
            &SettingManager::setDiffPrecise);
    connect(&SettingManager::instance(),
            &SettingManager::sigDiffSettingsChanged, this,
            &DiffSettingDialog::reload);
}

DiffSettingDialog::~DiffSettingDialog() { delete ui; }

void DiffSettingDialog::reload() {
    const auto &sm = SettingManager::instance();
    ui->sbFileLimit->setValue(int(sm.diffFileLimit() / WingDiff::MiB));
    ui->cbMemory->setCurrentIndex(
        ui->cbMemory->findData(QVariant::fromValue(sm.diffMemoryLimit())));
    ui->sbDetails->setValue(int(sm.diffDetailLimit()));
    ui->cbPrecise->setChecked(sm.diffPrecise());
}

QIcon DiffSettingDialog::categoryIcon() const {
    return ThemeManager::instance().standardIcon(
        ThemeManager::StandardIcon::Compare);
}

QString DiffSettingDialog::name() const { return tr("Compare"); }

QString DiffSettingDialog::id() const { return QStringLiteral("Diff"); }

void DiffSettingDialog::restore() {
    SettingManager::instance().reset(SettingManager::DIFF);
}

void DiffSettingDialog::setFileLimit(int mebibytes) {
    SettingManager::instance().setDiffFileLimit(quint64(mebibytes) *
                                                WingDiff::MiB);
}
void DiffSettingDialog::setMemoryBudget(int index) {
    SettingManager::instance().setDiffMemoryLimit(
        ui->cbMemory->itemData(index).toULongLong());
}

void DiffSettingDialog::setDetailedLimit(int count) {
    SettingManager::instance().setDiffDetailLimit(quint64(count));
}
