/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#include "encodingdialog.h"

#include "../utilities.h"
#include <QAction>
#include <QLabel>
#include <QListWidgetItem>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>

EncodingDialog::EncodingDialog(QWidget *parent) : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    this->setFixedSize(500, 600);
    auto l = new QLabel(tr("ChooseEncoding"), this);
    layout->addWidget(l);
    layout->addSpacing(5);
    enclist = new QListWidget(this);
    for (auto &item : Utilities::getEncodings()) {
        enclist->addItem(item);
    }
    layout->addWidget(enclist);
    layout->addSpacing(10);
    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this,
            &EncodingDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this,
            &EncodingDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &EncodingDialog::on_accept);
    layout->addWidget(dbbox);

    buildUpContent(widget);
    this->setWindowTitle(tr("Encoding"));
}

QString EncodingDialog::getResult() { return result; }

void EncodingDialog::on_accept() {
    auto s = enclist->selectedItems();
    if (s.count() > 0) {
        result = s.first()->text();
        done(1);
    } else {
        done(0);
    }
}

void EncodingDialog::on_reject() { done(0); }
