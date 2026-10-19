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

#ifndef MUTISAVEBUTTONDELEGATE_H
#define MUTISAVEBUTTONDELEGATE_H

#include <QStyledItemDelegate>

class MutiSaveButtonDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit MutiSaveButtonDelegate(QObject *parent = nullptr);

    void paint(QPainter *p, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
    static QRect getFileIconRect(const QRect &rect);

signals:
    void buttonClicked(const QModelIndex &index);
};

#endif // MUTISAVEBUTTONDELEGATE_H
