#include "clangformatsetdialog.h"
#include "ui_clangformatsetdialog.h"

#include "class/clangformatmanager.h"
#include "class/winginputdialog.h"
#include "class/wingmessagebox.h"
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

    reload();
}

ClangFormatSetDialog::~ClangFormatSetDialog() { delete ui; }

QStringList ClangFormatSetDialog::getEditableStyleTextLines() {
    auto clang = ClangFormatManager::instance();
    auto styles = clang.customStyleString().split(',');
    for (auto &style : styles) {
        style = style.simplified();
    }
    return styles;
}

QString ClangFormatSetDialog::getEditableStyleTexts() {
    return getEditableStyleTextLines().join('\n');
}

void ClangFormatSetDialog::reload() {
    auto clang = ClangFormatManager::instance();
    ui->cbEnabled->setChecked(clang.enabled());
    ui->cbAutoFmt->setChecked(clang.autoFormat());
    ui->cbStyle->setCurrentIndex(clang.clangCurrentStyleIndex());
    ui->btnStyleCustom->setToolTip(getEditableStyleTexts());
}

QIcon ClangFormatSetDialog::categoryIcon() const {
    return ICONRES(QStringLiteral("codeformat"));
}

QString ClangFormatSetDialog::name() const { return tr("ClangFormat"); }

QString ClangFormatSetDialog::id() const {
    return QStringLiteral("ClangFormat");
}

void ClangFormatSetDialog::apply() {
    auto clang = ClangFormatManager::instance();
    clang.setEnabled(ui->cbEnabled->isChecked());
    clang.setAutoFormat(ui->cbAutoFmt->isChecked());
    clang.setClangStyle(ui->cbStyle->currentText());
}

void ClangFormatSetDialog::reset() {
    auto clang = ClangFormatManager::instance();
    clang.reset();
    reload();
}

void ClangFormatSetDialog::cancel() { reload(); }

void ClangFormatSetDialog::on_cbStyle_currentTextChanged(const QString &arg1) {
    ui->btnStyleCustom->setEnabled(arg1 == QStringLiteral("Custom"));
}

void ClangFormatSetDialog::on_btnStyleCustom_clicked() {
    auto clang = ClangFormatManager::instance();
    bool ok = false;
    while (true) {
        auto style = WingInputDialog::getMultiLineText(
            this, tr("ClangFormat"), tr("PleaseInputFmt"),
            getEditableStyleTexts(), &ok);
        if (ok) {
            if (clang.setClangStyle(style.replace('\n', ',').simplified())) {
                break;
            } else {
                ok = false;
                auto ret = WingMessageBox::critical(
                    this, tr("ClangFormat"), tr("ErrInCustomClangFormat"),
                    QMessageBox::Yes | QMessageBox::No);
                if (ret == QMessageBox::No) {
                    break;
                }
            }
        } else {
            break;
        }
    }

    if (ok) {
        ui->btnStyleCustom->setToolTip(getEditableStyleTexts());
    }
}
