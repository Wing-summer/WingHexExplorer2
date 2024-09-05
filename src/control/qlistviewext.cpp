#include "qlistviewext.h"

#include <QScrollBar>

QListViewExt::QListViewExt(QWidget *parent) : QListView(parent) {
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

void QListViewExt::setModel(QAbstractItemModel *model) {
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

    QListView::setModel(model);
}
