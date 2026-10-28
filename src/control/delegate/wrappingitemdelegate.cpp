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

#include "wrappingitemdelegate.h"

#include <QAbstractTextDocumentLayout>
#include <QEvent>
#include <QListView>
#include <QPainter>
#include <QStyle>
#include <QTextDocument>
#include <QTextOption>
#include <algorithm>
#include <cmath>

namespace {
QRect textArea(const QStyleOptionViewItem &option, const QListView *view) {
    auto area = view->style()->subElementRect(QStyle::SE_ItemViewItemText,
                                              &option, view);
    area = area.intersected(option.rect).adjusted(2, 0, -2, 0);
    area.setTop(option.rect.top() + 2);
    area.setBottom(option.rect.bottom() - 2);
    return area;
}
void prepareText(QTextDocument &document, const QStyleOptionViewItem &option,
                 int width) {
    document.setDocumentMargin(0);
    document.setDefaultFont(option.font);
    auto textOption = document.defaultTextOption();
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    document.setDefaultTextOption(textOption);
    document.setPlainText(option.text);
    document.setTextWidth(std::max(1, width));
}
} // namespace

WrappingItemDelegate::WrappingItemDelegate(QListView *view)
    : QStyledItemDelegate(view), m_view(view) {
    m_relayout.setSingleShot(true);
    connect(&m_relayout, &QTimer::timeout, view, &QListView::doItemsLayout);
    view->viewport()->installEventFilter(this);
}

QSize WrappingItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const {
    auto item = option;
    initStyleOption(&item, index);
    item.decorationAlignment = Qt::AlignLeft | Qt::AlignTop;
    const int width =
        std::max(1, m_view->viewport()->width() - 2 * m_view->spacing());
    item.rect = QRect(0, 0, width, item.fontMetrics.height() + 4);
    QTextDocument document;
    prepareText(document, item, textArea(item, m_view).width());
    const int height = std::max(int(std::ceil(document.size().height())),
                                item.decorationSize.height()) +
                       4;
    const auto styled = m_view->style()->sizeFromContents(
        QStyle::CT_ItemViewItem, &item, QSize(width, height), m_view);
    // Some styles recompute CT_ItemViewItem from the original unbroken text.
    // Never let that override the wrapped document's actual height.
    return QSize(width, std::max(height, styled.height()));
}

void WrappingItemDelegate::paint(QPainter *painter,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const {
    auto item = option;
    initStyleOption(&item, index);
    item.decorationAlignment = Qt::AlignLeft | Qt::AlignTop;
    const auto area = textArea(item, m_view);
    QTextDocument document;
    prepareText(document, item, area.width());
    QAbstractTextDocumentLayout::PaintContext context;
    context.palette = item.palette;
    if (item.state.testFlag(QStyle::State_Selected)) {
        context.palette.setColor(QPalette::Text,
                                 item.palette.color(QPalette::HighlightedText));
    }
    item.text.clear();
    m_view->style()->drawControl(QStyle::CE_ItemViewItem, &item, painter,
                                 m_view);
    painter->save();
    painter->setClipRect(option.rect, Qt::IntersectClip);
    painter->translate(area.topLeft());
    document.documentLayout()->draw(painter, context);
    painter->restore();
}

bool WrappingItemDelegate::eventFilter(QObject *object, QEvent *event) {
    if (object == m_view->viewport() && event->type() == QEvent::Resize) {
        m_relayout.start(0);
    }
    return QStyledItemDelegate::eventFilter(object, event);
}
