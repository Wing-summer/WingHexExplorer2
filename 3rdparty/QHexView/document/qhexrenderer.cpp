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
**
** The original License is MIT from Dax89/QHexView. I have modified a lot so I
** decide to change the Open Source License. You can use the original library
** under MIT. Thanks for Dax89's efforts.
** =============================================================================
*/

#include "qhexrenderer.h"
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QWidget>
#include <cctype>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringDecoder>
#else
#include <QTextCodec>
#endif

#define HEX_UNPRINTABLE_CHAR '.'

constexpr qreal CONTRASTING_COLOR_BORDER = 0.35;

/*===================================*/
// added by wingsummer

bool QHexRenderer::stringVisible() { return m_asciiVisible; }

void QHexRenderer::setStringVisible(bool b) {
    m_asciiVisible = b;
    Q_EMIT m_document->documentChanged();
}

bool QHexRenderer::headerVisible() { return m_headerVisible; }

void QHexRenderer::setHeaderVisible(bool b) {
    m_headerVisible = b;
    Q_EMIT m_document->documentChanged();
}

bool QHexRenderer::addressVisible() { return m_addressVisible; }

void QHexRenderer::setAddressVisible(bool b) {
    m_addressVisible = b;
    Q_EMIT m_document->documentChanged();
}

void QHexRenderer::switchDoc(QHexDocument *doc) {
    if (doc)
        m_document = doc;
}

/*===================================*/

QHexRenderer::QHexRenderer(QHexDocument *document, QHexCursor *cursor,
                           const QFontMetricsF &fontmetrics, QObject *parent)
    : QObject(parent), m_document(document), m_cursor(cursor),
      m_fontmetrics(fontmetrics) {
    m_selectedarea = QHexRenderer::HexArea;
    m_cursorenabled = false;

    /*===================================*/
    // added by wingsummer

    m_asciiVisible = true;
    m_addressVisible = true;
    m_headerVisible = true;

    /*===================================*/
}

// modified by wingsummer
void QHexRenderer::renderFrame(QPainter *painter) {
    QRect rect = painter->window();
    int hexx = this->getHexColumnX();
    int asciix = this->getAsciiColumnX();
    int endx = this->getEndColumnX();

    painter->setPen(m_borderColor);

    // x coordinates are in absolute space
    // y coordinates are in viewport space
    // see QHexView::paintEvent where the painter has been shifted horizontally

    auto h = this->headerLineCount() * this->lineHeight();

    if (m_headerVisible)
        painter->drawLine(0, h, endx, h);
    if (m_addressVisible)
        painter->drawLine(hexx, rect.top(), hexx, rect.bottom());

    painter->drawLine(asciix, rect.top(), asciix, rect.bottom());

    if (m_asciiVisible)
        painter->drawLine(endx, rect.top(), endx, rect.bottom());
}

void QHexRenderer::renderAdditonalFrame(QPainter *painter, bool top,
                                        bool left) {
    QRect rect = painter->window();
    int endx = this->getEndColumnX();
    painter->setPen(m_borderColor);
    if (top) {
        painter->drawLine(0, 0, endx, 0);
    }
    if (left) {
        painter->drawLine(0, 0, 0, rect.bottom());
    }
}

// modified by wingsummer
void QHexRenderer::render(QPainter *painter, qsizetype begin, qsizetype end,
                          qsizetype firstline) {
    if (m_headerVisible)
        this->drawHeader(painter);

    auto documentLines = this->documentLines();
    for (qsizetype line = begin; line < std::min(end, documentLines); line++) {
        QRect linerect = this->getLineRect(line, firstline);
        if (line % 2)
            painter->fillRect(linerect, m_bytesBackground);
        else
            painter->fillRect(linerect, m_bytesAlterBackground);

        if (m_addressVisible)
            this->drawAddress(painter, linerect, line);

        this->drawHex(painter, linerect, line);

        if (m_asciiVisible)
            this->drawString(painter, linerect, line);
    }
}

void QHexRenderer::updateMetrics(const QFontMetricsF &fm) {
    m_fontmetrics = fm;
}
void QHexRenderer::enableCursor(bool b) { m_cursorenabled = b; }

void QHexRenderer::selectArea(const QPoint &pt) {
    int area = this->hitTestArea(pt);
    if (!this->editableArea(area))
        return;

    m_selectedarea = area;
}

