/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
** =============================================================================
*/

#include "testform.h"
#include "WingPlugin/iwingpluginbase.h"
#include "ui_testform.h"

#include "ctltestform.h"
#include "readertestform.h"

#include <QAbstractTableModel>
#include <QFileSystemModel>
#include <QMetaEnum>
#include <QScrollArea>
#include <QStandardItemModel>
#include <QStringListModel>

TestForm::TestForm(WingHex::IWingPlugin *plg, QWidget *parent)
    : WingHex::WingPluginWidget(plg, parent), ui(new Ui::TestForm) {
    ui->setupUi(this);

    ui->saReader->widget()->layout()->addWidget(
        new ReaderTestForm(plg, ui->tbReaderLogger, this));
    ui->saCtl->widget()->layout()->addWidget(
        new CtlTestForm(plg, ui->tbCtlLogger, this));

    ui->spltReader->setSizes({300, 150});
    ui->spltCtl->setSizes({300, 150});

    initLogCombo();
    initStyleCombo();
    initMsgBoxBtnCombo();
    initMsgBoxCheckedBtnCombo();
    initMsgBoxIconCombo();
    initFileDialogOps();

    ui->lblauthor->setPixmap(
        WingHex::HOSTRESPIMG(QStringLiteral("author"), QStringLiteral(".jpg")));
}

TestForm::~TestForm() { delete ui; }

void TestForm::initLogCombo() {
    auto e = QMetaEnum::fromType<Level>();
    for (int i = LEVEL_BEGIN; i < LEVEL_END; ++i) {
        ui->cbLogLevel->addItem(e.key(i));
    }
}

void TestForm::initStyleCombo() {
    auto style = this->style();
    auto e = QMetaEnum::fromType<QStyle::StandardPixmap>();
    for (int i = 0; i < QStyle::StandardPixmap::NStandardPixmap; ++i) {
        auto ee = QStyle::StandardPixmap(i);
        auto icon = style->standardIcon(ee);
        if (!icon.isNull()) {
            ui->cbToastIcon->addItem(icon, e.key(i), style->standardPixmap(ee));
        }
    }
}

void TestForm::initMsgBoxCheckedBtnCombo() {
    // Qt declared that QMessageBox::StandardButton and
    // QDialogButtonBox::StandardButton shoud be kept in sync
    auto e = QMetaEnum::fromType<QDialogButtonBox::StandardButtons>();
    ui->cbMsgButtons->setNoneCheckedText(
        e.valueToKey(QDialogButtonBox::StandardButton::NoButton));
    ui->cbMsgButtons->setAllCheckedText(QStringLiteral("ALL_BUTTONS"));
    ui->cbMsgButtons->setUnknownlyCheckedText(
        QStringLiteral("UNKNOWN_BUTTONS"));
    for (int i = 0; i < e.keyCount(); ++i) {
        if (e.value(i) != QDialogButtonBox::StandardButton::NoButton) {
            ui->cbMsgButtons->addCheckItem(e.key(i), e.value(i), Qt::Unchecked);
        }
    }
}

void TestForm::initMsgBoxBtnCombo() {
    // Qt declared that QMessageBox::StandardButton and
    // QDialogButtonBox::StandardButton shoud be kept in sync
    auto e = QMetaEnum::fromType<QDialogButtonBox::StandardButtons>();
    for (int i = 0; i < e.keyCount(); ++i) {
        ui->cbMsgDefButton->addItem(e.key(i), e.value(i));
    }
}

void TestForm::initMsgBoxIconCombo() {
    auto e = QMetaEnum::fromType<QMessageBox::Icon>();
    for (int i = 0; i < e.keyCount(); ++i) {
        ui->cbMsgIcon->addItem(e.key(i), e.value(i));
    }
}

void TestForm::initFileDialogOps() {
    auto e = QMetaEnum::fromType<QFileDialog::Option>();
    ui->cbFileDialogOptions->setNoneCheckedText(QStringLiteral("NO_OPTION"));
    ui->cbFileDialogOptions->setAllCheckedText(QStringLiteral("ALL_OPTIONS"));
    ui->cbFileDialogOptions->setUnknownlyCheckedText(
        QStringLiteral("UNKNOWN_OPTIONS"));

    for (int i = 0; i < e.keyCount(); ++i) {
        ui->cbFileDialogOptions->addCheckItem(e.key(i), e.value(i),
                                              Qt::Unchecked);
    }
}

