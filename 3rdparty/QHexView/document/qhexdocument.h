#ifndef QHEXDOCUMENT_H
#define QHEXDOCUMENT_H

#include "../define.h"
#include "buffer/qhexbuffer.h"
#include "qhexcursor.h"
#include "qhexmetadata.h"
#include <QFile>
#include <QMap>
#include <QUndoStack>

/*=========================*/
// added by wingsummer

struct BookMarkStruct {
    qlonglong pos;
    QString comment;
};

/*=========================*/

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
    QList<qsizetype> getsBookmarkPos(qsizetype line);

    bool setLockedFile(bool b);
    bool setKeepSize(bool b);
    bool isReadOnly();
    bool isKeepSize();
    bool isLocked();

    //----------------------------------
    bool AddBookMark(qsizetype pos, QString comment);
    bool RemoveBookMark(qindextype index);
    bool RemoveBookMarks(QList<qsizetype> &pos);
    bool ModBookMark(qsizetype pos, QString comment);
    bool ClearBookMark();
    //----------------------------------

    bool addBookMark(qsizetype pos, QString comment);
    bool modBookMark(qsizetype pos, QString comment);

    BookMarkStruct bookMarkByIndex(qindextype index);
    BookMarkStruct bookMark(qsizetype pos);

    QString bookMarkComment(qsizetype pos);
    const QList<BookMarkStruct> &getAllBookMarks();
    qsizetype bookMarksCount() const;
    void applyBookMarks(const QList<BookMarkStruct> &books);
    bool removeBookMarkByIndex(qindextype index);
    bool removeBookMark(qsizetype pos);
    bool clearBookMark();

    QList<BookMarkStruct> *bookMarksPtr();
    const QList<BookMarkStruct> &bookMarks() const;

    bool existBookMark(qsizetype pos);

    void findAllBytes(
        qint64 begin, qint64 end, QByteArray b, QList<qsizetype> &results,
        const std::function<bool()> &pred = [] { return true; });
    bool isDocSaved();
    void setDocSaved(bool b = true);

    void setMetafgVisible(bool b);
    void setMetabgVisible(bool b);
    void setMetaCommentVisible(bool b);

    void SetMetafgVisible(bool b);
    void SetMetabgVisible(bool b);
    void SetMetaCommentVisible(bool b);
    void SetMetaVisible(bool b);

    bool metafgVisible();
    bool metabgVisible();
    bool metaCommentVisible();

    /*======================*/

public:
    QByteArray read(qsizetype offset, qsizetype len = -1);

    char at(qsizetype offset) const;
    void SetBaseAddress(quintptr baseaddress);
    void setBaseAddress(quintptr baseaddress);
    void sync();

public slots:
    void undo();
    void redo();

    void Insert(QHexCursor *cursor, qsizetype offset, uchar b, int nibbleindex);
    void Insert(QHexCursor *cursor, qsizetype offset, const QByteArray &data,
                int nibbleindex);
    void Replace(QHexCursor *cursor, qsizetype offset, uchar b,
                 int nibbleindex);
    void Replace(QHexCursor *cursor, qsizetype offset, const QByteArray &data,
                 int nibbleindex = 0);
    bool Remove(QHexCursor *cursor, qsizetype offset, qsizetype len,
                int nibbleindex = 0);

    QByteArray read(qint64 offset, qsizetype len) const;
    bool saveTo(QIODevice *device, bool cleanUndo);

    // qint64 searchForward(const QByteArray &ba);
    // qint64 searchBackward(const QByteArray &ba);

    /*================================*/
    // added by wingsummer

    qint64 searchForward(qsizetype begin, const QByteArray &ba);
    qint64 searchBackward(qsizetype begin, const QByteArray &ba);

    bool insert(qsizetype offset, uchar b);
    bool insert(qsizetype offset, const QByteArray &data);
    bool replace(qsizetype offset, uchar b);
    bool replace(qsizetype offset, const QByteArray &data);
    bool remove(qsizetype offset, qsizetype len);

    /*================================*/

    /*================================*/
    // modified by wingsummer

public:
    template <typename T>
    static QHexDocument *fromDevice(QIODevice *iodevice, bool readonly = false);
    template <typename T>
    static QHexDocument *fromFile(QString filename, bool readonly = false);

    static QHexDocument *fromRegionFile(QString filename, qint64 start,
                                        qint64 length, bool readonly = false);

    template <typename T>
    static QHexDocument *fromMemory(char *data, int size,
                                    bool readonly = false);
    template <typename T>
    static QHexDocument *fromMemory(const QByteArray &ba,
                                    bool readonly = false);
    static QHexDocument *fromLargeFile(QString filename, bool readonly = false);

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
    QList<BookMarkStruct> bookmarks;

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
        iodevice->open(QIODevice::ReadOnly);
    }

    QHexBuffer *hexbuffer = new T();
    if (hexbuffer->read(iodevice)) {
        if (needsclose)
            iodevice->close();

        return new QHexDocument(hexbuffer, readonly);
    } else {
        delete hexbuffer;
    }

    return nullptr;
}

template <typename T>
QHexDocument *QHexDocument::fromFile(QString filename, bool readonly) {
    QFile f;
    QHexDocument *doc;
    if (filename.length()) {
        f.setFileName(filename);
        f.open(QFile::ReadOnly);
        doc = QHexDocument::fromDevice<T>(&f, readonly);
        f.close();
    } else {
        doc = new QHexDocument(new T(), readonly);
    }
    return doc;
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
