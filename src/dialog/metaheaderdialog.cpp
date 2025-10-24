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

#include "metaheaderdialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidgetItem>
#include <QShortcut>
#include <QVBoxLayout>

MetaHeaderDialog::MetaHeaderDialog(const QVector<bool> &origin, QWidget *parent)
    : FramelessDialogBase(parent), _result(origin) {
    QStringList header{tr("Begin"), tr("End"), tr("Foreground"),
                       tr("Background"), tr("Comment")};
    auto total = header.size();
    Q_ASSERT(origin.size() == total);

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    auto l = new QLabel(tr("ChooseHeader"), this);
    layout->addWidget(l);
    layout->addSpacing(5);
    headerlist = new QListWidget(this);

    for (qsizetype i = 0; i < total; ++i) {
        auto w = new QListWidgetItem(header.at(i), headerlist);
        w->setCheckState(origin.at(i) ? Qt::Unchecked : Qt::Checked);
    }

    layout->addWidget(headerlist);
    layout->addSpacing(10);
    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this,
            &MetaHeaderDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this,
            &MetaHeaderDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &MetaHeaderDialog::on_accept);
    layout->addWidget(dbbox);

    buildUpContent(widget);
    this->setWindowTitle(tr("MetaHeader"));
}

const QVector<bool> &MetaHeaderDialog::getResults() { return _result; }

void MetaHeaderDialog::on_accept() {
    for (int i = 0; i < headerlist->count(); ++i) {
        auto item = headerlist->item(i);
        _result[i] = (item->checkState() == Qt::Unchecked);
    }
    done(1);
}

void MetaHeaderDialog::on_reject() { done(0); }
