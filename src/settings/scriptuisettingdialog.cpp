#include "scriptuisettingdialog.h"
#include "ui_scriptuisettingdialog.h"

ScriptUISettingDialog::ScriptUISettingDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptUISettingDialog)
{
    ui->setupUi(this);
}

ScriptUISettingDialog::~ScriptUISettingDialog()
{
    delete ui;
}
