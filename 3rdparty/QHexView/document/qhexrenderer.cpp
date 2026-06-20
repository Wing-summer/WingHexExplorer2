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

#include <QApplication>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QStringDecoder>
#include <QTextLayout>
#include <QWidget>
#include <cctype>

constexpr char HEX_UNPRINTABLE_CHAR = '.';

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

void QHexRenderer::setStringEncoding(QStringConverter::Encoding encoding) {
    if (m_stringEncoding == encoding) {
        return;
    }

    m_stringEncoding = encoding;
    m_lineCaches.clear();
    Q_EMIT m_document->documentChanged();
}

QStringConverter::Encoding QHexRenderer::stringEncoding() const {
    return m_stringEncoding;
}

QString QHexRenderer::stringEncodingName() const {
    switch (m_stringEncoding) {
    case QStringConverter::Encoding::Utf16:
        return QStringLiteral("UTF-16");
    case QStringConverter::Encoding::Utf16LE:
        return QStringLiteral("UTF-16LE");
    case QStringConverter::Encoding::Utf16BE:
        return QStringLiteral("UTF-16BE");
    case QStringConverter::Encoding::Utf32:
        return QStringLiteral("UTF-32");
    case QStringConverter::Encoding::Utf32LE:
        return QStringLiteral("UTF-32LE");
    case QStringConverter::Encoding::Utf32BE:
        return QStringLiteral("UTF-32BE");
    case QStringConverter::Encoding::Utf8:
        return QStringLiteral("UTF-8");
    case QStringConverter::Encoding::Latin1:
    default:
        return QStringLiteral("ASCII");
    }
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
    m_selectedarea = Areas::HexArea;
    m_cursorenabled = false;

    /*===================================*/
    // added by wingsummer

    m_asciiVisible = true;
    m_addressVisible = true;
    m_headerVisible = true;
    m_cursorSync = true;

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
    if (m_headerVisible) {
        drawHeader(painter);
    }
    auto documentLines = this->documentLines();
    for (qsizetype line = begin; line < std::min(end, documentLines); line++) {
        auto linerect = getLineRect(line, firstline);
        if (line % 2) {
            painter->fillRect(linerect, m_bytesBackground);
        } else {
            painter->fillRect(linerect, m_bytesAlterBackground);
        }

        if (m_addressVisible) {
            drawAddress(painter, linerect, line);
        }
        auto fmts = buildUpLineFormats(line);
        drawHex(painter, linerect, fmts, line);
        if (m_asciiVisible) {
            drawString(painter, linerect, fmts, line);
        }
    }
}

void QHexRenderer::updateMetrics(const QFontMetricsF &fm) {
    m_fontmetrics = fm;
}
void QHexRenderer::enableCursor(bool b) { m_cursorenabled = b; }

void QHexRenderer::selectArea(const QPoint &pt) {
    auto area = this->hitTestArea(pt);
    if (!this->editableArea(area))
        return;

    m_selectedarea = area;
}

bool QHexRenderer::hitTest(const QPoint &pt, QHexPosition *position,
                           qsizetype firstline, qsizetype lastline) const {
    auto area = this->hitTestArea(pt);
    if (!this->editableArea(area))
        return false;

    const auto lineWidth = this->hexLineWidth();
    auto line = std::min(firstline + qsizetype(pt.y() / this->lineHeight()) -
                             headerLineCount(),
                         lastline);
    position->line = line;
    position->lineWidth = quint8(lineWidth);

    auto hspace = m_fontmetrics.horizontalAdvance(' ') / 2;

    if (area == Areas::HexArea) {
        auto relx =
            pt.x() - hspace - this->getHexColumnX() - this->borderSize();
        int column = int(relx / this->getCellWidth());
        position->column = column / 3;
        // first char is nibble 1, 2nd and space are 0
        if (line >= documentLastLine() &&
            position->column >= documentLastColumn()) {
            position->nibbleindex = 1;
        } else {
            position->nibbleindex = (column % 3 == 0) ? 1 : 0;
        }
    } else {
        auto relx =
            pt.x() - hspace - this->getAsciiColumnX() - this->borderSize();
        position->column = int(relx / this->getCellWidth());
        position->nibbleindex = 1;
    }

    // Check last line's columns
    if (position->line == this->documentLastLine()) {
        position->column =
            std::min(position->column, int(m_document->length() % lineWidth));
    } else {
        position->column = std::min(position->column, int(lineWidth - 1));
    }

    return true;
}

QHexRenderer::Areas QHexRenderer::hitTestArea(const QPoint &pt) const {
    if (pt.y() < headerLineCount() * lineHeight())
        return Areas::HeaderArea;

    return hitTestColArea(pt);
}

QHexRenderer::Areas QHexRenderer::hitTestColArea(const QPoint &pt) const {
    if ((pt.x() >= this->borderSize()) &&
        (pt.x() <= (this->getHexColumnX() - this->borderSize())))
        return Areas::AddressArea;

    if ((pt.x() > (this->getHexColumnX() + this->borderSize())) &&
        (pt.x() < (this->getAsciiColumnX() - this->borderSize())))
        return Areas::HexArea;

    if ((pt.x() > (this->getAsciiColumnX() + this->borderSize())) &&
        (pt.x() < (this->getEndColumnX() - this->borderSize())))
        return Areas::AsciiArea;

    return Areas::ExtraArea;
}

