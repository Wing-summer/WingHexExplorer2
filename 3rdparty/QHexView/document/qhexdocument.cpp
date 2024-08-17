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

/*======================*/
// added by wingsummer

QList<qsizetype> QHexDocument::getsBookmarkPos(qsizetype line) {
    QList<qsizetype> pos;
    auto begin = m_hexlinewidth * line;
    auto end = m_hexlinewidth + begin;
    for (auto item : bookmarks) {
        if (item.pos >= begin && item.pos <= end)
            pos.push_back(item.pos);
    }
    return pos;
}

bool QHexDocument::lineHasBookMark(qsizetype line) {
    auto begin = m_hexlinewidth * line;
    auto end = m_hexlinewidth + begin;
    for (auto item : bookmarks) {
        if (item.pos >= begin && item.pos <= end)
            return true;
    }
    return false;
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

bool QHexDocument::isDocSaved() {
    return m_undostack->isClean() && !m_pluginModed;
}
void QHexDocument::setDocSaved(bool b) {
    if (b) {
        m_undostack->setClean();
    }
    m_pluginModed = !b;
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

QList<BookMarkStruct> *QHexDocument::bookMarksPtr() { return &bookmarks; }

const QList<BookMarkStruct> &QHexDocument::bookMarks() const {
    return bookmarks;
}

bool QHexDocument::AddBookMark(qsizetype pos, QString comment) {
    if (!m_keepsize)
        return false;
    m_undostack->push(new BookMarkAddCommand(this, pos, comment));
    return true;
}

bool QHexDocument::ModBookMark(qsizetype pos, QString comment) {
    if (!m_keepsize)
        return false;
    m_undostack->push(
        new BookMarkReplaceCommand(this, pos, comment, bookMarkComment(pos)));
    return true;
}

bool QHexDocument::ClearBookMark() {
    if (!m_keepsize)
        return false;
    m_undostack->push(new BookMarkClearCommand(this, getAllBookMarks()));
    return true;
}

bool QHexDocument::addBookMark(qsizetype pos, QString comment) {
    if (m_keepsize && !existBookMark(pos)) {
        auto section = bookmarks.size();
        emit bookMarkChanging(BookMarkModEnum::Insert, section);
        BookMarkStruct b{pos, comment};
        bookmarks.append(b);
        setDocSaved(false);
        emit documentChanged();
        emit bookMarkChanged(BookMarkModEnum::Insert, section);
        return true;
    }
    return false;
}

QString QHexDocument::bookMarkComment(qsizetype pos) {
    if (pos > 0 && pos < m_buffer->length()) {
        for (auto item : bookmarks) {
            if (item.pos == pos) {
                return item.comment;
            }
        }
    }
    return QString();
}

BookMarkStruct QHexDocument::bookMark(qsizetype pos) {
    if (pos > 0 && pos < m_buffer->length()) {
        for (auto item : bookmarks) {
            if (item.pos == pos) {
                return item;
            }
        }
    }
    return BookMarkStruct{-1, ""};
}

BookMarkStruct QHexDocument::bookMarkByIndex(qindextype index) {
    if (index >= 0 && index < bookmarks.count()) {
        return bookmarks.at(index);
    } else {
        BookMarkStruct b;
        b.pos = -1;
        return b;
    }
}

bool QHexDocument::RemoveBookMarks(QList<qsizetype> &pos) {
    if (!m_keepsize)
        return false;
    m_undostack->beginMacro("RemoveBookMarks");
    for (auto p : pos) {
        m_undostack->push(
            new BookMarkRemoveCommand(this, p, bookMarkComment(p)));
    }
    m_undostack->endMacro();
    emit documentChanged();
    return true;
}

bool QHexDocument::RemoveBookMark(qindextype index) {
    if (!m_keepsize)
        return false;
    auto b = bookmarks.at(index);
    m_undostack->push(new BookMarkRemoveCommand(this, b.pos, b.comment));
    return true;
}

bool QHexDocument::removeBookMark(qsizetype pos) {
    if (m_keepsize && pos >= 0 && pos < m_buffer->length()) {
        int index = 0;
        for (auto item : bookmarks) {
            if (pos == item.pos) {
                emit bookMarkChanging(BookMarkModEnum::Remove, index);
                bookmarks.removeAt(index);
                setDocSaved(false);
                emit documentChanged();
                emit bookMarkChanged(BookMarkModEnum::Remove, index);
                break;
            }
            index++;
        }
        return true;
    }
    return false;
}

bool QHexDocument::removeBookMarkByIndex(qindextype index) {
    if (m_keepsize && index >= 0 && index < bookmarks.count()) {
        emit bookMarkChanging(BookMarkModEnum::Remove, index);
        bookmarks.removeAt(index);
        setDocSaved(false);
        emit documentChanged();
        emit bookMarkChanged(BookMarkModEnum::Remove, index);
        return true;
    }
    return false;
}

bool QHexDocument::modBookMark(qsizetype pos, QString comment) {
    if (m_keepsize && pos > 0 && pos < m_buffer->length()) {
        int index = 0;
        for (auto &item : bookmarks) {
            if (item.pos == pos) {
                emit bookMarkChanging(BookMarkModEnum::Modify, index);
                item.comment = comment;
                setDocSaved(false);
                emit bookMarkChanged(BookMarkModEnum::Modify, index);
                return true;
            }
            index++;
        }
    }
    return false;
}

bool QHexDocument::clearBookMark() {
    if (m_keepsize) {
        auto section = bookmarks.size();
        emit bookMarkChanging(BookMarkModEnum::Clear, section);
        bookmarks.clear();
        setDocSaved(false);
        emit documentChanged();
        emit bookMarkChanged(BookMarkModEnum::Clear, section);
        return true;
    }
    return false;
}

bool QHexDocument::existBookMark(qsizetype pos) {
    for (auto item : bookmarks) {
        if (item.pos == pos) {
            return true;
        }
    }
    return false;
}

const QList<BookMarkStruct> &QHexDocument::getAllBookMarks() {
    return bookmarks;
}

qsizetype QHexDocument::bookMarksCount() const { return bookmarks.count(); }

void QHexDocument::applyBookMarks(const QList<BookMarkStruct> &books) {
    bookmarks.append(books);
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
    m_buffer->insert(offset, data);
    setDocSaved(false);
    emit documentChanged();
    return true;
}

bool QHexDocument::replace(qsizetype offset, uchar b) {
    if (m_readonly || m_islocked)
        return false;
    return this->replace(offset, QByteArray(1, char(b)));
}

bool QHexDocument::replace(qsizetype offset, const QByteArray &data) {
    if (m_readonly || m_islocked)
        return false;
    m_buffer->replace(offset, data);
    setDocSaved(false);
    emit documentChanged();
    return true;
}

bool QHexDocument::remove(qsizetype offset, qsizetype len) {
    if (m_keepsize || m_readonly || m_islocked || m_metadata->hasMetadata())
        return false;
    m_buffer->remove(offset, len);
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

    connect(m_metadata, &QHexMetadata::metadataChanged, this,
            [=](qsizetype line) { emit QHexDocument::metaLineChanged(line); });
    connect(m_metadata, &QHexMetadata::metadataCleared, this,
            [=] { emit QHexDocument::documentChanged(); });

    /*=======================*/
    // added by wingsummer
    connect(m_undostack, &QUndoStack::canUndoChanged, this,
            &QHexDocument::canUndoChanged);
    connect(m_undostack, &QUndoStack::canRedoChanged, this,
            &QHexDocument::canRedoChanged);
    connect(m_undostack, &QUndoStack::cleanChanged, this, [=](bool b) {
        emit QHexDocument::documentSaved(b && !m_pluginModed);
    });
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
QByteArray QHexDocument::read(qsizetype offset, qsizetype len) {
    return m_buffer->read(offset, len);
}

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
    if (m_keepsize || m_readonly || m_islocked ||
        (offset < m_buffer->length() && m_metadata->hasMetadata()))
        return;
    if (!m_metadata->hasMetadata())
        m_undostack->push(
            new InsertCommand(m_buffer, offset, data, cursor, nibbleindex));
    else
        m_buffer->insert(offset, data);
    emit documentChanged();
}

void QHexDocument::Replace(QHexCursor *cursor, qsizetype offset,
                           const QByteArray &data, int nibbleindex) {
    if (m_readonly || m_islocked)
        return;
    m_undostack->push(
        new ReplaceCommand(m_buffer, offset, data, cursor, nibbleindex));
    emit documentChanged();
}

bool QHexDocument::Remove(QHexCursor *cursor, qsizetype offset, qsizetype len,
                          int nibbleindex) {
    if (m_keepsize || m_readonly || m_islocked || m_metadata->hasMetadata())
        return false;
    m_undostack->push(
        new RemoveCommand(m_buffer, offset, len, cursor, nibbleindex));
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

    m_pluginModed = false;
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

QHexDocument *QHexDocument::fromLargeFile(QString filename, bool readonly) {

    auto f = new QFile;
    if (!filename.isEmpty()) {
        f->setFileName(filename);
        QHexBuffer *hexbuffer = new QFileBuffer();
        if (hexbuffer->read(f)) {
            return new QHexDocument(hexbuffer,
                                    readonly); // modified by wingsummer
        } else {
            delete hexbuffer;
        }
    } else {
        return new QHexDocument(new QFileBuffer(), readonly);
    }

    return nullptr;
}
