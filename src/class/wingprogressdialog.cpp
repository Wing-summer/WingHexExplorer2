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

#include "wingprogressdialog.h"

WingProgressDialog::WingProgressDialog(const QString &labelText,
                                       const QString &cancelButtonText,
                                       int minimum, int maximum,
                                       QWidget *parent) {
    m_dialog = new QProgressDialog(labelText, cancelButtonText, minimum,
                                   maximum, parent);
    m_dialog->setWindowFlag(Qt::Widget);

    m_d = new FramelessDialogBase(parent);
    m_d->buildUpContent(m_dialog);
    m_d->setWindowTitle(labelText);

    QObject::connect(m_dialog, &QProgressDialog::finished, m_d,
                     &FramelessDialogBase::done);
}

WingProgressDialog::~WingProgressDialog() {
    delete m_dialog;
    delete m_d;
}

FramelessDialogBase *WingProgressDialog::pdialog() const { return m_d; }

QProgressDialog *WingProgressDialog::dialog() const { return m_dialog; }