bool QHexRenderer::hitTest(const QPoint &pt, QHexPosition *position,
                           qsizetype firstline) const {
    int area = this->hitTestArea(pt);
    if (!this->editableArea(area))
        return false;

    position->line =
        std::min(firstline + pt.y() / this->lineHeight() - headerLineCount(),
                 this->documentLastLine());
    position->lineWidth = quint8(this->hexLineWidth());

    auto hspace = m_fontmetrics.horizontalAdvance(' ') / 2;

    if (area == QHexRenderer::HexArea) {
        auto relx =
            pt.x() - hspace - this->getHexColumnX() - this->borderSize();
        int column = int(relx / this->getCellWidth());
        position->column = column / 3;
        // first char is nibble 1, 2nd and space are 0
        position->nibbleindex = (column % 3 == 0) ? 1 : 0;
    } else {
        auto relx =
            pt.x() - hspace - this->getAsciiColumnX() - this->borderSize();
        position->column = int(relx / this->getCellWidth());
        position->nibbleindex = 1;
    }

    if (position->line == this->documentLastLine()) // Check last line's columns
    {
        QByteArray ba = this->getLine(position->line);
        position->column = std::min(position->column, int(ba.length()));
    } else
        position->column = std::min(position->column, int(hexLineWidth() - 1));

    return true;
}

int QHexRenderer::hitTestArea(const QPoint &pt) const {
    if (pt.y() < headerLineCount() * lineHeight())
        return QHexRenderer::HeaderArea;

    if ((pt.x() >= this->borderSize()) &&
        (pt.x() <= (this->getHexColumnX() - this->borderSize())))
        return QHexRenderer::AddressArea;

    if ((pt.x() > (this->getHexColumnX() + this->borderSize())) &&
        (pt.x() < (this->getAsciiColumnX() - this->borderSize())))
        return QHexRenderer::HexArea;

    if ((pt.x() > (this->getAsciiColumnX() + this->borderSize())) &&
        (pt.x() < (this->getEndColumnX() - this->borderSize())))
        return QHexRenderer::AsciiArea;

    return QHexRenderer::ExtraArea;
}

int QHexRenderer::selectedArea() const { return m_selectedarea; }
bool QHexRenderer::editableArea(int area) const {
    return (area == QHexRenderer::HexArea || area == QHexRenderer::AsciiArea);
}
qsizetype QHexRenderer::documentLastLine() const {
    return this->documentLines() - 1;
}
qsizetype QHexRenderer::documentLastColumn() const {
    return this->getLine(this->documentLastLine()).length();
}
qsizetype QHexRenderer::documentLines() const {
    auto r = std::lldiv(this->rendererLength(), hexLineWidth());
    return r.quot + (r.rem > 0 ? 1 : 0);
}
int QHexRenderer::documentWidth() const { return this->getEndColumnX(); }
int QHexRenderer::lineHeight() const { return qRound(m_fontmetrics.height()); }

QRect QHexRenderer::getLineRect(qsizetype line, qsizetype firstline) const {
    return QRect(0, int((line - firstline + headerLineCount()) * lineHeight()),
                 this->getEndColumnX(), lineHeight());
}

// modified by wingsummer
int QHexRenderer::headerLineCount() const { return m_headerVisible ? 1 : 0; }

int QHexRenderer::borderSize() const {
    if (m_document)
        return this->getNCellsWidth(m_document->areaIndent());
    return this->getNCellsWidth(DEFAULT_AREA_IDENTATION);
}

int QHexRenderer::hexLineWidth() const {
    if (m_document)
        return m_document->hexLineWidth();
    return DEFAULT_HEX_LINE_LENGTH;
}

QString QHexRenderer::hexString(qsizetype line, QByteArray *rawline) const {
    QByteArray lrawline = this->getLine(line);
    if (rawline)
        *rawline = lrawline;

    return toHexSequence(lrawline);
}

// modified by wingsummer
QString QHexRenderer::decodeString(qsizetype line, QByteArray *rawline) const {
    QByteArray lrawline = this->getLine(line);
    if (rawline)
        *rawline = lrawline;

    QByteArray ascii = lrawline;
    this->unprintableChars(ascii);
    return ascii;
}

QByteArray QHexRenderer::getLine(qsizetype line) const {
    return m_document->read(line * hexLineWidth(), hexLineWidth());
}

void QHexRenderer::blinkCursor() { m_cursorenabled = !m_cursorenabled; }

