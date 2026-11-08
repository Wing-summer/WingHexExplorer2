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

#include "layoutdeldialog.h"
#include "ui_layoutdeldialog.h"

#include <QListWidgetItem>

#include "class/settings/thememanager.h"
#include "utilities.h"

LayoutDelDialog::LayoutDelDialog(const QStringList &layouts, QWidget *parent)
    : QWidget(parent), ui(new Ui::LayoutDelDialog) {
    ui->setupUi(this);

    for (const auto &l : layouts) {
        auto lw = new QListWidgetItem(ThemeManager::instance().standardIcon(
                                          ThemeManager::StandardIcon::Layout),
                                      l);
        lw->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable |
                     Qt::ItemIsSelectable);
        lw->setCheckState(Qt::Unchecked);
        ui->lwLayouts->addItem(lw);
    }

    m_dialog = new FramelessDialogBase(parent);
    m_dialog->buildUpContent(this);
    m_dialog->setWindowTitle(this->windowTitle());
    m_dialog->setMinimumSize(300, 400);
    Utilities::moveToCenter(m_dialog);
}

LayoutDelDialog::~LayoutDelDialog() { delete ui; }

QStringList LayoutDelDialog::getResult() const {
    QStringList ret;
    auto total = ui->lwLayouts->count();
    for (int i = 0; i < total; ++i) {
        auto item = ui->lwLayouts->item(i);
        auto cs = item->checkState();
        if (cs == Qt::Checked) {
            ret.append(item->text());
        }
    }
    return ret;
}

void LayoutDelDialog::on_buttonBox_accepted() { m_dialog->accept(); }

void LayoutDelDialog::on_buttonBox_rejected() { m_dialog->reject(); }

int LayoutDelDialog::exec() { return m_dialog->exec(); }
