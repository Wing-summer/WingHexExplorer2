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

#include "compositeiconengine.h"

#include <QPainter>

CompositeIconEngine::CompositeIconEngine(const QIcon &main,
                                         const QIcon &overlay)
    : mainIcon(main), overlayIcon(overlay) {}

QIconEngine *CompositeIconEngine::clone() const {
    return new CompositeIconEngine(mainIcon, overlayIcon);
}

void CompositeIconEngine::paint(QPainter *painter, const QRect &rect,
                                QIcon::Mode mode, QIcon::State state) {
    mainIcon.paint(painter, rect, Qt::AlignCenter, mode, state);
    auto w = rect.width() / 2;
    auto h = rect.height() / 2;
    auto x = rect.x() + w;
    auto y = rect.y() + h;
    QRect overlayRect(x, y, w, h);
    overlayIcon.paint(painter, overlayRect, Qt::AlignCenter, mode, state);
}

QPixmap CompositeIconEngine::pixmap(const QSize &size, QIcon::Mode mode,
                                    QIcon::State state) {
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    paint(&painter, QRect(QPoint(0, 0), size), mode, state);
    return pixmap;
}
