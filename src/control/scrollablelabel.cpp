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

#include "scrollablelabel.h"

#include <QPainter>
#include <QScrollBar>
#include <QWheelEvent>

ScrollableLabel::ScrollableLabel(QWidget *parent) : QScrollArea(parent) {
    setupUI();
}

void ScrollableLabel::setText(const QString &text) {
    label.setText(text);
    updateLabelSize();
}

QSize ScrollableLabel::sizeHint() const { return label.sizeHint(); }

QSize ScrollableLabel::minimumSizeHint() const {
    return label.minimumSizeHint();
}

void ScrollableLabel::wheelEvent(QWheelEvent *event) {
    if (shouldScroll()) {
        QPoint delta = event->angleDelta();
        if (!delta.isNull()) {
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() -
                                            delta.y());
            event->accept();
            return;
        }
    }
    event->ignore();
}

void ScrollableLabel::resizeEvent(QResizeEvent *event) {
    QScrollArea::resizeEvent(event);
    updateLabelSize();
    adjustDisplayLogic();
}

void ScrollableLabel::setupUI() {
    setWidgetResizable(false);
    setFrameShape(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    label.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    label.setWordWrap(false);
    label.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    setWidget(&label);
}

void ScrollableLabel::updateLabelSize() {
    QFontMetrics fm(label.font());
    const int textWidth = fm.horizontalAdvance(label.text());
    const int textHeight = fm.height();

    if (textWidth > width()) {
        label.setFixedSize(textWidth, textHeight);
    } else {
        label.setFixedSize(width(), textHeight);
    }
}

bool ScrollableLabel::shouldScroll() const {
    return label.width() > viewport()->width();
}

void ScrollableLabel::adjustDisplayLogic() {
    QFontMetrics fm(label.font());
    const int contentWidth = fm.horizontalAdvance(label.text());
    const int containerWidth = width();

    if (contentWidth > containerWidth) {
        label.setFixedSize(contentWidth, fm.height());
        label.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        horizontalScrollBar()->setValue(0);
    } else {
        label.setFixedSize(containerWidth, fm.height());
        label.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
}
