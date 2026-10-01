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

#include "lspsettingdialog.h"
#include "ui_lspsettingdialog.h"

#include "class/scriptsettings.h"
#include "class/settingmanager.h"
#include "luau/luauformatter.h"
#include "utilities.h"

LspSettingDialog::LspSettingDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::LspSettingDialog) {
    ui->setupUi(this);

    ui->cbQuoteStyle->addItem(tr("Preserve"),
                              int(LuauFormat::QuoteStyle::Preserve));
    ui->cbQuoteStyle->addItem(tr("PreferDouble"),
                              int(LuauFormat::QuoteStyle::PreferDouble));
    ui->cbQuoteStyle->addItem(tr("PreferSingle"),
                              int(LuauFormat::QuoteStyle::PreferSingle));
    ui->cbQuoteStyle->addItem(tr("ForceDouble"),
                              int(LuauFormat::QuoteStyle::ForceDouble));
    ui->cbQuoteStyle->addItem(tr("ForceSingle"),
                              int(LuauFormat::QuoteStyle::ForceSingle));

    ui->cbQuoteStyle->setCurrentIndex(0);

    reload();

    auto &set = SettingManager::instance();
    if (set.scriptEnabled()) {
        auto s = &ScriptSettings::instance();

        connect(ui->sbIndent, &QSpinBox::valueChanged, s,
                &ScriptSettings::setFmtIndentSpace);
        connect(ui->cbQuoteStyle, &QComboBox::currentIndexChanged, s,
                [this](int index) {
                    auto qs = ui->cbQuoteStyle->itemData(index).toInt();
                    ScriptSettings::instance().setFmtStrQuoteStyle(qs);
                });
        connect(ui->cbUseTabStop,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
                &QCheckBox::checkStateChanged,
#else
                    &QCheckBox::stateChanged,
#endif
                s, [](Qt::CheckState state) {
                    ScriptSettings::instance().setFmtUseTabIndent(
                        state != Qt::Unchecked);
                });

        connect(ui->cbKeepNewLineGap,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
                &QCheckBox::checkStateChanged,
#else
                &QCheckBox::stateChanged,
#endif
                s, [](Qt::CheckState state) {
                    ScriptSettings::instance().setFmtKeepNewLineGap(
                        state != Qt::Unchecked);
                });

        connect(ui->cbAutoFmt,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
                &QCheckBox::checkStateChanged,
#else
                        &QCheckBox::stateChanged,
#endif
                s, [](Qt::CheckState state) {
                    ScriptSettings::instance().setAutofmt(state !=
                                                          Qt::Unchecked);
                });
    }
}

LspSettingDialog::~LspSettingDialog() { delete ui; }

QIcon LspSettingDialog::categoryIcon() const {
    return ICONRES(QStringLiteral("luau"));
}

QString LspSettingDialog::name() const { return QStringLiteral("Luau"); }

QString LspSettingDialog::id() const { return QStringLiteral("Luau"); }

void LspSettingDialog::restore() {
    ScriptSettings::instance().reset(ScriptSettings::FORMAT);
}

void LspSettingDialog::reload() {
    if (SettingManager::instance().scriptEnabled()) {
        auto &s = ScriptSettings::instance();
        ui->sbIndent->setValue(s.fmtIndentSpace());
        ui->cbQuoteStyle->setCurrentIndex(s.fmtStrQuoteStyle());
        ui->cbUseTabStop->setChecked(s.fmtUseTabIndent());
        ui->cbKeepNewLineGap->setChecked(s.fmtKeepNewLineGap());
        ui->cbAutoFmt->setChecked(s.autofmt());
    } else {
        setEnabled(false);
    }
}