QHexRenderer::Areas QHexRenderer::selectedArea() const {
    return m_selectedarea;
}
bool QHexRenderer::editableArea(QHexRenderer::Areas area) const {
    return (area == Areas::HexArea || area == Areas::AsciiArea);
}

void QHexRenderer::setCacheLineCount(qsizetype count) {
    m_lineCaches.setMaxCost(count);
}

qsizetype QHexRenderer::documentLastLine() const {
    return this->documentLines() - 1;
}
qsizetype QHexRenderer::documentLastColumn() const {
    return this->rendererLength() % hexLineWidth() - 1;
}
qsizetype QHexRenderer::documentLines() const {
    auto r = std::lldiv(this->rendererLength(), hexLineWidth());
    return r.quot + (r.rem > 0 ? 1 : 0);
}
qreal QHexRenderer::documentWidth() const { return this->getEndColumnX(); }
qreal QHexRenderer::lineHeight() const { return m_fontmetrics.height(); }

QRectF QHexRenderer::getLineRect(qsizetype line, qsizetype firstline) const {
    return QRectF(0, (line - firstline + headerLineCount()) * lineHeight(),
                  this->getEndColumnX(), lineHeight());
}

// modified by wingsummer
int QHexRenderer::headerLineCount() const { return m_headerVisible ? 1 : 0; }

qreal QHexRenderer::borderSize() const {
    if (m_document)
        return this->getNCellsWidth(m_document->areaIndent());
    return this->getNCellsWidth(DEFAULT_AREA_IDENTATION);
}

int QHexRenderer::hexLineWidth() const {
    if (m_document)
        return m_document->hexLineWidth();
    return DEFAULT_HEX_LINE_LENGTH;
}

