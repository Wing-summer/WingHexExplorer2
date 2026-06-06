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
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QStringDecoder>
#include <QTextCharFormat>
#include <QTextCursor>
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
    auto area = this->hitTestArea(pt);
    if (!this->editableArea(area))
        return;

    m_selectedarea = area;
}

bool QHexRenderer::hitTest(const QPoint &pt, QHexPosition *position,
                           qsizetype firstline) const {
    auto area = this->hitTestArea(pt);
    if (!this->editableArea(area))
        return false;

    position->line =
        std::min(firstline + pt.y() / this->lineHeight() - headerLineCount(),
                 this->documentLastLine());
    position->lineWidth = quint8(this->hexLineWidth());

    auto hspace = m_fontmetrics.horizontalAdvance(' ') / 2;

    if (area == Areas::HexArea) {
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

bool QHexRenderer::asciiCellAt(qsizetype line, int column, int *start,
                               int *length) const {
    const QByteArray rawline = getLine(line);
    if (column < 0 || column >= rawline.size()) {
        return false;
    }

    const auto cells = buildAsciiCells(rawline);
    for (const auto &cell : cells) {
        if (column >= cell.start && column < cell.start + cell.length) {
            if (start) {
                *start = cell.start;
            }
            if (length) {
                *length = cell.length;
            }
            return true;
        }
    }

    return false;
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

    auto str = toHexSequence(lrawline);
    if (lrawline.size() < hexLineWidth()) {
        str.append(QByteArrayLiteral("\t\t "));
    }
    return str;
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
    return m_fontmetrics.horizontalAdvance(' ');
}

int QHexRenderer::getNCellsWidth(int n) const {
    return qCeil(n * getCellWidth());
}

void QHexRenderer::unprintableChars(QByteArray &ascii) const {
    for (char &ch : ascii) {
        if (std::isprint(static_cast<unsigned char>(ch)))
            continue;

        ch = HEX_UNPRINTABLE_CHAR;
    }
}

QVector<QHexRenderer::AsciiCell>
QHexRenderer::buildAsciiCells(const QByteArray &rawline) const {
    switch (m_stringEncoding) {
    case QStringConverter::Encoding::Utf8:
        return buildAsciiCellsUtf8(rawline);
    case QStringConverter::Encoding::Utf16:
        return buildAsciiCellsUtf16(
            rawline, QSysInfo::ByteOrder == QSysInfo::LittleEndian, true);
    case QStringConverter::Encoding::Utf16LE:
        return buildAsciiCellsUtf16(rawline, true, false);
    case QStringConverter::Encoding::Utf16BE:
        return buildAsciiCellsUtf16(rawline, false, false);
    case QStringConverter::Encoding::Utf32:
        return buildAsciiCellsUtf32(
            rawline, QSysInfo::ByteOrder == QSysInfo::LittleEndian, true);
    case QStringConverter::Encoding::Utf32LE:
        return buildAsciiCellsUtf32(rawline, true, false);
    case QStringConverter::Encoding::Utf32BE:
        return buildAsciiCellsUtf32(rawline, false, false);
    case QStringConverter::Encoding::Latin1:
    default:
        return buildAsciiCellsLatin1(rawline);
    }
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

void QHexRenderer::applyHexBasicStyle(QTextCursor &textcursor,
                                      const QByteArray &rawline) const {
    QColor color = m_bytesColor;

    if (color.lightness() < 50) {
        if (color == Qt::black) {
            color = Qt::gray;
        } else {
            color = color.darker();
        }
    } else {
        color = color.lighter();
    }

    QTextCharFormat charformat;
    charformat.setForeground(color);

    for (int i = 0; i < rawline.length(); i++) {
        if ((rawline[i] != 0x00) && (uchar(rawline[i]) != 0xFF))
            continue;

        textcursor.setPosition(i * Hex);
        textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                Hex);
        textcursor.mergeCharFormat(charformat);
    }
}

void QHexRenderer::applyHexMetadata(QTextCursor &textcursor,
                                    qsizetype line) const {
    QHexMetadata *metadata = m_document->metadata();

    if (!metadata->lineHasMetadata(line))
        return;

    const QHexLineMetadata &linemetadata = metadata->gets(line);
    for (const auto &mi : linemetadata) {
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

        textcursor.setPosition(int(mi.start * Hex));
        textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                int((mi.length * Hex)));
        textcursor.mergeCharFormat(charformat);
    }
}

void QHexRenderer::applyHexSelection(QTextCursor &textcursor,
                                     qsizetype line) const {
    if (!m_cursor->isLineSelected(line))
        return;

    auto total = m_cursor->selectionCount();
    for (int i = 0; i < total; ++i) {
        applyHexSelection(m_cursor->selection(i), textcursor, line, false,
                          false);
    }

    if (m_cursor->hasPreviewSelection()) {
        applyHexSelection(
            m_cursor->previewSelection(), textcursor, line,
            m_cursor->previewSelectionMode() == QHexCursor::SelectionRemove,
            m_cursor->previewSelectionMode() == QHexCursor::SelectionNormal &&
                m_cursor->hasInternalSelection());
    }
}

void QHexRenderer::applyHexSelection(const QHexSelection &selection,
                                     QTextCursor &textcursor, qsizetype line,
                                     bool strikeOut, bool hasSelection) const {
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

    applyHexSelection(metas, textcursor, line * startsel.lineWidth, begin, end,
                      strikeOut, hasSelection);
}

void QHexRenderer::applyHexSelection(
    const QVector<QHexMetadata::MetaInfo> &metas, QTextCursor &textcursor,
    qsizetype startLineOffset, qsizetype lineStart, qsizetype lineEnd,
    bool strikeOut, bool hasSelection) const {
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

            textcursor.setPosition(fmtBegin * Hex);
            textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                    blen * Hex);

            textcursor.mergeCharFormat(charfmt);
            textcursor.clearSelection();
            totallen -= blen;

            textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                    mlen * Hex);

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
                                    totallen * Hex);
            textcursor.mergeCharFormat(charfmt);
            textcursor.clearSelection();
        }
    } else {
        textcursor.setPosition(lineStart * Hex);
        textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                totallen * Hex);
        textcursor.mergeCharFormat(charfmt);
    }
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

        if (m_cursor->previewSelectionMode() == QHexCursor::SelectionRemove) {
            if (strikeOut) {
                *strikeOut = true;
            }
        } else if (m_cursor->previewSelectionMode() ==
                       QHexCursor::SelectionNormal &&
                   internalSelected) {
            if (hasSelection) {
                *hasSelection = true;
            }
        }
    }

    return selected;
}