qsizetype QHexRenderer::rendererLength() const {
    return m_document->length() + 1;
}

// modified by wingsummer
int QHexRenderer::getAddressWidth() const {
    auto base = m_document->baseAddress();
    quint64 maxAddr = base + quint64(this->rendererLength());
    if (base <= 0xFFFFFFFF && maxAddr <= 0xFFFFFFFF)
        return 8;
    else
        return 16;
    return QString::number(maxAddr, 16).length();
}

int QHexRenderer::getHexColumnX() const {
    if (m_addressVisible) {
        return this->getNCellsWidth(this->getAddressWidth()) +
               2 * this->borderSize();
    }
    return 0;
}
int QHexRenderer::getAsciiColumnX() const {
    return this->getHexColumnX() + this->getNCellsWidth(hexLineWidth() * 3) +
           2 * this->borderSize();
}

int QHexRenderer::getEndColumnX() const {
    if (m_asciiVisible) {
        return this->getAsciiColumnX() + this->getNCellsWidth(hexLineWidth()) +
               2 * this->borderSize();
    }
    return this->getAsciiColumnX();
}

qreal QHexRenderer::getCellWidth() const {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    return m_fontmetrics.horizontalAdvance(" ");
#else
    return m_fontmetrics.width(" ");
#endif
}

int QHexRenderer::getNCellsWidth(int n) const {
    return qRound(n * getCellWidth());
}

void QHexRenderer::unprintableChars(QByteArray &ascii) const {
    for (char &ch : ascii) {
        if (std::isprint(static_cast<unsigned char>(ch)))
            continue;

        ch = HEX_UNPRINTABLE_CHAR;
    }
}

QByteArray QHexRenderer::toHexSequence(const QByteArray &arr) {
    if (arr.isEmpty()) {
        return QByteArray(1, '\t');
    }
    const int length = arr.size() * 4;
    QByteArray hex(length, Qt::Uninitialized);
    char *hexData = hex.data();
    const uchar *data = (const uchar *)arr.data();
    for (int i = 0, o = 1; i < arr.size(); ++i) {
        hexData[o++] = toHexUpper(data[i] >> 4);
        hexData[o++] = toHexUpper(data[i] & 0xf);
        if (o < length) {
            hexData[o++] = '\t';
        }
        if (o < length) {
            hexData[o++] = '\t';
        }
    }

    hex.front() = '\t';
    return hex;
}

char QHexRenderer::toHexUpper(uint value) noexcept {
    return "0123456789ABCDEF"[value & 0xF];
}

void QHexRenderer::applyDocumentStyles(QPainter *painter,
                                       QTextDocument *textdocument) const {
    textdocument->setDocumentMargin(0);
    textdocument->setUndoRedoEnabled(false);
    auto font = painter->font();
    textdocument->setDefaultFont(font);
    auto textopt = textdocument->defaultTextOption();
    textopt.setTabStopDistance(QFontMetricsF(font).horizontalAdvance(' ') / 2);
    textdocument->setDefaultTextOption(textopt);
}

void QHexRenderer::applyBasicStyle(QTextCursor &textcursor,
                                   const QByteArray &rawline,
                                   Factor factor) const {
    QColor color = m_bytesColor;

    if (color.lightness() < 50) {
        if (color == Qt::black)
            color = Qt::gray;
        else
            color = color.darker();
    } else
        color = color.lighter();

    QTextCharFormat charformat;
    charformat.setForeground(color);

    for (int i = 0; i < rawline.length(); i++) {
        if ((rawline[i] != 0x00) && (uchar(rawline[i]) != 0xFF))
            continue;

        textcursor.setPosition(i * factor);
        textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                factor);
        textcursor.mergeCharFormat(charformat);
    }
}

