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

#include <QProgressBar>

WingProgressDialog::WingProgressDialog(QWidget *parent) {
    m_dialog = new QProgressDialog(parent, Qt::Widget);
    auto pb = new QProgressBar(this);
    pb->setTextVisible(false);
    m_dialog->setBar(pb);

    buildUpContent(m_dialog);
    connect(m_dialog, &QProgressDialog::canceled, this,
            &WingProgressDialog::canceled);
    connect(m_dialog, &QProgressDialog::finished, this,
            &FramelessDialogBase::done);
}

WingProgressDialog::WingProgressDialog(const QString &labelText,
                                       const QString &cancelButtonText,
                                       int minimum, int maximum,
                                       QWidget *parent) {
    m_dialog = new QProgressDialog(labelText, cancelButtonText, minimum,
                                   maximum, this);
    m_dialog->setWindowFlag(Qt::Widget);
    auto pb = new QProgressBar(this);
    pb->setTextVisible(false);
    m_dialog->setBar(pb);

    buildUpContent(m_dialog);

    connect(m_dialog, &QProgressDialog::canceled, this,
            &WingProgressDialog::canceled);
    connect(m_dialog, &QProgressDialog::finished, this,
            &FramelessDialogBase::done);
}

WingProgressDialog::~WingProgressDialog() {}

void WingProgressDialog::setLabel(QLabel *label) { m_dialog->setLabel(label); }

void WingProgressDialog::setCancelButton(QPushButton *button) {
    m_dialog->setCancelButton(button);
}

void WingProgressDialog::setBar(QProgressBar *bar) { m_dialog->setBar(bar); }

bool WingProgressDialog::wasCanceled() const { return m_dialog->wasCanceled(); }

int WingProgressDialog::minimum() const { return m_dialog->minimum(); }

int WingProgressDialog::maximum() const { return m_dialog->maximum(); }

int WingProgressDialog::value() const { return m_dialog->value(); }

QString WingProgressDialog::labelText() const { return m_dialog->labelText(); }

int WingProgressDialog::minimumDuration() const {
    return m_dialog->minimumDuration();
}

void WingProgressDialog::setAutoReset(bool reset) {
    m_dialog->setAutoReset(reset);
}

bool WingProgressDialog::autoReset() const { return m_dialog->autoReset(); }

void WingProgressDialog::setAutoClose(bool close) {
    m_dialog->setAutoClose(close);
}

bool WingProgressDialog::autoClose() const { return m_dialog->autoClose(); }

void WingProgressDialog::open(QObject *receiver, const char *member) {
    m_dialog->open(receiver, member);
}

void WingProgressDialog::cancel() { m_dialog->cancel(); }

void WingProgressDialog::reset() { m_dialog->reset(); }

void WingProgressDialog::setMaximum(int maximum) {
    m_dialog->setMaximum(maximum);
}

void WingProgressDialog::setMinimum(int minimum) {
    m_dialog->setMinimum(minimum);
}

void WingProgressDialog::setRange(int minimum, int maximum) {
    m_dialog->setRange(minimum, maximum);
}

void WingProgressDialog::setValue(int progress) {
    m_dialog->setValue(progress);
}

void WingProgressDialog::setLabelText(const QString &text) {
    m_dialog->setLabelText(text);
}

void WingProgressDialog::setCancelButtonText(const QString &text) {
    m_dialog->setCancelButtonText(text);
}

void WingProgressDialog::setMinimumDuration(int ms) {
    m_dialog->setMinimumDuration(ms);
}
