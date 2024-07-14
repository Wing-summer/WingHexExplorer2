#include "qhexcursor.h"
#include <QWidget>

void QHexCursor::setSelection(qsizetype offset, qsizetype length) {
    auto lld = lldiv(offset, m_position.lineWidth);
    m_position.line = lld.quot;
    m_position.column = int(lld.rem);
    moveTo(m_position);
    lld = lldiv(offset + length - 1, m_position.lineWidth);
    m_selection.line = lld.quot;
    m_selection.column = int(lld.rem);
}

QHexCursor::QHexCursor(QObject *parent)
    : QObject(parent), m_insertionmode(QHexCursor::OverwriteMode) {
    m_position.line = m_position.column = 0;
    m_position.line = m_position.column = 0;

    m_selection.line = m_selection.column = 0;
    m_selection.line = m_selection.column = 0;

    m_position.nibbleindex = m_selection.nibbleindex = 1;
    setLineWidth(DEFAULT_HEX_LINE_LENGTH);
}

const QHexPosition &QHexCursor::selectionStart() const {
    if (m_position.line < m_selection.line)
        return m_position;

    if (m_position.line == m_selection.line) {
        if (m_position.column < m_selection.column)
            return m_position;
    }

    return m_selection;
}

const QHexPosition &QHexCursor::selectionEnd() const {
    if (m_position.line > m_selection.line)
        return m_position;

    if (m_position.line == m_selection.line) {
        if (m_position.column > m_selection.column)
            return m_position;
    }

    return m_selection;
}

const QHexPosition &QHexCursor::position() const { return m_position; }
QHexCursor::InsertionMode QHexCursor::insertionMode() const {
    return m_insertionmode;
}
qsizetype QHexCursor::selectionLength() const {
    return this->selectionEnd() - this->selectionStart() + 1;
}
qsizetype QHexCursor::currentLine() const { return m_position.line; }
int QHexCursor::currentColumn() const { return m_position.column; }
int QHexCursor::currentNibble() const { return m_position.nibbleindex; }
qsizetype QHexCursor::selectionLine() const { return m_selection.line; }
qsizetype QHexCursor::selectionColumn() const { return m_selection.column; }
int QHexCursor::selectionNibble() const { return m_selection.nibbleindex; }

bool QHexCursor::isLineSelected(qsizetype line) const {
    if (!this->hasSelection())
        return false;

    auto first = std::min(m_position.line, m_selection.line);
    auto last = std::max(m_position.line, m_selection.line);

    if ((line < first) || (line > last))
        return false;

    return true;
}

bool QHexCursor::hasSelection() const { return m_position != m_selection; }

void QHexCursor::clearSelection() {
    m_selection = m_position;
    emit positionChanged();
}

void QHexCursor::moveTo(const QHexPosition &pos) {
    this->moveTo(pos.line, pos.column, pos.nibbleindex);
}
void QHexCursor::select(const QHexPosition &pos) {
    this->select(pos.line, pos.column, pos.nibbleindex);
}

void QHexCursor::moveTo(qsizetype line, int column, int nibbleindex) {
    m_selection.line = line;
    m_selection.column = column;
    m_selection.nibbleindex = nibbleindex;

    this->select(line, column, nibbleindex);
}

void QHexCursor::select(qsizetype line, int column, int nibbleindex) {
    m_position.line = line;
    m_position.column = qMax(0, column); // fix the bug by wingsummer
    m_position.nibbleindex = nibbleindex;

    emit positionChanged();
}

void QHexCursor::moveTo(qsizetype offset) {
    auto line = offset / m_lineWidth;
    this->moveTo(line, int(offset - (line * m_lineWidth)));
}

void QHexCursor::setPos(qsizetype offset, int nibbleindex) {
    auto line = offset / m_lineWidth;
    this->moveTo(line, int(offset - (line * m_lineWidth)), nibbleindex);
}

void QHexCursor::select(qsizetype length) {
    this->select(
        m_position.line,
        std::min(m_lineWidth - 1, int(m_position.column + length - 1)));
}

void QHexCursor::selectOffset(qsizetype offset, qsizetype length) {
    this->moveTo(offset);
    this->select(length);
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
}

void QHexCursor::switchInsertionMode() {
    if (m_insertionmode == QHexCursor::OverwriteMode)
        m_insertionmode = QHexCursor::InsertMode;
    else
        m_insertionmode = QHexCursor::OverwriteMode;

    emit insertionModeChanged();
}
