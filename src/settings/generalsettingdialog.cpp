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

#include "generalsettingdialog.h"
#include "ui_generalsettingdialog.h"

#include <QMetaEnum>

#include "AngelScript/sdk/angelscript/include/angelscript.h"
#include "class/languagemanager.h"
#include "class/settingmanager.h"
#include "class/skinmanager.h"
#include "utilities.h"

#include <QLibraryInfo>
#include <QSysInfo>

GeneralSettingDialog::GeneralSettingDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::GeneralSettingDialog) {
    ui->setupUi(this);

    ui->cbLanguage->addItem(tr("SystemDefault"));

    auto e = QMetaEnum::fromType<SkinManager::Theme>();
    for (int i = 0; i < e.keyCount(); ++i) {
        ui->cbTheme->addItem(SkinManager::tr(e.valueToKey(i)));
    }

    ui->cbWinState->addItems({tr("Normal"), tr("Maximized"), tr("FullScreen")});

    ui->lblQtVersion->setText(QLibraryInfo::build());
    ui->lblOS->setText(QSysInfo::prettyProductName());
    ui->lblSoftVersion->setText(WINGHEX_VERSION);
    ui->lblScriptVersion->setText(ANGELSCRIPT_VERSION_STRING);

    Utilities::addSpecialMark(ui->lblLanguage);
    Utilities::addSpecialMark(ui->lblTheme);
    Utilities::addSpecialMark(ui->lblFont);
    Utilities::addSpecialMark(ui->lblFontSize);
    Utilities::addSpecialMark(ui->lblWinState);

    reload();

    connect(ui->cbLanguage, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneralSettingDialog::optionNeedRestartChanged);
    connect(ui->cbTheme, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneralSettingDialog::optionNeedRestartChanged);
    connect(ui->cbFont, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneralSettingDialog::optionNeedRestartChanged);
    connect(ui->sbFontSize, &QSpinBox::editingFinished, this,
            &GeneralSettingDialog::optionNeedRestartChanged);
    connect(ui->cbWinState, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneralSettingDialog::optionNeedRestartChanged);

    auto sm = &SettingManager::instance();
    connect(ui->cbLanguage, &QComboBox::currentIndexChanged, sm,
            [this](int index) {
                auto data = ui->cbLanguage->itemData(index).toString();
                SettingManager::instance().setDefaultLang(data);
            });
    connect(ui->sbFontSize, &QSpinBox::valueChanged, sm,
            &SettingManager::setAppfontSize);
    connect(ui->cbFont, &QFontComboBox::currentTextChanged, sm,
            &SettingManager::setAppFontFamily);
    connect(ui->cbTheme, &QComboBox::currentIndexChanged, sm,
            &SettingManager::setThemeID);
    connect(ui->cbWinState, &QComboBox::currentIndexChanged, sm, [](int index) {
        Qt::WindowState state;
        switch (index) {
        case 0:
            state = Qt::WindowState::WindowNoState;
            break;
        case 1:
            state = Qt::WindowState::WindowMaximized;
            break;
        default:
            state = Qt::WindowState::WindowFullScreen;
            break;
        }
        SettingManager::instance().setDefaultWinState(state);
    });
}

GeneralSettingDialog::~GeneralSettingDialog() { delete ui; }

void GeneralSettingDialog::reload() {
    this->blockSignals(true);

    auto &set = SettingManager::instance();

    auto &lang = LanguageManager::instance();
    auto langs = lang.langs();
    for (auto &l : langs) {
        ui->cbLanguage->addItem(lang.langDisplay(l), l);
    }

    auto deflang = set.defaultLang();
    if (deflang.isEmpty()) {
        ui->cbLanguage->setCurrentIndex(0);
    } else {
        ui->cbLanguage->setCurrentIndex(langs.indexOf(deflang) + 1);
    }

    ui->sbFontSize->setValue(set.appfontSize());
    ui->cbFont->setCurrentText(set.appFontFamily());
    ui->cbTheme->setCurrentIndex(set.themeID());

    int s;
    switch (set.defaultWinState()) {
    case Qt::WindowNoState:
    case Qt::WindowActive:
        s = 0;
        break;
    case Qt::WindowMinimized:
    case Qt::WindowMaximized:
        s = 1;
        break;
    case Qt::WindowFullScreen:
        s = 2;
        break;
    }

    ui->cbWinState->setCurrentIndex(s);

    this->blockSignals(false);
}

QIcon GeneralSettingDialog::categoryIcon() const { return ICONRES("general"); }

QString GeneralSettingDialog::name() const { return tr("General"); }

QString GeneralSettingDialog::id() const { return QStringLiteral("General"); }

void GeneralSettingDialog::restore() {
    SettingManager::instance().reset(SettingManager::SETTING::APP);
    reload();
}
