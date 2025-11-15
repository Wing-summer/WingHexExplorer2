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

#include "splashdialog.h"
#include "ui_splashdialog.h"

#include <QTimer>

SplashDialog::SplashDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::SplashDialog) {
    ui->setupUi(this);
    ui->label->setText(
        QStringLiteral("<html><head/><body><p><span style=\" font-size:16pt; "
                       "font-weight:600;\">%1</span></p></body></html>")
            .arg(qAppName()));
    setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);
    show();
}

SplashDialog::~SplashDialog() { delete ui; }

void SplashDialog::setInfoText(const QString &text) {
    ui->lblinfo->setText(text + QStringLiteral("..."));
    qApp->processEvents();
}

void SplashDialog::cancel() { close(); }
