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

#include "pluginpaintcontext.h"

#include <stdexcept>

#include <QThread>

PluginPaintContext::PluginPaintContext(QPainter *painter, QObject *parent)
    : QObject(parent), m_painter(painter) {}

QPainter *PluginPaintContext::checked() const {
    if (!m_painter || thread() != QThread::currentThread()) {
        throw std::runtime_error("Paint callback has expired");
    }
    return m_painter;
}

void PluginPaintContext::drawText(int x, int y, const QString &text) {
    checked()->drawText(x, y, text);
}

void PluginPaintContext::drawLine(int x1, int y1, int x2, int y2) {
    checked()->drawLine(x1, y1, x2, y2);
}

void PluginPaintContext::fillRect(int x, int y, int width, int height,
                                  const QColor &color) {
    checked()->fillRect(x, y, width, height, color);
}

void PluginPaintContext::setPen(const QColor &color) {
    checked()->setPen(color);
}

void PluginPaintContext::expire() { m_painter = nullptr; }
