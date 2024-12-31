#ifndef RICHTEXTITEMDELEGATE_H
#define RICHTEXTITEMDELEGATE_H

#include <QStyledItemDelegate>

class RichTextItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit RichTextItemDelegate(QObject *parent = nullptr);

    // QAbstractItemDelegate interface
public:
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const override;
};

#endif // RICHTEXTITEMDELEGATE_H
