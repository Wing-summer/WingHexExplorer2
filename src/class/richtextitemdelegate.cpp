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
    if (!index.isValid())
        return;

    // Get the rich text content
    QString text = index.data(Qt::DisplayRole).toString();
    if (text.isEmpty())
        return;

    // Get alignment
    QVariant alignmentVariant = index.data(Qt::TextAlignmentRole);
    Qt::Alignment alignment = alignmentVariant.isValid()
                                  ? Qt::Alignment(alignmentVariant.toInt())
                                  : Qt::AlignLeft;

    // Set up a QTextDocument to render the HTML content
    QTextDocument doc;
    doc.setHtml(text);

    // Disable word wrapping
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::NoWrap);
    doc.setDefaultTextOption(textOption);

    // Clip the painter to the cell rectangle
    painter->save();
    painter->setClipRect(option.rect);

    // Calculate the rendering rectangle based on alignment
    QRect rect = option.rect;
    QSize contentSize = doc.size().toSize();
    if (alignment & Qt::AlignHCenter) {
        rect.setLeft(rect.left() + (rect.width() - contentSize.width()) / 2);
    } else if (alignment & Qt::AlignRight) {
        rect.setLeft(rect.right() - contentSize.width());
    }

    if (alignment & Qt::AlignVCenter) {
        rect.setTop(rect.top() + (rect.height() - contentSize.height()) / 2);
    } else if (alignment & Qt::AlignBottom) {
        rect.setTop(rect.bottom() - contentSize.height());
    }

    // Render the document in the adjusted rectangle
    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette = option.widget->palette();

    painter->translate(rect.topLeft());
    doc.setTextWidth(option.rect.width());
    doc.documentLayout()->draw(painter, ctx);

    painter->restore();
}

QSize RichTextItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const {
    QString text = index.data(Qt::DisplayRole).toString();
    if (text.isEmpty())
        return QStyledItemDelegate::sizeHint(option, index);

    QTextDocument doc;
    doc.setHtml(text);
    doc.setTextWidth(option.rect.width());
    return doc.size().toSize();
}
