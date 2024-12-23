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

#include "ctltestform.h"
#include "ui_ctltestform.h"

CtlTestForm::CtlTestForm(WingHex::IWingPlugin *plg, QTextBrowser *br,
                         QWidget *parent)
    : QWidget(parent), ui(new Ui::CtlTestForm), _plg(plg), _br(br) {
    ui->setupUi(this);
}

CtlTestForm::~CtlTestForm() { delete ui; }

void CtlTestForm::on_btnInt8_clicked() {
    bool ok;
    auto ret = emit _plg->inputbox.getInt(this, QStringLiteral("Test"),
                                          tr("PleaseInputInt8"), 0, INT8_MIN,
                                          UINT8_MAX, 1, &ok);
    if (ok) {
        auto buffer = qint8(ret);
        if (ui->rbInsert->isChecked()) {
            ok = insertContent(ui->sbOffset->value(), buffer);
        } else if (ui->rbWrite->isChecked()) {
            ok = writeContent(ui->sbOffset->value(), buffer);
        } else {
            ok = appendContent(buffer);
        }

        if (ok) {
            _br->append(QStringLiteral("[Int8] (true) ") %
                        QString::number(buffer));
        } else {
            _br->append(QStringLiteral("[Int8] (false) ") %
                        QString::number(buffer));
        }
    }
}

void CtlTestForm::on_btnInt16_clicked() {
    bool ok;
    auto ret = emit _plg->inputbox.getInt(this, QStringLiteral("Test"),
                                          tr("PleaseInputInt16"), 0, INT16_MIN,
                                          UINT16_MAX, 1, &ok);
    if (ok) {
        auto buffer = qint16(ret);
        if (ui->rbInsert->isChecked()) {
            ok = insertContent(ui->sbOffset->value(), buffer);
        } else if (ui->rbWrite->isChecked()) {
            ok = writeContent(ui->sbOffset->value(), buffer);
        } else {
            ok = appendContent(buffer);
        }

        if (ok) {
            _br->append(QStringLiteral("[Int16] (true) ") %
                        QString::number(buffer));
        } else {
            _br->append(QStringLiteral("[Int16] (false) ") %
                        QString::number(buffer));
        }
    }
}

void CtlTestForm::on_btnInt32_clicked() {
    bool ok;
    auto ret = emit _plg->inputbox.getInt(this, QStringLiteral("Test"),
                                          tr("PleaseInputInt32"), 0, INT32_MIN,
                                          UINT32_MAX, 1, &ok);
    if (ok) {
        auto buffer = qint32(ret);
        if (ui->rbInsert->isChecked()) {
            ok = insertContent(ui->sbOffset->value(), buffer);
        } else if (ui->rbWrite->isChecked()) {
            ok = writeContent(ui->sbOffset->value(), buffer);
        } else {
            ok = appendContent(buffer);
        }

        if (ok) {
            _br->append(QStringLiteral("[Int32] (true) ") %
                        QString::number(buffer));
        } else {
            _br->append(QStringLiteral("[Int32] (false) ") %
                        QString::number(buffer));
        }
    }
}

void CtlTestForm::on_btnInt64_clicked() {
    bool ok;
    auto ret = emit _plg->inputbox.getText(
        this, QStringLiteral("Test"), tr("PleaseInputInt64"), QLineEdit::Normal,
        QStringLiteral("0"), &ok);
    if (ok) {
        auto buffer = qint64(ret.toULongLong(&ok));
        if (ok) {
            if (ui->rbInsert->isChecked()) {
                ok = insertContent(ui->sbOffset->value(), buffer);
            } else if (ui->rbWrite->isChecked()) {
                ok = writeContent(ui->sbOffset->value(), buffer);
            } else {
                ok = appendContent(buffer);
            }

            if (ok) {
                _br->append(QStringLiteral("[Int64] (true) ") %
                            QString::number(buffer));
            } else {
                _br->append(QStringLiteral("[Int64] (false) ") %
                            QString::number(buffer));
            }
        }
    }
}

void CtlTestForm::on_btnFloat_clicked() {
    bool ok;
    auto limit = std::numeric_limits<float>();
    auto ret = emit _plg->inputbox.getDouble(
        this, QStringLiteral("Test"), tr("PleaseInputFloat"), 0, limit.min(),
        limit.max(), 0.0, &ok);
    if (ok) {
        auto buffer = float(ret);
        if (ui->rbInsert->isChecked()) {
            ok = insertContent(ui->sbOffset->value(), buffer);
        } else if (ui->rbWrite->isChecked()) {
            ok = writeContent(ui->sbOffset->value(), buffer);
        } else {
            ok = appendContent(buffer);
        }

        if (ok) {
            _br->append(QStringLiteral("[Float] (true) ") %
                        QString::number(buffer));
        } else {
            _br->append(QStringLiteral("[Float] (false) ") %
                        QString::number(buffer));
        }
    }
}

void CtlTestForm::on_btnDouble_clicked() {
    bool ok;
    auto limit = std::numeric_limits<double>();
    auto ret = emit _plg->inputbox.getDouble(
        this, QStringLiteral("Test"), tr("PleaseInputFloat"), 0, limit.min(),
        limit.max(), 0.0, &ok);
    if (ok) {
        auto buffer = double(ret);
        if (ui->rbInsert->isChecked()) {
            ok = insertContent(ui->sbOffset->value(), buffer);
        } else if (ui->rbWrite->isChecked()) {
            ok = writeContent(ui->sbOffset->value(), buffer);
        } else {
            ok = appendContent(buffer);
        }

        if (ok) {
            _br->append(QStringLiteral("[Double] (true) ") %
                        QString::number(buffer));
        } else {
            _br->append(QStringLiteral("[Double] (false) ") %
                        QString::number(buffer));
        }
    }
}

void CtlTestForm::on_btnString_clicked() {
    bool ok;
    auto buffer = emit _plg->inputbox.getText(
        this, QStringLiteral("Test"), tr("PleaseInputString"),
        QLineEdit::Normal, WingHex::WINGSUMMER, &ok);
    if (ok) {
        if (ui->rbInsert->isChecked()) {
            ok = insertContent(ui->sbOffset->value(), buffer);
        } else if (ui->rbWrite->isChecked()) {
            ok = writeContent(ui->sbOffset->value(), buffer);
        } else {
            ok = appendContent(buffer);
        }
    }
}

void CtlTestForm::on_btnByteArray_clicked() {
    bool ok;
    auto ret = emit _plg->inputbox.getText(
        this, QStringLiteral("Test"), tr("PleaseInputByteArray(00 23 5A)"),
        QLineEdit::Normal, QStringLiteral("00"), &ok);
    if (ok) {
        auto buffer = QByteArray::fromHex(ret.toUtf8());
        if (buffer.isEmpty()) {
            ok = false;
        } else {
            if (ui->rbInsert->isChecked()) {
                ok = insertContent(ui->sbOffset->value(), buffer);
            } else if (ui->rbWrite->isChecked()) {
                ok = writeContent(ui->sbOffset->value(), buffer);
            } else {
                ok = appendContent(buffer);
            }
        }

        if (ok) {
            _br->append(QStringLiteral("[ByteArray] (true) ") % ret);
        } else {
            _br->append(QStringLiteral("[ByteArray] (false) ") % ret);
        }
    }
}
