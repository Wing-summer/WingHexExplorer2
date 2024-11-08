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
    bool isReadOnly();
    bool isKeepSize();
    bool isLocked();

    //----------------------------------
    bool AddBookMark(qsizetype pos, QString comment);
    bool RemoveBookMark(qsizetype pos);
    bool RemoveBookMarks(const QList<qsizetype> &pos);
    bool ModBookMark(qsizetype pos, QString comment);
    bool ClearBookMark();
    //----------------------------------

    bool addBookMark(qsizetype pos, QString comment);
    bool modBookMark(qsizetype pos, const QString &comment);

    QString bookMark(qsizetype pos);
    bool bookMarkExists(qsizetype pos);

    // note: maybe changed when bookmarks are chaged
    qsizetype bookMarkPos(qsizetype index);

    qsizetype bookMarksCount() const;
    void applyBookMarks(const QMap<qsizetype, QString> &books);
    bool removeBookMark(qsizetype pos);
    bool clearBookMark();

    const QMap<qsizetype, QString> &bookMarks() const;

    bool existBookMark(qsizetype pos);

    void findAllBytes(
        qsizetype begin, qsizetype end, QByteArray b, QList<qsizetype> &results,
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
    QByteArray read(qsizetype offset, qsizetype len = -1) const;

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

    bool saveTo(QIODevice *device, bool cleanUndo);

    // qsizetype searchForward(const QByteArray &ba);
    // qsizetype searchBackward(const QByteArray &ba);

    /*================================*/
    // added by wingsummer

    qsizetype searchForward(qsizetype begin, const QByteArray &ba);
    qsizetype searchBackward(qsizetype begin, const QByteArray &ba);

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

    static QHexDocument *fromRegionFile(QString filename, qsizetype start,
                                        qsizetype length,
                                        bool readonly = false);

    template <typename T>
    static QHexDocument *fromMemory(char *data, int size,
                                    bool readonly = false);
    template <typename T>
    static QHexDocument *fromMemory(const QByteArray &ba,
                                    bool readonly = false);
    static QHexDocument *fromLargeFile(const QString &filename,
                                       bool readonly = false);
    static QHexDocument *fromStorageDriver(const QStorageInfo &storage,
                                           bool readonly = false);

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
