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

#include "qhexcursor.h"
#include <QWidget>

QHexCursor::QHexCursor(QObject *parent)
    : QObject(parent), m_insertionmode(QHexCursor::OverwriteMode) {
    m_position.line = m_position.column = 0;
    m_position.line = m_position.column = 0;

    m_selection.line = m_selection.column = 0;
    m_selection.line = m_selection.column = 0;

    m_position.nibbleindex = m_selection.nibbleindex = 1;

    setLineWidth(DEFAULT_HEX_LINE_LENGTH);
}

const QHexPosition &QHexCursor::selectionStart(qsizetype index) const {
    return m_sels.at(index).begin;
}

const QHexPosition &QHexCursor::selectionEnd(qsizetype index) const {
    return m_sels.at(index).end;
}

const QHexPosition &QHexCursor::position() const { return m_position; }

qsizetype QHexCursor::selectionCount() const { return m_sels.size(); }

const QHexSelection &QHexCursor::selection(qsizetype index) const {
    return m_sels.at(index);
}

QHexCursor::InsertionMode QHexCursor::insertionMode() const {
    return m_insertionmode;
}

qsizetype QHexCursor::selectionLength(qsizetype index) const {
    return m_sels.at(index).length();
}

qsizetype QHexCursor::currentSelectionLength() const {
    if (hasPreviewSelection() && m_preMode != SelectionRemove) {
        return qAbs(m_position - m_selection) + 1;
    }

    qsizetype len = 0;
    auto res = QtConcurrent::blockingMappedReduced(
        m_sels, &QHexSelection::length,
        QOverload<
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            const qsizetype &
#else
            QVector<qsizetype>::parameter_type
#endif
            >::of(&QVector<qsizetype>::append));
    for (auto &item : res) {
        len += item;
    }
    return len;
}

qsizetype QHexCursor::currentLine() const { return m_selection.line; }

int QHexCursor::currentColumn() const { return m_selection.column; }

int QHexCursor::currentNibble() const { return m_selection.nibbleindex; }

bool QHexCursor::isLineSelected(qsizetype line) const {
    if (!this->hasSelection())
        return false;

    if (isLineSelected(previewSelection(), line)) {
        return true;
    }

    for (auto &sel : m_sels) {
        if (isLineSelected(sel, line)) {
            return true;
        }
    }

    return false;
}

bool QHexCursor::isSelected(const QHexPosition &pos) const {
    if (!this->hasSelection())
        return false;

    if (previewSelection().contains(pos)) {
        return true;
    }

    for (auto &sel : m_sels) {
        if (sel.contains(pos)) {
            return true;
        }
    }

    return false;
}

bool QHexCursor::hasSelection() const {
    return hasPreviewSelection() || hasInternalSelection();
}

bool QHexCursor::hasInternalSelection() const { return !m_sels.isEmpty(); }

void QHexCursor::clearPreviewSelection() {
    m_selection = m_position;
    m_preMode = SelectionNormal;
    emit selectionChanged();
}

void QHexCursor::clearSelection() { m_sels.clear(); }

void QHexCursor::moveTo(const QHexPosition &pos, bool clearSelection) {
    this->moveTo(pos.line, pos.column, pos.nibbleindex, clearSelection);
}
void QHexCursor::select(const QHexPosition &pos,
                        QHexCursor::SelectionModes mode) {
    this->select(pos.line, pos.column, mode);
}

void QHexCursor::moveTo(qsizetype line, int column, int nibbleindex,
                        bool clearSelection) {
    m_position.line = line;
    m_position.column = qMax(0, column); // fix the bug by wingsummer
    m_position.nibbleindex = nibbleindex;

    m_selection = m_position;

    if (clearSelection) {
        m_sels.clear();
    }

    if (clearSelection) {
        emit selectionChanged();
    }

    emit positionChanged();
}

