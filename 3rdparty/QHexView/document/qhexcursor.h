#ifndef QHEXCURSOR_H
#define QHEXCURSOR_H

#include <QMutex>
#include <QObject>

#include <optional>

#define DEFAULT_HEX_LINE_LENGTH 0x10
#define DEFAULT_AREA_IDENTATION 0x01

struct QHexPosition {
    qsizetype line = -1;
    int column = -1;
    quint8 lineWidth = 0;
    int nibbleindex = -1;

    QHexPosition() = default;
    inline qsizetype offset() const {
        return static_cast<qsizetype>(line * lineWidth) + column;
    }
    inline qsizetype operator-(const QHexPosition &rhs) const {
        return this->offset() - rhs.offset();
    }
    inline void operator++() {
        this->column++;
        if (this->column >= lineWidth) {
            this->line++;
            this->column = 0;
        }
    }
    inline void operator--() {
        this->column--;
        if (this->column < 0) {
            this->line--;
            this->column = lineWidth - 1;
        }
    }
    inline bool operator==(const QHexPosition &rhs) const {
        return (line == rhs.line) && (column == rhs.column) &&
               (nibbleindex == rhs.nibbleindex);
    }
    inline bool operator!=(const QHexPosition &rhs) const {
        return (line != rhs.line) || (column != rhs.column) ||
               (nibbleindex != rhs.nibbleindex);
    }
    inline bool operator<(const QHexPosition &rhs) const {
        return this->offset() < rhs.offset();
    }
    inline bool operator>(const QHexPosition &rhs) const {
        return this->offset() > rhs.offset();
    }
    inline bool operator<=(const QHexPosition &rhs) const {
        return this->offset() <= rhs.offset();
    }
    inline bool operator>=(const QHexPosition &rhs) const {
        return this->offset() >= rhs.offset();
    }
};
Q_DECLARE_METATYPE(QHexPosition)

struct QHexSelection {
    QHexPosition start;
    QHexPosition end;

    void normalize(QMutex *locker = nullptr) {
        if (locker) {
            locker->lock();
        }
        if (end < start) {
            std::swap(start, end);
        }
        if (locker) {
            locker->unlock();
        }
    }

    qsizetype length() const { return qAbs(end - start) + 1; }

    bool contains(const QHexSelection &sel) const {
        Q_ASSERT(isNormalized());
        return this->start <= sel.start && this->end >= sel.end;
    }

    bool isLineSelected(qsizetype line) const {
        Q_ASSERT(isNormalized());
        if (this->start.line == line || this->end.line == line) {
            return true;
        }
        return this->start.line < line && line < this->end.line;
    }

    bool isNormalized() const { return end >= start; }

    QHexSelection normalized() const {
        QHexSelection sel = *this;
        if (end < start) {
            std::swap(sel.start, sel.end);
        }
        return sel;
    }

    bool isIntersected(const QHexSelection &sel) const {
        Q_ASSERT(isNormalized());
        return !(sel.end < this->start || sel.start > this->end);
    }

    void intersect(const QHexSelection &sel, QMutex *locker = nullptr) {
        Q_ASSERT(isNormalized());
        auto s = sel.normalized();
        if (locker) {
            locker->lock();
        }
        this->start = qMax(this->start, s.start);
        this->end = qMin(this->end, s.end);
        if (locker) {
            locker->unlock();
        }
    }

    Q_REQUIRED_RESULT std::optional<QHexSelection>
    removeSelection(const QHexSelection &sel, QMutex *locker = nullptr) {
        Q_ASSERT(isNormalized());
        Q_ASSERT(sel.isNormalized());
        if (locker) {
            locker->lock();
        }

        if (sel.start <= this->start) {
            if (sel.end >= this->start) {
                if (sel.end < this->end) {
                    this->start = sel.end;
                    ++this->start;
                } else {
                    // makes it invalid, delete later
                    this->end = this->start;
                }
            }
        } else if (sel.start > this->start && sel.start < this->end) {
            this->end = sel.start;
            --this->end;
            if (sel.end < this->end) {
                // break into two ranges
                QHexSelection sel;
                sel.start = sel.end;
                sel.end = this->end;

                if (locker) {
                    locker->unlock();
                }

                return sel;
            }
        }

        if (locker) {
            locker->unlock();
        }
        return {};
    }

    bool mergeSelection(const QHexSelection &sel, QMutex *locker = nullptr) {
        Q_ASSERT(isNormalized());
        if (isIntersected(sel)) {
            if (locker) {
                locker->lock();
            }
            this->start = qMin(this->start, sel.start);
            this->end = qMax(this->end, sel.end);
            if (locker) {
                locker->unlock();
            }
            return true;
        }
        return false;
    }
};

class QHexCursor : public QObject {
    Q_OBJECT

public:
    enum InsertionMode { OverwriteMode, InsertMode };

    enum SelectionMode {
        SelectionNormal = 0,
        SelectionAdd = 1,
        SelectionRemove = 2,
        SelectionSingle = 4,
        SelectionPreview = 16 // a flag
    };

    Q_DECLARE_FLAGS(SelectionModes, SelectionMode)

public:
    explicit QHexCursor(QObject *parent = nullptr);

public:
    const QHexPosition &position() const;

    qsizetype selectionCount() const;
    const QHexSelection &selection(qsizetype index) const;
    const QHexPosition &selectionStart(qsizetype index) const;
    const QHexPosition &selectionEnd(qsizetype index) const;

    InsertionMode insertionMode() const;

    qsizetype currentLine() const;
    int currentColumn() const;
    int currentNibble() const;

    qsizetype selectionLength(qsizetype index) const;

    qsizetype currentSelectionLength() const;

    bool atEnd() const;

    bool isLineSelected(qsizetype line) const;
    bool hasSelection() const;
    bool hasInternalSelection() const;

    void clearPreviewSelection();
    void clearSelection();

public:
    void moveTo(const QHexPosition &pos, bool clearSelection = false);
    void moveTo(qsizetype line, int column, int nibbleindex = 1,
                bool clearSelection = false);
    void moveTo(qsizetype offset, bool clearSelection = false);
    void setPos(qsizetype offset, int nibbleindex, bool clearSelection = false);

    void select(const QHexPosition &pos,
                QHexCursor::SelectionModes mode = SelectionNormal);
    void select(qsizetype line, int column, int nibbleindex = 1,
                QHexCursor::SelectionModes mode = SelectionNormal);
    void select(qsizetype length,
                QHexCursor::SelectionModes mode = SelectionNormal);

    void selectOffset(qsizetype offset, qsizetype length);
    void setInsertionMode(InsertionMode mode);
    void setLineWidth(quint8 width);
    void switchInsertionMode();

    bool hasPreviewSelection() const;
    QHexSelection previewSelection() const;
    void setPreviewSelectionMode(SelectionMode mode);
    SelectionMode previewSelectionMode() const;
    void mergePreviewSelection();

private:
    void mergeRemove(const QHexSelection &sel);
    void mergeAdd(const QHexSelection &sel, QMutex *locker = nullptr);

    bool isLineSelected(const QHexSelection &sel, qsizetype line) const;

signals:
    void positionChanged();
    void insertionModeChanged();
    void selectionChanged();

private:
    InsertionMode m_insertionmode;
    quint8 m_lineWidth;
    QHexPosition m_position, m_selection;

    SelectionMode m_preMode;
    QList<QHexSelection> m_sels;
};

#endif // QHEXCURSOR_H
