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
#include "buffer/qfilebuffer.h"
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
    if (command)
        m_undostack->push(command);
}

void QHexDocument::setMetabgVisible(bool b) {
    m_metabg = b;
    emit documentChanged();
    emit metabgVisibleChanged(b);
}

void QHexDocument::setMetafgVisible(bool b) {
    m_metafg = b;
    emit documentChanged();
    emit metafgVisibleChanged(b);
}

void QHexDocument::setMetaCommentVisible(bool b) {
    m_metacomment = b;
    emit documentChanged();
    emit metaCommentVisibleChanged(b);
}

bool QHexDocument::metabgVisible() { return m_metabg; }

bool QHexDocument::metafgVisible() { return m_metafg; }

bool QHexDocument::metaCommentVisible() { return m_metacomment; }

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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto rmbegin = _bookmarks.lowerBound(offset);
#else
    auto rmbegin = std::as_const(_bookmarks).lowerBound(offset);
#endif
    auto addbegin = _bookmarks.upperBound(offset);

    for (auto p = rmbegin; p != addbegin; ++p) {
        rmbms.insert(p.key(), p.value());
    }

    for (auto p = addbegin; p != _bookmarks.end(); ++p) {
        bms.insert(p.key() - length, p.value());
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    for (auto it = rmbegin; it != _bookmarks.end();) {
        it = _bookmarks.erase(it);
    }
#else
    _bookmarks.erase(rmbegin, _bookmarks.cend());
#endif

    _bookmarks.insert(bms);

    return rmbms;
}