QByteArray QHexRenderer::getLine(qsizetype line) const {
    auto width = hexLineWidth();
    return m_document->read(line * width, width);
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

qreal QHexRenderer::getHexColumnX() const {
    if (m_addressVisible) {
        return this->getNCellsWidth(this->getAddressWidth()) +
               2 * this->borderSize();
    }
    return 0;
}
qreal QHexRenderer::getAsciiColumnX() const {
    return this->getHexColumnX() + this->getNCellsWidth(hexLineWidth() * 3) +
           2 * this->borderSize();
}

qreal QHexRenderer::getEndColumnX() const {
    if (m_asciiVisible) {
        return this->getAsciiColumnX() + this->getNCellsWidth(hexLineWidth()) +
               2 * this->borderSize();
    }
    return this->getAsciiColumnX();
}

qreal QHexRenderer::getCellWidth() const {
    return m_fontmetrics.horizontalAdvance(' ');
}

qreal QHexRenderer::getNCellsWidth(int n) const { return n * getCellWidth(); }

void QHexRenderer::unprintableChars(QByteArray &ascii) const {
    for (char &ch : ascii) {
        if (std::isprint(static_cast<unsigned char>(ch)))
            continue;

        ch = HEX_UNPRINTABLE_CHAR;
    }
}

QChar QHexRenderer::toHexUpper(uint value) noexcept {
    return "0123456789ABCDEF"[value & 0xF];
}

QVector<QHexRenderer::AsciiCell>
QHexRenderer::buildAsciiCells(qsizetype line) const {
    auto rawline = this->getLine(line);
    if (auto *entry = m_lineCaches.object(line)) {
        if (entry->first == rawline) {
            return entry->second;
        }
        m_lineCaches.remove(line);
    }

    QVector<QHexRenderer::AsciiCell> ret;
    switch (m_stringEncoding) {
    case QStringConverter::Encoding::Utf8:
        ret = buildAsciiCellsUtf8(rawline);
        break;
    case QStringConverter::Encoding::Utf16:
        ret = buildAsciiCellsUtf16(
            rawline, QSysInfo::ByteOrder == QSysInfo::LittleEndian, true);
        break;
    case QStringConverter::Encoding::Utf16LE:
        ret = buildAsciiCellsUtf16(rawline, true, false);
        break;
    case QStringConverter::Encoding::Utf16BE:
        ret = buildAsciiCellsUtf16(rawline, false, false);
        break;
    case QStringConverter::Encoding::Utf32:
        ret = buildAsciiCellsUtf32(
            rawline, QSysInfo::ByteOrder == QSysInfo::LittleEndian, true);
        break;
    case QStringConverter::Encoding::Utf32LE:
        ret = buildAsciiCellsUtf32(rawline, true, false);
        break;
    case QStringConverter::Encoding::Utf32BE:
        ret = buildAsciiCellsUtf32(rawline, false, false);
        break;
    case QStringConverter::Encoding::Latin1:
    default:
        ret = buildAsciiCellsLatin1(rawline);
        break;
    }

    m_lineCaches.insert(
        line,
        new QPair<QByteArray, QVector<QHexRenderer::AsciiCell>>(rawline, ret));
    return ret;
}

QVector<QHexRenderer::AsciiCell>
QHexRenderer::buildAsciiCellsLatin1(const QByteArray &rawline) const {
    QVector<AsciiCell> cells;
    cells.reserve(rawline.size());

    for (int i = 0; i < rawline.size(); ++i) {
        uchar value = static_cast<uchar>(rawline.at(i));
        AsciiCell cell;
        cell.start = i;
        cell.length = 1;
        cell.text = std::isprint(value) ? QString(QChar::fromLatin1(value))
                                        : QString(HEX_UNPRINTABLE_CHAR);
        cells.append(cell);
    }

    return cells;
}

QVector<QHexRenderer::AsciiCell>
QHexRenderer::buildAsciiCellsUtf16(const QByteArray &rawline, bool littleEndian,
                                   bool useBom) const {
    QVector<AsciiCell> cells;
    cells.reserve(rawline.size());

    for (int i = 0; i < rawline.size();) {
        if (useBom && i + 1 < rawline.size()) {
            const quint16 markerLE = readUInt16(rawline, i, true);
            if (markerLE == 0xFEFF) {
                AsciiCell cell;
                cell.start = i;
                cell.length = 2;
                cell.text = QString(HEX_UNPRINTABLE_CHAR);
                cells.append(cell);
                i += 2;
                continue;
            }

            if (markerLE == 0xFFFE) {
                AsciiCell cell;
                cell.start = i;
                cell.length = 2;
                cell.text = QString(HEX_UNPRINTABLE_CHAR);
                cells.append(cell);
                i += 2;
                continue;
            }
        }

        int length = 0;
        char32_t codepoint = 0;
        if (decodeUtf16At(rawline, i, littleEndian, &length, &codepoint)) {
            AsciiCell cell;
            cell.start = i;
            cell.length = length;
            if (codepoint < 0x20 || codepoint == 0x7F) {
                cell.text = QString(HEX_UNPRINTABLE_CHAR);
            } else {
                cell.text = QString::fromUcs4(&codepoint, 1);
            }
            if (cell.text.isEmpty()) {
                cell.text = QString(HEX_UNPRINTABLE_CHAR);
                cell.length = 2;
            }
            cells.append(cell);
            i += cell.length;
            continue;
        }

        AsciiCell cell;
        cell.start = i;
        cell.length = qMin(2, rawline.size() - i);
        cell.text = QString(HEX_UNPRINTABLE_CHAR);
        cells.append(cell);
        i += cell.length;
    }

    return cells;
}

QVector<QHexRenderer::AsciiCell>
QHexRenderer::buildAsciiCellsUtf32(const QByteArray &rawline, bool littleEndian,
                                   bool useBom) const {
    QVector<AsciiCell> cells;
    cells.reserve(rawline.size());

    for (int i = 0; i < rawline.size();) {
        if (useBom && i + 3 < rawline.size()) {
            const quint32 markerLE = readUInt32(rawline, i, true);
            if (markerLE == 0x0000FEFFU || markerLE == 0xFFFE0000U) {
                AsciiCell cell;
                cell.start = i;
                cell.length = 4;
                cell.text = QString(HEX_UNPRINTABLE_CHAR);
                cells.append(cell);
                i += 4;
                continue;
            }
        }

        int length = 0;
        char32_t codepoint = 0;
        if (decodeUtf32At(rawline, i, littleEndian, &length, &codepoint)) {
            AsciiCell cell;
            cell.start = i;
            cell.length = length;
            if (codepoint < 0x20 || codepoint == 0x7F) {
                cell.text = QString(HEX_UNPRINTABLE_CHAR);
            } else {
                cell.text = QString::fromUcs4(&codepoint, 1);
            }
            if (cell.text.isEmpty()) {
                cell.text = QString(HEX_UNPRINTABLE_CHAR);
                cell.length = 4;
            }
            cells.append(cell);
            i += cell.length;
            continue;
        }

        AsciiCell cell;
        cell.start = i;
        cell.length = qMin(4, rawline.size() - i);
        cell.text = QString(HEX_UNPRINTABLE_CHAR);
        cells.append(cell);
        i += cell.length;
    }

    return cells;
}

QVector<QHexRenderer::AsciiCell>
QHexRenderer::buildAsciiCellsUtf8(const QByteArray &rawline) const {
    QVector<AsciiCell> cells;
    cells.reserve(rawline.size());

    for (int i = 0; i < rawline.size();) {
        int length = 0;
        char32_t codepoint = 0;

        if (decodeUtf8At(rawline, i, &length, &codepoint)) {
            AsciiCell cell;
            cell.start = i;
            cell.length = length;
            if (length == 1 && codepoint < 0x80 &&
                !std::isprint(static_cast<unsigned char>(codepoint))) {
                cell.text = QString(HEX_UNPRINTABLE_CHAR);
            } else {
                cell.text = QString::fromUcs4(&codepoint, 1);
            }
            if (cell.text.isEmpty()) {
                cell.text = QString(HEX_UNPRINTABLE_CHAR);
                cell.length = 1;
            }

            cells.append(cell);
            i += cell.length;
            continue;
        }

        AsciiCell cell;
        cell.start = i;
        cell.length = 1;
        cell.text = QString(HEX_UNPRINTABLE_CHAR);
        cells.append(cell);
        ++i;
    }

    return cells;
}

bool QHexRenderer::decodeUtf8At(const QByteArray &rawline, int index,
                                int *length, char32_t *codepoint) {
    if (index < 0 || index >= rawline.size()) {
        return false;
    }

    const uchar lead = static_cast<uchar>(rawline.at(index));
    if (lead < 0x80) {
        *length = 1;
        *codepoint = lead;
        return true;
    }

    int expected = 0;
    char32_t value = 0;
    char32_t minValue = 0;

    if ((lead & 0xE0U) == 0xC0U) {
        expected = 2;
        value = lead & 0x1FU;
        minValue = 0x80;
    } else if ((lead & 0xF0U) == 0xE0U) {
        expected = 3;
        value = lead & 0x0FU;
        minValue = 0x800;
    } else if ((lead & 0xF8U) == 0xF0U) {
        expected = 4;
        value = lead & 0x07U;
        minValue = 0x10000;
    } else {
        return false;
    }

    if (index + expected > rawline.size()) {
        return false;
    }

    for (int i = 1; i < expected; ++i) {
        const uchar next = static_cast<uchar>(rawline.at(index + i));
        if ((next & 0xC0U) != 0x80U) {
            return false;
        }

        value = (value << 6) | (next & 0x3FU);
    }

    if (value < minValue || value > 0x10FFFF ||
        (value >= 0xD800 && value <= 0xDFFF)) {
        return false;
    }

    *length = expected;
    *codepoint = value;
    return true;
}

quint16 QHexRenderer::readUInt16(const QByteArray &rawline, int index,
                                 bool littleEndian) {
    const uchar b0 = static_cast<uchar>(rawline.at(index));
    const uchar b1 = static_cast<uchar>(rawline.at(index + 1));
    return littleEndian ? quint16(b0 | (b1 << 8)) : quint16((b0 << 8) | b1);
}

quint32 QHexRenderer::readUInt32(const QByteArray &rawline, int index,
                                 bool littleEndian) {
    const uchar b0 = static_cast<uchar>(rawline.at(index));
    const uchar b1 = static_cast<uchar>(rawline.at(index + 1));
    const uchar b2 = static_cast<uchar>(rawline.at(index + 2));
    const uchar b3 = static_cast<uchar>(rawline.at(index + 3));
    if (littleEndian) {
        return quint32(b0) | (quint32(b1) << 8) | (quint32(b2) << 16) |
               (quint32(b3) << 24);
    }
    return (quint32(b0) << 24) | (quint32(b1) << 16) | (quint32(b2) << 8) |
           quint32(b3);
}

bool QHexRenderer::decodeUtf16At(const QByteArray &rawline, int index,
                                 bool littleEndian, int *length,
                                 char32_t *codepoint) {
    if (index + 1 >= rawline.size()) {
        return false;
    }

    const quint16 first = readUInt16(rawline, index, littleEndian);
    if (first >= 0xD800 && first <= 0xDBFF) {
        if (index + 3 >= rawline.size()) {
            return false;
        }

        const quint16 second = readUInt16(rawline, index + 2, littleEndian);
        if (second < 0xDC00 || second > 0xDFFF) {
            return false;
        }

        *length = 4;
        *codepoint = 0x10000 + (((first - 0xD800) << 10) | (second - 0xDC00));
        return true;
    }

    if (first >= 0xDC00 && first <= 0xDFFF) {
        return false;
    }

    *length = 2;
    *codepoint = first;
    return true;
}

bool QHexRenderer::decodeUtf32At(const QByteArray &rawline, int index,
                                 bool littleEndian, int *length,
                                 char32_t *codepoint) {
    if (index + 3 >= rawline.size()) {
        return false;
    }

    const quint32 value = readUInt32(rawline, index, littleEndian);
    if (value > 0x10FFFF || (value >= 0xD800 && value <= 0xDFFF)) {
        return false;
    }

    *length = 4;
    *codepoint = value;
    return true;
}

QRectF QHexRenderer::hexVisualRect(qreal cellWidth, qreal height,
                                   int byteIndex) const {
    const auto x = byteIndex * 3.0 * cellWidth;
    const auto w = 3.0 * cellWidth;
    return QRectF(x, 0, w, height);
}

QRectF QHexRenderer::hexCoreRect(qreal cellWidth, qreal height,
                                 int byteIndex) const {
    const auto x = byteIndex * 3.0 * cellWidth + cellWidth / 2.0;
    const auto w = 2.0 * cellWidth;
    return QRectF(x, 0, w, height);
}

QRectF QHexRenderer::hexNibbleRect(qreal cellWidth, qreal height, int byteIndex,
                                   int nibble) const {
    const QRectF core = hexCoreRect(cellWidth, height, byteIndex);
    const auto half = core.width() / 2;
    if (nibble == 1) {
        return QRectF(core.left(), 0, half, height);
    }
    return QRectF(core.left() + half, 0, core.width() - half, height);
}

QVector<QHexRenderer::CellFormat>
QHexRenderer::buildUpLineFormats(qsizetype line) const {
    QVector<CellFormat> lineFormats;
    lineFormats.reserve(hexLineWidth());
    for (qsizetype i = 0; i < hexLineWidth(); ++i) {
        lineFormats.append(cellFormat(line, i));
    }
    return lineFormats;
}

QHexRenderer::CellFormat QHexRenderer::cellFormat(qsizetype line,
                                                  int column) const {
    CellFormat fmt;
    fmt.foreground = m_bytesColor;

    // we just call QHexRenderer::cellFormat in sequential way for almost time
    // , so cache will help a lot
    static std::optional<QHexMetadataItem> meta;
    static qsizetype cidx = -1;
    auto offset = line * hexLineWidth() + column;
    bool retry;

    do {
        retry = false;
        if (meta) {
            if (offset < meta->begin || offset > meta->end) {
                meta.reset();
            }
        }
        if (!meta) {
            auto m = m_document->metadata();
            // try to get next
            if (cidx >= 0 && cidx + 1 < m->size()) {
                meta = m->at(cidx + 1);
                cidx = -1;
                retry = true;
                continue;
            }
            cidx = m->getIndex(offset);
            if (cidx >= 0) {
                meta = m->at(cidx);
            }
        }
    } while (retry);

    if (meta) {
        if (m_document->metabgVisible() && meta->background.isValid() &&
            meta->background.alpha()) {
            fmt.background = meta->background;
            fmt.fillBackground = true;
        }
        if (m_document->metafgVisible() && meta->foreground.isValid() &&
            meta->foreground.alpha()) {
            fmt.foreground = meta->foreground;
        }
        if (m_document->metaCommentVisible() && !meta->comment.isEmpty()) {
            fmt.underline = true;
        }
    }

    bool strikeOut = false;
    bool hasSelection = false;
    if (isStrByteSelected(line, column, &strikeOut, &hasSelection)) {
        fmt.background = (strikeOut || hasSelection)
                             ? m_selBackgroundColor.darker()
                             : m_selBackgroundColor;
        fmt.foreground =
            strikeOut ? m_selectionColor.darker() : m_selectionColor;
        fmt.fillBackground = true;
        fmt.strikeOut = strikeOut;
        fmt.italic = meta.has_value() || strikeOut;
    }

    QColor bg = fmt.fillBackground
                    ? fmt.background
                    : (line % 2 ? m_bytesBackground : m_bytesAlterBackground);
    if (!QHexMetadata::areColorsContrast(bg, fmt.foreground)) {
        fmt.bold = true;
        fmt.outline = QHexMetadata::generateContrastingColor(bg);
    }

    return fmt;
}

void QHexRenderer::drawAddress(QPainter *painter, const QRectF &linerect,
                               qsizetype line) {
    auto addr = quintptr(line * hexLineWidth()) + m_document->baseAddress();
    QString addrStr =
        QString::number(addr, 16)
            .rightJustified(this->getAddressWidth(), QLatin1Char('0'))
            .toUpper();

    auto addressrect = linerect;
    addressrect.setWidth(this->getHexColumnX());

    painter->save();
    painter->setPen(m_addressColor);
    painter->drawText(addressrect, Qt::AlignHCenter | Qt::AlignVCenter,
                      addrStr);
    painter->restore();
}

void QHexRenderer::drawHex(QPainter *painter, const QRectF &linerect,
                           const QVector<CellFormat> &lineFormats,
                           qsizetype line) {
    const QByteArray rawline = this->getLine(line);

    auto hexrect = linerect;
    hexrect.setX(this->getHexColumnX() + this->borderSize());
    hexrect.setWidth(this->getNCellsWidth(hexLineWidth() * 3));

    painter->save();
    painter->translate(hexrect.topLeft());

    const auto cellWidth = getCellWidth();
    const auto height = lineHeight();

    for (qsizetype i = 0; i < lineFormats.size(); ++i) {
        const auto &fmt = lineFormats.at(i);
        const auto visualRect = hexVisualRect(cellWidth, height, i);

        if (fmt.background.isValid()) {
            painter->fillRect(visualRect, fmt.background);
        }

        bool strikeOut = false;
        bool hasSelection = false;
        const bool selected =
            isStrByteSelected(line, i, &strikeOut, &hasSelection);
        if (selected) {
            painter->fillRect(visualRect, (strikeOut || hasSelection)
                                              ? m_selBackgroundColor.darker()
                                              : m_selBackgroundColor);
        }
    }

    drawUnderLine(painter, lineFormats,
                  [this, cellWidth, height](qreal start, qreal end) {
                      const auto startRect =
                          hexVisualRect(cellWidth, height, start);
                      const auto endRect =
                          hexVisualRect(cellWidth, height, end);
                      auto x1 = startRect.left();
                      auto x2 = endRect.right();
                      return qMakePair(x1, x2);
                  });

    for (qsizetype i = 0; i < rawline.size(); ++i) {
        const auto baseFmt = lineFormats.at(i);
        const uchar value = static_cast<uchar>(rawline.at(i));
        const QString upperHex(toHexUpper(value >> 4));
        const QString lowerHex(toHexUpper(value & 0xf));
        const auto coreRect = hexCoreRect(cellWidth, height, i);
        if (hasBookmark(line, i)) {
            applyBookmark(painter, coreRect, m_bytesColor);
        }

        const int cursorNibble = m_cursor->currentNibble();
        const auto nibbleRect =
            hexNibbleRect(cellWidth, height, i, cursorNibble);

        if (Q_UNLIKELY(drawCursor(painter, nibbleRect,
                                  cursorNibble ? upperHex : lowerHex, line, i,
                                  Areas::HexArea))) {
            if (cursorNibble) {
                drawText(painter, hexNibbleRect(cellWidth, height, i, 0),
                         lowerHex, baseFmt);
            } else {
                drawText(painter, hexNibbleRect(cellWidth, height, i, 1),
                         upperHex, baseFmt);
            }
        } else {
            drawText(painter, hexNibbleRect(cellWidth, height, i, 1), upperHex,
                     baseFmt);
            drawText(painter, hexNibbleRect(cellWidth, height, i, 0), lowerHex,
                     baseFmt);
        }
    }

    // when the cursor sits on the virtual "next byte" slot, the loop above
    // has nothing to draw. This keeps the caret visible on the trailing blank
    // slot and on the extra wrap line.
    if (m_cursor->currentColumn() == rawline.size()) {
        const auto cursorColumn = m_cursor->currentColumn();
        const auto placeholderRect = hexNibbleRect(
            cellWidth, height, cursorColumn, m_cursor->currentNibble());
        drawCursor(painter, placeholderRect, {}, line, cursorColumn,
                   Areas::HexArea);
    }

    painter->restore();
}

bool QHexRenderer::selectionCoversByte(const QHexSelection &selection,
                                       qsizetype line, int column) const {
    if (!selection.isLineSelected(line)) {
        return false;
    }

    const auto byteOffset = line * hexLineWidth() + column;
    return byteOffset >= selection.begin.offset() &&
           byteOffset <= selection.end.offset();
}

bool QHexRenderer::isStrByteSelected(qsizetype line, int column,
                                     bool *strikeOut,
                                     bool *hasSelection) const {
    if (strikeOut) {
        *strikeOut = false;
    }
    if (hasSelection) {
        *hasSelection = false;
    }

    bool internalSelected = false;
    for (int i = 0; i < m_cursor->selectionCount(); ++i) {
        if (selectionCoversByte(m_cursor->selection(i), line, column)) {
            internalSelected = true;
            break;
        }
    }

    bool selected = internalSelected;
    if (m_cursor->hasPreviewSelection() &&
        selectionCoversByte(m_cursor->previewSelection(), line, column)) {
        selected = true;
        if (strikeOut) {
            *strikeOut =
                m_cursor->previewSelectionMode() == QHexCursor::SelectionRemove;
        }
        if (hasSelection) {
            *hasSelection =
                internalSelected &&
                m_cursor->previewSelectionMode() == QHexCursor::SelectionNormal;
        }
    }

    return selected;
}

bool QHexRenderer::hasBookmark(qsizetype line, int column) const {
    return m_document->bookMarkExists(line * hexLineWidth() + column);
}

QHexRenderer::CellFormat
QHexRenderer::mergeCellFormat(const QVector<CellFormat> &formats) const {
    auto merged = formats.constFirst();

    for (qsizetype i = 1; i < formats.size(); ++i) {
        const auto &fmt = formats.at(i);
        if (fmt.fillBackground) {
            merged.background = fmt.background;
            merged.fillBackground = true;
        }
        if (fmt.foreground != merged.foreground) {
            merged.foreground = fmt.foreground;
        }
        merged.underline = merged.underline || fmt.underline;
        merged.bold = merged.bold || fmt.bold;
        merged.italic = merged.italic || fmt.italic;
        merged.strikeOut = merged.strikeOut || fmt.strikeOut;
        if (fmt.outline.isValid()) {
            merged.outline = fmt.outline;
        }
    }

    return merged;
}

bool QHexRenderer::asciiCellNeedsByteFallback(
    qsizetype line, const AsciiCell &cell,
    const QVector<CellFormat> &formats) const {
    if (cell.length <= 1) {
        return false;
    }

    const bool asciiCursorVisible =
        m_cursorSync || m_selectedarea == Areas::AsciiArea;
    const auto col = m_cursor->currentColumn();
    const bool cursorInsideCell =
        asciiCursorVisible && line == m_cursor->currentLine() &&
        col >= cell.start && col < (cell.start + cell.length);

    if (cursorInsideCell) {
        // Blink-off: keep the composed glyph (emoji, etc.).
        // Blink-on: show the byte fallback for this same cell.
        return m_cursorenabled;
    }

    if (formats.isEmpty()) {
        return false;
    }

    const auto &base = formats.constFirst();
    for (qsizetype i = 1; i < formats.size(); ++i) {
        const auto &fmt = formats.at(i);
        if (fmt.fillBackground != base.fillBackground ||
            fmt.background != base.background ||
            fmt.foreground != base.foreground || fmt.outline != base.outline ||
            fmt.underline != base.underline || fmt.bold != base.bold ||
            fmt.italic != base.italic || fmt.strikeOut != base.strikeOut) {
            return true;
        }
    }

    return false;
}

QRectF QHexRenderer::byteRectAt(qreal cellWidth, qreal height,
                                int byteIndex) const {
    const auto x = byteIndex * cellWidth;
    const auto w = ((byteIndex + 1) * cellWidth) - x;
    return QRectF(x, 0, w, height);
}

void QHexRenderer::applyBookmark(QPainter *painter, const QRectF &rect,
                                 const QColor &fallbackColor) const {
    auto markColor = fallbackColor.isValid() ? fallbackColor : m_bytesColor;
    markColor.setAlpha(180);
    painter->save();
    painter->setPen(QPen(markColor, 1, Qt::DotLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);
    painter->restore();
}

void QHexRenderer::drawText(QPainter *painter, const QRectF &rect,
                            const QString &text, const CellFormat &fmt) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);

    QFont font = painter->font();
    font.setBold(fmt.bold);
    font.setItalic(fmt.italic);
    font.setStrikeOut(fmt.strikeOut);
    font.setUnderline(false);

    QTextLayout layout(text);
    layout.setFont(font);

    QVector<QTextLayout::FormatRange> formats;
    QTextLayout::FormatRange range;
    range.start = 0;
    range.length = text.length();
    range.format.setFont(font);
    range.format.setForeground(fmt.foreground);

    if (fmt.outline.isValid()) {
        range.format.setTextOutline(
            QPen(fmt.outline, CONTRASTING_COLOR_BORDER));
    }
    formats.append(range);
    layout.setFormats(formats);

    QTextOption option;
    option.setAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout.setTextOption(option);

    layout.beginLayout();
    layout.createLine();
    layout.endLayout();

    QRectF textRect = layout.boundingRect();
    qreal x = rect.x() + (rect.width() - textRect.width()) / 2.0;
    qreal y = rect.y() + (rect.height() - textRect.height()) / 2.0;
    layout.draw(painter, QPointF(x, y));
    painter->restore();
}

