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

#include <QAction>
#include <QLabel>
#include <QListWidgetItem>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>

#include "class/settings/thememanager.h"
#include "utilities.h"

EncodingDialog::EncodingDialog(QStringConverter::Encoding enc, QWidget *parent)
    : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    this->setFixedSize(500, 600);
    auto l = new QLabel(tr("Select text encoding"), this);
    layout->addWidget(l);
    layout->addSpacing(5);
    m_enclist = new QListWidget(this);
    m_enclist->addItems(Utilities::getEncodings());
    m_enclist->setCurrentRow(enc);

    layout->addWidget(m_enclist);
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
    this->setWindowIcon(ThemeManager::instance().standardIcon(
        ThemeManager::StandardIcon::Encoding));
    this->setWindowTitle(tr("Encoding"));
}

QStringConverter::Encoding EncodingDialog::getResult() { return m_result; }

void EncodingDialog::on_accept() {
    auto row = m_enclist->currentRow();
    if (row >= 0) {
        m_result = QStringConverter::Encoding(row);
        done(1);
    } else {
        done(0);
    }
}

void EncodingDialog::on_reject() { done(0); }
