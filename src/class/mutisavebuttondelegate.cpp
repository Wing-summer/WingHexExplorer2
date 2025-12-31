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

#include "mutisavebuttondelegate.h"
#include "utilities.h"

#include <QMouseEvent>
#include <QPainter>

MutiSaveButtonDelegate::MutiSaveButtonDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

void MutiSaveButtonDelegate::paint(QPainter *p,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const {
    QStyledItemDelegate::paint(p, option, index);
    auto btnRect = getFileIconRect(option.rect);
    p->drawPixmap(btnRect, NAMEICONRES(QStringLiteral("file")));
}

QSize MutiSaveButtonDelegate::sizeHint(const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const {
    QSize s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(qMax(s.height(), 28));
    return s;
}

bool MutiSaveButtonDelegate::editorEvent(QEvent *event,
                                         QAbstractItemModel *model,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) {
    if (event->type() == QEvent::MouseButtonRelease) {
        auto me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            auto btnRect = getFileIconRect(option.rect);
            if (btnRect.contains(me->pos())) {
                Q_EMIT buttonClicked(index);
                return true;
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QRect MutiSaveButtonDelegate::getFileIconRect(const QRect &rect) {

    constexpr auto padding = 8;
    auto s = rect.height() - padding;
    return QRect(rect.right() - s - padding, rect.center().y() - s / 2, s, s);
}
