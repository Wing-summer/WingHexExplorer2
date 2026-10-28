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

#ifndef WRAPPINGITEMDELEGATE_H
#define WRAPPINGITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QTimer>

class QListView;

// Plain diagnostic text, including long identifiers, wraps to the viewport.
class WrappingItemDelegate : public QStyledItemDelegate {
public:
    explicit WrappingItemDelegate(QListView *view);
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    QListView *m_view;
    QTimer m_relayout;
};
#endif
