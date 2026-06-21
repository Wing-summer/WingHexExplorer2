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
#ifndef QHEXRENDERER_H
#define QHEXRENDERER_H

/*
 * Nibble encoding:
 *           AB -> [A][B]
 * Nibble Index:    1  0
 */

#include "qhexdocument.h"
#include <QCache>
#include <QPainter>
#include <QStringConverter>
#include <QVector>

class QHexRenderer : public QObject {
    Q_OBJECT

public:
    enum class Areas { HeaderArea, AddressArea, HexArea, AsciiArea, ExtraArea };

public:
    explicit QHexRenderer(QHexDocument *document, QHexCursor *cursor,
                          const QFontMetricsF &fontmetrics,
                          QObject *parent = nullptr);
    void renderFrame(QPainter *painter);
    void renderAdditonalFrame(QPainter *painter, bool top, bool left);
    void render(QPainter *painter, qsizetype start, qsizetype end,
                qsizetype firstline); // begin included, end excluded
    void updateMetrics(const QFontMetricsF &fm);
    void enableCursor(bool b = true);
    void selectArea(const QPoint &pt);
    void toggleArea();

    /*==============================*/
    // added by wingsummer

    void setStringVisible(bool b);
    bool stringVisible();
    void setAddressVisible(bool b);
    bool addressVisible();
    void setHeaderVisible(bool b);
    bool headerVisible();
    void setStringEncoding(QStringConverter::Encoding encoding);

    QStringConverter::Encoding stringEncoding() const;

    QString stringEncodingName() const;

    void switchDoc(QHexDocument *doc);

    /*==============================*/

public:
    void blinkCursor();
    bool hitTest(const QPoint &pt, QHexPosition *position, qsizetype firstline,
                 qsizetype lastline) const;
    Areas hitTestArea(const QPoint &pt) const;
    Areas hitTestColArea(const QPoint &pt) const;
    Areas selectedArea() const;
    bool editableArea(Areas area) const;
    qsizetype documentLastLine() const;
    qsizetype documentLastColumn() const;
    qsizetype documentLines() const;
    qreal documentWidth() const;
    qreal lineHeight() const;
    QRectF getLineRect(qsizetype line, qsizetype firstline) const;
    int headerLineCount() const;
    qreal borderSize() const;
    int hexLineWidth() const;

    QColor headerColor() const;
    void setHeaderColor(const QColor &newHeaderColor);

    QColor addressColor() const;
    void setAddressColor(const QColor &newAddressColor);

    QColor bytesBackground() const;
    void setBytesBackground(const QColor &newBytesBackground);

    QColor bytesAlterBackground() const;
    void setBytesAlterBackground(const QColor &newBytesAlterBackground);

    QColor bytesColor() const;
    void setBytesColor(const QColor &newBytesColor);

    QColor selectionColor() const;
    void setSelectionColor(const QColor &newSelectionColor);

    QColor selBackgroundColor() const;
    void setSelBackgroundColor(const QColor &newSelBackgroundColor);

    QColor borderColor() const;
    void setBorderColor(const QColor &newBorderColor);

    bool cursorSync() const;
    void setCursorSync(bool newCursorSync);

    QHexCursor *cursor() const;
    void setCursor(QHexCursor *newCursor);

    int normalizeAsciiColumn(qsizetype line, int column) const;

    void setCacheLineCount(qsizetype count);

private:
    QByteArray getLine(qsizetype line) const;
    qsizetype rendererLength() const;

public:
    int getAddressWidth() const;
    qreal getHexColumnX() const;
    qreal getAsciiColumnX() const;
    qreal getEndColumnX() const;
    qreal getCellWidth() const;
    qreal getNCellsWidth(int n) const;
    void unprintableChars(QByteArray &ascii) const;

private:
    static QChar toHexUpper(uint value) noexcept;

    struct AsciiCell {
        qsizetype start = 0;
        qsizetype length = 1;
        QString text;
    };

    struct CellFormat {
        QColor foreground;
        QColor background;
        QColor outline;
        bool fillBackground = false;
        bool underline = false;
        bool bold = false;
        bool italic = false;
        bool strikeOut = false;
    };

private:
    void drawAddress(QPainter *painter, const QRectF &linerect, qsizetype line);
    void drawHex(QPainter *painter, const QRectF &linerect,
                 const QVector<CellFormat> &lineFormats, qsizetype line);
    void drawString(QPainter *painter, const QRectF &linerect,
                    const QVector<CellFormat> &lineFormats, qsizetype line);
    void drawHeader(QPainter *painter);

