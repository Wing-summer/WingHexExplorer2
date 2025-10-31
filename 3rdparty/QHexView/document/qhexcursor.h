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
#ifndef QHEXCURSOR_H
#define QHEXCURSOR_H

#include <QMutex>
#include <QObject>

#include "qhexregionobject.h"

#define DEFAULT_HEX_LINE_LENGTH 0x10
#define DEFAULT_AREA_IDENTATION 0x01

struct QHexPosition {
    qsizetype line = -1;
    int column = -1;
    quint8 lineWidth = 0;
    int nibbleindex = -1;

    inline QHexPosition() = default;
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

struct QHexSelection : QHexRegionObject<QHexPosition, QHexSelection> {
    inline QHexSelection() = default;

    inline explicit QHexSelection(const QHexPosition &begin,
                                  const QHexPosition &end) {
        this->begin = begin;
        this->end = end;
    }

    inline bool isLineSelected(qsizetype line) const {
        Q_ASSERT(isNormalized());
        if (this->begin.line == line || this->end.line == line) {
            return true;
        }
        return this->begin.line < line && line < this->end.line;
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

    bool isLineSelected(qsizetype line) const;
    bool isSelected(const QHexPosition &pos) const;
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
    void select(qsizetype line, int column,
                QHexCursor::SelectionModes mode = SelectionNormal);
    void select(qsizetype length,
                QHexCursor::SelectionModes mode = SelectionNormal);

    void setInsertionMode(InsertionMode mode);
    void setLineWidth(quint8 width);
    void switchInsertionMode();

    bool hasPreviewSelection() const;
    QHexSelection previewSelection() const;
    void setPreviewSelectionMode(SelectionMode mode);
    SelectionMode previewSelectionMode() const;
    void mergePreviewSelection();

private:
    bool isLineSelected(const QHexSelection &sel, qsizetype line) const;

signals:
    void positionChanged();
    void insertionModeChanged();
    void selectionChanged();

private:
    InsertionMode m_insertionmode;
    quint8 m_lineWidth;
    QHexPosition m_position, m_selection;

    SelectionMode m_preMode = SelectionNormal;
    QHexRegionObjectList<QHexPosition, QHexSelection> m_sels;
};

#endif // QHEXCURSOR_H
