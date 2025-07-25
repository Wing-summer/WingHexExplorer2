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
#include <QPainter>
#include <QPalette>
#include <QTextDocument>

class QHexRenderer : public QObject {
    Q_OBJECT

public:
    enum { HeaderArea, AddressArea, HexArea, AsciiArea, ExtraArea };

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

    /*==============================*/
    // added by wingsummer

    void setStringVisible(bool b);
    bool stringVisible();
    void setAddressVisible(bool b);
    bool addressVisible();
    void setHeaderVisible(bool b);
    bool headerVisible();

    void switchDoc(QHexDocument *doc);

    /*==============================*/

public:
    void blinkCursor();
    bool hitTest(const QPoint &pt, QHexPosition *position,
                 qsizetype firstline) const;
    int hitTestArea(const QPoint &pt) const;
    int selectedArea() const;
    bool editableArea(int area) const;
    qsizetype documentLastLine() const;
    qsizetype documentLastColumn() const;
    qsizetype documentLines() const;
    int documentWidth() const;
    int lineHeight() const;
    QRect getLineRect(qsizetype line, qsizetype firstline) const;
    int headerLineCount() const;
    int borderSize() const;
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

    QHexCursor *cursor() const;
    void setCursor(QHexCursor *newCursor);

private:
    QString hexString(qsizetype line, QByteArray *rawline = nullptr) const;

    QString decodeString(qsizetype line, QByteArray *rawline = nullptr) const;

    QByteArray getLine(qsizetype line) const;
    qsizetype rendererLength() const;

public:
    int getAddressWidth() const;
    int getHexColumnX() const;
    int getAsciiColumnX() const;
    int getEndColumnX() const;
    qreal getCellWidth() const;
    int getNCellsWidth(int n) const;
    void unprintableChars(QByteArray &ascii) const;

private:
    static QByteArray toHexSequence(const QByteArray &arr);
    static char toHexUpper(uint value) noexcept;

private:
    // modified by wingsummer
    enum Factor { String = 1, Hex = 4 };

    void applyDocumentStyles(QPainter *painter,
                             QTextDocument *textdocument) const;
    void applyBasicStyle(QTextCursor &textcursor, const QByteArray &rawline,
                         Factor factor) const;
    void applyMetadata(QTextCursor &textcursor, qsizetype line,
                       Factor factor) const;

    void applySelection(QTextCursor &textcursor, qsizetype line,
                        Factor factor) const;
    void applySelection(const QHexSelection &selection, QTextCursor &textcursor,
                        qsizetype line, Factor factor, bool strikeOut,
                        bool hasSelection) const;
    void applySelection(const QVector<QHexMetadata::MetaInfo> &metas,
                        QTextCursor &textcursor, qsizetype startLine,
                        qsizetype lineStart, qsizetype lineEnd, Factor factor,
                        bool strikeOut, bool hasSelection) const;

    // added by wingsummer
    void applyBookMark(QPainter *painter, QTextCursor &textcursor,
                       qsizetype line, Factor factor);

    void applyCursorAscii(QTextCursor &textcursor, qsizetype line) const;
    void applyCursorHex(QTextCursor &textcursor, qsizetype line) const;
    void drawAddress(QPainter *painter, const QRect &linerect, qsizetype line);
    void drawHex(QPainter *painter, const QRect &linerect, qsizetype line);
    void drawString(QPainter *painter, const QRect &linerect, qsizetype line);
    void drawHeader(QPainter *painter);

private:
    QHexDocument *m_document;
    QHexCursor *m_cursor;
    QFontMetricsF m_fontmetrics;

    int m_selectedarea;
    bool m_cursorenabled;

    /*==============================*/
    // added by wingsummer

    bool m_asciiVisible;
    bool m_addressVisible;
    bool m_headerVisible;

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
