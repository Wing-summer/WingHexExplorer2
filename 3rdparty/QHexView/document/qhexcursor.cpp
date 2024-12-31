#include "qhexcursor.h"
#include <QWidget>

#include <QtConcurrent/QtConcurrentMap>

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
    return m_sels.at(index).start;
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
        return qAbs(m_position - m_selection);
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
    this->select(pos.line, pos.column, pos.nibbleindex, mode);
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

    emit positionChanged();
}

void QHexCursor::select(qsizetype line, int column, int nibbleindex,
                        SelectionModes modes) {
    if (modes.testFlag(SelectionPreview)) {
        m_selection.line = line;
        m_selection.column = qMax(0, column); // fix the bug by wingsummer
        m_selection.lineWidth = m_position.lineWidth;
        m_selection.nibbleindex = nibbleindex;

        modes.setFlag(SelectionPreview, false);
        m_preMode = SelectionMode(int(modes));
    } else {
        QHexSelection sel;

        sel.start = m_position;

        sel.end.line = line;
        sel.end.column = column;
        sel.end.lineWidth = m_position.lineWidth;
        sel.end.nibbleindex = nibbleindex;

        sel.normalize();

        switch (modes) {
        case SelectionAdd:
            mergeAdd(sel);
            break;
        case SelectionNormal:
            m_sels.clear();
            m_sels.append(sel);
            break;
        case SelectionRemove:
            mergeRemove(sel);
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
    this->select(m_position.line,
                 std::min(m_lineWidth - 1, int(m_position.column + length - 1)),
                 1, mode);
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

    for (auto &sel : m_sels) {
        sel.start.lineWidth = width;
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

void QHexCursor::mergeRemove(const QHexSelection &sel) {
    Q_ASSERT(sel.isNormalized());

    QList<QHexSelection> buffer;
    QMutex locker;
    QtConcurrent::blockingMap(m_sels,
                              [&buffer, &locker, &sel](QHexSelection &s) {
                                  auto r = s.removeSelection(sel);
                                  if (r.has_value()) {
                                      QMutexLocker l(&locker);
                                      buffer.append(r.value());
                                  }
                              });

    // clean up invalid selections
    auto cleanup = [](const QHexSelection &s) { return s.start == s.end; };
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_sels.removeIf(cleanup);
#else
    m_sels.erase(std::remove_if(m_sels.begin(), m_sels.end(), cleanup));
#endif

    QtConcurrent::blockingMap(
        buffer, [&locker, this](QHexSelection &s) { mergeAdd(s, &locker); });
}

void QHexCursor::mergeAdd(const QHexSelection &sel, QMutex *locker) {
    bool merged = false;
    Q_ASSERT(sel.isNormalized());

    for (auto p = m_sels.begin(); p != m_sels.end(); ++p) {
        merged = p->mergeSelection(sel, locker);
        if (merged) {
            break;
        }
    }

    if (!merged) {
        m_sels.append(sel);
    }
}

bool QHexCursor::isLineSelected(const QHexSelection &sel,
                                qsizetype line) const {
    auto first = std::min(sel.start.line, sel.end.line);
    auto last = std::max(sel.start.line, sel.end.line);

    if ((line >= first) && (line <= last))
        return true;

    return false;
}

QHexSelection QHexCursor::previewSelection() const {
    QHexSelection sel;
    sel.start = m_position;
    sel.end = m_selection;
    return sel;
}

void QHexCursor::setPreviewSelectionMode(SelectionMode mode) {
    m_preMode = mode;
}

QHexCursor::SelectionMode QHexCursor::previewSelectionMode() const {
    return m_preMode;
}

void QHexCursor::mergePreviewSelection() {
    auto ss = QHexSelection{m_position, m_selection}.normalized();
    switch (m_preMode) {
    case SelectionNormal:
        if (m_sels.isEmpty()) {
            m_sels.append(ss);
        }
        break;
    case SelectionAdd:
        mergeAdd(ss);
        break;
    case SelectionRemove:
        mergeRemove(ss);
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