void QHexDocument::insertBookMarkAdjustRevert(qsizetype offset,
                                              qsizetype length) {
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
        bms.insert(p.key() - length, p.value());
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

void QHexDocument::removeBookMarkAdjustRevert(
    const QMap<qsizetype, QString> &rmbms, qsizetype offset, qsizetype length) {
    QMap<qsizetype, QString> bms;

    if (!_bookmarks.isEmpty()) {
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

    _bookmarks.insert(rmbms);
}

bool QHexDocument::isDocSaved() { return m_undostack->isClean(); }

bool QHexDocument::isUndoByteModified() { return m_bytesModFlag > 0; }

void QHexDocument::setDocSaved(bool b) {
    if (b) {
        m_undostack->setClean();
    }
    emit documentSaved(b);
}

bool QHexDocument::isReadOnly() { return m_readonly; }
bool QHexDocument::isKeepSize() { return m_keepsize; }
bool QHexDocument::isLocked() { return m_islocked; }

void QHexDocument::setLockKeepSize(bool b) { m_lockKeepSize = b; }

bool QHexDocument::lockKeepSize() const { return m_lockKeepSize; }

bool QHexDocument::setLockedFile(bool b) {
    if (m_readonly)
        return false;
    m_islocked = b;
    emit documentLockedFile(b);
    return true;
}
bool QHexDocument::setKeepSize(bool b) {
    if (m_readonly || m_lockKeepSize)
        return false;

    m_keepsize = b;
    emit documentKeepSize(b);
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

bool QHexDocument::addBookMark(qsizetype pos, QString comment) {
    if (!existBookMark(pos)) {
        _bookmarks.insert(pos, comment);
        setDocSaved(false);
        emit documentChanged();
        emit bookMarkChanged();
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
    m_undostack->beginMacro("RemoveBookMarks");
    for (auto p : pos) {
        m_undostack->push(new BookMarkRemoveCommand(this, p, bookMark(p)));
    }
    m_undostack->endMacro();
    emit documentChanged();
    return true;
}

bool QHexDocument::removeBookMark(qsizetype pos) {
    if (m_keepsize) {
        auto ret = _bookmarks.remove(pos) != 0;
        if (ret) {
            setDocSaved(false);
        }
        return ret;
    }
    return false;
}

bool QHexDocument::modBookMark(qsizetype pos, const QString &comment) {
    if (m_keepsize) {
        if (_bookmarks.contains(pos)) {
            _bookmarks[pos] = comment;
            setDocSaved(false);
            return true;
        }
    }
    return false;
}

bool QHexDocument::clearBookMark() {
    if (m_keepsize) {
        _bookmarks.clear();
        setDocSaved(false);
        emit documentChanged();
        emit bookMarkChanged();
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
    setDocSaved(false);
    emit documentChanged();
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

qsizetype QHexDocument::findAllBytesExt(qsizetype begin, qsizetype end,
                                        const QString &pattern,
                                        QList<qsizetype> &results,
                                        const std::function<bool()> &pred) {
    results.clear();
    auto patterns = parseConvertPattern(pattern);
    if (patterns.isEmpty()) {
        return 0;
    }

    qsizetype p = begin > 0 ? begin : 0;
    qsizetype e = end > begin ? end : -1;

    qsizetype offset = 0;
    for (auto &p : patterns) {
        if (std::holds_alternative<QByteArray>(p)) {
            offset += std::get<QByteArray>(p).length();
        } else if (std::holds_alternative<size_t>(p)) {
            offset += std::get<size_t>(p);
        } else if (std::holds_alternative<HexWildItem>(p)) {
            offset += 1;
        }
    }

    while (pred()) {
        p = findNextExt(p, pattern);
        if (p < 0 || (e > 0 && p > e)) {
            break;
        }

        if (results.size() > QHEXVIEW_FIND_LIMIT) {
            break;
        }
        results.append(p);
        p += offset;
    }

    return offset;
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
    setDocSaved(false);
    emit documentChanged();
    return true;
}

bool QHexDocument::_replace(qsizetype offset, uchar b) {
    return this->_replace(offset, QByteArray(1, char(b)));
}

bool QHexDocument::_replace(qsizetype offset, const QByteArray &data) {
    m_buffer->replace(offset, data);
    setDocSaved(false);
    emit documentChanged();
    return true;
}

bool QHexDocument::_remove(qsizetype offset, qsizetype len) {
    m_buffer->remove(offset, len);
    setDocSaved(false);
    emit documentChanged();
    return true;
}

qsizetype QHexDocument::findNextExt(qsizetype begin,
                                    const QList<FindStep> &patterns) {
    auto op = [this](qsizetype &pos, const FindStep &step,
                     qsizetype *begin = nullptr) -> bool {
        if (pos < 0 || pos >= length()) {
            return false;
        }
        if (std::holds_alternative<QByteArray>(step)) {
            auto v = std::get<QByteArray>(step);
            auto len = v.length();
            auto r = findNext(pos, v);
            if (r >= 0) {
                if (begin) {
                    *begin = r;
                } else {
                    if (r != pos) {
                        pos = -1;
                        return false;
                    }
                }
                pos = r + len;
                return true;
            } else {
                pos = -1;
                return false;
            }
        } else if (std::holds_alternative<HexWildItem>(step)) {
            auto v = std::get<HexWildItem>(step);
            auto wc = uchar(at(pos));
            pos += 1;

            if (v.higher == '?') {
                if ((wc & 0xf) == v.lower) {
                    return true;
                }
            } else {
                if ((wc >> 4) == v.higher) {
                    return true;
                }
            }
        } else if (std::holds_alternative<size_t>(step)) {
            auto v = std::get<size_t>(step);
            pos += v;
            if (v + pos < length()) {
                return true;
            }
        }
        return false;
    };

    while (begin < length()) {
        auto pos = begin;

        auto p = patterns.cbegin();
        auto r = op(pos, *p, &begin);
        if (!r) {
            if (pos < 0) {
                return -1;
            }
            continue;
        }
        ++p;

        bool ok = true;
        for (; p != patterns.cend(); ++p) {
            auto r = op(pos, *p);
            if (!r) {
                ok = false;
                if (pos < 0) {
                    return -1;
                }
                begin = pos;
                break;
            }
        }

        if (ok) {
            return begin;
        }
    }

    return -1;
}

qsizetype QHexDocument::findPreviousExt(qsizetype begin,
                                        const QList<FindStep> &patterns) {
    auto op = [this](qsizetype &pos, const FindStep &step,
                     qsizetype *begin = nullptr) -> bool {
        if (pos < 0 || pos >= length()) {
            return false;
        }
        if (std::holds_alternative<QByteArray>(step)) {
            auto v = std::get<QByteArray>(step);
            auto len = v.length();
            auto r = findPrevious(pos, v);
            if (r >= 0) {
                if (begin) {
                    *begin = r;
                } else {
                    if (r + len != pos) {
                        pos = -1;
                        return false;
                    }
                }
                pos = r - len;
                return true;
            } else {
                pos = -1;
                return false;
            }
        } else if (std::holds_alternative<HexWildItem>(step)) {
            auto v = std::get<HexWildItem>(step);
            auto wc = uchar(at(pos));
            pos -= 1;

            if (v.higher == '?') {
                if ((wc & 0xf) == v.lower) {
                    return true;
                }
            } else {
                if ((wc >> 4) == v.higher) {
                    return true;
                }
            }
        } else if (std::holds_alternative<size_t>(step)) {
            auto v = std::get<size_t>(step);
            pos -= v;
            if (v - pos >= 0) {
                return true;
            }
        }
        return false;
    };

    while (begin >= 0) {
        auto pos = begin;

        auto p = patterns.crbegin();
        auto r = op(pos, *p, &begin);
        if (!r) {
            if (pos < 0) {
                return -1;
            }
            continue;
        }
        ++p;

        bool ok = true;
        for (; p != patterns.crend(); ++p) {
            auto r = op(pos, *p);
            if (!r) {
                ok = false;
                if (pos < 0) {
                    return -1;
                }
                begin = pos;
                break;
            }
        }

        if (ok) {
            return begin;
        }
    }

    return -1;
}

QList<QHexDocument::FindStep>
QHexDocument::parseConvertPattern(const QString &pattern) {
    // process hex pattern
    QList<HexWildItem> words;
    std::optional<uchar> higher;
    for (auto pchar = pattern.cbegin(); pchar != pattern.cend(); ++pchar) {
        if (pchar->isSpace()) {
            if (higher) {
                return {};
            } else {
                continue;
            }
        }

        auto c = pchar->unicode();
        if (c >= '0' && c <= '9') {
            if (higher) {
                HexWildItem item;
                item.higher = higher.value();
                item.lower = uchar(c) - '0';
                words.append(item);
                higher.reset();
            } else {
                higher = uchar(c) - '0';
            }
        } else if (c >= 'A' && c <= 'F') {
            if (higher) {
                HexWildItem item;
                item.higher = higher.value();
                item.lower = uchar(c) - 'A' + 10;
                words.append(item);
                higher.reset();
            } else {
                higher = uchar(c) - 'A' + 10;
            }
        } else if (c >= 'a' && c <= 'f') {
            if (higher) {
                HexWildItem item;
                item.higher = higher.value();
                item.lower = uchar(c) - 'a' + 10;
                words.append(item);
                higher.reset();
            } else {
                higher = uchar(c) - 'a' + 10;
            }
        } else if (c == '?') {
            if (higher) {
                HexWildItem item;
                item.higher = higher.value();
                item.lower = '?';
                words.append(item);
                higher.reset();
            } else {
                higher = '?';
            }
        }
    }

    if (higher) {
        return {};
    }

    if (!words.isEmpty()) {
        QList<FindStep> steps;

        // parsing...
        QByteArray buffer;
        size_t len = 0;
        for (auto pw = words.cbegin(); pw != words.cend(); ++pw) {
            auto higher = pw->higher;
            auto lower = pw->lower;
            if (higher == '?' || lower == '?') {
                if (higher == '?' && lower == '?') {
                    if (!buffer.isEmpty()) {
                        steps.append(buffer);
                        buffer.clear();
                    }
                    len++;
                } else {
                    if (len != 0) {
                        steps.append(len);
                        len = 0;
                    }
                    if (!buffer.isEmpty()) {
                        steps.append(buffer);
                        buffer.clear();
                    }
                    HexWildItem item;
                    item.higher = higher;
                    item.lower = lower;
                    steps.append(item);
                }
            } else {
                if (len != 0) {
                    steps.append(len);
                    len = 0;
                }
                buffer.append(char(pw->higher << 4 | pw->lower));
            }
        }

        // clean up
        if (len != 0) {
            steps.append(len);
        }
        if (!buffer.isEmpty()) {
            steps.append(buffer);
        }
        return steps;
    }
    return {};
}

/*======================*/

// modified by wingsummer
QHexDocument::QHexDocument(QHexBuffer *buffer, bool readonly)
    : QObject(nullptr), m_baseaddress(0), m_readonly(false), m_keepsize(false),
      m_islocked(false) {

    m_buffer = buffer;
    m_areaindent = DEFAULT_AREA_IDENTATION;
    m_hexlinewidth = DEFAULT_HEX_LINE_LENGTH;

    /*=======================*/
    // added by wingsummer
    m_readonly = readonly;
    if (m_readonly) {
        m_islocked = true;
        m_keepsize = true;
    }
    /*=======================*/

    m_undostack = new QUndoStack(this);
    m_metadata = new QHexMetadata(m_undostack, this);
    m_metadata->setLineWidth(m_hexlinewidth);

    connect(m_metadata, &QHexMetadata::metadataChanged, this, [this] {
        setDocSaved(false);
        emit metaDataChanged();
    });

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
    emit hexLineWidthChanged();
}

QHexMetadata *QHexDocument::metadata() const { return m_metadata; }

char QHexDocument::at(qsizetype offset) const {
    return char(m_buffer->at(offset));
}

void QHexDocument::SetBaseAddress(quint64 baseaddress) {
    m_undostack->push(new BaseAddrCommand(this, m_baseaddress, baseaddress));
}

void QHexDocument::setBaseAddress(quint64 baseaddress) {
    if (m_baseaddress == baseaddress)
        return;

    m_baseaddress = baseaddress;
    emit documentChanged();
}

void QHexDocument::sync() { emit documentChanged(); }

void QHexDocument::undo() {
    m_undostack->undo();
    emit documentChanged();
}

void QHexDocument::redo() {
    m_undostack->redo();
    emit documentChanged();
}

void QHexDocument::beginMarco(const QString &text) {
    m_undostack->beginMacro(text);
}

void QHexDocument::endMarco() { m_undostack->endMacro(); }

void QHexDocument::Insert(QHexCursor *cursor, qsizetype offset, uchar b,
                          int nibbleindex) {
    this->Insert(cursor, offset, QByteArray(1, char(b)), nibbleindex);
}

void QHexDocument::Replace(QHexCursor *cursor, qsizetype offset, uchar b,
                           int nibbleindex) {
    this->Replace(cursor, offset, QByteArray(1, char(b)), nibbleindex);
}

void QHexDocument::Insert(QHexCursor *cursor, qsizetype offset,
                          const QByteArray &data, int nibbleindex) {
    if (m_keepsize || m_readonly || m_islocked)
        return;

    auto cmd = MakeInsert(nullptr, cursor, offset, data, nibbleindex);
    if (cmd) {
        m_undostack->push(cmd);
    }

    emit documentChanged();
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
    }
    emit documentChanged();
}

void QHexDocument::Replace(QHexCursor *cursor, qsizetype offset,
                           const QByteArray &data, int nibbleindex) {
    if (m_readonly || m_islocked)
        return;
    auto cmd = MakeReplace(nullptr, cursor, offset, data, nibbleindex);
    if (cmd) {
        m_undostack->push(cmd);
    }
    emit documentChanged();
}

bool QHexDocument::Remove(QHexCursor *cursor, qsizetype offset, qsizetype len,
                          int nibbleindex) {
    if (m_keepsize || m_readonly || m_islocked)
        return false;
    auto cmd = MakeRemove(nullptr, cursor, offset, len, nibbleindex);
    if (cmd) {
        m_undostack->push(cmd);
    }
    emit documentChanged();
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
    if (!device->isWritable())
        return false;

    m_buffer->write(device);
    if (cleanUndo)
        m_undostack->setClean(); // added by wingsummer

    return true;
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
    auto patterns = parseConvertPattern(pattern);
    if (patterns.isEmpty()) {
        return -1;
    }

    return findNextExt(begin, patterns);
}

qsizetype QHexDocument::findPreviousExt(qsizetype begin,
                                        const QString &pattern) {
    auto patterns = parseConvertPattern(pattern);
    if (patterns.isEmpty()) {
        return -1;
    }

    return findPreviousExt(begin, patterns);
}

QHexDocument *QHexDocument::fromLargeFile(const QString &filename,
                                          bool readonly) {

    auto f = new QFile;
    if (!filename.isEmpty()) {
        f->setFileName(filename);
        QHexBuffer *hexbuffer = new QFileBuffer();
        if (f->open(readonly ? QFile::ReadOnly : QFile::ReadWrite) &&
            hexbuffer->read(f)) {
            return new QHexDocument(hexbuffer,
                                    readonly); // modified by wingsummer
        } else {
            delete hexbuffer;
        }
    } else {
        delete f;
        return new QHexDocument(new QFileBuffer(), readonly);
    }

    return nullptr;
}

QHexBuffer *QHexDocument::buffer() const { return m_buffer; }
