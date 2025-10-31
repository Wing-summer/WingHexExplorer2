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
** The original License is LPGL from Andres6936/QHexEdit. I have modified a lot
** so I decide to change the Open Source License. You can use the original
** library under LPGL. Thanks for Andres6936's efforts.
** =============================================================================
*/

#include "qhexdocument.h"
#include "commands/baseaddrcommand.h"
#include "commands/bookmark/bookmarkaddcommand.h"
#include "commands/bookmark/bookmarkclearcommand.h"
#include "commands/bookmark/bookmarkremovecommand.h"
#include "commands/bookmark/bookmarkreplacecommand.h"
#include "commands/hex/insertcommand.h"
#include "commands/hex/removecommand.h"
#include "commands/hex/replacecommand.h"
#include "document/commands/hex/appendcommand.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QFile>

/*======================*/
// added by wingsummer

QList<qsizetype> QHexDocument::getLineBookmarksPos(qsizetype line) {
    QList<qsizetype> pos;
    auto begin = m_hexlinewidth * line;
    auto end = m_hexlinewidth + begin - 1;

    auto lbound = _bookmarks.lowerBound(begin);
    auto ubound = _bookmarks.upperBound(end);

    for (auto p = lbound; p != ubound; ++p) {
        pos.append(p.key());
    }

    return pos;
}

bool QHexDocument::lineHasBookMark(qsizetype line) {
    auto begin = m_hexlinewidth * line;
    auto end = m_hexlinewidth + begin - 1;

    auto lbound = _bookmarks.lowerBound(begin);
    auto ubound = _bookmarks.upperBound(end);

    return lbound != ubound;
}

void QHexDocument::addUndoCommand(QUndoCommand *command) {
    if (command) {
        m_undostack->push(command);
    }
}

void QHexDocument::setMetabgVisible(bool b) {
    m_metabg = b;
    Q_EMIT documentChanged();
    Q_EMIT metabgVisibleChanged(b);
}

void QHexDocument::setMetafgVisible(bool b) {
    m_metafg = b;
    Q_EMIT documentChanged();
    Q_EMIT metafgVisibleChanged(b);
}

void QHexDocument::setMetaCommentVisible(bool b) {
    m_metacomment = b;
    Q_EMIT documentChanged();
    Q_EMIT metaCommentVisibleChanged(b);
}

bool QHexDocument::metabgVisible() const { return m_metabg; }

bool QHexDocument::metafgVisible() const { return m_metafg; }

bool QHexDocument::metaCommentVisible() const { return m_metacomment; }

