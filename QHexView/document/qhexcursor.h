#ifndef QHEXCURSOR_H
#define QHEXCURSOR_H

#include <QObject>

#define DEFAULT_HEX_LINE_LENGTH 0x10
#define DEFAULT_AREA_IDENTATION 0x01

struct QHexPosition {
    qsizetype line;
    int column;
    quint8 lineWidth;
    int nibbleindex;

    QHexPosition() = default;
    inline qsizetype offset() const {
        return static_cast<qsizetype>(line * lineWidth) + column;
    }
    inline int operator-(const QHexPosition &rhs) const {
        return int(this->offset() - rhs.offset());
    }
    inline bool operator==(const QHexPosition &rhs) const {
        return (line == rhs.line) && (column == rhs.column) &&
               (nibbleindex == rhs.nibbleindex);
    }
    inline bool operator!=(const QHexPosition &rhs) const {
        return (line != rhs.line) || (column != rhs.column) ||
               (nibbleindex != rhs.nibbleindex);
    }
};

class QHexCursor : public QObject {
    Q_OBJECT

public:
    enum InsertionMode { OverwriteMode, InsertMode };

public:
    explicit QHexCursor(QObject *parent = nullptr);

public:
    const QHexPosition &selectionStart() const;
    const QHexPosition &selectionEnd() const;
    const QHexPosition &position() const;
    void setSelection(qsizetype offset,
                      qsizetype length); // added by wingsummer
    InsertionMode insertionMode() const;
    qsizetype selectionLength() const;
    qsizetype currentLine() const;
    int currentColumn() const;
    int currentNibble() const;
    qsizetype selectionLine() const;
    qsizetype selectionColumn() const;
    int selectionNibble() const;
    bool atEnd() const;
    bool isLineSelected(qsizetype line) const;
    bool hasSelection() const;
    void clearSelection();

public:
    void moveTo(const QHexPosition &pos);
    void moveTo(qsizetype line, int column, int nibbleindex = 1);
    void moveTo(qsizetype offset);
    void setPos(qsizetype offset, int nibbleindex);
    // 和 moveTo 其实一样，只是为了不冲突
    void select(const QHexPosition &pos);
    void select(qsizetype line, int column, int nibbleindex = 1);
    void select(qsizetype length);
    void selectOffset(qsizetype offset, qsizetype length);
    void setInsertionMode(InsertionMode mode);
    void setLineWidth(quint8 width);
    void switchInsertionMode();

signals:
    void positionChanged();
    void insertionModeChanged();

private:
    InsertionMode m_insertionmode;
    quint8 m_lineWidth;
    QHexPosition m_position, m_selection;
};

#endif // QHEXCURSOR_H
