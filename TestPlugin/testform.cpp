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
#include "ui_testform.h"

#include "ctltestform.h"
#include "readertestform.h"
#include "testtablemodel.h"

#include <QAbstractTableModel>
#include <QFileSystemModel>
#include <QMetaEnum>
#include <QScrollArea>
#include <QStandardItemModel>

TestForm::TestForm(WingHex::IWingPlugin *plg, QWidget *parent)
    : QWidget(parent), ui(new Ui::TestForm), _plg(plg) {
    ui->setupUi(this);

    ui->teDataVisual->setAcceptRichText(false);

    ui->saReader->widget()->layout()->addWidget(
        new ReaderTestForm(_plg, ui->tbReaderLogger, this));
    ui->saCtl->widget()->layout()->addWidget(
        new CtlTestForm(_plg, ui->tbCtlLogger, this));

    ui->spltReader->setSizes({300, 150});
    ui->spltCtl->setSizes({300, 150});

    initLogCombo();
    initStyleCombo();
    initMsgBoxBtnCombo();
    initMsgBoxCheckedBtnCombo();
    initMsgBoxIconCombo();
    initFileDialogOps();

    _click = std::bind(&TestForm::onDVClicked, this, std::placeholders::_1);
    _dblclick =
        std::bind(&TestForm::onDVDoubleClicked, this, std::placeholders::_1);
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

void TestForm::onDVClicked(const QModelIndex &index) {
    emit _plg->warn(QStringLiteral("[Test - Click] ") +
                    index.model()->data(index).toString());
}

void TestForm::onDVDoubleClicked(const QModelIndex &index) {
    emit _plg->msgbox.warning(this, QStringLiteral("Test - DoubleClick"),
                              index.model()->data(index).toString());
}

void TestForm::on_btnSendLog_clicked() {
    auto txt = ui->leLogText->text();
    switch (Level(ui->cbLogLevel->currentIndex())) {
    case q1ERROR:
        emit _plg->error(txt);
        break;
    case q2WARN:
        emit _plg->warn(txt);
        break;
    case q3INFO:
        emit _plg->info(txt);
        break;
    case q4DEBUG:
        emit _plg->debug(txt);
        break;
    case q5TRACE:
        emit _plg->trace(txt);
        break;
    default:
        break;
    }
}

void TestForm::on_btnSendToast_clicked() {
    auto idx = ui->cbToastIcon->currentIndex();
    Q_ASSERT(idx >= 0);
    auto icon = ui->cbToastIcon->itemData(idx).value<QPixmap>();
    emit _plg->toast(icon, ui->leToastText->text());
}

void TestForm::on_btnAboutQt_clicked() {
    emit _plg->msgbox.aboutQt(this, ui->leAboutTitle->text());
}

void TestForm::on_btnQuestion_clicked() {
    emit _plg->msgbox.question(
        this, ui->leMsgTitle->text(), ui->leMsgText->text(), getMsgButtons(),
        QMessageBox::StandardButton(ui->cbMsgDefButton->currentData().toInt()));
}

void TestForm::on_btnWarning_clicked() {
    emit _plg->msgbox.warning(
        this, ui->leMsgTitle->text(), ui->leMsgText->text(), getMsgButtons(),
        QMessageBox::StandardButton(ui->cbMsgDefButton->currentData().toInt()));
}

void TestForm::on_btnCritical_clicked() {
    emit _plg->msgbox.critical(
        this, ui->leMsgTitle->text(), ui->leMsgText->text(), getMsgButtons(),
        QMessageBox::StandardButton(ui->cbMsgDefButton->currentData().toInt()));
}

void TestForm::on_btnAbout_clicked() {
    emit _plg->msgbox.about(this, ui->leMsgTitle->text(),
                            ui->leMsgText->text());
}

void TestForm::on_btnMsgBox_clicked() {
    emit _plg->msgbox.msgbox(
        this, ui->cbMsgIcon->currentData().value<QMessageBox::Icon>(),
        ui->leMsgTitle->text(), ui->leMsgText->text(), getMsgButtons(),
        QMessageBox::StandardButton(ui->cbMsgDefButton->currentData().toInt()));
}

void TestForm::on_btnText_clicked() {
    bool ok = false;
    auto ret = emit _plg->inputbox.getText(
        this, ui->leInputTitle->text(), ui->leInputLabel->text(),
        QLineEdit::Normal, __FUNCTION__, &ok);
    ui->tbInputLogger->append(
        QStringLiteral("[getText] ( ") %
        (ok ? QStringLiteral("true") : QStringLiteral("false")) %
        QStringLiteral(" ) ") % ret);
}

void TestForm::on_btnMultiLineText_clicked() {
    bool ok = false;
    auto ret = emit _plg->inputbox.getMultiLineText(
        this, ui->leInputTitle->text(), ui->leInputLabel->text(), __FUNCTION__,
        &ok);
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
    auto ret =
        emit _plg->inputbox.getItem(this, ui->leInputTitle->text(),
                                    ui->leInputLabel->text(), l, 0, true, &ok);
    ui->tbInputLogger->append(
        QStringLiteral("[getItem] ( ") %
        (ok ? QStringLiteral("true") : QStringLiteral("false")) %
        QStringLiteral(" ) ") % ret);
}

void TestForm::on_btnInt_clicked() {
    bool ok = false;
    auto ret = emit _plg->inputbox.getInt(this, ui->leInputTitle->text(),
                                          ui->leInputLabel->text(), 0, 0,
                                          WingHex::SDKVERSION, 1, &ok);
    ui->tbInputLogger->append(
        QStringLiteral("[getInt] ( ") %
        (ok ? QStringLiteral("true") : QStringLiteral("false")) %
        QStringLiteral(" ) ") % QString::number(ret));
}

void TestForm::on_btnDouble_clicked() {
    bool ok = false;
    auto ret = emit _plg->inputbox.getDouble(
        this, ui->leInputTitle->text(), ui->leInputLabel->text(),
        QLineEdit::Normal, -double(WingHex::SDKVERSION), 0.0,
        double(WingHex::SDKVERSION), &ok);
    ui->tbInputLogger->append(
        QStringLiteral("[getDouble] ( ") %
        (ok ? QStringLiteral("true") : QStringLiteral("false")) %
        QStringLiteral(" ) ") % QString::number(ret));
}

void TestForm::on_btnExistingDirectory_clicked() {
    auto ret = emit _plg->filedlg.getExistingDirectory(
        this, ui->leFileCaption->text(), qApp->applicationDirPath(),
        getFileDialogOptions());
    ui->tbFileLogger->append(QStringLiteral("[getExistingDirectory] ") % ret);
}

void TestForm::on_btnOpenFileName_clicked() {
    auto ret = emit _plg->filedlg.getOpenFileName(
        this, ui->leFileCaption->text(), qApp->applicationDirPath(),
        ui->leFileFilter->text(), nullptr, getFileDialogOptions());
    ui->tbFileLogger->append(QStringLiteral("[getOpenFileName] ") % ret);
}

void TestForm::on_btnOpenFileNames_clicked() {
    auto ret = emit _plg->filedlg.getOpenFileNames(
        this, ui->leFileCaption->text(), qApp->applicationDirPath(),
        ui->leFileFilter->text(), nullptr, getFileDialogOptions());
    ui->tbFileLogger->append(QStringLiteral("[getOpenFileName] ") %
                             ret.join(';'));
}

void TestForm::on_btnSaveFileName_clicked() {
    auto ret = emit _plg->filedlg.getSaveFileName(
        this, ui->leFileCaption->text(), qApp->applicationDirPath(),
        ui->leFileFilter->text(), nullptr, getFileDialogOptions());
    ui->tbFileLogger->append(QStringLiteral("[getSaveFileName] ") % ret);
}

void TestForm::on_btnGetColor_clicked() {
    auto ret = emit _plg->colordlg.getColor(ui->leColorCaption->text(), this);
    if (ret.isValid()) {
        ui->wColor->setStyleSheet(QStringLiteral("background-color:") +
                                  ret.name());
    } else {
        ui->wColor->setStyleSheet({});
    }
}

void TestForm::on_btnText_2_clicked() {
    emit _plg->visual.updateText(ui->teDataVisual->toPlainText());
}

void TestForm::on_btnTextList_clicked() {
    auto txts = ui->teDataVisual->toPlainText().split('\n');
    emit _plg->visual.updateTextList(txts, _click, _dblclick);
}

void TestForm::on_btnTextTree_clicked() {
    auto ret = emit _plg->visual.updateTextTree(ui->teDataVisual->toPlainText(),
                                                _click, _dblclick);
    if (!ret) {
        emit _plg->msgbox.critical(this, QStringLiteral("Test"),
                                   tr("UpdateTextTreeError"));
    }
}

void TestForm::on_btnTextTable_clicked() {
    auto ret = emit _plg->visual.updateTextTable(
        ui->teDataVisual->toPlainText(),
        {WingHex::WINGSUMMER, WingHex::WINGSUMMER}, {}, _click, _dblclick);
    if (!ret) {
        emit _plg->msgbox.critical(this, QStringLiteral("Test"),
                                   tr("UpdateTextTreeError"));
    }
}

void TestForm::on_btnTextListByModel_clicked() {
    auto model = new QStringListModel;
    QStringList buffer;
    for (int i = 0; i < WingHex::SDKVERSION; ++i) {
        buffer.append(WingHex::WINGSUMMER % QString::number(i));
    }
    model->setStringList(buffer);
    auto ret =
        emit _plg->visual.updateTextListByModel(model, _click, _dblclick);
    if (!ret) {
        emit _plg->msgbox.critical(this, QStringLiteral("Test"),
                                   tr("UpdateTextListByModelError"));
    }
}

void TestForm::on_btnTextTableByModel_clicked() {
    auto model = new TestTableModel;
    auto ret =
        emit _plg->visual.updateTextTableByModel(model, _click, _dblclick);
    if (!ret) {
        emit _plg->msgbox.critical(this, QStringLiteral("Test"),
                                   tr("UpdateTextTableByModelError"));
    }
}

void TestForm::on_btnTextTreeByModel_clicked() {
    auto model = new QFileSystemModel;
    model->setRootPath(QDir::currentPath());
    auto ret =
        emit _plg->visual.updateTextTreeByModel(model, _click, _dblclick);
    if (!ret) {
        emit _plg->msgbox.critical(this, QStringLiteral("Test"),
                                   tr("UpdateTextTreeByModelError"));
    }
}
