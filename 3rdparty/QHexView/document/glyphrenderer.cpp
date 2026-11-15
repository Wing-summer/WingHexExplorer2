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

#include "glyphrenderer.h"

#include <QFontDatabase>
#include <QGlyphRun>
#include <QPainter>
#include <QPainterPath>

namespace {
constexpr qreal OutlineWidth = 0.35;
}

void GlyphRenderer::clearCache() {
    m_cellFonts.clear();
    m_scriptFamilies.clear();
}

QRawFont GlyphRenderer::fontForCharacter(const QFont &font,
                                         char32_t codepoint) const {
    const auto key = font.key();
    auto *fonts = m_cellFonts.object(key);
    if (!fonts) {
        fonts = new CellFonts;
        fonts->families.insert(QString(), QRawFont::fromFont(font));
        m_cellFonts.insert(key, fonts);
    }
    if (auto *cached = fonts->characters.object(codepoint)) {
        return *cached;
    }

    auto rawFont = fonts->families.value(QString());
    if (!rawFont.supportsCharacter(codepoint)) {
        rawFont = {};
        const auto script = QChar::script(codepoint);
        auto families = m_scriptFamilies.constFind(script);
        if (families == m_scriptFamilies.cend()) {
            QStringList candidates;
            // Qt exposes writing systems rather than a script-to-family API.
            // Derive the preferred groups from its Unicode sample strings.
            if (script != QChar::Script_Common &&
                script != QChar::Script_Inherited &&
                script != QChar::Script_Unknown) {
                for (auto system : QFontDatabase::writingSystems()) {
                    const auto sample =
                        QFontDatabase::writingSystemSample(system);
                    for (auto character : sample.toUcs4()) {
                        if (QChar::script(character) == script) {
                            candidates.append(QFontDatabase::families(system));
                            break;
                        }
                    }
                }
            }
            candidates.append(QFontDatabase::families());
            candidates.removeDuplicates();
            m_scriptFamilies.insert(script, candidates);
            families = m_scriptFamilies.constFind(script);
        }
        auto candidates = font.families();
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        candidates.append(
            QFontDatabase::applicationFallbackFontFamilies(script));
#endif
        candidates.append(*families);
        for (const auto &family : candidates) {
            auto candidate = fonts->families.constFind(family);
            if (candidate == fonts->families.cend()) {
                auto fallback = font;
                fallback.setFamily(family);
                fallback.setStyleName(QString());
                fonts->families.insert(family, QRawFont::fromFont(fallback));
                candidate = fonts->families.constFind(family);
            }
            if (candidate->supportsCharacter(codepoint)) {
                rawFont = *candidate;
                break;
            }
        }
    }
    // Cache misses too: unsupported characters in binary data are common.
    fonts->characters.insert(codepoint, new QRawFont(rawFont));
    return rawFont;
}

void GlyphRenderer::drawGlyph(QPainter *painter, const QRectF &rect,
                              const QRawFont &font, char32_t codepoint,
                              const GlyphStyle &fmt) const {
    const auto glyphs =
        font.glyphIndexesForString(QString::fromUcs4(&codepoint, 1));
    const auto advances = font.advancesForGlyphIndexes(glyphs);
    QList<QPointF> positions;
    QPointF advance;
    for (const auto &offset : advances) {
        positions.append(advance);
        advance += offset;
    }
    const QPointF origin(rect.center().x() - advance.x() / 2,
                         rect.center().y() +
                             (font.ascent() - font.descent()) / 2);
    if (fmt.outline.isValid()) {
        QPainterPath path;
        for (qsizetype i = 0; i < glyphs.size(); ++i) {
            path.addPath(font.pathForGlyph(glyphs.at(i))
                             .translated(origin + positions.at(i)));
        }
        // Bitmap/color fonts may not supply an outline. Keep their glyphs
        // visible using the native glyph renderer instead.
        if (!path.isEmpty()) {
            if (fmt.strikeOut) {
                path.addRect(origin.x(), origin.y() - font.ascent() / 3,
                             advance.x(), font.lineThickness());
            }
            painter->setPen(QPen(fmt.outline, OutlineWidth));
            painter->setBrush(fmt.foreground);
            painter->drawPath(path);
            return;
        }
    }
    QGlyphRun run;
    run.setRawFont(font);
    run.setGlyphIndexes(glyphs);
    run.setPositions(positions);
    run.setStrikeOut(fmt.strikeOut);
    painter->setPen(fmt.foreground);
    painter->drawGlyphRun(origin, run);
}
