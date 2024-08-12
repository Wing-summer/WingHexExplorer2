#ifndef QHEXVIEW_H
#define QHEXVIEW_H

#define QHEXVIEW_VERSION 3.0

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
    Q_PROPERTY(qreal scaleRate READ scaleRate WRITE setScaleRate NOTIFY
                   scaleRateChanged FINAL)

public:
    explicit QHexView(QWidget *parent = nullptr);
    virtual ~QHexView();

    QSharedPointer<QHexDocument> document();
    QHexRenderer *renderer();

    void setDocument(const QSharedPointer<QHexDocument> &document,
                     QHexCursor *cursor = nullptr);

    /*=============================*/
    // added by wingsummer
    bool setLockedFile(bool b);
    bool setKeepSize(bool b);
    bool isReadOnly();
    bool isKeepSize();
    bool isLocked();
    qsizetype documentLines();
    qsizetype documentBytes();
    qsizetype currentRow();
    qsizetype currentColumn();
    qsizetype currentOffset();
    qsizetype selectlength();

    void setAsciiVisible(bool b);
    bool asciiVisible();
    void setAddressVisible(bool b);
    bool addressVisible();
    void setHeaderVisible(bool b);
    bool headerVisible();

    quintptr addressBase();
    void setAddressBase(quintptr base);

    bool isSaved();
    static QFont getHexeditorFont();
    void getStatus();

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

    void setFontSize(qreal size);
    qreal fontSize() const;

    void setScaleRate(qreal rate);
    qreal scaleRate() const;

    qindextype searchForward(qsizetype begin, const QByteArray &ba);
    qindextype searchBackward(qsizetype begin, const QByteArray &ba);

    void gotoBookMark(qindextype index);
    bool existBookMarkByIndex(qindextype &index);
    bool RemoveSelection(int nibbleindex = 1);
    bool removeSelection();
    bool atEnd() const;

    QByteArray selectedBytes() const;

    bool cut(bool hex);
    bool copy(bool hex = false);
    void paste(bool hex = false);

    bool Cut(bool hex = false, int nibbleindex = 0);
    void Paste(int nibbleindex = 0, bool hex = false);
    void Replace(qindextype offset, uchar b, int nibbleindex);
    void Replace(qindextype offset, const QByteArray &data,
                 int nibbleindex = 0);

    qsizetype copyLimit() const;
    void setCopyLimit(qsizetype newCopylimit);

    QHexCursor *cursor() const;

private:
    void establishSignal(QHexDocument *doc);

signals:
    void documentChanged(QHexDocument *doc);
    void cursorLocationChanged();
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);
    void documentSaved(bool saved);
    void documentLockedFile(bool locked);
    void documentKeepSize(bool keep);

    void documentBookMarkChanging(BookMarkModEnum flag, qsizetype section);
    void documentBookMarkChanged(BookMarkModEnum flag, qsizetype section);

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

    qsizetype m_copylimit = 1; // MB
};

#endif // QHEXVIEW_H
