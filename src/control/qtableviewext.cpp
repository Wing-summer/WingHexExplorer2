#include "qtableviewext.h"

#include <QScrollBar>

QTableViewExt::QTableViewExt(QWidget *parent) : QTableView(parent) {
    connect(verticalScrollBar(), &QScrollBar::rangeChanged, this, [=] {
        if (model()) {
            auto barv = model()->property("barv").toInt();
            verticalScrollBar()->setValue(barv);
        }
    });
}

void QTableViewExt::setModel(QAbstractItemModel *model) {
    auto vbar = verticalScrollBar();
    auto barv = vbar->value();
    if (this->model()) {
        this->model()->setProperty("barv", barv);
    }

    // reset to default range without signal emited
    vbar->blockSignals(true);
    vbar->setRange(0, 0);
    vbar->blockSignals(false);

    QTableView::setModel(model);
}