void QHexCursor::select(qsizetype line, int column, SelectionModes modes) {
    if (modes.testFlag(SelectionPreview)) {
        m_selection.line = line;
        m_selection.column = qMax(0, column); // fix the bug by wingsummer
        m_selection.lineWidth = m_position.lineWidth;
        m_selection.nibbleindex = 0;

        modes.setFlag(SelectionPreview, false);
        m_preMode = SelectionMode(int(modes));
    } else {
        QHexSelection sel;

        sel.begin = m_position;
        sel.begin.nibbleindex = 1;

        sel.end.line = line;
        sel.end.column = column;
        sel.end.lineWidth = m_position.lineWidth;
        sel.end.nibbleindex = 0;

        sel.normalize();

        switch (modes) {
        case SelectionAdd:
            m_sels.mergeAdd(sel);
            break;
        case SelectionNormal:
            m_sels.clear();
            m_sels.append(sel);
            break;
        case SelectionRemove:
            m_sels.mergeRemove(sel);
            break;
        }
    }

    emit selectionChanged();
}

void QHexCursor::moveTo(qsizetype offset, bool clearSelection) {
    auto line = offset / m_lineWidth;
    this->moveTo(line, int(offset - (line * m_lineWidth)), clearSelection);
}

void QHexCursor::setPos(qsizetype offset, int nibbleindex,
                        bool clearSelection) {
    auto line = offset / m_lineWidth;
    this->moveTo(line, int(offset - (line * m_lineWidth)), nibbleindex,
                 clearSelection);
}

void QHexCursor::select(qsizetype length, QHexCursor::SelectionModes mode) {
    auto div = std::div(qsizetype(m_position.column + length - 1),
                        qsizetype(m_lineWidth));
    this->select(m_position.line + div.quot, div.rem, mode);
}

void QHexCursor::setInsertionMode(QHexCursor::InsertionMode mode) {
    bool differentmode = (m_insertionmode != mode);
    m_insertionmode = mode;

    if (differentmode)
        emit insertionModeChanged();
}

void QHexCursor::setLineWidth(quint8 width) {
    m_lineWidth = width;
    m_position.lineWidth = width;
    m_selection.lineWidth = width;

    for (auto &sel : m_sels) {
        sel.begin.lineWidth = width;
        sel.end.lineWidth = width;
    }
}

void QHexCursor::switchInsertionMode() {
    if (m_insertionmode == QHexCursor::OverwriteMode)
        m_insertionmode = QHexCursor::InsertMode;
    else
        m_insertionmode = QHexCursor::OverwriteMode;

    emit insertionModeChanged();
}

bool QHexCursor::hasPreviewSelection() const {
    return m_selection != m_position;
}

bool QHexCursor::isLineSelected(const QHexSelection &sel,
                                qsizetype line) const {
    Q_ASSERT(sel.isNormalized());

    if ((line >= sel.begin.line) && (line <= sel.end.line))
        return true;

    return false;
}

QHexSelection QHexCursor::previewSelection() const {
    QHexSelection sel;
    sel.begin = m_position;
    sel.end = m_selection;
    return sel.normalized();
}

void QHexCursor::setPreviewSelectionMode(SelectionMode mode) {
    m_preMode = mode;
}

QHexCursor::SelectionMode QHexCursor::previewSelectionMode() const {
    return m_preMode;
}

void QHexCursor::mergePreviewSelection() {
    auto ss = previewSelection();
    switch (m_preMode) {
    case SelectionNormal:
        if (m_sels.isEmpty()) {
            m_sels.append(ss);
        }
        break;
    case SelectionAdd:
        m_sels.mergeAdd(ss);
        break;
    case SelectionRemove:
        m_sels.mergeRemove(ss);
        break;
    case SelectionSingle:
        m_sels.clear();
        m_sels.append(ss);
        break;
    case SelectionPreview:
        // should not go here
        break;
    }
    clearPreviewSelection();

    emit selectionChanged();
}
