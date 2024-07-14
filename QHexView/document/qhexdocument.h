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

enum class BookMarkModEnum { Insert, Modify, Remove, Apply, Clear };

/*=========================*/

class QHexDocument : public QObject {
    Q_OBJECT

private:
    explicit QHexDocument(QHexBuffer *buffer, bool readonly = false,
                          QObject *parent = nullptr); // modified by wingsummer

public:
    bool isEmpty() const;
    bool atEnd() const;
    bool canUndo() const;
    bool canRedo() const;
    qsizetype length() const;
    quintptr baseAddress() const;
    QHexCursor *cursor() const;
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
    QList<BookMarkStruct> getAllBookMarks();
    void applyBookMarks(QList<BookMarkStruct> books);
    bool removeBookMarkByIndex(qindextype index);
    bool removeBookMark(qsizetype pos);
    bool clearBookMark();
    void getBookMarks(QList<BookMarkStruct> &bookmarks);
    void gotoBookMark(qindextype index);
    bool existBookMarkByIndex(qindextype &index);
    bool existBookMark();
    bool existBookMark(qsizetype pos);

    void findAllBytes(qint64 begin, qint64 end, QByteArray b,
                      QList<quint64> &results, qsizetype maxCount = -1);
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

    void setCopyLimit(qsizetype count);
    qsizetype copyLimit();

    /*======================*/

public:
    bool RemoveSelection(int nibbleindex = 1);
    QByteArray read(qint64 offset, qsizetype len = 0);
    QByteArray selectedBytes() const;
    char at(qsizetype offset) const;
    void SetBaseAddress(quintptr baseaddress);
    void setBaseAddress(quintptr baseaddress);
    void sync();

public slots:
    void undo();
    void redo();
    bool Cut(bool hex = false, int nibbleindex = 0);
    bool copy(bool hex = false);
    void Paste(int nibbleindex = 0, bool hex = false);
    void Insert(qint64 offset, uchar b, int nibbleindex);
    void Insert(qint64 offset, const QByteArray &data, int nibbleindex);
    void Replace(qint64 offset, uchar b, int nibbleindex);
    void Replace(qint64 offset, const QByteArray &data, int nibbleindex = 0);
    bool Remove(qint64 offset, qsizetype len, int nibbleindex = 0);
    QByteArray read(qint64 offset, qsizetype len) const;
    bool saveTo(QIODevice *device, bool cleanUndo);

    // qint64 searchForward(const QByteArray &ba);
    // qint64 searchBackward(const QByteArray &ba);

    /*================================*/
    // added by wingsummer

    qint64 searchForward(qsizetype begin, const QByteArray &ba);
    qint64 searchBackward(qsizetype begin, const QByteArray &ba);
    bool cut(bool hex = false);
    void paste(bool hex = false);
    bool insert(qsizetype offset, uchar b);
    bool insert(qsizetype offset, const QByteArray &data);
    bool replace(qsizetype offset, uchar b);
    bool replace(qsizetype offset, const QByteArray &data);
    bool remove(qsizetype offset, qsizetype len);
    bool removeSelection();

    /*================================*/

    /*================================*/
    // modified by wingsummer

public:
    template <typename T>
    static QHexDocument *fromDevice(QIODevice *iodevice, bool readonly = false,
                                    QObject *parent = nullptr);
    template <typename T>
    static QHexDocument *fromFile(QString filename, bool readonly = false,
                                  QObject *parent = nullptr);

    static QHexDocument *fromRegionFile(QString filename, qint64 start,
                                        qint64 length, bool readonly = false,
                                        QObject *parent = nullptr);

    template <typename T>
    static QHexDocument *fromMemory(char *data, int size, bool readonly = false,
                                    QObject *parent = nullptr);
    template <typename T>
    static QHexDocument *fromMemory(const QByteArray &ba, bool readonly = false,
                                    QObject *parent = nullptr);
    static QHexDocument *fromLargeFile(QString filename, bool readonly = false,
                                       QObject *parent = nullptr);

signals:

    /*================================*/

    /*================================*/
    // added by wingsummer

    void documentSaved(bool saved);
    void bookMarkChanged(BookMarkModEnum flag, qsizetype index, qint64 pos,
                         QString comment);
    void metafgVisibleChanged(bool b);
    void metabgVisibleChanged(bool b);
    void metaCommentVisibleChanged(bool b);

    void documentLockedFile(bool locked);
    void documentKeepSize(bool keep);

    void copyLimitRaised();

    /*================================*/

    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);
    void documentChanged();
    void metaLineChanged(qsizetype line);

private:
    QHexBuffer *m_buffer;
    QHexMetadata *m_metadata;
    QUndoStack *m_undostack;
    QHexCursor *m_cursor;
    quintptr m_baseaddress;
    quint8 m_areaindent;
    quint8 m_hexlinewidth;

    /*======================*/
    // added by wingsummer

    bool m_readonly;
    bool m_keepsize;
    bool m_islocked;
    QList<BookMarkStruct> bookmarks;
    bool m_pluginModed = false;

    bool m_metafg = true;
    bool m_metabg = true;
    bool m_metacomment = true;

    qsizetype m_copylimit = 1; // MB

    /*======================*/
};

/*====================================*/
// modified by wingsummer

template <typename T>
QHexDocument *QHexDocument::fromDevice(QIODevice *iodevice, bool readonly,
                                       QObject *parent) {
    bool needsclose = false;

    if (!iodevice->isOpen()) {
        needsclose = true;
        iodevice->open(QIODevice::ReadOnly);
    }

    QHexBuffer *hexbuffer = new T();
    if (hexbuffer->read(iodevice)) {
        if (needsclose)
            iodevice->close();

        return new QHexDocument(hexbuffer, readonly, parent);
    } else {
        delete hexbuffer;
    }

    return nullptr;
}

template <typename T>
QHexDocument *QHexDocument::fromFile(QString filename, bool readonly,
                                     QObject *parent) {
    QFile f;
    QHexDocument *doc;
    if (filename.length()) {
        f.setFileName(filename);
        f.open(QFile::ReadOnly);
        doc = QHexDocument::fromDevice<T>(&f, readonly, parent);
        f.close();
    } else {
        doc = new QHexDocument(new T(), readonly, parent);
    }
    return doc;
}

template <typename T>
QHexDocument *QHexDocument::fromMemory(char *data, int size, bool readonly,
                                       QObject *parent) {
    QHexBuffer *hexbuffer = new T();
    hexbuffer->read(data, size);
    return new QHexDocument(hexbuffer, readonly, parent);
}

template <typename T>
QHexDocument *QHexDocument::fromMemory(const QByteArray &ba, bool readonly,
                                       QObject *parent) {
    QHexBuffer *hexbuffer = new T();
    hexbuffer->read(ba);
    return new QHexDocument(hexbuffer, readonly, parent);
}

/*================================================*/

#endif // QHEXEDITDATA_H
