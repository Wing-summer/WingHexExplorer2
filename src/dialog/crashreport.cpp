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

#include "crashreport.h"
#include "ui_crashreport.h"

#include <QClipboard>
#include <QPushButton>

#include "class/settings/thememanager.h"
#include "control/toast.h"
#include "utilities.h"

CrashReport::CrashReport(QWidget *parent)
    : QWidget(parent), ui(new Ui::CrashReport) {
    ui->setupUi(this);
    setWindowIcon(ThemeManager::instance().standardIcon(
        ThemeManager::StandardIcon::Crash));

    m_dialog = new FramelessDialogBase(parent);
    m_dialog->buildUpContent(this);
    m_dialog->setWindowTitle(this->windowTitle());
    m_dialog->setMinimumSize(600, 500);

    Utilities::moveToCenter(m_dialog);
}

CrashReport::~CrashReport() { delete ui; }

void CrashReport::setInfomation(const QString &info) {
    ui->textBrowser->setText(info);
}

int CrashReport::exec() { return m_dialog->exec(); }

void CrashReport::on_buttonBox_clicked(QAbstractButton *button) {
    if (button == ui->buttonBox->button(QDialogButtonBox::Save)) {
        auto clipboard = qApp->clipboard();
        clipboard->setText(ui->textBrowser->toPlainText());
        Toast::toast(m_dialog,
                     ThemeManager::instance().standardPixmap(
                         ThemeManager::StandardIcon::Copy),
                     tr("Copy to clipboard"));
    } else {
        m_dialog->done(1);
    }
}
