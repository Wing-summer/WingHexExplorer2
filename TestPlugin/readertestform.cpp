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

#include "readertestform.h"
#include "ui_readertestform.h"

#include <QStringBuilder>

ReaderTestForm::ReaderTestForm(WingHex::IWingPlugin *plg, QTextBrowser *br,
                               QWidget *parent)
    : QWidget(parent), ui(new Ui::ReaderTestForm), _plg(plg), _br(br) {
    ui->setupUi(this);
    ui->sbOffset->setRange(0, INT_MAX);
}

ReaderTestForm::~ReaderTestForm() { delete ui; }

void ReaderTestForm::on_btnReadInt8_clicked() {
    auto v = emit _plg->reader.readInt8(ui->sbOffset->value());
    _br->append(QStringLiteral("[ReadInt8] ") + QString::number(v));
}

void ReaderTestForm::on_btnReadInt16_clicked() {
    auto v = emit _plg->reader.readInt16(ui->sbOffset->value());
    _br->append(QStringLiteral("[ReadInt16] ") + QString::number(v));
}

void ReaderTestForm::on_btnReadInt32_clicked() {
    auto v = emit _plg->reader.readInt32(ui->sbOffset->value());
    _br->append(QStringLiteral("[ReadInt32] ") + QString::number(v));
}

void ReaderTestForm::on_btnReadInt64_clicked() {
    auto v = emit _plg->reader.readInt64(ui->sbOffset->value());
    _br->append(QStringLiteral("[ReadInt64] ") + QString::number(v));
}

void ReaderTestForm::on_btnReadFloat_clicked() {
    auto v = emit _plg->reader.readFloat(ui->sbOffset->value());
    _br->append(QStringLiteral("[ReadFloat] ") + QString::number(v));
}

void ReaderTestForm::on_btnReadDouble_clicked() {
    auto v = emit _plg->reader.readDouble(ui->sbOffset->value());
    _br->append(QStringLiteral("[ReadDouble] ") + QString::number(v));
}

void ReaderTestForm::on_btnReadString_clicked() {
    auto v = emit _plg->reader.readString(ui->sbOffset->value());
    _br->append(QStringLiteral("[ReadString] ") + v);
}

void ReaderTestForm::on_btnReadByteArray_clicked() {
    auto v =
        emit _plg->reader.readBytes(ui->sbOffset->value(), ui->sbLen->value());
    _br->append(QStringLiteral("[ReadByteArray] ") + v.toHex(' '));
}

void ReaderTestForm::on_btnStatus_clicked() {
    static auto lf = QStringLiteral("\n");
    static auto strue = QStringLiteral("true");
    static auto sfalse = QStringLiteral("false");
    _br->clear();
    _br->append(QStringLiteral("[Status]") % lf %
                QStringLiteral("getSupportedEncodings: ") %
                (emit _plg->reader.getSupportedEncodings().join(';')) % lf %
                QStringLiteral("currentEncoding: ") %
                emit _plg->reader.currentEncoding() % lf %
                QStringLiteral("isCurrentDocEditing: ") %
                (emit _plg->reader.isCurrentDocEditing() ? strue : sfalse) %
                lf % QStringLiteral("currentDocFilename: ") %
                emit _plg->reader.currentDocFilename() % lf %
                QStringLiteral("isReadOnly: ") %
                (emit _plg->reader.isReadOnly() ? strue : sfalse) % lf %
                QStringLiteral("isKeepSize: ") %
                (emit _plg->reader.isKeepSize() ? strue : sfalse) % lf %
                QStringLiteral("isLocked: ") %
                (emit _plg->reader.isLocked() ? strue : sfalse) % lf %
                QStringLiteral("isModified: ") %
                (emit _plg->reader.isModified() ? strue : sfalse) % lf %
                QStringLiteral("stringVisible: ") %
                (emit _plg->reader.stringVisible() ? strue : sfalse) % lf %
                QStringLiteral("addressVisible: ") %
                (emit _plg->reader.addressVisible() ? strue : sfalse) % lf %
                QStringLiteral("headerVisible: ") %
                (emit _plg->reader.headerVisible() ? strue : sfalse) % lf %
                QStringLiteral("addressBase: ") % QStringLiteral("0x") %
                QString::number(emit _plg->reader.addressBase(), 16).toUpper() %
                lf % QStringLiteral("documentLines: ") %
                QString::number(emit _plg->reader.documentLines()) % lf %
                QStringLiteral("documentBytes: ") %
                QString::number(emit _plg->reader.documentBytes()) % lf %
                QStringLiteral("currentPos: ") %
                getPrintableHexPosition(emit _plg->reader.currentPos()) % lf %
                QStringLiteral("currentRow: ") %
                QString::number(emit _plg->reader.currentRow()) % lf %
                QStringLiteral("currentColumn: ") %
                QString::number(emit _plg->reader.currentColumn()) % lf %
                QStringLiteral("currentOffset: ") %
                QString::number(emit _plg->reader.currentOffset()) % lf %
                QStringLiteral("selectedLength: ") %
                QString::number(emit _plg->reader.selectedLength()));

    _br->append(QStringLiteral("[Selection]"));
    auto total = emit _plg->reader.selectionCount();
    _br->append(QStringLiteral("selectionCount: ") % QString::number(total));
    for (decltype(total) i = 0; i < total; ++i) {
        _br->append(QStringLiteral("{ ") % QString::number(i) %
                    QStringLiteral(" }"));
        _br->append(
            QStringLiteral("selectionStart: ") %
            getPrintableHexPosition(emit _plg->reader.selectionStart(i)) % lf %
            QStringLiteral("selectionEnd: ") %
            getPrintableHexPosition(emit _plg->reader.selectionEnd(i)) % lf %
            QStringLiteral("selectionLength: ") %
            QString::number(emit _plg->reader.selectionLength(i)) % lf %
            QStringLiteral("selectedBytes: ") %
            (emit _plg->reader.selectedBytes(i)).toHex(' '));
    }

    _br->append(QStringLiteral("[Selections]"));
    _br->append((emit _plg->reader.selectionBytes()).join('\n').toHex(' '));
}

QString
ReaderTestForm::getPrintableHexPosition(const WingHex::HexPosition &pos) {
    return QStringLiteral("WingHex::HexPosition { line = ") %
           QString::number(pos.line) % QStringLiteral(", column = ") %
           QString::number(pos.column) % QStringLiteral(", lineWidth = ") %
           QString::number(pos.lineWidth) % QStringLiteral(", nibbleindex = ") %
           QString::number(pos.nibbleindex) % QStringLiteral(" }");
}
