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

#ifndef BINARYTEXTITEMDELEGATE_H
#define BINARYTEXTITEMDELEGATE_H

#include <QStyledItemDelegate>

#include "QHexView/document/glyphrenderer.h"

// Displays decoded binary scalars without natural-language shaping/fallback.
class BinaryTextItemDelegate : public QStyledItemDelegate {
public:
    explicit BinaryTextItemDelegate(QObject *parent = nullptr);

public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    GlyphRenderer m_glyphRenderer;
};

#endif // BINARYTEXTITEMDELEGATE_H
