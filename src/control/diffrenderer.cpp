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

#include "diffrenderer.h"

DiffRenderer::DiffRenderer(QHexDocument *document, QHexCursor *cursor,
                           const QFontMetricsF &metrics, QObject *parent,
                           bool right)
    : QHexRenderer(document, cursor, metrics, parent), m_right(right) {}

void DiffRenderer::setResults(
    std::shared_ptr<const WingDiff::ResultStore> results) {
    m_results = std::move(results);
    Q_EMIT repaintRequested();
}

void DiffRenderer::setDarkTheme(bool dark) {
    m_dark = dark;
    Q_EMIT repaintRequested();
}

void DiffRenderer::applyLineFormats(qsizetype line,
                                    QVector<CellFormat> &formats) const {
    if (!m_results) {
        return;
    }
    const auto offset = quint64(line) * quint64(hexLineWidth());
    const auto results =
        m_results->intersect(offset, quint64(formats.size()), m_right);
    for (const auto &range : results) {
        const auto start = m_right ? range.rightOffset : range.leftOffset;
        const auto length = m_right ? range.rightLength : range.leftLength;
        QColor color;
        switch (range.type) {
        case WingDiff::Type::Insert:
            color = m_dark ? QColor(40, 110, 65) : QColor(185, 235, 195);
            break;
        case WingDiff::Type::Delete:
            color = m_dark ? QColor(130, 50, 55) : QColor(250, 195, 195);
            break;
        default:
            color = m_dark ? QColor(120, 95, 30) : QColor(245, 225, 150);
            break;
        }
        for (int column = 0; column < formats.size(); ++column) {
            const auto pos = offset + quint64(column);
            if ((length && pos >= start && pos - start < length) ||
                (!length && pos == start)) {
                auto &format = formats[column];
                format.background = color;
                format.fillBackground = true;
                format.underline = !length;
            }
        }
    }
}