void QHexDocument::insertBookMarkAdjust(qsizetype offset, qsizetype length) {
    QMap<qsizetype, QString> bms;

    if (_bookmarks.isEmpty()) {
        return;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto rmbegin = _bookmarks.lowerBound(offset);
#else
    auto rmbegin = std::as_const(_bookmarks).lowerBound(offset);
#endif
    auto addbegin = _bookmarks.upperBound(offset);

    for (auto p = addbegin; p != _bookmarks.end(); ++p) {
        bms.insert(p.key() + length, p.value());
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    for (auto it = rmbegin; it != _bookmarks.end();) {
        it = _bookmarks.erase(it);
    }
#else
    _bookmarks.erase(rmbegin, _bookmarks.cend());
#endif

    _bookmarks.insert(bms);
}

QMap<qsizetype, QString> QHexDocument::removeBookMarkAdjust(qsizetype offset,
                                                            qsizetype length) {
    QMap<qsizetype, QString> rmbms;
    QMap<qsizetype, QString> bms;

    if (_bookmarks.isEmpty()) {
        return {};
    }

    auto rmbegin = std::as_const(_bookmarks).lowerBound(offset);
    auto addbegin = std::as_const(_bookmarks).upperBound(offset);

    for (auto p = rmbegin; p != addbegin; ++p) {
        rmbms.insert(p.key(), p.value());
    }

    for (auto p = addbegin; p != _bookmarks.cend(); ++p) {
        bms.insert(p.key() - length, p.value());
    }

    _bookmarks.erase(rmbegin, _bookmarks.cend());
    _bookmarks.insert(bms);

    return rmbms;
}

void QHexDocument::insertBookMarkAdjustRevert(qsizetype offset,
                                              qsizetype length) {
    QMap<qsizetype, QString> bms;

    if (_bookmarks.isEmpty()) {
        return;
    }

    auto rmbegin = std::as_const(_bookmarks).lowerBound(offset);
    auto addbegin = std::as_const(_bookmarks).upperBound(offset);

    for (auto p = addbegin; p != _bookmarks.cend(); ++p) {
        bms.insert(p.key() - length, p.value());
    }

    _bookmarks.erase(rmbegin, _bookmarks.cend());
    _bookmarks.insert(bms);
}

void QHexDocument::removeBookMarkAdjustRevert(
    const QMap<qsizetype, QString> &rmbms, qsizetype offset, qsizetype length) {
    QMap<qsizetype, QString> bms;

    if (!_bookmarks.isEmpty()) {
        auto rmbegin = std::as_const(_bookmarks).lowerBound(offset);
        auto addbegin = std::as_const(_bookmarks).upperBound(offset);

        for (auto p = addbegin; p != _bookmarks.cend(); ++p) {
            bms.insert(p.key() + length, p.value());
        }

        _bookmarks.erase(rmbegin, _bookmarks.cend());
        _bookmarks.insert(bms);
    }

    _bookmarks.insert(rmbms);
}

bool QHexDocument::isDocSaved() const { return m_undostack->isClean(); }

void QHexDocument::setDocSaved() { m_undostack->setClean(); }

bool QHexDocument::isReadOnly() const { return m_readonly; }
bool QHexDocument::isKeepSize() const { return m_keepsize; }
bool QHexDocument::isLocked() const { return m_islocked; }

void QHexDocument::setLockKeepSize(bool b) { m_lockKeepSize = b; }

bool QHexDocument::lockKeepSize() const { return m_lockKeepSize; }

bool QHexDocument::setLockedFile(bool b) {
    if (m_readonly)
        return false;
    m_islocked = b;
    Q_EMIT documentLockedFile(b);
    return true;
}
bool QHexDocument::setKeepSize(bool b) {
    if (m_readonly || m_lockKeepSize)
        return false;

    m_keepsize = b;
    Q_EMIT documentKeepSize(b);
    return true;
}

const QMap<qsizetype, QString> &QHexDocument::bookMarks() const {
    return _bookmarks;
}

bool QHexDocument::AddBookMark(qsizetype pos, const QString &comment) {
    auto cmd = MakeAddBookMark(nullptr, pos, comment);
    if (cmd) {
        m_undostack->push(cmd);
    }
    return true;
}

bool QHexDocument::RemoveBookMark(qsizetype pos) {
    auto cmd = MakeRemoveBookMark(nullptr, pos);
    if (cmd) {
        m_undostack->push(cmd);
    }
    return true;
}

bool QHexDocument::ModBookMark(qsizetype pos, const QString &comment) {
    auto cmd = MakeModBookMark(nullptr, pos, comment);
    if (cmd) {
        m_undostack->push(cmd);
    }
    return true;
}

bool QHexDocument::ClearBookMark() {
    auto cmd = MakeClearBookMark(nullptr);
    if (cmd) {
        m_undostack->push(cmd);
    }
    return true;
}

bool QHexDocument::SetBaseAddress(quintptr addr) {
    auto cmd = MakeSetBaseAddr(nullptr, addr);
    if (cmd) {
        m_undostack->push(cmd);
    }
    return true;
}

bool QHexDocument::isMetaDataUnsaved() const {
    for (int i = UndoCommandBase::UndoID_MetaCmdBegin;
         i < UndoCommandBase::UndoID_MetaCmdEnd; ++i) {
        if (m_undostack->hasUnsavedId(i)) {
            return true;
        }
    }
    return false;
}

QUndoCommand *QHexDocument::MakeAddBookMark(QUndoCommand *parent, qsizetype pos,
                                            QString comment) {
    if (pos < length()) {
        return new BookMarkAddCommand(this, pos, comment, parent);
    }
    return nullptr;
}

QUndoCommand *QHexDocument::MakeRemoveBookMark(QUndoCommand *parent,
                                               qsizetype pos) {
    if (_bookmarks.contains(pos)) {
        return new BookMarkRemoveCommand(this, pos, bookMark(pos), parent);
    }
    return nullptr;
}

QUndoCommand *QHexDocument::MakeModBookMark(QUndoCommand *parent, qsizetype pos,
                                            QString comment) {
    if (_bookmarks.contains(pos)) {
        return new BookMarkReplaceCommand(this, pos, comment, bookMark(pos),
                                          parent);
    }
    return nullptr;
}

QUndoCommand *QHexDocument::MakeClearBookMark(QUndoCommand *parent) {
    if (_bookmarks.isEmpty()) {
        return nullptr;
    }
    return new BookMarkClearCommand(this, _bookmarks, parent);
}

QUndoCommand *QHexDocument::MakeSetBaseAddr(QUndoCommand *parent,
                                            quintptr addr) {
    return new BaseAddrCommand(this, m_baseaddress, addr, parent);
}

bool QHexDocument::addBookMark(qsizetype pos, QString comment) {
    if (!existBookMark(pos)) {
        _bookmarks.insert(pos, comment);
        Q_EMIT documentChanged();
        Q_EMIT bookMarkChanged();
        return true;
    }
    return false;
}

QString QHexDocument::bookMark(qsizetype pos) { return _bookmarks.value(pos); }

bool QHexDocument::bookMarkExists(qsizetype pos) {
    return _bookmarks.contains(pos);
}

QList<qsizetype> QHexDocument::bookMarkRange(qsizetype begin, qsizetype end) {
    Q_ASSERT(end >= begin && begin >= 0);
    auto it = _bookmarks.lowerBound(begin);
    auto pend = _bookmarks.upperBound(end);

    QList<qsizetype> ret;
    while (it != pend) {
        ret.append(it.key());
        it++;
    }
    return ret;
}

qsizetype QHexDocument::bookMarkPos(qsizetype index) {
    Q_ASSERT(index >= 0 && index < _bookmarks.size());
    return *(std::next(_bookmarks.keyBegin(), index));
}

bool QHexDocument::RemoveBookMarks(const QList<qsizetype> &pos) {
    if (!m_keepsize)
        return false;
    if (pos.isEmpty()) {
        return true;
    }
    if (pos.size() == 1) {
        auto p = pos.front();
        m_undostack->push(new BookMarkRemoveCommand(this, p, bookMark(p)));
    } else {
        m_undostack->beginMacro(
            QStringLiteral("[B-G] {cnt: %1}").arg(pos.size()));
        for (auto &p : pos) {
            m_undostack->push(new BookMarkRemoveCommand(this, p, bookMark(p)));
        }
        m_undostack->endMacro();
    }
    Q_EMIT documentChanged();
    return true;
}

bool QHexDocument::removeBookMark(qsizetype pos) {
    if (m_keepsize) {
        auto ret = _bookmarks.remove(pos) != 0;
        if (ret) {
            Q_EMIT bookMarkChanged();
        }
        return ret;
    }
    return false;
}

bool QHexDocument::modBookMark(qsizetype pos, const QString &comment) {
    if (m_keepsize) {
        if (_bookmarks.contains(pos)) {
            _bookmarks[pos] = comment;
            Q_EMIT bookMarkChanged();
            return true;
        }
    }
    return false;
}

bool QHexDocument::clearBookMark() {
    if (m_keepsize) {
        _bookmarks.clear();
        Q_EMIT documentChanged();
        Q_EMIT bookMarkChanged();
        return true;
    }
    return false;
}

bool QHexDocument::existBookMark(qsizetype pos) {
    return _bookmarks.contains(pos);
}

qsizetype QHexDocument::bookMarksCount() const { return _bookmarks.count(); }

void QHexDocument::applyBookMarks(const QMap<qsizetype, QString> &books) {
    _bookmarks = books;
    Q_EMIT documentChanged();
    Q_EMIT bookMarkChanged();
}

void QHexDocument::findAllBytes(qsizetype begin, qsizetype end,
                                const QByteArray &b, QList<qsizetype> &results,
                                const std::function<bool()> &pred) {
    results.clear();
    if (!b.length())
        return;
    qsizetype p = begin > 0 ? begin : 0;
    qsizetype e = end > begin ? end : -1;
    auto offset = b.size();
    while (pred()) {
        p = findNext(p, b);
        if (p < 0 || (e > 0 && p > e)) {
            break;
        }

        if (results.size() > QHEXVIEW_FIND_LIMIT) {
            break;
        }
        results.append(p);
        p += offset;
    }
}

void QHexDocument::findAllBytesExt(qsizetype begin, qsizetype end,
                                   const QString &pattern,
                                   QList<qsizetype> &results,
                                   const std::function<bool()> &pred) {
    results.clear();
    if (end < 0) {
        end = m_buffer->length();
    }

    if (pattern.isEmpty() || end > m_buffer->length() || begin >= end) {
        return;
    }
    qsizetype pos = begin;
    qsizetype n = pattern.size();
    while (pos + n <= end && pred()) {
        qsizetype f = findNextExt(pos, pattern);
        if (f < 0 || f + n > end)
            break;
        results.append(f);
        pos = f + 1;
        if (results.size() >= QHEXVIEW_FIND_LIMIT) {
            break;
        }
    }
}

bool QHexDocument::insert(qsizetype offset, uchar b) {
    if (m_keepsize || m_readonly || m_islocked ||
        (offset < m_buffer->length() && m_metadata->hasMetadata()))
        return false;
    return this->insert(offset, QByteArray(1, char(b)));
}

bool QHexDocument::insert(qsizetype offset, const QByteArray &data) {
    if (m_keepsize || m_readonly || m_islocked ||
        (offset < m_buffer->length() && m_metadata->hasMetadata()))
        return false;
    return this->_insert(offset, data);
}

bool QHexDocument::replace(qsizetype offset, uchar b) {
    if (m_readonly || m_islocked)
        return false;
    return this->replace(offset, QByteArray(1, char(b)));
}

bool QHexDocument::replace(qsizetype offset, const QByteArray &data) {
    if (m_readonly || m_islocked)
        return false;
    return this->_replace(offset, data);
}

bool QHexDocument::remove(qsizetype offset, qsizetype len) {
    if (m_keepsize || m_readonly || m_islocked || m_metadata->hasMetadata())
        return false;
    return this->_remove(offset, len);
}

bool QHexDocument::_insert(qsizetype offset, uchar b) {
    return this->_insert(offset, QByteArray(1, char(b)));
}

bool QHexDocument::_insert(qsizetype offset, const QByteArray &data) {
    m_buffer->insert(offset, data);
    Q_EMIT documentChanged();
    return true;
}

bool QHexDocument::_replace(qsizetype offset, uchar b) {
    return this->_replace(offset, QByteArray(1, char(b)));
}

bool QHexDocument::_replace(qsizetype offset, const QByteArray &data) {
    m_buffer->replace(offset, data);
    Q_EMIT documentChanged();
    return true;
}

bool QHexDocument::_remove(qsizetype offset, qsizetype len) {
    m_buffer->remove(offset, len);
    Q_EMIT documentChanged();
    return true;
}

bool QHexDocument::parsePattern(const QString &pattern,
                                QList<PatternByte> &out) {
    out.clear();
    QString p = pattern;
    p.remove(' ');
    if (p.size() % 2 != 0) {
        return false;
    }
    for (int i = 0; i < p.size(); i += 2) {
        QChar hi = p[i], lo = p[i + 1];
        PatternByte pb;
        // high nibble
        if (hi != '?') {
            int v = hex2Int(hi);
            if (v < 0) {
                return false;
            }
            pb.value |= quint8(v << 4);
            pb.mask |= 0xF0;
        }
        // low nibble
        if (lo != '?') {
            int v = hex2Int(lo);
            if (v < 0) {
                return false;
            }
            pb.value |= quint8(v);
            pb.mask |= 0x0F;
        }
        out.append(pb);
    }
    return true;
}

int QHexDocument::hex2Int(const QChar &c) {
    ushort u = c.unicode();
    if (u >= '0' && u <= '9')
        return u - '0';
    else if (u >= 'A' && u <= 'F')
        return 10 + (u - 'A');
    else if (u >= 'a' && u <= 'f')
        return 10 + (u - 'a');
    else
        return -1;
}

qsizetype QHexDocument::findNextExt(qsizetype begin,
                                    const QList<PatternByte> &patterns) {
    if (patterns.isEmpty() || begin < 0 ||
        begin + patterns.size() > m_buffer->length())
        return -1;
    qsizetype n = patterns.size();
    for (qsizetype pos = begin; pos + n <= m_buffer->length(); ++pos) {
        bool ok = true;
        for (qsizetype i = 0; i < n; ++i) {
            if (!matchByte(m_buffer->at(pos + i), patterns[i])) {
                ok = false;
                break;
            }
        }
        if (ok)
            return pos;
    }
    return -1;
}

qsizetype QHexDocument::findPreviousExt(qsizetype begin,
                                        const QList<PatternByte> &patterns) {
    if (patterns.isEmpty() || begin < 0)
        return -1;
    qsizetype n = patterns.size();
    qsizetype maxStart = qMin(begin, m_buffer->length() - n);
    for (qsizetype pos = maxStart; pos >= 0; --pos) {
        bool ok = true;
        for (qsizetype i = 0; i < n; ++i) {
            if (!matchByte(m_buffer->at(pos + i), patterns[i])) {
                ok = false;
                break;
            }
        }
        if (ok)
            return pos;
    }
    return -1;
}

/*======================*/

// modified by wingsummer
QHexDocument::QHexDocument(QHexBuffer *buffer)
    : QObject(nullptr), m_baseaddress(0), m_readonly(false), m_keepsize(false),
      m_islocked(false) {
    Q_ASSERT(buffer);
    buffer->setParent(this);
    m_buffer = buffer;
    m_areaindent = DEFAULT_AREA_IDENTATION;
    m_hexlinewidth = DEFAULT_HEX_LINE_LENGTH;

    /*=======================*/
    // added by wingsummer
    m_readonly = !buffer->isWritable();
    if (m_readonly) {
        m_islocked = true;
        m_keepsize = true;
    }
    /*=======================*/

    m_undostack = new QHexUndoStack(this);
    m_metadata = new QHexMetadata(m_undostack, this);
    m_metadata->setLineWidth(m_hexlinewidth);

    connect(m_metadata, &QHexMetadata::metadataChanged, this,
            &QHexDocument::metaDataChanged);

    /*=======================*/
    // added by wingsummer
    connect(m_undostack, &QUndoStack::canUndoChanged, this,
            &QHexDocument::canUndoChanged);
    connect(m_undostack, &QUndoStack::canRedoChanged, this,
            &QHexDocument::canRedoChanged);
    connect(m_undostack, &QUndoStack::cleanChanged, this,
            &QHexDocument::documentSaved);

    /*=======================*/
}

QHexDocument::~QHexDocument() { m_buffer->close(); }

bool QHexDocument::isEmpty() const { return m_buffer->isEmpty(); }

bool QHexDocument::canUndo() const { return m_undostack->canUndo(); }
bool QHexDocument::canRedo() const { return m_undostack->canRedo(); }
qsizetype QHexDocument::length() const { return m_buffer->length(); }
quintptr QHexDocument::baseAddress() const { return m_baseaddress; }

int QHexDocument::areaIndent() const { return m_areaindent; }
void QHexDocument::setAreaIndent(quint8 value) { m_areaindent = value; }
int QHexDocument::hexLineWidth() const { return m_hexlinewidth; }

void QHexDocument::setHexLineWidth(quint8 value) {
    m_hexlinewidth = value;
    m_metadata->setLineWidth(value);
    Q_EMIT hexLineWidthChanged();
}

QHexMetadata *QHexDocument::metadata() const { return m_metadata; }

char QHexDocument::at(qsizetype offset) const {
    return char(m_buffer->at(offset));
}

bool QHexDocument::setBaseAddress(quint64 baseaddress) {
    if (m_baseaddress == baseaddress) {
        return false;
    }

    m_baseaddress = baseaddress;
    Q_EMIT documentChanged();
    return true;
}

void QHexDocument::undo() {
    m_undostack->undo();
    Q_EMIT documentChanged();
}

void QHexDocument::redo() {
    m_undostack->redo();
    Q_EMIT documentChanged();
}

void QHexDocument::beginMarco(const QString &text) {
    m_undostack->beginMacro(text);
}

void QHexDocument::endMarco() { m_undostack->endMacro(); }

bool QHexDocument::Insert(QHexCursor *cursor, qsizetype offset, uchar b,
                          int nibbleindex) {
    return this->Insert(cursor, offset, QByteArray(1, char(b)), nibbleindex);
}

bool QHexDocument::Replace(QHexCursor *cursor, qsizetype offset, uchar b,
                           int nibbleindex) {
    return this->Replace(cursor, offset, QByteArray(1, char(b)), nibbleindex);
}

bool QHexDocument::Insert(QHexCursor *cursor, qsizetype offset,
                          const QByteArray &data, int nibbleindex) {
    if (m_keepsize || m_readonly || m_islocked) {
        return false;
    }

    auto cmd = MakeInsert(nullptr, cursor, offset, data, nibbleindex);
    if (cmd) {
        m_undostack->push(cmd);
        Q_EMIT documentChanged();
    } else {
        return false;
    }

    return true;
}

void QHexDocument::Append(QHexCursor *cursor, uchar b, int nibbleindex) {
    this->Append(cursor, QByteArray(1, char(b)), nibbleindex);
}

void QHexDocument::Append(QHexCursor *cursor, const QByteArray &data,
                          int nibbleindex) {
    if (m_keepsize || m_readonly || m_islocked)
        return;
    auto cmd = MakeAppend(nullptr, cursor, data, nibbleindex);
    if (cmd) {
        m_undostack->push(cmd);
        Q_EMIT documentChanged();
    }
}

bool QHexDocument::Replace(QHexCursor *cursor, qsizetype offset,
                           const QByteArray &data, int nibbleindex) {
    if (m_readonly || m_islocked) {
        return false;
    }
    auto cmd = MakeReplace(nullptr, cursor, offset, data, nibbleindex);
    if (cmd) {
        m_undostack->push(cmd);
        Q_EMIT documentChanged();
    } else {
        return false;
    }
    return true;
}

bool QHexDocument::Remove(QHexCursor *cursor, qsizetype offset, qsizetype len,
                          int nibbleindex) {
    if (m_keepsize || m_readonly || m_islocked)
        return false;
    auto cmd = MakeRemove(nullptr, cursor, offset, len, nibbleindex);
    if (cmd) {
        m_undostack->push(cmd);
        Q_EMIT documentChanged();
    }
    return true;
}

QUndoCommand *QHexDocument::MakeInsert(QUndoCommand *parent, QHexCursor *cursor,
                                       qsizetype offset, uchar b,
                                       int nibbleindex) {
    return MakeInsert(parent, cursor, offset, QByteArray(1, char(b)),
                      nibbleindex);
}

QUndoCommand *QHexDocument::MakeInsert(QUndoCommand *parent, QHexCursor *cursor,
                                       qsizetype offset, const QByteArray &data,
                                       int nibbleindex) {
    if (offset > length()) {
        return nullptr;
    }
    return new InsertCommand(this, cursor, offset, data, nibbleindex, parent);
}

QUndoCommand *QHexDocument::MakeAppend(QUndoCommand *parent, QHexCursor *cursor,
                                       uchar b, int nibbleindex) {
    return MakeAppend(parent, cursor, QByteArray(1, char(b)), nibbleindex);
}

QUndoCommand *QHexDocument::MakeAppend(QUndoCommand *parent, QHexCursor *cursor,
                                       const QByteArray &data,
                                       int nibbleindex) {
    return new AppendCommand(this, cursor, data, nibbleindex, parent);
}

QUndoCommand *QHexDocument::MakeReplace(QUndoCommand *parent,
                                        QHexCursor *cursor, qsizetype offset,
                                        uchar b, int nibbleindex) {
    return MakeReplace(parent, cursor, offset, QByteArray(1, char(b)),
                       nibbleindex);
}

QUndoCommand *QHexDocument::MakeReplace(QUndoCommand *parent,
                                        QHexCursor *cursor, qsizetype offset,
                                        const QByteArray &data,
                                        int nibbleindex) {
    if (offset + data.length() > length()) {
        return nullptr;
    }
    return new ReplaceCommand(this, offset, data, cursor, nibbleindex, parent);
}

QUndoCommand *QHexDocument::MakeRemove(QUndoCommand *parent, QHexCursor *cursor,
                                       qsizetype offset, qsizetype len,
                                       int nibbleindex) {
    if (offset + len > length()) {
        return nullptr;
    }
    return new RemoveCommand(this, offset, len, cursor, nibbleindex, parent);
}

void QHexDocument::pushMakeUndo(QUndoCommand *cmd) {
    if (cmd) {
        m_undostack->push(cmd);
    }
}

QByteArray QHexDocument::read(qsizetype offset, qsizetype len) const {
    return m_buffer->read(offset, len);
}

bool QHexDocument::saveTo(QIODevice *device, bool cleanUndo) {
    auto r = m_buffer->save(device);
    if (r && cleanUndo) {
        m_undostack->setClean(); // added by wingsummer
    }
    return r;
}

qsizetype QHexDocument::findNext(qsizetype begin, const QByteArray &ba) {
    if (begin < 0) {
        return -1;
    }
    return m_buffer->indexOf(ba, begin);
}

qsizetype QHexDocument::findPrevious(qsizetype begin, const QByteArray &ba) {
    if (begin < 0) {
        return -1;
    }
    return m_buffer->lastIndexOf(ba, begin);
}

qsizetype QHexDocument::findNextExt(qsizetype begin, const QString &pattern) {
    QList<PatternByte> patterns;
    if (!parsePattern(pattern, patterns)) {
        return -1;
    }
    return findNextExt(begin, patterns);
}

qsizetype QHexDocument::findPreviousExt(qsizetype begin,
                                        const QString &pattern) {
    QList<PatternByte> patterns;
    if (!parsePattern(pattern, patterns)) {
        return -1;
    }
    return findPreviousExt(begin, patterns);
}

QHexBuffer *QHexDocument::buffer() const { return m_buffer; }

void QHexDocument::setBuffer(QHexBuffer *buffer) {
    if (buffer) {
        m_buffer->deleteLater();
        m_buffer = buffer;
    }
}

QUndoStack *QHexDocument::undoStack() const { return m_undostack; }
