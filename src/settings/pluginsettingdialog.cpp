#include "pluginsettingdialog.h"
#include "ui_pluginsettingdialog.h"

PluginSettingDialog::PluginSettingDialog(QWidget *parent)
    : QWidget(parent), ui(new Ui::PluginSettingDialog) {
    ui->setupUi(this);
}

PluginSettingDialog::~PluginSettingDialog() { delete ui; }
