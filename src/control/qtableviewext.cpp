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

#include "qtableviewext.h"

#include <QScrollBar>

QTableViewExt::QTableViewExt(QWidget *parent) : QTableView(parent) {
    connect(verticalScrollBar(), &QScrollBar::rangeChanged, this, [=] {
        auto model = this->model();
        if (model) {
            auto barv = model->property("barv").toInt();
            auto selv = model->property("selv").value<QItemSelection>();
            verticalScrollBar()->setValue(barv);
            selectionModel()->select(selv, QItemSelectionModel::SelectCurrent);
        }
    });
}

void QTableViewExt::setModel(QAbstractItemModel *model) {
    auto vbar = verticalScrollBar();
    auto barv = vbar->value();

    auto old = this->model();
    if (old) {
        old->setProperty("barv", barv);
        auto selmod = this->selectionModel();
        auto selv = selmod->selection();
        old->setProperty("selv", QVariant::fromValue(selv));
    }

    // reset to default range without signal emited
    vbar->blockSignals(true);
    vbar->setRange(0, 0);
    vbar->blockSignals(false);

    QTableView::setModel(model);

    emit modelChanged();
}