void QHexRenderer::drawUnderLine(
    QPainter *painter, const QVector<CellFormat> &lineFormats,
    const std::function<QPair<qreal, qreal>(qreal, qreal)> &fn) {
    const auto underlineY = qMax(1.0, lineHeight() - 2);
    const auto count = lineFormats.size();
    for (qsizetype i = 0; i < count;) {
        const auto fmt = lineFormats.at(i);
        if (!fmt.underline) {
            ++i;
            continue;
        }

        const QColor color = fmt.outline;
        qsizetype start = i;
        qsizetype end = i;
        while (end + 1 < count) {
            const auto &next = lineFormats.at(end + 1);
            if (!next.underline || next.outline != color) {
                break;
            }
            ++end;
        }

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        const auto [startX, endX] = fn(start, end);
        QPainterPath linePath;
        linePath.moveTo(startX, underlineY);
        linePath.lineTo(endX, underlineY);

        if (color.isValid()) {
            // outline
            QPainterPathStroker stroker;
            stroker.setWidth(1.0 + CONTRASTING_COLOR_BORDER * 2);
            painter->fillPath(stroker.createStroke(linePath), color);
        }
        painter->setPen(QPen(fmt.foreground, 1));
        painter->drawPath(linePath);
        painter->restore();
        i = end + 1;
    }
}