bool QHexRenderer::selectionCoversByte(const QHexSelection &selection,
                                       qsizetype line, int column) const {
    if (!selection.isLineSelected(line)) {
        return false;
    }

    const QHexSelection normalized = selection.normalized();
    const QHexPosition &startsel = normalized.begin;
    const QHexPosition &endsel = normalized.end;

    qsizetype begin;
    qsizetype end;

    if (startsel.line == endsel.line) {
        begin = startsel.column;
        end = endsel.column;
    } else {
        begin = (line == startsel.line) ? startsel.column : 0;
        end = (line == endsel.line) ? endsel.column : (startsel.lineWidth - 1);
    }

    return begin <= qsizetype(column) && qsizetype(column) <= end;
}

bool QHexRenderer::hasBookmark(qsizetype line, int column) const {
    return m_document->bookMarkExists(line * hexLineWidth() + column);
}

QHexRenderer::AsciiCellFormat
QHexRenderer::asciiCellFormat(qsizetype line, int column, uchar value) const {
    AsciiCellFormat fmt;
    fmt.foreground = m_bytesColor;

    if (value == 0x00 || value == 0xFF) {
        if (fmt.foreground.lightness() < 50) {
            fmt.foreground = fmt.foreground == Qt::black
                                 ? Qt::gray
                                 : fmt.foreground.darker();
        } else {
            fmt.foreground = fmt.foreground.lighter();
        }
    }

    auto meta = m_document->metadata()->get(line * hexLineWidth() + column);
    if (meta.has_value()) {
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

    if (line == m_cursor->currentLine() &&
        column == m_cursor->currentColumn() && m_cursorenabled) {
        if (m_cursorSync || m_selectedarea == Areas::AsciiArea) {
            if (m_selectedarea == Areas::HexArea) {
                if (m_bytesBackground.lightnessF() < 0.5) {
                    fmt.foreground = m_bytesBackground.lighter(95);
                } else {
                    fmt.foreground = m_bytesBackground.darker(125);
                }
                if (m_bytesColor.lightnessF() < 0.5) {
                    fmt.background = m_bytesColor.lighter(95);
                } else {
                    fmt.background = m_bytesColor.darker(125);
                }
            } else {
                fmt.foreground = m_bytesBackground;
                fmt.background = m_bytesColor;
            }
            fmt.fillBackground = true;
            if (m_cursor->insertionMode() == QHexCursor::OverwriteMode) {
                fmt.underline = false;
            } else {
                fmt.underline = true;
            }
        }
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

bool QHexRenderer::asciiCellNeedsByteFallback(
    qsizetype, const AsciiCell &cell,
    const QVector<AsciiCellFormat> &formats) const {
    if (cell.length <= 1 || formats.isEmpty()) {
        return false;
    }

    const auto &base = formats.constFirst();
    for (int i = 1; i < formats.size(); ++i) {
        const auto &fmt = formats.at(i);
        if (fmt.foreground != base.foreground ||
            fmt.background != base.background || fmt.outline != base.outline ||
            fmt.fillBackground != base.fillBackground ||
            fmt.underline != base.underline || fmt.bold != base.bold ||
            fmt.italic != base.italic || fmt.strikeOut != base.strikeOut) {
            return true;
        }
    }

    return false;
}

void QHexRenderer::applyCursorHex(QTextCursor &textcursor,
                                  qsizetype line) const {
    if ((line != m_cursor->currentLine()) || !m_cursorenabled ||
        (!m_cursorSync && m_selectedarea != Areas::HexArea))
        return;

    textcursor.clearSelection();

    auto col = m_cursor->currentColumn();
    textcursor.setPosition(col * Factor::Hex);

    if (m_cursor->currentNibble()) {
        textcursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
    } else {
        textcursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 2);
    }

    textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);

    QTextCharFormat charformat;

    if (m_selectedarea == Areas::AsciiArea) {
        if (m_bytesBackground.lightnessF() < 0.5) {
            charformat.setForeground(m_bytesBackground.lighter(95));
        } else {
            charformat.setForeground(m_bytesBackground.darker(125));
        }
        if (m_bytesColor.lightnessF() < 0.5) {
            charformat.setBackground(m_bytesColor.lighter(95));
        } else {
            charformat.setBackground(m_bytesColor.darker(125));
        }
    } else {
        charformat.setForeground(m_bytesBackground);
        charformat.setBackground(m_bytesColor);
    }
    if (m_cursor->insertionMode() == QHexCursor::InsertMode) {
        charformat.setUnderlineStyle(
            QTextCharFormat::UnderlineStyle::SingleUnderline);
    }
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
    this->applyHexBasicStyle(textcursor, rawline);

    auto dis = !m_document->metabgVisible() && !m_document->metafgVisible() &&
               !m_document->metaCommentVisible();
    if (!dis) {
        this->applyHexMetadata(textcursor, line);
    }

    this->applyHexSelection(textcursor, line);
    this->applyCursorHex(textcursor, line);

    painter->save();
    painter->translate(hexrect.topLeft());

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette.setColor(QPalette::Text, m_bytesColor);
    textdocument.documentLayout()->draw(painter, ctx);

    this->applyHexBookMark(painter, textcursor, line);
    painter->restore();
}

void QHexRenderer::applyHexBookMark(QPainter *painter, QTextCursor &textcursor,
                                    qsizetype line) {

    if (!m_document->lineHasBookMark(line))
        return;

    painter->save();

    auto pos = m_document->getLineBookmarksPos(line);
    auto height = lineHeight() - 2;

    for (const auto &item : std::as_const(pos)) {
        auto off = item % hexLineWidth();

        qreal begin, width;
        // add some paddings
        begin = getCellWidth() * off * 3 + 1;
        begin += getCellWidth() / 2;
        width = getCellWidth() * 2;
        textcursor.setPosition(off * Hex);

        textcursor.movePosition(QTextCursor::NextCharacter);
        auto charformat = textcursor.charFormat();
        auto textOutline = charformat.textOutline();

        constexpr auto ALPHA = 180;
        painter->setBrush(Qt::transparent);
        painter->setBackground(Qt::transparent);

        if (textOutline.style() != Qt::NoPen) {
            auto color = m_bytesColor;
            color.setAlpha(ALPHA);
            QPen pen(color, 2);
            painter->setPen(pen);
            painter->drawRect(begin, 1, width, height);

            auto outColor = textOutline.color();
            outColor.setAlpha(ALPHA);
            QPen outPen(outColor, 1, Qt::DotLine);
            painter->setPen(outPen);
            painter->drawRect(begin, 1, width, height);
        } else {
            if (m_cursor->currentLine() == line &&
                m_cursor->currentColumn() == off) {
                auto color = m_bytesColor;
                color.setAlpha(ALPHA);
                QPen pen(color, 1, Qt::DotLine);
                painter->setPen(pen);
            } else {
                auto foreground = charformat.foreground();
                auto textColor = foreground.color();
                if (textColor.isValid() && foreground.style() != Qt::NoBrush) {
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
            painter->drawRect(begin, 1, width, height);
        }
    }

    painter->restore();
}

QRect QHexRenderer::byteRectAt(qreal cellWidth, int height,
                               int byteIndex) const {
    const int x = qRound(byteIndex * cellWidth);
    const int w = qRound((byteIndex + 1) * cellWidth) - x;
    return QRect(x, 0, w, height);
}

void QHexRenderer::applyStrBookmark(QPainter *painter, qreal cellWidth,
                                    int height, int byteIndex,
                                    const QColor &fallbackColor) const {
    QRect byteRect = byteRectAt(cellWidth, height, byteIndex);
    QColor markColor = fallbackColor.isValid() ? fallbackColor : m_bytesColor;
    markColor.setAlpha(180);
    painter->save();
    painter->setPen(QPen(markColor, 1, Qt::DotLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(QRect(byteRect.x() + 1, byteRect.y() + 1,
                            qMax(0, byteRect.width() - 2),
                            qMax(0, byteRect.height() - 3)));
    painter->restore();
}

QHexRenderer::AsciiCellFormat
QHexRenderer::mergeCellFormat(const QVector<AsciiCellFormat> &formats) const {
    AsciiCellFormat merged = formats.constFirst();

    for (int i = 1; i < formats.size(); ++i) {
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

void QHexRenderer::drawAsciiText(QPainter *painter, const QRect &rect,
                                 const QString &text,
                                 const AsciiCellFormat &fmt) const {
    QFont font = painter->font();
    font.setBold(fmt.bold);
    font.setItalic(fmt.italic);
    font.setStrikeOut(fmt.strikeOut);
    font.setUnderline(false);

    painter->save();
    painter->setFont(font);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    const QFontMetricsF fm(font);
    const QRectF inkRect = fm.tightBoundingRect(text);

    const qreal baselineY =
        rect.y() + (rect.height() + fm.ascent() - fm.descent()) / 2.0;
    qreal textX =
        rect.x() + (rect.width() - inkRect.width()) / 2.0 - inkRect.x();

    const QPointF pos(textX, baselineY);

    if (fmt.outline.isValid()) {
        painter->setPen(fmt.outline);
        constexpr qreal d = CONTRASTING_COLOR_BORDER;
        const QPointF offsets[] = {
            QPointF(-d, 0.0),
            QPointF(d, 0.0),
            QPointF(0.0, -d),
            QPointF(0.0, d),
        };

        for (const QPointF &offset : offsets) {
            painter->drawText(pos + offset, text);
        }
    }

    painter->setPen(fmt.foreground);
    painter->drawText(pos, text);

    painter->restore();
}

bool QHexRenderer::cursorSync() const { return m_cursorSync; }

void QHexRenderer::setCursorSync(bool newCursorSync) {
    m_cursorSync = newCursorSync;
}

void QHexRenderer::drawString(QPainter *painter, const QRect &linerect,
                              qsizetype line) {
    const QByteArray rawline = this->getLine(line);

    QRect asciirect = linerect;
    asciirect.setX(this->getAsciiColumnX() + this->borderSize());
    asciirect.setWidth(this->getNCellsWidth(hexLineWidth()));

    painter->save();
    painter->translate(asciirect.topLeft());

    const auto cells = buildAsciiCells(rawline);
    const qreal cellWidth = getCellWidth();
    const int height = lineHeight();

    QVector<AsciiCellFormat> lineFormats;
    lineFormats.reserve(rawline.size());
    for (int i = 0; i < rawline.size(); ++i) {
        lineFormats.append(
            asciiCellFormat(line, i, static_cast<uchar>(rawline.at(i))));
    }

    struct PreparedCell {
        AsciiCell cell;
        QVector<AsciiCellFormat> formats;
        bool fallback = false;
    };

    QVector<PreparedCell> preparedCells;
    preparedCells.reserve(cells.size());

    for (const auto &cell : cells) {
        PreparedCell prepared;
        prepared.cell = cell;
        prepared.formats.reserve(cell.length);
        for (int i = 0; i < cell.length; ++i) {
            prepared.formats.append(lineFormats.at(cell.start + i));
        }
        prepared.fallback =
            asciiCellNeedsByteFallback(line, cell, prepared.formats);
        preparedCells.append(std::move(prepared));
    }

    for (const auto &prepared : preparedCells) {
        for (int i = 0; i < prepared.cell.length; ++i) {
            const auto &fmt = prepared.formats.at(i);
            if (fmt.fillBackground) {
                painter->fillRect(
                    byteRectAt(cellWidth, height, prepared.cell.start + i),
                    fmt.background);
            }
        }
    }

    for (const auto &prepared : preparedCells) {
        const int startX = qRound(prepared.cell.start * cellWidth);
        const int width = qRound(prepared.cell.length * cellWidth);
        const QRect cellRect(startX, 0, width, height);

        if (prepared.fallback) {
            for (int i = 0; i < prepared.cell.length; ++i) {
                const int byteIndex = prepared.cell.start + i;
                drawAsciiText(painter,
                              byteRectAt(cellWidth, height, byteIndex)
                                  .adjusted(0, 0, 0, -1),
                              QString(HEX_UNPRINTABLE_CHAR),
                              prepared.formats.at(i));
            }

            for (int i = 0; i < prepared.cell.length; ++i) {
                const int byteIndex = prepared.cell.start + i;
                if (!hasBookmark(line, byteIndex)) {
                    continue;
                }
                applyStrBookmark(painter, cellWidth, height, byteIndex,
                                 prepared.formats.at(i).foreground);
            }

            continue;
        }

        const AsciiCellFormat merged = mergeCellFormat(prepared.formats);

        drawAsciiText(painter, cellRect.adjusted(0, 0, 0, -1),
                      prepared.cell.text, merged);

        for (int i = 0; i < prepared.cell.length; ++i) {
            const int byteIndex = prepared.cell.start + i;
            if (!hasBookmark(line, byteIndex)) {
                continue;
            }
            applyStrBookmark(painter, cellWidth, height, byteIndex,
                             prepared.formats.at(i).foreground);
        }
    }

    const int underlineY = qMax(1, height - 2);
    const int count = lineFormats.size();
    for (int i = 0; i < count;) {
        const auto fmt = lineFormats.at(i);
        if (!fmt.underline) {
            ++i;
            continue;
        }

        const QColor color =
            fmt.foreground.isValid() ? fmt.foreground : m_bytesColor;

        int start = i;
        int end = i;
        while (end + 1 < count) {
            const auto &next = lineFormats.at(end + 1);
            if (!next.underline || next.foreground != color) {
                break;
            }
            ++end;
        }

        const int startX = qRound(start * cellWidth);
        const int endX = qRound((end + 1) * cellWidth);

        painter->save();
        painter->setPen(QPen(color, 1, Qt::SolidLine));
        painter->drawLine(startX, underlineY, endX, underlineY);
        painter->restore();

        i = end + 1;
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
