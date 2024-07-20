#include "scriptsettingdialog.h"
#include "ui_scriptsettingdialog.h"

#include "../utilities.h"

ScriptSettingDialog::ScriptSettingDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::ScriptSettingDialog) {
    ui->setupUi(this);
}

ScriptSettingDialog::~ScriptSettingDialog() { delete ui; }

QIcon ScriptSettingDialog::categoryIcon() const { return ICONRES("script"); }

QString ScriptSettingDialog::name() const { return tr("Script"); }

bool ScriptSettingDialog::isInPluginPage() const { return false; }

void ScriptSettingDialog::apply() {}

void ScriptSettingDialog::reset() {}

void ScriptSettingDialog::cancel() {}