void QHexRenderer::applyMetadata(QTextCursor &textcursor, qsizetype line,
                                 Factor factor) const {
    QHexMetadata *metadata = m_document->metadata();

    if (!metadata->lineHasMetadata(line))
        return;

    const QHexLineMetadata &linemetadata = metadata->gets(line);
    for (auto &mi : linemetadata) {
        QTextCharFormat charformat;

        QColor bg, fg;
        if (m_document->metabgVisible() && mi.background.isValid() &&
            mi.background.alpha()) {
            charformat.setBackground(mi.background);
            bg = mi.background;
        }
        if (m_document->metafgVisible() && mi.foreground.isValid() &&
            mi.foreground.alpha()) {
            charformat.setForeground(mi.foreground);
            fg = mi.foreground;
        }
        if (m_document->metaCommentVisible() && !mi.comment.isEmpty()) {
            charformat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            charformat.setToolTip(mi.comment);
        }

        if (!bg.isValid()) {
            bg = m_bytesBackground;
        }
        if (!fg.isValid()) {
            fg = m_bytesColor;
        }

        if (!QHexMetadata::areColorsContrast(bg, fg)) {
            charformat.setFontWeight(QFont::Bold);
            charformat.setTextOutline(
                QPen(QHexMetadata::generateContrastingColor(bg),
                     CONTRASTING_COLOR_BORDER, Qt::SolidLine));
        }

        textcursor.setPosition(int(mi.start * factor));
        textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                int((mi.length * factor)));
        textcursor.mergeCharFormat(charformat);
    }
}

void QHexRenderer::applySelection(QTextCursor &textcursor, qsizetype line,
                                  Factor factor) const {
    if (!m_cursor->isLineSelected(line))
        return;

    auto total = m_cursor->selectionCount();
    for (int i = 0; i < total; ++i) {
        applySelection(m_cursor->selection(i), textcursor, line, factor, false,
                       false);
    }

    if (m_cursor->hasPreviewSelection()) {
        applySelection(
            m_cursor->previewSelection(), textcursor, line, factor,
            m_cursor->previewSelectionMode() == QHexCursor::SelectionRemove,
            m_cursor->previewSelectionMode() == QHexCursor::SelectionNormal &&
                m_cursor->hasInternalSelection());
    }
}

void QHexRenderer::applySelection(const QHexSelection &selection,
                                  QTextCursor &textcursor, qsizetype line,
                                  Factor factor, bool strikeOut,
                                  bool hasSelection) const {
    if (!selection.isLineSelected(line)) {
        return;
    }

    QTextCharFormat charfmt;
    charfmt.setBackground(strikeOut || hasSelection
                              ? m_selBackgroundColor.darker()
                              : m_selBackgroundColor);
    charfmt.setForeground(strikeOut ? m_selectionColor.darker()
                                    : m_selectionColor);
    charfmt.setFontStrikeOut(strikeOut);
    charfmt.setFontItalic(strikeOut);

    QTextCharFormat charfmt_meta = charfmt;
    charfmt_meta.setFontWeight(QFont::Bold);
    charfmt_meta.setFontUnderline(true);
    charfmt_meta.setFontItalic(true);

    const QHexPosition &startsel = selection.begin;
    const QHexPosition &endsel = selection.end;

    QHexSelection lineSel;
    lineSel.begin.line = line;
    lineSel.begin.lineWidth = startsel.lineWidth;
    lineSel.begin.column = 0;
    lineSel.begin.nibbleindex = 0;
    lineSel.end = lineSel.begin;
    lineSel.end.column = startsel.lineWidth - 1;

    QVector<QHexMetadata::MetaInfo> metas;
    if (lineSel.isIntersected(selection)) {
        lineSel.intersect(selection);
        metas = m_document->metadata()->getRealMetaRange(lineSel.begin.offset(),
                                                         lineSel.end.offset());
    }

    qsizetype begin;
    qsizetype end;

    if (startsel.line == endsel.line) {
        begin = startsel.column;
        end = endsel.column;
    } else {
        if (line == startsel.line)
            begin = startsel.column;
        else
            begin = 0;

        if (line == endsel.line)
            end = endsel.column;
        else
            end = startsel.lineWidth - 1;
    }

    applySelection(metas, textcursor, line * startsel.lineWidth, begin, end,
                   factor, strikeOut, hasSelection);
}