    void drawCursorBlock(QPainter *painter, const QRectF &cursorRect,
                         const AsciiCell &cell, qsizetype line, int col,
                         const CellFormat &fmt) const;
    void drawCursorStrBlock(QPainter *painter, const QRectF &cursorRect,
                            const QString &cursorText, qsizetype line, int col,
                            const CellFormat &fmt) const;

private:
    bool drawCursor(QPainter *painter, const QRectF &cursorRect,
                    const QString &cursorText, qsizetype line, int col,
                    Areas area) const;
    bool drawCursor(QPainter *painter, const QRectF &cursorRect,
                    const AsciiCell &cell, qsizetype line, int col) const;
    void drawCursor(QPainter *painter, const QRectF &cursorRect,
                    const QString &cursorText) const;

private:
    QRectF hexVisualRect(qreal cellWidth, qreal height, int byteIndex) const;
    QRectF hexCoreRect(qreal cellWidth, qreal height, int byteIndex) const;
    QRectF hexNibbleRect(qreal cellWidth, qreal height, int byteIndex,
                         int nibble) const;

private:
    QVector<AsciiCell> buildAsciiCells(qsizetype line) const;
    QVector<AsciiCell> buildAsciiCellsLatin1(const QByteArray &rawline) const;
    QVector<AsciiCell> buildAsciiCellsUtf8(const QByteArray &rawline) const;
    QVector<AsciiCell> buildAsciiCellsUtf16(const QByteArray &rawline,
                                            bool littleEndian,
                                            bool useBom) const;
    QVector<AsciiCell> buildAsciiCellsUtf32(const QByteArray &rawline,
                                            bool littleEndian,
                                            bool useBom) const;
    CellFormat cellFormat(qsizetype line, int column) const;

    QVector<CellFormat> buildUpLineFormats(qsizetype line) const;

private:
    bool selectionCoversByte(const QHexSelection &selection, qsizetype line,
                             int column) const;
    bool asciiCellNeedsByteFallback(qsizetype line, const AsciiCell &cell,
                                    const QVector<CellFormat> &formats) const;
    bool isStrByteSelected(qsizetype line, int column, bool *strikeOut,
                           bool *hasSelection) const;
    bool hasBookmark(qsizetype line, int column) const;

private:
    static bool decodeUtf8At(const QByteArray &rawline, int index, int *length,
                             char32_t *codepoint);
    static bool decodeUtf16At(const QByteArray &rawline, int index,
                              bool littleEndian, int *length,
                              char32_t *codepoint);
    static bool decodeUtf32At(const QByteArray &rawline, int index,
                              bool littleEndian, int *length,
                              char32_t *codepoint);
    static quint16 readUInt16(const QByteArray &rawline, int index,
                              bool littleEndian);
    static quint32 readUInt32(const QByteArray &rawline, int index,
                              bool littleEndian);

private:
    QRectF byteRectAt(qreal cellWidth, qreal height, int byteIndex) const;

    void applyBookmark(QPainter *painter, const QRectF &rect,
                       const QColor &fallbackColor) const;
    void drawText(QPainter *painter, const QRectF &rect, const QString &text,
                  const CellFormat &fmt) const;
    void drawUnderLine(
        QPainter *painter, const QVector<CellFormat> &lineFormats,
        const std::function<QPair<qreal, qreal>(qreal start, qreal end)> &fn);
    CellFormat mergeCellFormat(const QVector<CellFormat> &formats) const;

private:
    QHexDocument *m_document;
    QHexCursor *m_cursor;
    QFontMetricsF m_fontmetrics;

    mutable QCache<qsizetype,
                   QPair<QByteArray, QVector<QHexRenderer::AsciiCell>>>
        m_lineCaches{30};
    QHexRenderer::Areas m_selectedarea;
    bool m_cursorenabled;

    /*==============================*/
    // added by wingsummer

    bool m_asciiVisible;
    bool m_addressVisible;
    bool m_headerVisible;
    bool m_cursorSync;
    QStringConverter::Encoding m_stringEncoding =
        QStringConverter::Encoding::Latin1;

    // color
    QColor m_headerColor = Qt::cyan;
    QColor m_addressColor = Qt::cyan;
    QColor m_bytesBackground = Qt::white;
    QColor m_bytesAlterBackground = Qt::gray;
    QColor m_bytesColor = Qt::black;
    QColor m_selectionColor = Qt::white;
    QColor m_selBackgroundColor = Qt::blue;
    QColor m_borderColor = Qt::white;
    /*==============================*/
};

#endif // QHEXRENDERER_H
