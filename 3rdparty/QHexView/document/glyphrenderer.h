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
**
** The original License is MIT from Dax89/QHexView. I have modified a lot so I
** decide to change the Open Source License. You can use the original library
** under MIT. Thanks for Dax89's efforts.
** =============================================================================
*/

#ifndef GLYPHRENDERER_H
#define GLYPHRENDERER_H

#include <QCache>
#include <QColor>
#include <QHash>
#include <QRawFont>
#include <QStringList>

class QPainter;
class QRectF;

struct GlyphStyle {
    QColor foreground;
    QColor outline;
    bool strikeOut = false;
};

// Isolated glyphs for binary-data previews, not natural-language text layout.
// Owned by a GUI-thread view/delegate; QRawFont caches stay on that thread.
class GlyphRenderer {
public:
    void clearCache();
    QRawFont fontForCharacter(const QFont &font, char32_t codepoint) const;
    void drawGlyph(QPainter *painter, const QRectF &rect, const QRawFont &font,
                   char32_t codepoint, const GlyphStyle &style) const;

private:
    struct CellFonts {
        QHash<QString, QRawFont> families;
        QCache<quint32, QRawFont> characters{1024};
    };

    mutable QCache<QString, CellFonts> m_cellFonts{8};
    mutable QHash<int, QStringList> m_scriptFamilies;
};

#endif // GLYPHRENDERER_H
