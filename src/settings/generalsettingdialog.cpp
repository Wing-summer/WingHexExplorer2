#include "generalsettingdialog.h"
#include "ui_generalsettingdialog.h"

#include <QMetaEnum>

#include "../AngelScript/angelscript/include/angelscript.h"
#include "../class/languagemanager.h"
#include "../class/settingmanager.h"
#include "../class/skinmanager.h"
#include "../utilities.h"

#include <QLibraryInfo>
#include <QSysInfo>

GeneralSettingDialog::GeneralSettingDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::GeneralSettingDialog) {
    ui->setupUi(this);

    ui->cbLanguage->addItems(LanguageManager::instance().langsDisplay());

    auto e = QMetaEnum::fromType<SkinManager::Theme>();
    for (int i = 0; i < e.keyCount(); ++i) {
        ui->cbTheme->addItem(SkinManager::tr(e.valueToKey(i)));
    }

    ui->cbWinState->addItems(
        {tr("Normal"), tr("Minimized"), tr("Maximized"), tr("FullScreen")});

    ui->lblQtVersion->setText(QLibraryInfo::build());
    ui->lblOS->setText(QSysInfo::prettyProductName());
    ui->lblSoftVersion->setText(WINGHEX_VERSION);
    ui->lblScriptVersion->setText(ANGELSCRIPT_VERSION_STRING);

    reload();
}

GeneralSettingDialog::~GeneralSettingDialog() { delete ui; }

void GeneralSettingDialog::reload() {
    auto &set = SettingManager::instance();
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
        s = 1;
        break;
    case Qt::WindowMaximized:
        s = 2;
        break;
    case Qt::WindowFullScreen:
        s = 3;
        break;
    }

    ui->cbWinState->setCurrentIndex(s);
}

QIcon GeneralSettingDialog::categoryIcon() const { return ICONRES("general"); }

QString GeneralSettingDialog::name() const { return tr("General"); }

bool GeneralSettingDialog::isInPluginPage() const { return false; }

void GeneralSettingDialog::apply() {
    auto &set = SettingManager::instance();
    set.setAppfontSize(ui->sbFontSize->value());
    set.setAppFontFamily(ui->cbFont->currentText());
    set.setThemeID(ui->cbTheme->currentIndex());
    auto s = ui->cbWinState->currentIndex();
    Qt::WindowState state;
    switch (s) {
    case 0:
        state = Qt::WindowState::WindowNoState;
        break;
    case 1:
        state = Qt::WindowState::WindowMinimized;
        break;
    case 2:
        state = Qt::WindowState::WindowMaximized;
        break;
    default:
        state = Qt::WindowState::WindowFullScreen;
        break;
    }
    set.setDefaultWinState(state);
    set.save(SettingManager::SETTING::APP);

    emit sigAppfontSizeChanged(set.appfontSize());
    emit sigAppFontFamilyChanged(set.appFontFamily());
    emit sigThemeIDChanged(set.themeID());
    emit sigDefaultWinStateChanged(set.defaultWinState());
}

void GeneralSettingDialog::reset() {
    SettingManager::instance().reset(SettingManager::SETTING::APP);
}

void GeneralSettingDialog::cancel() { reload(); }
