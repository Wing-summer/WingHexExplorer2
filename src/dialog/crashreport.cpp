#include "crashreport.h"
#include "ui_crashreport.h"

#include <QClipboard>
#include <QPushButton>

#include "utilities.h"

#include "control/toast.h"

CrashReport::CrashReport(QWidget *parent)
    : QWidget(parent), ui(new Ui::CrashReport) {
    ui->setupUi(this);

    _dialog = new FramelessDialogBase(parent);
    _dialog->buildUpContent(this);
    _dialog->setWindowTitle(this->windowTitle());
    _dialog->setMinimumSize(600, 500);

    Utilities::moveToCenter(_dialog);
}

CrashReport::~CrashReport() { delete ui; }

void CrashReport::setInfomation(const QString &info) {
    ui->textBrowser->setText(info);
}

int CrashReport::exec() { return _dialog->exec(); }

void CrashReport::on_buttonBox_clicked(QAbstractButton *button) {
    if (button == ui->buttonBox->button(QDialogButtonBox::Save)) {
        auto clipboard = qApp->clipboard();
        clipboard->setText(ui->textBrowser->toPlainText());
        Toast::toast(_dialog, NAMEICONRES(QStringLiteral("copy")),
                     tr("CopyToClipBoard"));
    } else {
        _dialog->done(1);
    }
}
