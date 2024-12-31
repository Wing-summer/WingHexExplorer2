#include "qhexdocument.h"
#include "buffer/qfilebuffer.h"
#include "buffer/qfileregionbuffer.h"
#include "commands/baseaddrcommand.h"
#include "commands/bookmark/bookmarkaddcommand.h"
#include "commands/bookmark/bookmarkclearcommand.h"
#include "commands/bookmark/bookmarkremovecommand.h"
#include "commands/bookmark/bookmarkreplacecommand.h"
#include "commands/hex/insertcommand.h"
#include "commands/hex/removecommand.h"
#include "commands/hex/replacecommand.h"
#include "commands/meta/metashowcommand.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QFile>

#ifdef Q_OS_WIN
#include "qstoragedevice.h"
#endif

/*======================*/
// added by wingsummer

QList<qsizetype> QHexDocument::getLineBookmarksPos(qsizetype line) {
    QList<qsizetype> pos;
    auto begin = m_hexlinewidth * line;
    auto end = m_hexlinewidth + begin;

    auto lbound = _bookmarks.lowerBound(begin);
    auto ubound = _bookmarks.upperBound(end);

    for (auto p = lbound; p != ubound; ++p) {
        pos.append(p.key());
    }

    return pos;
}

bool QHexDocument::lineHasBookMark(qsizetype line) {
    auto begin = m_hexlinewidth * line;
    auto end = m_hexlinewidth + begin;

    auto lbound = _bookmarks.lowerBound(begin);
    auto ubound = _bookmarks.upperBound(end);

    return lbound != ubound;
}

void QHexDocument::addUndoCommand(QUndoCommand *command) {
    if (command)
        m_undostack->push(command);
}

void QHexDocument::SetMetaVisible(bool b) {
    m_undostack->push(new MetaShowCommand(this, ShowType::All, b));
}

void QHexDocument::SetMetabgVisible(bool b) {
    m_undostack->push(new MetaShowCommand(this, ShowType::BgColor, b));
}

void QHexDocument::SetMetafgVisible(bool b) {
    m_undostack->push(new MetaShowCommand(this, ShowType::FgColor, b));
}

