#include "richtextitemdelegate.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QPainter>
#include <QTextDocument>

RichTextItemDelegate::RichTextItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

void RichTextItemDelegate::paint(QPainter *painter,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const {
    auto options = option;
    initStyleOption(&options, index);

    painter->save();

    QTextDocument doc;
    doc.setHtml(options.text);

    options.text.clear();
    options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options,
                                         painter);

    // shift text right to make icon visible
    QSize iconSize = options.icon.actualSize(options.rect.size());
    painter->translate(options.rect.left() + iconSize.width(),
                       options.rect.top());
    QRect clip(0, 0, options.rect.width() + iconSize.width(),
               options.rect.height());

    painter->setClipRect(clip);
    QAbstractTextDocumentLayout::PaintContext ctx;
    // set text color to red for selected item
    if (option.state & QStyle::State_Selected)
        ctx.palette.setColor(QPalette::Text, Qt::red);

    ctx.clip = clip;
    doc.documentLayout()->draw(painter, ctx);

    painter->restore();
}

QSize RichTextItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const {
    auto options = option;
    initStyleOption(&options, index);

    QTextDocument doc;
    doc.setHtml(options.text);
    doc.setTextWidth(options.rect.width());
    return QSize(doc.idealWidth(), doc.size().height());
}