QMessageBox::StandardButtons TestForm::getMsgButtons() const {
    QMessageBox::StandardButtons buttons =
        QMessageBox::StandardButton::NoButton;
    for (int i = 0; i < ui->cbMsgButtons->count(); ++i) {
        if (ui->cbMsgButtons->itemData(i, Qt::CheckStateRole)
                .value<Qt::CheckState>() == Qt::Checked) {
            buttons.setFlag(QMessageBox::StandardButton(
                ui->cbMsgButtons->itemData(i, Qt::UserRole + 1).toInt()));
        }
    }
    return buttons;
}

QFileDialog::Options TestForm::getFileDialogOptions() const {
    QFileDialog::Options options;
    for (int i = 0; i < ui->cbFileDialogOptions->count(); ++i) {
        if (ui->cbFileDialogOptions->itemData(i, Qt::CheckStateRole)
                .value<Qt::CheckState>() == Qt::Checked) {
            options.setFlag(QFileDialog::Option(
                ui->cbMsgButtons->itemData(i, Qt::UserRole + 1).toInt()));
        }
    }
    return options;
}

void TestForm::on_btnSendLog_clicked() {
    auto txt = ui->leLogText->text();
    switch (Level(ui->cbLogLevel->currentIndex())) {
    case q1ERROR:
        logError(txt);
        break;
    case q2WARN:
        logWarn(txt);
        break;
    case q3INFO:
        logInfo(txt);
        break;
    case q4DEBUG:
        logDebug(txt);
        break;
    case q5TRACE:
        logTrace(txt);
        break;
    default:
        break;
    }
}

void TestForm::on_btnSendToast_clicked() {
    auto idx = ui->cbToastIcon->currentIndex();
    Q_ASSERT(idx >= 0);
    auto icon = ui->cbToastIcon->itemData(idx).value<QPixmap>();
    toast(icon, ui->leToastText->text());
}

void TestForm::on_btnAboutQt_clicked() {
    msgAboutQt(this, ui->leAboutTitle->text());
}

void TestForm::on_btnQuestion_clicked() {
    msgQuestion(
        this, ui->leMsgTitle->text(), ui->leMsgText->text(), getMsgButtons(),
        QMessageBox::StandardButton(ui->cbMsgDefButton->currentData().toInt()));
}

void TestForm::on_btnWarning_clicked() {
    msgWarning(
        this, ui->leMsgTitle->text(), ui->leMsgText->text(), getMsgButtons(),
        QMessageBox::StandardButton(ui->cbMsgDefButton->currentData().toInt()));
}

void TestForm::on_btnCritical_clicked() {
    msgCritical(
        this, ui->leMsgTitle->text(), ui->leMsgText->text(), getMsgButtons(),
        QMessageBox::StandardButton(ui->cbMsgDefButton->currentData().toInt()));
}

void TestForm::on_btnAbout_clicked() {
    msgAbout(this, ui->leMsgTitle->text(), ui->leMsgText->text());
}

void TestForm::on_btnMsgBox_clicked() {
    msgbox(
        this, ui->cbMsgIcon->currentData().value<QMessageBox::Icon>(),
        ui->leMsgTitle->text(), ui->leMsgText->text(), getMsgButtons(),
        QMessageBox::StandardButton(ui->cbMsgDefButton->currentData().toInt()));
}

void TestForm::on_btnText_clicked() {
    bool ok = false;
    auto ret =
        dlgGetText(this, ui->leInputTitle->text(), ui->leInputLabel->text(),
                   QLineEdit::Normal, __FUNCTION__, &ok);
    ui->tbInputLogger->append(
        QStringLiteral("[getText] ( ") %
        (ok ? QStringLiteral("true") : QStringLiteral("false")) %
        QStringLiteral(" ) ") % ret);
}

void TestForm::on_btnMultiLineText_clicked() {
    bool ok = false;
    auto ret = dlgGetMultiLineText(this, ui->leInputTitle->text(),
                                   ui->leInputLabel->text(), __FUNCTION__, &ok);
    ui->tbInputLogger->append(
        QStringLiteral("[getText] ( ") %
        (ok ? QStringLiteral("true") : QStringLiteral("false")) %
        QStringLiteral(" ) ") % ret);
}

