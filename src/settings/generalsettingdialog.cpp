#include "generalsettingdialog.h"
#include "ui_generalsettingdialog.h"

GeneralSettingDialog::GeneralSettingDialog(QWidget *parent)
    : QWidget(parent), ui(new Ui::GeneralSettingDialog) {
    ui->setupUi(this);
}

GeneralSettingDialog::~GeneralSettingDialog() { delete ui; }