void QHexRenderer::applySelection(const QVector<QHexMetadata::MetaInfo> &metas,
                                  QTextCursor &textcursor,
                                  qsizetype startLineOffset,
                                  qsizetype lineStart, qsizetype lineEnd,
                                  Factor factor, bool strikeOut,
                                  bool hasSelection) const {
    auto totallen = lineEnd - lineStart + 1;

    QTextCharFormat charfmt;
    charfmt.setBackground(strikeOut || hasSelection
                              ? m_selBackgroundColor.darker()
                              : m_selBackgroundColor);
    charfmt.setForeground(strikeOut ? m_selectionColor.darker()
                                    : m_selectionColor);
    charfmt.setFontStrikeOut(strikeOut);
    charfmt.setFontItalic(strikeOut);

    if (!metas.isEmpty()) {
        auto fmtBegin = lineStart;
        for (int i = 0; i < metas.size(); ++i) {
            QTextCharFormat charfmt_meta = charfmt;
            charfmt_meta.setFontWeight(QFont::Bold);
            charfmt_meta.setFontItalic(true);

            auto mi = metas.at(i);
            auto begin = mi.begin - startLineOffset;
            auto mlen = mi.end - mi.begin + 1;

            auto blen = begin - fmtBegin;

            textcursor.setPosition(fmtBegin * factor);
            textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                    blen * factor);

            textcursor.mergeCharFormat(charfmt);
            textcursor.clearSelection();
            totallen -= blen;

            textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                    mlen * factor);

            if (!strikeOut) {
                QColor fg, bg = charfmt.background().color();

                if (m_document->metafgVisible() && mi.foreground.isValid() &&
                    mi.foreground.alpha()) {
                    fg = mi.foreground.darker();
                    charfmt_meta.setForeground(fg);
                }

                if (m_document->metaCommentVisible() && !mi.comment.isEmpty()) {
                    charfmt_meta.setUnderlineStyle(
                        QTextCharFormat::SingleUnderline);
                }

                if (!fg.isValid()) {
                    fg = m_selectionColor;
                }

                if (!QHexMetadata::areColorsContrast(bg, fg)) {
                    charfmt_meta.setFontWeight(QFont::Bold);
                    charfmt_meta.setTextOutline(
                        QPen(QHexMetadata::generateContrastingColor(bg),
                             CONTRASTING_COLOR_BORDER, Qt::SolidLine));
                }
            }

            textcursor.mergeCharFormat(charfmt_meta);
            textcursor.clearSelection();
            totallen -= mlen;
            fmtBegin = mi.end - startLineOffset + 1;
        }

        if (totallen > 0) {
            textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                    totallen * factor);
            textcursor.mergeCharFormat(charfmt);
            textcursor.clearSelection();
        }
    } else {
        textcursor.setPosition(lineStart * factor);
        textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                totallen * factor);
        textcursor.mergeCharFormat(charfmt);
    }
}

void QHexRenderer::applyCursorAscii(QTextCursor &textcursor,
                                    qsizetype line) const {
    if ((line != m_cursor->currentLine()) || !m_cursorenabled)
        return;

    textcursor.clearSelection();
    textcursor.setPosition(m_cursor->currentColumn());
    textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);

    QTextCharFormat charformat;

    if ((m_cursor->insertionMode() == QHexCursor::OverwriteMode) ||
        (m_selectedarea != QHexRenderer::AsciiArea)) {
        charformat.setForeground(m_bytesBackground);
        if (m_selectedarea == QHexRenderer::AsciiArea)
            charformat.setBackground(m_bytesColor);
        else
            charformat.setBackground(m_bytesColor.lighter(250));
    } else
        charformat.setUnderlineStyle(
            QTextCharFormat::UnderlineStyle::SingleUnderline);

    textcursor.mergeCharFormat(charformat);
}

void QHexRenderer::applyCursorHex(QTextCursor &textcursor,
                                  qsizetype line) const {
    if ((line != m_cursor->currentLine()) || !m_cursorenabled)
        return;

    textcursor.clearSelection();

    auto col = m_cursor->currentColumn();
    textcursor.setPosition(col * Factor::Hex);

    if (m_selectedarea == QHexRenderer::HexArea) {
        if (m_cursor->currentNibble()) {
            textcursor.movePosition(QTextCursor::Right,
                                    QTextCursor::MoveAnchor);
        } else {
            textcursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor,
                                    2);
        }
    }

    textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);

    if (m_selectedarea == QHexRenderer::AsciiArea)
        textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    QTextCharFormat charformat;

    if ((m_cursor->insertionMode() == QHexCursor::OverwriteMode) ||
        (m_selectedarea != QHexRenderer::HexArea)) {
        charformat.setForeground(m_bytesBackground);
        if (m_selectedarea == QHexRenderer::HexArea)
            charformat.setBackground(m_bytesColor);
        else
            charformat.setBackground(m_bytesColor.lighter(250));
    } else
        charformat.setUnderlineStyle(
            QTextCharFormat::UnderlineStyle::SingleUnderline);

    textcursor.setCharFormat(charformat);
}

