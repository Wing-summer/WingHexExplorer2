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

#include "checksumdialog.h"
#include "utilities.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidgetItem>
#include <QShortcut>
#include <QVBoxLayout>

CheckSumDialog::CheckSumDialog(QWidget *parent) : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    this->setFixedSize(500, 600);
    auto l = new QLabel(tr("ChooseCheckSum"), this);
    layout->addWidget(l);
    layout->addSpacing(5);
    hashlist = new QListWidget(this);
    for (auto &item : Utilities::supportedHashAlgorithmStringList()) {
        auto w = new QListWidgetItem(item, hashlist);
        w->setCheckState(Qt::Checked);
    }
    layout->addWidget(hashlist);
    layout->addSpacing(10);
    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this,
            &CheckSumDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this,
            &CheckSumDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &CheckSumDialog::on_accept);
    layout->addWidget(dbbox);

    buildUpContent(widget);
    this->setWindowTitle(tr("CheckSum"));
}

const QVector<int> &CheckSumDialog::getResults() { return _result; }

void CheckSumDialog::on_accept() {
    _result.clear();
    for (int i = 0; i < hashlist->count(); ++i) {
        auto item = hashlist->item(i);
        if (item->checkState() == Qt::Checked) {
            _result.append(i);
        }
    }
    done(1);
}

void CheckSumDialog::on_reject() { done(0); }
