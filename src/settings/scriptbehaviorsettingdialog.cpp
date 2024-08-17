#include "scriptbehaviorsettingdialog.h"
#include "ui_scriptbehaviorsettingdialog.h"

ScriptBehaviorSettingDialog::ScriptBehaviorSettingDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptBehaviorSettingDialog)
{
    ui->setupUi(this);
}

ScriptBehaviorSettingDialog::~ScriptBehaviorSettingDialog()
{
    delete ui;
}