void QHexRenderer::drawAddress(QPainter *painter, const QRect &linerect,
                               qsizetype line) {
    auto addr = quintptr(line * hexLineWidth()) + m_document->baseAddress();
    QString addrStr =
        QString::number(addr, 16)
            .rightJustified(this->getAddressWidth(), QLatin1Char('0'))
            .toUpper();

    QRect addressrect = linerect;
    addressrect.setWidth(this->getHexColumnX());

    painter->save();
    painter->setPen(m_addressColor);
    painter->drawText(addressrect, Qt::AlignHCenter | Qt::AlignVCenter,
                      addrStr);
    painter->restore();
}

void QHexRenderer::drawHex(QPainter *painter, const QRect &linerect,
                           qsizetype line) {
    QTextDocument textdocument;
    QTextCursor textcursor(&textdocument);
    QByteArray rawline;

    textcursor.insertText(this->hexString(line, &rawline));

    if (line == this->documentLastLine())
        textcursor.insertText(QStringLiteral(" "));

    QRect hexrect = linerect;
    hexrect.setX(this->getHexColumnX() + this->borderSize());

    this->applyDocumentStyles(painter, &textdocument);
    this->applyBasicStyle(textcursor, rawline, Hex);

    auto dis = !m_document->metabgVisible() && !m_document->metafgVisible() &&
               !m_document->metaCommentVisible();
    if (!dis)
        this->applyMetadata(textcursor, line, Hex);

    this->applySelection(textcursor, line, Hex);
    this->applyCursorHex(textcursor, line);

    painter->save();
    painter->translate(hexrect.topLeft());

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette.setColor(QPalette::Text, m_bytesColor);
    textdocument.documentLayout()->draw(painter, ctx);

    this->applyBookMark(painter, textcursor, line, Hex);
    painter->restore();
}

void QHexRenderer::applyBookMark(QPainter *painter, QTextCursor &textcursor,
                                 qsizetype line, Factor factor) {

    if (!m_document->lineHasBookMark(line))
        return;

    painter->save();

    auto pos = m_document->getLineBookmarksPos(line);
    for (auto &item : pos) {
        auto off = item % hexLineWidth();

        qreal begin, width;
        auto height = lineHeight();

        // add some paddings
        if (factor == Hex) {
            begin = getCellWidth() * off * 3 + 1;
            begin += getCellWidth() / 2;
            width = getCellWidth() * 2 + 2;
            textcursor.setPosition(off * factor);
        } else {
            begin = getCellWidth() * off + 1;
            width = getCellWidth();
            textcursor.setPosition(m_cursor->currentColumn());
        }

        auto charformat = textcursor.charFormat();
        auto textOutline = charformat.textOutline();

        constexpr auto ALPHA = 180;

        if (textOutline.style() != Qt::NoPen) {
            auto outColor = textOutline.color();
            outColor.setAlpha(ALPHA);
            QPen pen(outColor, 1, Qt::DotLine);
            painter->setPen(pen);
        } else {
            if (m_cursor->currentLine() == line &&
                m_cursor->currentColumn() == off) {
                auto color = m_bytesColor;
                color.setAlpha(ALPHA);
                QPen pen(color, 1, Qt::DotLine);
                painter->setPen(pen);
            } else {
                auto foreground = charformat.foreground();
                if (foreground.style() != Qt::NoBrush) {
                    auto textColor = foreground.color();
                    textColor.setAlpha(ALPHA);
                    QPen pen(textColor, 1, Qt::DotLine);
                    painter->setPen(pen);
                } else {
                    auto color = m_bytesColor;
                    color.setAlpha(ALPHA);
                    QPen pen(color, 1, Qt::DotLine);
                    painter->setPen(pen);
                }
            }
        }

        painter->setBrush(Qt::transparent);
        painter->setBackground(Qt::transparent);
        painter->drawRect(begin, 0, width, height);
    }

    painter->restore();
}