void QHexDocument::SetMetaCommentVisible(bool b) {
    m_undostack->push(new MetaShowCommand(this, ShowType::Comment, b));
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

void QHexDocument::removeBookMarkAdjust(qsizetype offset, qsizetype length) {
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

bool QHexDocument::isDocSaved() { return m_isSaved; }

void QHexDocument::setDocSaved(bool b) {
    if (b) {
        m_undostack->setClean();
    }
    m_isSaved = b;
    emit documentSaved(b);
}

bool QHexDocument::isReadOnly() { return m_readonly; }
bool QHexDocument::isKeepSize() { return m_keepsize; }
bool QHexDocument::isLocked() { return m_islocked; }

bool QHexDocument::setLockedFile(bool b) {
    if (m_readonly)
        return false;
    m_islocked = b;
    emit documentLockedFile(b);
    return true;
}
bool QHexDocument::setKeepSize(bool b) {
    if (m_readonly)
        return false;

    m_keepsize = b;
    emit documentKeepSize(b);
    return true;
}

const QMap<qsizetype, QString> &QHexDocument::bookMarks() const {
    return _bookmarks;
}

bool QHexDocument::AddBookMark(qsizetype pos, QString comment) {
    if (!m_keepsize)
        return false;
    m_undostack->push(new BookMarkAddCommand(this, pos, comment));
    return true;
}

bool QHexDocument::RemoveBookMark(qsizetype pos) {
    m_undostack->push(new BookMarkRemoveCommand(this, pos, bookMark(pos)));
    return true;
}

bool QHexDocument::ModBookMark(qsizetype pos, QString comment) {
    if (!m_keepsize)
        return false;
    m_undostack->push(
        new BookMarkReplaceCommand(this, pos, comment, bookMark(pos)));
    return true;
}

bool QHexDocument::ClearBookMark() {
    if (!m_keepsize)
        return false;
    m_undostack->push(new BookMarkClearCommand(this, _bookmarks));
    return true;
}

bool QHexDocument::addBookMark(qsizetype pos, QString comment) {
    if (m_keepsize && !existBookMark(pos)) {
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

void QHexDocument::findAllBytes(qsizetype begin, qsizetype end, QByteArray b,
                                QList<qsizetype> &results,
                                const std::function<bool()> &pred) {
    results.clear();
    if (!b.length())
        return;
    qsizetype p = begin > 0 ? begin : 0;
    qsizetype e = end > begin ? end : -1;
    auto offset = b.size();
    while (pred()) {
        p = m_buffer->indexOf(b, p);
        if (p < 0 || (e > 0 && p > e)) {
            break;
        }

        if (results.size() ==
            std::numeric_limits<QList<qsizetype>::size_type>::max()) {
            break;
        }
        results.append(p);
        p += offset + 1;
    }
}

QHexDocument *QHexDocument::fromRegionFile(QString filename, qsizetype start,
                                           qsizetype length, bool readonly) {

    QFile iodevice(filename);
    auto hexbuffer = new QFileRegionBuffer;
    hexbuffer->setReadOffset(start);
    hexbuffer->setReadMaxBytes(length);

    if (hexbuffer->read(&iodevice)) {
        return new QHexDocument(hexbuffer, readonly);
    } else {
        delete hexbuffer;
    }

    return nullptr;
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
    auto len = data.size();
    insertBookMarkAdjust(offset, len);
    m_metadata->insertAdjust(offset, len);
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
    removeBookMarkAdjust(offset, len);
    m_metadata->removeAdjust(offset, len);
    setDocSaved(false);
    emit documentChanged();
    return true;
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
    if (m_keepsize || m_readonly || m_islocked)
        return;
    this->Insert(cursor, offset, QByteArray(1, char(b)), nibbleindex);
}

void QHexDocument::Replace(QHexCursor *cursor, qsizetype offset, uchar b,
                           int nibbleindex) {
    if (m_readonly || m_islocked)
        return;
    this->Replace(cursor, offset, QByteArray(1, char(b)), nibbleindex);
}

void QHexDocument::Insert(QHexCursor *cursor, qsizetype offset,
                          const QByteArray &data, int nibbleindex) {
    if (m_keepsize || m_readonly || m_islocked)
        return;

    m_undostack->push(
        new InsertCommand(this, cursor, offset, data, nibbleindex));

    emit documentChanged();
}

void QHexDocument::Replace(QHexCursor *cursor, qsizetype offset,
                           const QByteArray &data, int nibbleindex) {
    if (m_readonly || m_islocked)
        return;
    m_undostack->push(
        new ReplaceCommand(this, offset, data, cursor, nibbleindex));
    emit documentChanged();
}

bool QHexDocument::Remove(QHexCursor *cursor, qsizetype offset, qsizetype len,
                          int nibbleindex) {
    if (m_keepsize || m_readonly || m_islocked || m_metadata->hasMetadata())
        return false;
    m_undostack->push(
        new RemoveCommand(this, offset, len, cursor, nibbleindex));
    emit documentChanged();
    return true;
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

qsizetype QHexDocument::searchForward(qsizetype begin, const QByteArray &ba) {
    if (begin < 0) {
        return -1;
    }
    return m_buffer->indexOf(ba, begin);
}

qsizetype QHexDocument::searchBackward(qsizetype begin, const QByteArray &ba) {
    if (begin < 0) {
        return -1;
    }
    return m_buffer->lastIndexOf(ba, begin);
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

QHexDocument *QHexDocument::fromStorageDriver(const QStorageInfo &storage,
                                              bool readonly) {
#ifdef Q_OS_WIN
    auto f = new QStorageDevice;
    f->setStorage(storage);
    auto hexbuffer = new QFileBuffer();
    if (f->open(readonly ? QStorageDevice::ReadOnly
                         : QStorageDevice::ReadWrite) &&
        hexbuffer->read(f)) {
        return new QHexDocument(hexbuffer, readonly);
    } else {
        delete hexbuffer;
    }
    return nullptr;
#else
    return fromLargeFile(storage.device(), readonly);
#endif
}

QHexBuffer *QHexDocument::buffer() const { return m_buffer; }
