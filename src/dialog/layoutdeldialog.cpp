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

#include "utilities.h"

#include <QListWidgetItem>

LayoutDelDialog::LayoutDelDialog(const QStringList &layouts, QWidget *parent)
    : QWidget(parent), ui(new Ui::LayoutDelDialog) {
    ui->setupUi(this);

    for (auto &l : layouts) {
        auto lw = new QListWidgetItem(ICONRES(QStringLiteral("layout")), l);
        lw->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable |
                     Qt::ItemIsSelectable);
        lw->setCheckState(Qt::Unchecked);
        ui->lwLayouts->addItem(lw);
    }

    _dialog = new FramelessDialogBase(parent);
    _dialog->buildUpContent(this);
    _dialog->setWindowTitle(this->windowTitle());
    _dialog->setMinimumSize(300, 400);
    Utilities::moveToCenter(_dialog);
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

void LayoutDelDialog::on_buttonBox_accepted() { _dialog->accept(); }

void LayoutDelDialog::on_buttonBox_rejected() { _dialog->reject(); }

int LayoutDelDialog::exec() { return _dialog->exec(); }