void TestForm::on_btnItem_clicked() {
    QStringList l;
    for (int i = 0; i < WingHex::SDKVERSION; ++i) {
        l.append(QStringLiteral("WingSummer WingHex2 - %1").arg(i));
    }
    bool ok = false;
    auto ret = dlgGetItem(this, ui->leInputTitle->text(),
                          ui->leInputLabel->text(), l, 0, true, &ok);
    ui->tbInputLogger->append(
        QStringLiteral("[getItem] ( ") %
        (ok ? QStringLiteral("true") : QStringLiteral("false")) %
        QStringLiteral(" ) ") % ret);
}

void TestForm::on_btnInt_clicked() {
    bool ok = false;
    auto ret =
        dlgGetInt(this, ui->leInputTitle->text(), ui->leInputLabel->text(), 0,
                  0, WingHex::SDKVERSION, 1, &ok);
    ui->tbInputLogger->append(
        QStringLiteral("[getInt] ( ") %
        (ok ? QStringLiteral("true") : QStringLiteral("false")) %
        QStringLiteral(" ) ") % QString::number(ret));
}

void TestForm::on_btnDouble_clicked() {
    bool ok = false;
    auto ret =
        dlgGetDouble(this, ui->leInputTitle->text(), ui->leInputLabel->text(),
                     QLineEdit::Normal, -double(WingHex::SDKVERSION), 0.0,
                     double(WingHex::SDKVERSION), &ok);
    ui->tbInputLogger->append(
        QStringLiteral("[getDouble] ( ") %
        (ok ? QStringLiteral("true") : QStringLiteral("false")) %
        QStringLiteral(" ) ") % QString::number(ret));
}

void TestForm::on_btnExistingDirectory_clicked() {
    auto ret = dlgGetExistingDirectory(this, ui->leFileCaption->text(),
                                       qApp->applicationDirPath(),
                                       getFileDialogOptions());
    ui->tbFileLogger->append(QStringLiteral("[getExistingDirectory] ") % ret);
}

void TestForm::on_btnOpenFileName_clicked() {
    auto ret = dlgGetOpenFileName(
        this, ui->leFileCaption->text(), qApp->applicationDirPath(),
        ui->leFileFilter->text(), nullptr, getFileDialogOptions());
    ui->tbFileLogger->append(QStringLiteral("[getOpenFileName] ") % ret);
}

void TestForm::on_btnOpenFileNames_clicked() {
    auto ret = dlgGetOpenFileNames(
        this, ui->leFileCaption->text(), qApp->applicationDirPath(),
        ui->leFileFilter->text(), nullptr, getFileDialogOptions());
    ui->tbFileLogger->append(QStringLiteral("[getOpenFileName] ") %
                             ret.join(';'));
}

void TestForm::on_btnSaveFileName_clicked() {
    auto ret = dlgGetSaveFileName(
        this, ui->leFileCaption->text(), qApp->applicationDirPath(),
        ui->leFileFilter->text(), nullptr, getFileDialogOptions());
    ui->tbFileLogger->append(QStringLiteral("[getSaveFileName] ") % ret);
}

void TestForm::on_btnGetColor_clicked() {
    auto ret = dlgGetColor(ui->leColorCaption->text(), this);
    if (ret.isValid()) {
        ui->wColor->setStyleSheet(QStringLiteral("background-color:") +
                                  ret.name());
    } else {
        ui->wColor->setStyleSheet({});
    }
}

void TestForm::on_btnStatusVisible_clicked() {
    if (ui->rbLockedFile->isChecked()) {
        Q_UNUSED(setLockedFile(true));
    } else if (ui->rbAddressVisible->isChecked()) {
        Q_UNUSED(setAddressVisible(true));
    } else if (ui->rbHeaderVisible->isChecked()) {
        Q_UNUSED(setHeaderVisible(true));
    } else if (ui->rbKeepSize->isChecked()) {
        Q_UNUSED(setKeepSize(true));
    } else if (ui->rbStringVisible->isChecked()) {
        Q_UNUSED(setStringVisible(true));
    }
}

void TestForm::on_btnStatusInvisible_clicked() {
    if (ui->rbLockedFile->isChecked()) {
        Q_UNUSED(setLockedFile(false));
    } else if (ui->rbAddressVisible->isChecked()) {
        Q_UNUSED(setAddressVisible(false));
    } else if (ui->rbHeaderVisible->isChecked()) {
        Q_UNUSED(setHeaderVisible(false));
    } else if (ui->rbKeepSize->isChecked()) {
        Q_UNUSED(setKeepSize(false));
    } else if (ui->rbStringVisible->isChecked()) {
        Q_UNUSED(setStringVisible(false));
    }
}
