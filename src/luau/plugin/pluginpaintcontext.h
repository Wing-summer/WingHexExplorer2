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

#ifndef PLUGINPAINTCONTEXT_H
#define PLUGINPAINTCONTEXT_H

#include <QColor>
#include <QObject>
#include <QPainter>
#include <QRect>

class PluginPaintContext final : public QObject {
    Q_OBJECT
public:
    explicit PluginPaintContext(QPainter *painter, QObject *parent);

public:
    void drawText(int x, int y, const QString &text);
    void drawLine(int x1, int y1, int x2, int y2);
    void fillRect(int x, int y, int width, int height, const QColor &color);
    void setPen(const QColor &color);
    void expire();

private:
    QPainter *checked() const;
    QPainter *m_painter;
};
#endif // PLUGINPAINTCONTEXT_H
