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

#include "settingdialog.h"
#include "ui_settingdialog.h"

#include "class/wingmessagebox.h"
#include "control/toast.h"
#include "utilities.h"

#include <QApplication>
#include <QPushButton>

SettingDialog::SettingDialog(QWidget *parent)
    : QWidget(parent), ui(new Ui::SettingDialog) {
    ui->setupUi(this);

    _dialog = new FramelessDialogBase(parent);
    _dialog->buildUpContent(this);
    _dialog->setWindowTitle(this->windowTitle());
    connect(_dialog, &FramelessDialogBase::rejected, this, [=] {
        for (auto &page : m_pages) {
            page->cancel();
        }
    });

    Utilities::moveToCenter(this);
}

SettingDialog::~SettingDialog() {
    delete ui;
    _dialog->deleteLater();
}

void SettingDialog::addPage(WingHex::SettingPage *page) {
    if (page == nullptr) {
        return;
    }
    m_pages << page;
}

void SettingDialog::build() {
    for (auto &page : m_pages) {
        auto name = page->name();
        auto icon = page->categoryIcon();
        auto item = new QListWidgetItem(icon, name);
        item->setSizeHint(QSize(0, 40));
        ui->listWidget->addItem(item);
        ui->stackedWidget->addWidget(page);
    }
    if (m_pages.size()) {
        ui->listWidget->setCurrentRow(0);
    }
}

void SettingDialog::showConfig(int index) {
    if (index >= 0 && index < m_pages.size()) {
        ui->listWidget->setCurrentRow(index);
    }
    Utilities::moveToCenter(this);
    _dialog->exec();
}

void SettingDialog::showConfig(const QString &id) {
    if (id.isEmpty()) {
        ui->listWidget->setCurrentRow(0);
    }
    auto r = std::find_if(
        m_pages.begin(), m_pages.end(),
        [id](const WingHex::SettingPage *page) { return page->id() == id; });
    if (r == m_pages.end()) {
        ui->listWidget->setCurrentRow(0);
    }
    ui->listWidget->setCurrentRow(m_pages.indexOf(*r));
    Utilities::moveToCenter(this);
    _dialog->exec();
}

void SettingDialog::toastTakeEffectReboot() {
    auto page = qobject_cast<WingHex::SettingPage *>(sender());
    if (page) {
        auto icon = page->categoryIcon();
        auto avsize = icon.availableSizes();
        Q_ASSERT(!avsize.isEmpty());
        Toast::toast(_dialog, icon.pixmap(avsize.first()),
                     tr("TakeEffectRestart"));
    }
}

void SettingDialog::on_buttonBox_clicked(QAbstractButton *button) {
    auto btnbox = ui->buttonBox;
    if (button == btnbox->button(QDialogButtonBox::Ok)) {
        for (auto &page : m_pages) {
            page->apply();
        }
        _dialog->done(1);
    } else if (button == btnbox->button(QDialogButtonBox::Apply)) {
        for (auto &page : m_pages) {
            page->apply();
        }
    } else if (button == btnbox->button(QDialogButtonBox::RestoreDefaults)) {
        for (auto &page : m_pages) {
            page->reset();
        }

    } else if (button == btnbox->button(QDialogButtonBox::Reset)) {
        auto res = WingMessageBox::warning(
            this, qAppName(),
            tr("This will reset all settings. Are you sure to continue?"),
            QMessageBox::Yes | QMessageBox::No);
        if (res == QMessageBox::No)
            return;

        for (auto &page : m_pages) {
            page->reset();
        }
    } else if (button == btnbox->button(QDialogButtonBox::Cancel)) {
        for (auto &page : m_pages) {
            page->cancel();
        }
        _dialog->done(0);
    }
}

void SettingDialog::on_listWidget_currentRowChanged(int currentRow) {
    ui->stackedWidget->setCurrentWidget(m_pages.at(currentRow));
}
