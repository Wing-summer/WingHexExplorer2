/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include "binarytextitemdelegate.h"

#include <QApplication>
#include <QGuiApplication>
#include <QPainter>
#include <QStyle>

namespace {
struct PreviewGlyph {
    QRawFont font;
    char32_t codepoint;
    QSizeF size;
};

struct PreviewText {
    QList<PreviewGlyph> glyphs;
    QSizeF size;
};

PreviewText prepareText(const GlyphRenderer &renderer, const QFont &font,
                        const QString &text) {
    PreviewText result;
    for (auto &character : text.toUcs4()) {
        char32_t codepoint = QChar::isPrint(character) ? character : U'.';
        auto rawFont = renderer.fontForCharacter(font, codepoint);
        if (!rawFont.isValid()) {
            codepoint = U'.';
            rawFont = renderer.fontForCharacter(font, codepoint);
        }
        const auto indexes =
            rawFont.glyphIndexesForString(QString::fromUcs4(&codepoint, 1));
        qreal width = 0;
        for (auto &advance : rawFont.advancesForGlyphIndexes(indexes)) {
            width += advance.x();
        }
        const auto height = rawFont.ascent() + rawFont.descent();
        result.glyphs.append({rawFont, codepoint, {width, height}});
        result.size.rwidth() += width;
        result.size.setHeight(qMax(result.size.height(), height));
    }
    return result;
}

QStyle *itemStyle(const QStyleOptionViewItem &item) {
    return item.widget ? item.widget->style() : QApplication::style();
}
} // namespace

BinaryTextItemDelegate::BinaryTextItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {
    connect(qGuiApp, &QGuiApplication::fontDatabaseChanged, this,
            [this] { m_glyphRenderer.clearCache(); });
}

QSize BinaryTextItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const {
    auto item = option;
    initStyleOption(&item, index);
    const auto text = prepareText(m_glyphRenderer, item.font, item.text);
    item.text.clear();
    const auto content = text.size.toSize();
    return itemStyle(item)
        ->sizeFromContents(QStyle::CT_ItemViewItem, &item, content, item.widget)
        .expandedTo(content + QSize(6, 2));
}

void BinaryTextItemDelegate::paint(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const {
    auto item = option;
    initStyleOption(&item, index);
    auto text = prepareText(m_glyphRenderer, item.font, item.text);
    item.text.clear();
    auto *style = itemStyle(item);
    const auto area =
        style->subElementRect(QStyle::SE_ItemViewItemText, &item, item.widget);
    style->drawControl(QStyle::CE_ItemViewItem, &item, painter, item.widget);

    if (text.size.width() > area.width() &&
        item.textElideMode != Qt::ElideNone) {
        const auto ellipsis =
            prepareText(m_glyphRenderer, item.font, QString(QChar(0x2026)));
        while (!text.glyphs.isEmpty() &&
               text.size.width() + ellipsis.size.width() > area.width()) {
            const auto removeIndex = item.textElideMode == Qt::ElideLeft ? 0
                                     : item.textElideMode == Qt::ElideMiddle
                                         ? text.glyphs.size() / 2
                                         : text.glyphs.size() - 1;
            text.size.rwidth() -= text.glyphs.takeAt(removeIndex).size.width();
        }
        const auto insertIndex = item.textElideMode == Qt::ElideLeft ? 0
                                 : item.textElideMode == Qt::ElideMiddle
                                     ? (text.glyphs.size() + 1) / 2
                                     : text.glyphs.size();
        for (const auto &glyph : ellipsis.glyphs) {
            text.glyphs.insert(insertIndex, glyph);
        }
        text.size.rwidth() += ellipsis.size.width();
        text.size.setHeight(qMax(text.size.height(), ellipsis.size.height()));
    }
    const auto bounds = QStyle::alignedRect(
        item.direction, item.displayAlignment, text.size.toSize(), area);
    const auto group =
        !(item.state & QStyle::State_Enabled) ? QPalette::Disabled
        : (item.state & QStyle::State_Active) ? QPalette::Active
                                              : QPalette::Inactive;
    const auto role = item.state & QStyle::State_Selected
                          ? QPalette::HighlightedText
                          : QPalette::Text;
    const auto foreground = item.palette.color(group, role);
    painter->save();
    painter->setClipRect(area, Qt::IntersectClip);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);
    qreal x = bounds.x();
    for (const auto &glyph : std::as_const(text.glyphs)) {
        m_glyphRenderer.drawGlyph(
            painter,
            QRectF(x, bounds.y(), glyph.size.width(), text.size.height()),
            glyph.font, glyph.codepoint, {foreground, {}, false});
        x += glyph.size.width();
    }
    painter->restore();
}