bool QHexRenderer::cursorSync() const { return m_cursorSync; }

void QHexRenderer::setCursorSync(bool newCursorSync) {
    m_cursorSync = newCursorSync;
}

void QHexRenderer::drawString(QPainter *painter, const QRectF &linerect,
                              const QVector<CellFormat> &lineFormats,
                              qsizetype line) {
    auto asciirect = linerect;
    asciirect.setX(this->getAsciiColumnX() + this->borderSize());
    asciirect.setWidth(this->getNCellsWidth(hexLineWidth()));

    painter->save();
    painter->translate(asciirect.topLeft());

    const auto cells = buildAsciiCells(line);
    const auto cellWidth = getCellWidth();
    const auto height = lineHeight();

    struct PreparedCell {
        AsciiCell cell;
        QVector<CellFormat> formats;
        bool fallback = false;
    };

    QVector<PreparedCell> preparedCells;
    preparedCells.reserve(cells.size());

    for (const auto &cell : cells) {
        PreparedCell prepared;
        prepared.cell = cell;
        auto &fmts = prepared.formats;
        fmts.reserve(cell.length);
        for (qsizetype i = 0; i < cell.length; ++i) {
            const qsizetype col = cell.start + i;
            auto fmt = lineFormats.at(col);
            fmts.append(fmt);
        }
        prepared.fallback = asciiCellNeedsByteFallback(line, cell, fmts);
        preparedCells.append(prepared);
    }

    for (const auto &prepared : preparedCells) {
        const auto &cell = prepared.cell;
        const auto &fmts = prepared.formats;
        for (qsizetype i = 0; i < cell.length; ++i) {
            const auto &fmt = fmts.at(i);
            if (fmt.fillBackground) {
                painter->fillRect(byteRectAt(cellWidth, height, cell.start + i),
                                  fmt.background);
            }
        }
    }

    drawUnderLine(painter, lineFormats, [this](qreal start, qreal end) {
        const auto cellWidth = getCellWidth();
        const auto startX = start * cellWidth;
        const auto endX = (end + 1) * cellWidth;
        return qMakePair(startX, endX);
    });

    for (const auto &prepared : preparedCells) {
        const auto &cell = prepared.cell;
        const auto &fmts = prepared.formats;
        const auto startX = cell.start * cellWidth;
        const auto width = cell.length * cellWidth;
        const QRectF cellRect(startX, 0, width, height);

        if (prepared.fallback) {
            for (qsizetype i = 0; i < cell.length; ++i) {
                const auto &fmt = fmts.at(i);
                const qsizetype col = cell.start + i;
                auto r = byteRectAt(cellWidth, height, col);
                if (hasBookmark(line, col)) {
                    applyBookmark(painter, r, fmt.foreground);
                }

                drawCursorBlock(painter, r, cell, line, col, fmt);
            }

            continue;
        }

        for (qsizetype i = 0; i < cell.length; ++i) {
            const qsizetype col = cell.start + i;
            if (!hasBookmark(line, col)) {
                continue;
            }
            auto byteRect = byteRectAt(cellWidth, height, col);
            applyBookmark(painter, byteRect, fmts.at(i).foreground);
        }

        drawCursorBlock(painter, cellRect, cell, line, -1,
                        mergeCellFormat(fmts));
    }

    // when the cursor sits on the virtual "next byte" slot, the loop above
    // has nothing to draw. This keeps the caret visible on the trailing blank
    // slot and on the extra wrap line.
    if (m_cursor->currentColumn() >= documentLastColumn()) {
        const auto cursorColumn = m_cursor->currentColumn();
        const auto placeholderRect =
            byteRectAt(cellWidth, height, cursorColumn);
        drawCursor(painter, placeholderRect, {}, line, cursorColumn,
                   Areas::AsciiArea);
    }

    painter->restore();
}

