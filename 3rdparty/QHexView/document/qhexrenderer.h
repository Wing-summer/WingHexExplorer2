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

    QString encoding();
    bool setEncoding(const QString &encoding);
    void SetEncoding(QString encoding);

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

    QHexCursor *cursor() const;
    void setCursor(QHexCursor *newCursor);

private:
    QString hexString(qsizetype line, QByteArray *rawline = nullptr) const;

    /*======================================*/
    // added by wingsummer

    QString decodeString(qsizetype line, QString encoding,
                         QByteArray *rawline = nullptr) const;
    void unprintableWChars(QString &unicode) const;

    /*======================================*/

    QByteArray getLine(qsizetype line) const;
    qsizetype rendererLength() const;
    int getAddressWidth() const;
    int getHexColumnX() const;
    int getAsciiColumnX() const;
    int getEndColumnX() const;
    qreal getCellWidth() const;
    int getNCellsWidth(int n) const;
    void unprintableChars(QByteArray &ascii) const;

private:
    // modified by wingsummer
    enum Factor { String = 1, Hex = 3 };

    void applyDocumentStyles(QPainter *painter,
                             QTextDocument *textdocument) const;
    void applyBasicStyle(QTextCursor &textcursor, const QByteArray &rawline,
                         Factor factor) const;
    void applyMetadata(QTextCursor &textcursor, qsizetype line,
                       Factor factor) const;
    void applySelection(QTextCursor &textcursor, qsizetype line,
                        Factor factor) const;
    void applyBookMark(QTextCursor &textcursor, qsizetype line,
                       Factor factor); // added by wingsummer
    void applyCursorAscii(QTextCursor &textcursor, qsizetype line) const;
    void applyCursorHex(QTextCursor &textcursor, qsizetype line) const;
    void drawAddress(QPainter *painter, const QRect &linerect, qsizetype line);
    void drawHex(QPainter *painter, const QRect &linerect, qsizetype line);
    void drawString(QPainter *painter, const QRect &linerect, qsizetype line,
                    QString encoding = "ASCII");
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
    QString m_encoding;

    // color
    QColor m_headerColor = Qt::cyan;
    QColor m_addressColor = Qt::cyan;
    QColor m_bytesBackground = Qt::white;
    QColor m_bytesAlterBackground = Qt::gray;
    QColor m_bytesColor = Qt::black;
    QColor m_selectionColor = Qt::white;
    QColor m_selBackgroundColor = Qt::blue;
    /*==============================*/
};

#endif // QHEXRENDERER_H
