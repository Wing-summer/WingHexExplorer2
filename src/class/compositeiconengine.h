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

#ifndef COMPOSITEICONENGINE_H
#define COMPOSITEICONENGINE_H

#include <QIconEngine>

class CompositeIconEngine : public QIconEngine {
public:
    explicit CompositeIconEngine(const QIcon &main, const QIcon &overlay);

public:
    QIconEngine *clone() const override;

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode,
               QIcon::State state) override;

    QPixmap pixmap(const QSize &size, QIcon::Mode mode,
                   QIcon::State state) override;

private:
    QIcon mainIcon;
    QIcon overlayIcon;
};

#endif // COMPOSITEICONENGINE_H