void QHexRenderer::drawHeader(QPainter *painter) {
    QRect rect = QRect(0, 0, this->getEndColumnX(),
                       this->headerLineCount() * this->lineHeight());

    auto len = this->hexLineWidth();
    QString hexheader;
    hexheader.reserve(len * 3);
    for (int i = 0; i < len; i++) {
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
                          QStringLiteral("Offset"));

    // align left for maximum consistency with drawHex() which prints from the
    // left. so hex and positions are aligned vertically
    painter->drawText(hexrect, Qt::AlignLeft | Qt::AlignVCenter, hexheader);

    if (m_asciiVisible)
        painter->drawText(asciirect, Qt::AlignHCenter | Qt::AlignVCenter,
                          stringEncodingName());
    painter->restore();
}

void QHexRenderer::drawCursorBlock(QPainter *painter, const QRectF &cursorRect,
                                   const AsciiCell &cell, qsizetype line,
                                   int col, const CellFormat &fmt) const {
    if (cell.length == 1) {
        drawCursorStrBlock(painter, cursorRect, cell.text, line, cell.start,
                           fmt);
        return;
    }
    if (Q_UNLIKELY(!drawCursor(painter, cursorRect, cell, line, col))) {
        QString text;
        if (col >= 0) {
            text = QString(HEX_UNPRINTABLE_CHAR);
        } else {
            text = cell.text;
        }
        drawText(painter, cursorRect, text, fmt);
    }
}