void QHexRenderer::drawString(QPainter *painter, const QRect &linerect,
                              qsizetype line) {
    QTextDocument textdocument;
    QTextCursor textcursor(&textdocument);
    QByteArray rawline;
    // modified by wingsummer
    textcursor.insertText(this->decodeString(line, &rawline));

    if (line == this->documentLastLine())
        textcursor.insertText(" ");

    QRect asciirect = linerect;
    asciirect.setX(this->getAsciiColumnX() + this->borderSize());

    this->applyDocumentStyles(painter, &textdocument);
    this->applyBasicStyle(textcursor, rawline, String);

    auto dis = !m_document->metabgVisible() && !m_document->metafgVisible() &&
               !m_document->metaCommentVisible();
    if (!dis)
        this->applyMetadata(textcursor, line, String);

    this->applySelection(textcursor, line, String);
    this->applyCursorAscii(textcursor, line);

    painter->save();
    painter->translate(asciirect.topLeft());

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette.setColor(QPalette::Text, m_bytesColor);
    textdocument.documentLayout()->draw(painter, ctx);

    this->applyBookMark(painter, textcursor, line, String);
    painter->restore();
}

void QHexRenderer::drawHeader(QPainter *painter) {
    QRect rect = QRect(0, 0, this->getEndColumnX(),
                       this->headerLineCount() * this->lineHeight());

    auto len = this->hexLineWidth();
    QString hexheader;
    hexheader.reserve(len * 3);
    for (quint8 i = 0; i < len; i++) {
        hexheader.append(toHexUpper(i >> 4));
        hexheader.append(toHexUpper(i & 0xf));
        hexheader.append(' ');
    }

    auto font = painter->font();
    auto space = QFontMetricsF(font).horizontalAdvance(' ') / 2;

    QRect addressrect = rect;
    addressrect.setWidth(this->getHexColumnX());

    QRect hexrect = rect;

    hexrect.setX(m_addressVisible
                     ? (this->getHexColumnX() + this->borderSize() + space)
                     : this->borderSize());
    hexrect.setWidth(this->getNCellsWidth(hexLineWidth() * 3));

    QRect asciirect = rect;
    asciirect.setX(this->getAsciiColumnX());
    asciirect.setWidth(this->getEndColumnX() - this->getAsciiColumnX());

    painter->save();
    painter->setPen(m_headerColor);

    if (m_addressVisible)
        painter->drawText(addressrect, Qt::AlignHCenter | Qt::AlignVCenter,
                          QString("Offset"));

    // align left for maximum consistency with drawHex() which prints from the
    // left. so hex and positions are aligned vertically
    painter->drawText(hexrect, Qt::AlignLeft | Qt::AlignVCenter, hexheader);

    if (m_asciiVisible)
        painter->drawText(asciirect, Qt::AlignHCenter | Qt::AlignVCenter,
                          QStringLiteral("ASCII"));
    painter->restore();
}

void QHexRenderer::setBorderColor(const QColor &newBorderColor) {
    m_borderColor = newBorderColor;
}

QColor QHexRenderer::borderColor() const { return m_borderColor; }

QHexCursor *QHexRenderer::cursor() const { return m_cursor; }

void QHexRenderer::setCursor(QHexCursor *newCursor) { m_cursor = newCursor; }

QColor QHexRenderer::bytesColor() const { return m_bytesColor; }

void QHexRenderer::setBytesColor(const QColor &newBytesColor) {
    m_bytesColor = newBytesColor;
}

QColor QHexRenderer::selectionColor() const { return m_selectionColor; }

void QHexRenderer::setSelectionColor(const QColor &newSelectionColor) {
    m_selectionColor = newSelectionColor;
}

QColor QHexRenderer::selBackgroundColor() const { return m_selBackgroundColor; }

void QHexRenderer::setSelBackgroundColor(const QColor &newSelBackgroundColor) {
    m_selBackgroundColor = newSelBackgroundColor;
}

QColor QHexRenderer::bytesAlterBackground() const {
    return m_bytesAlterBackground;
}

void QHexRenderer::setBytesAlterBackground(
    const QColor &newBytesAlterBackground) {
    m_bytesAlterBackground = newBytesAlterBackground;
}

QColor QHexRenderer::bytesBackground() const { return m_bytesBackground; }

void QHexRenderer::setBytesBackground(const QColor &newBytesBackground) {
    m_bytesBackground = newBytesBackground;
}

QColor QHexRenderer::addressColor() const { return m_addressColor; }

void QHexRenderer::setAddressColor(const QColor &newAddressColor) {
    m_addressColor = newAddressColor;
}

QColor QHexRenderer::headerColor() const { return m_headerColor; }

void QHexRenderer::setHeaderColor(const QColor &newHeaderColor) {
    m_headerColor = newHeaderColor;
}
