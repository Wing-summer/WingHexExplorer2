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
#ifndef QHEXDOCUMENT_H
#define QHEXDOCUMENT_H

#include "buffer/qhexbuffer.h"
#include "qhexcursor.h"
#include "qhexmetadata.h"
#include <QFile>
#include <QMap>
#include <QStorageInfo>
#include <QUndoStack>

class QHexDocument : public QObject {
    Q_OBJECT

private:
    explicit QHexDocument(QHexBuffer *buffer,
                          bool readonly = false); // modified by wingsummer

public:
    bool isEmpty() const;
    bool canUndo() const;
    bool canRedo() const;
    qsizetype length() const;
    quintptr baseAddress() const;

    QHexMetadata *metadata() const;
    int areaIndent() const;
    void setAreaIndent(quint8 value);
    int hexLineWidth() const;
    void setHexLineWidth(quint8 value);

    /*======================*/
    // added by wingsummer

    void addUndoCommand(QUndoCommand *command);
    bool lineHasBookMark(qsizetype line);
    QList<qsizetype> getLineBookmarksPos(qsizetype line);

    bool setLockedFile(bool b);
    bool setKeepSize(bool b);
    bool isReadOnly() const;
    bool isKeepSize() const;
    bool isLocked() const;

    void setLockKeepSize(bool b);
    bool lockKeepSize() const;

    //----------------------------------
    bool AddBookMark(qsizetype pos, const QString &comment);
    bool RemoveBookMark(qsizetype pos);
    bool RemoveBookMarks(const QList<qsizetype> &pos);
    bool ModBookMark(qsizetype pos, const QString &comment);
    bool ClearBookMark();

    QUndoCommand *MakeAddBookMark(QUndoCommand *parent, qsizetype pos,
                                  QString comment);
    QUndoCommand *MakeRemoveBookMark(QUndoCommand *parent, qsizetype pos);
    QUndoCommand *MakeModBookMark(QUndoCommand *parent, qsizetype pos,
                                  QString comment);
    QUndoCommand *MakeClearBookMark(QUndoCommand *parent);

    //----------------------------------

    bool addBookMark(qsizetype pos, QString comment);
    bool modBookMark(qsizetype pos, const QString &comment);

    QString bookMark(qsizetype pos);
    bool bookMarkExists(qsizetype pos);
    QList<qsizetype> bookMarkRange(qsizetype begin, qsizetype end);

    // note: maybe changed when bookmarks are chaged
    qsizetype bookMarkPos(qsizetype index);

    qsizetype bookMarksCount() const;
    void applyBookMarks(const QMap<qsizetype, QString> &books);
    bool removeBookMark(qsizetype pos);
    bool clearBookMark();

    const QMap<qsizetype, QString> &bookMarks() const;

    bool existBookMark(qsizetype pos);

    void findAllBytes(
        qsizetype begin, qsizetype end, const QByteArray &b,
        QList<qsizetype> &results,
        const std::function<bool()> &pred = [] { return true; });

    void findAllBytesExt(
        qsizetype begin, qsizetype end, const QString &pattern,
        QList<qsizetype> &results,
        const std::function<bool()> &pred = [] { return true; });

    bool isDocSaved() const;
    void setDocSaved(bool b = true);

    void setMetafgVisible(bool b);
    void setMetabgVisible(bool b);
    void setMetaCommentVisible(bool b);

    bool metafgVisible() const;
    bool metabgVisible() const;
    bool metaCommentVisible() const;

    void insertBookMarkAdjust(qsizetype offset, qsizetype length);
    QMap<qsizetype, QString> removeBookMarkAdjust(qsizetype offset,
                                                  qsizetype length);
    void insertBookMarkAdjustRevert(qsizetype offset, qsizetype length);
    void removeBookMarkAdjustRevert(const QMap<qsizetype, QString> &rmbms,
                                    qsizetype offset, qsizetype length);

    /*======================*/

public:
    QByteArray read(qsizetype offset, qsizetype len = -1) const;

    char at(qsizetype offset) const;
    bool setBaseAddress(quintptr baseaddress);

public slots:
    void undo();
    void redo();

    void beginMarco(const QString &text);
    void endMarco();

    bool Insert(QHexCursor *cursor, qsizetype offset, uchar b, int nibbleindex);
    bool Insert(QHexCursor *cursor, qsizetype offset, const QByteArray &data,
                int nibbleindex);
    void Append(QHexCursor *cursor, uchar b, int nibbleindex);
    void Append(QHexCursor *cursor, const QByteArray &data, int nibbleindex);
    bool Replace(QHexCursor *cursor, qsizetype offset, uchar b,
                 int nibbleindex);
    bool Replace(QHexCursor *cursor, qsizetype offset, const QByteArray &data,
                 int nibbleindex = 0);
    bool Remove(QHexCursor *cursor, qsizetype offset, qsizetype len,
                int nibbleindex = 0);

    QUndoCommand *MakeInsert(QUndoCommand *parent, QHexCursor *cursor,
                             qsizetype offset, uchar b, int nibbleindex = 0);
    QUndoCommand *MakeInsert(QUndoCommand *parent, QHexCursor *cursor,
                             qsizetype offset, const QByteArray &data,
                             int nibbleindex = 0);
    QUndoCommand *MakeAppend(QUndoCommand *parent, QHexCursor *cursor, uchar b,
                             int nibbleindex = 0);
    QUndoCommand *MakeAppend(QUndoCommand *parent, QHexCursor *cursor,
                             const QByteArray &data, int nibbleindex = 0);
    QUndoCommand *MakeReplace(QUndoCommand *parent, QHexCursor *cursor,
                              qsizetype offset, uchar b, int nibbleindex = 0);
    QUndoCommand *MakeReplace(QUndoCommand *parent, QHexCursor *cursor,
                              qsizetype offset, const QByteArray &data,
                              int nibbleindex = 0);
    QUndoCommand *MakeRemove(QUndoCommand *parent, QHexCursor *cursor,
                             qsizetype offset, qsizetype len,
                             int nibbleindex = 0);

