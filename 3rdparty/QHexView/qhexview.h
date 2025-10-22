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

#ifndef QHEXVIEW_H
#define QHEXVIEW_H

#define QHEXVIEW_VERSION 3.1

#include "document/qhexdocument.h"
#include "document/qhexrenderer.h"
#include <QAbstractScrollArea>
#include <QTimer>

class QHexView : public QAbstractScrollArea {
    Q_OBJECT

    Q_PROPERTY(QColor headerColor READ headerColor WRITE setHeaderColor NOTIFY
                   headerColorChanged FINAL)
    Q_PROPERTY(QColor addressColor READ addressColor WRITE setAddressColor
                   NOTIFY addressColorChanged FINAL)
    Q_PROPERTY(QColor bytesBackground READ bytesBackground WRITE
                   setBytesBackground NOTIFY bytesBackgroundChanged FINAL)
    Q_PROPERTY(
        QColor bytesAlterBackground READ bytesAlterBackground WRITE
            setBytesAlterBackground NOTIFY bytesAlterBackgroundChanged FINAL)
    Q_PROPERTY(QColor bytesColor READ bytesColor WRITE setBytesColor NOTIFY
                   bytesColorChanged FINAL)
    Q_PROPERTY(QColor selectionColor READ selectionColor WRITE setSelectionColor
                   NOTIFY selectionColorChanged FINAL)
    Q_PROPERTY(QColor selBackgroundColor READ selBackgroundColor WRITE
                   setSelBackgroundColor NOTIFY selBackgroundColorChanged FINAL)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY
                   borderColorChanged FINAL)
    Q_PROPERTY(qreal scaleRate READ scaleRate WRITE setScaleRate NOTIFY
                   scaleRateChanged FINAL)

public:
    explicit QHexView(QWidget *parent = nullptr);
    virtual ~QHexView();

public:
    QSharedPointer<QHexDocument> document();
    QHexRenderer *renderer();

    bool lockKeepSize() const;

    bool isReadOnly();
    bool isKeepSize();
    bool isLocked();
    qsizetype documentLines();
    qsizetype documentBytes();
    qsizetype currentRow();
    qsizetype currentColumn();
    qsizetype currentOffset();
    qsizetype currentSelectionLength();

    qsizetype selectionCount();
    bool hasSelection();

    bool asciiVisible();
    bool addressVisible();
    bool headerVisible();

    quintptr addressBase();

    bool isSaved();
    static QFont getHexeditorFont();
    void getStatus();

    QColor headerColor() const;
    QColor addressColor() const;
    QColor bytesBackground() const;
    QColor bytesAlterBackground() const;
    QColor bytesColor() const;
    QColor selectionColor() const;
    QColor selBackgroundColor() const;
    QColor borderColor() const;

    qreal fontSize() const;
    qreal scaleRate() const;

    bool atEnd() const;

    QByteArray selectedBytes(qsizetype index) const;
    QByteArray previewSelectedBytes() const;
    QByteArrayList selectedBytes() const;

    qsizetype copyLimit() const;
    QHexCursor *cursor() const;

    bool disableInternalPaint() const;
    void setDisableInternalPaint(bool newDisableInternalPaint);

public slots:
    void setDocument(const QSharedPointer<QHexDocument> &document,
                     QHexCursor *cursor = nullptr);
    bool RemoveSelection(int nibbleindex = 1);
    bool removeSelection();

    bool setLockedFile(bool b);
    bool setKeepSize(bool b);
    void setLockKeepSize(bool b);
    void setAddressBase(quintptr base);

    void setHeaderColor(const QColor &newHeaderColor);
    void setAddressColor(const QColor &newAddressColor);
    void setBytesBackground(const QColor &newBytesBackground);
    void setBytesAlterBackground(const QColor &newBytesAlterBackground);
    void setBytesColor(const QColor &newBytesColor);
    void setSelectionColor(const QColor &newSelectionColor);
    void setSelBackgroundColor(const QColor &newSelBackgroundColor);
    void setBorderColor(const QColor &newBorderColor);

    void setFontSize(qreal size);
    void setScaleRate(qreal rate);
    qsizetype findNext(qsizetype begin, const QByteArray &ba);
    qsizetype findPrevious(qsizetype begin, const QByteArray &ba);

    void setAsciiVisible(bool b);
    void setAddressVisible(bool b);
    void setHeaderVisible(bool b);

    bool cut(bool hex);
    bool copy(bool hex = false);
    bool paste(bool hex = false);

    bool Cut(bool hex = false, int nibbleindex = 0);
    bool Paste(bool hex = false, int nibbleindex = 0);
    void Replace(qsizetype offset, uchar b, int nibbleindex);
    void Replace(qsizetype offset, const QByteArray &data, int nibbleindex = 0);

    void setCopyLimit(qsizetype newCopylimit);

private:
    void establishSignal(QHexDocument *doc);

signals:
    void cursorLocationChanged();
    void cursorSelectionChanged();
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);
    void documentSaved(bool saved);
    void documentLockedFile(bool locked);
    void documentKeepSize(bool keep);

    void metafgVisibleChanged(bool b);
    void metabgVisibleChanged(bool b);
    void metaCommentVisibleChanged(bool b);
    void metaStatusChanged();

    void scaleRateChanged();
    void copyLimitRaised();

    /*=============================*/

    void headerColorChanged();
    void addressColorChanged();
    void bytesBackgroundChanged();
    void bytesAlterBackgroundChanged();
    void bytesColorChanged();
    void selectionColorChanged();
    void selBackgroundColorChanged();
    void borderColorChanged();

    void onPaintCustomEventBegin();
    void onPaintCustomEvent(int XOffset, qsizetype firstVisible,
                            qsizetype begin, qsizetype end);

protected:
    virtual bool event(QEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);
    virtual void wheelEvent(QWheelEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void paintEvent(QPaintEvent *e);

private slots:
    void renderCurrentLine();
    void moveToSelection();
    void blinkCursor();

private:
    void moveNext(bool select = false);
    void movePrevious(bool select = false);

    QHexCursor::SelectionMode getSelectionMode() const;

private:
    bool processMove(QHexCursor *cur, QKeyEvent *e);
    bool processTextInput(QHexCursor *cur, QKeyEvent *e);
    bool processAction(QHexCursor *cur, QKeyEvent *e);
    void adjustScrollBars();
    void renderLine(qsizetype line);
    qsizetype firstVisibleLine() const;
    qsizetype lastVisibleLine() const;
    qsizetype visibleLines() const;
    bool isLineVisible(qsizetype line) const;

    int documentSizeFactor() const;

    QPoint absolutePosition(const QPoint &pos) const;

private:
    QSharedPointer<QHexDocument> m_document;
    QHexCursor *m_cursor = nullptr;
    QHexRenderer *m_renderer;
    QTimer *m_blinktimer;

    qreal m_fontSize;
    qreal m_scaleRate = 1.0;
    bool m_disableInternalPaint = false;

    qsizetype m_copylimit = 1; // MB
};

#endif // QHEXVIEW_H