void QHexRenderer::drawCursorStrBlock(QPainter *painter,
                                      const QRectF &cursorRect,
                                      const QString &cursorText, qsizetype line,
                                      int col, const CellFormat &fmt) const {
    if (Q_UNLIKELY(!drawCursor(painter, cursorRect, cursorText, line, col,
                               Areas::AsciiArea))) {
        drawText(painter, cursorRect, cursorText, fmt);
    }
}

bool QHexRenderer::drawCursor(QPainter *painter, const QRectF &cursorRect,
                              const QString &cursorText, qsizetype line,
                              int col, Areas area) const {
    if (!m_cursorenabled || !m_cursor) {
        return false;
    }

    if (!m_cursorSync && m_selectedarea != area) {
        return false;
    }

    if (line != m_cursor->currentLine() || col != m_cursor->currentColumn()) {
        return false;
    }

    drawCursor(painter, cursorRect, cursorText);
    return true;
}

bool QHexRenderer::drawCursor(QPainter *painter, const QRectF &cursorRect,
                              const AsciiCell &cell, qsizetype line,
                              int col) const {
    if (!m_cursorSync && m_selectedarea != Areas::AsciiArea) {
        return false;
    }

    auto pos = m_cursor->currentColumn();
    if (line != m_cursor->currentLine()) {
        return false;
    }

    const auto fallback = col >= 0;
    if (fallback) {
        if (pos != col) {
            return false;
        }
    } else {
        return false;
    }

    drawCursor(painter, cursorRect, QString(HEX_UNPRINTABLE_CHAR));
    return true;
}

void QHexRenderer::drawCursor(QPainter *painter, const QRectF &cursorRect,
                              const QString &cursorText) const {
    const auto height = lineHeight();
    painter->save();
    const qreal underlineY = qMax(1.0, height - 2.0);
    painter->fillRect(cursorRect, m_bytesColor);
    drawText(painter, cursorRect, cursorText, {m_bytesBackground});
    if (m_cursor->insertionMode() != QHexCursor::OverwriteMode) {
        QPen pen(m_bytesBackground, 1);
        painter->setPen(pen);
        painter->drawLine(QPointF(cursorRect.left(), underlineY),
                          QPointF(cursorRect.right(), underlineY));
    }
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
