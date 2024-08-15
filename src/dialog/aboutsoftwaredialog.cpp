#include "aboutsoftwaredialog.h"
#include "ui_aboutsoftwaredialog.h"

#include "class/languagemanager.h"
#include "utilities.h"

AboutSoftwareDialog::AboutSoftwareDialog(QWidget *parent)
    : QWidget(parent), ui(new Ui::AboutSoftwareDialog) {
    ui->setupUi(this);

    auto data = LanguageManager::instance().data();
    ui->tbAbout->setMarkdown(data.about);
    ui->tbBaseObj->setMarkdown(data.component);
    ui->tbCredit->setMarkdown(data.credit);
    ui->tbDev->setMarkdown(data.dev);
    ui->tbTr->setMarkdown(data.trans);
    ui->lblVersion->setText(qApp->applicationVersion());

    _dialog = new FramelessDialogBase(parent);
    _dialog->buildUpContent(this);
    _dialog->setWindowTitle(this->windowTitle());
    _dialog->setMinimumSize(800, 600);

    Utilities::moveToCenter(this);
}

AboutSoftwareDialog::~AboutSoftwareDialog() {
    delete ui;
    _dialog->deleteLater();
}

int AboutSoftwareDialog::exec() { return _dialog->exec(); }