    void pushMakeUndo(QUndoCommand *cmd);

    bool saveTo(QIODevice *device, bool cleanUndo);

    /*================================*/
    // added by wingsummer

    qsizetype findNext(qsizetype begin, const QByteArray &ba);
    qsizetype findPrevious(qsizetype begin, const QByteArray &ba);

    qsizetype findNextExt(qsizetype begin, const QString &pattern);
    qsizetype findPreviousExt(qsizetype begin, const QString &pattern);

    bool insert(qsizetype offset, uchar b);
    bool insert(qsizetype offset, const QByteArray &data);
    bool replace(qsizetype offset, uchar b);
    bool replace(qsizetype offset, const QByteArray &data);
    bool remove(qsizetype offset, qsizetype len);

    bool _insert(qsizetype offset, uchar b);
    bool _insert(qsizetype offset, const QByteArray &data);
    bool _replace(qsizetype offset, uchar b);
    bool _replace(qsizetype offset, const QByteArray &data);
    bool _remove(qsizetype offset, qsizetype len);

private:
    // PatternByte: represents a byte in the pattern, with nibble-level wildcard
    // support
    struct PatternByte {
        quint8 value = 0; // fixed bits
        quint8 mask =
            0; // mask: which bits to match (1 = must match, 0 = wildcard)
    };

    // Parse pattern string (e.g., "00 ?? AB C? 88" or "00??ABC?88") into
    // PatternByte list
    bool parsePattern(const QString &pattern, QList<PatternByte> &out);

    // Byte match using mask
    inline bool matchByte(quint8 data, const PatternByte &pb) {
        return (data & pb.mask) == (pb.value & pb.mask);
    }

    int hex2Int(const QChar &c);

    qsizetype findNextExt(qsizetype begin, const QList<PatternByte> &patterns);
    qsizetype findPreviousExt(qsizetype begin,
                              const QList<PatternByte> &patterns);

    /*================================*/
    // modified by wingsummer

public:
    template <typename T>
    static QHexDocument *fromDevice(QIODevice *iodevice, bool readonly = false);
    template <typename T>
    static QHexDocument *fromFile(QString filename, bool readonly = false);

    template <typename T>
    static QHexDocument *fromMemory(char *data, int size,
                                    bool readonly = false);
    template <typename T>
    static QHexDocument *fromMemory(const QByteArray &ba,
                                    bool readonly = false);

    QHexBuffer *buffer() const;
    void setBuffer(QHexBuffer *buffer);

    QUndoStack *undoStack() const;

signals:

    /*================================*/
    // added by wingsummer

    void documentSaved(bool saved);

    void bookMarkChanged();

    void metafgVisibleChanged(bool b);
    void metabgVisibleChanged(bool b);
    void metaCommentVisibleChanged(bool b);

    void documentLockedFile(bool locked);
    void documentKeepSize(bool keep);

    void hexLineWidthChanged();

    /*================================*/

    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);
    void documentChanged();
    void metaDataChanged();

private:
    QHexBuffer *m_buffer;
    QHexMetadata *m_metadata;

    QUndoStack *m_undostack;

    quintptr m_baseaddress;
    quint8 m_areaindent;
    quint8 m_hexlinewidth;

    /*======================*/
    // added by wingsummer

    bool m_readonly;
    bool m_keepsize;
    bool m_islocked;
    bool m_lockKeepSize = false;
    QMap<qsizetype, QString> _bookmarks;

    bool m_metafg = true;
    bool m_metabg = true;
    bool m_metacomment = true;

    /*======================*/
};

/*====================================*/
// modified by wingsummer

template <typename T>
QHexDocument *QHexDocument::fromDevice(QIODevice *iodevice, bool readonly) {
    bool needsclose = false;

    if (!iodevice->isOpen()) {
        needsclose = true;
    }

    QHexBuffer *hexbuffer = new T();
    if (hexbuffer->read(iodevice)) {
        if (needsclose)
            iodevice->close();

        return new QHexDocument(hexbuffer, readonly);
    } else {
        if (needsclose)
            iodevice->close();

        delete hexbuffer;
    }

    return nullptr;
}

template <typename T>
QHexDocument *QHexDocument::fromFile(QString filename, bool readonly) {
    auto f = new QFile;
    if (!filename.isEmpty()) {
        f->setFileName(filename);
        QHexBuffer *hexbuffer = new T();
        if (f->open(readonly ? QFile::ReadOnly : QFile::ReadWrite)) {
            f->close();
            if (hexbuffer->read(f)) {
                // modified by wingsummer
                return new QHexDocument(hexbuffer, readonly);
            }
        } else {
            delete hexbuffer;
        }
    } else {
        delete f;
        return new QHexDocument(new T(), readonly);
    }
    return nullptr;
}

template <typename T>
QHexDocument *QHexDocument::fromMemory(char *data, int size, bool readonly) {
    QHexBuffer *hexbuffer = new T();
    hexbuffer->read(data, size);
    return new QHexDocument(hexbuffer, readonly);
}

template <typename T>
QHexDocument *QHexDocument::fromMemory(const QByteArray &ba, bool readonly) {
    QHexBuffer *hexbuffer = new T();
    hexbuffer->read(ba);
    return new QHexDocument(hexbuffer, readonly);
}

/*================================================*/

#endif // QHEXEDITDATA_H
