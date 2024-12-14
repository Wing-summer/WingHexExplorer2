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

#include <QMetaEnum>

TestForm::TestForm(WingHex::IWingPlugin *plg, QWidget *parent)
    : QWidget(parent), ui(new Ui::TestForm), _plg(plg) {
    ui->setupUi(this);

    initLogCombo();
    initStyleCombo();
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
        auto icon = style->standardIcon(QStyle::StandardPixmap(i));
        if (!icon.isNull()) {
            ui->cbToastIcon->addItem(icon, e.key(i));
        }
    }
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

void TestForm::on_btnSendToast_clicked() {}
