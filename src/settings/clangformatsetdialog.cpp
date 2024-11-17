#include "clangformatsetdialog.h"
#include "class/clangformatmanager.h"
#include "class/winginputdialog.h"
#include "ui_clangformatsetdialog.h"
#include "utilities.h"

ClangFormatSetDialog::ClangFormatSetDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::ClangFormatSetDialog) {
    ui->setupUi(this);

    auto clang = ClangFormatManager::instance();
    ui->cbStyle->addItems(clang.supportedStyles());

    ui->leLocation->setText(ClangFormatManager::getProgramName());

    if (clang.exists()) {
        ui->lblClangPath->setText(clang.path());
        ui->lblClangPath->setToolTip(clang.path());
        ui->lblClangVersion->setText(clang.version());
    } else {
        ui->lblClangPath->setStyleSheet(QStringLiteral("color:red"));
    }
}

ClangFormatSetDialog::~ClangFormatSetDialog() { delete ui; }

QIcon ClangFormatSetDialog::categoryIcon() const {
    return ICONRES(QStringLiteral("codeformat"));
}

QString ClangFormatSetDialog::name() const { return tr("ClangFormat"); }

QString ClangFormatSetDialog::id() const {
    return QStringLiteral("ClangFormat");
}

void ClangFormatSetDialog::apply() {}

void ClangFormatSetDialog::reset() {}

void ClangFormatSetDialog::cancel() {}

void ClangFormatSetDialog::on_cbStyle_currentTextChanged(const QString &arg1) {
    ui->btnStyleCustom->setEnabled(arg1 == QStringLiteral("Custom"));
}

void ClangFormatSetDialog::on_btnStyleCustom_clicked() {
    // TODO
    WingInputDialog::getMultiLineText(this, tr("ClangFormat"), "");
}
