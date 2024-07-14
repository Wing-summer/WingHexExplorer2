#ifndef QHEXMETADATA_H
#define QHEXMETADATA_H

#include <QObject>
#include <QtGlobal>
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QLinkedList>
#else
#include <list>
#endif
#include <QColor>
#include <QHash>
#include <QUndoStack>
#include <QVector>

struct QHexMetadataAbsoluteItem {
    qint64 begin;
    qint64 end;
    QColor foreground, background;
    QString comment;

    // added by wingsummer
    bool operator==(const QHexMetadataAbsoluteItem &item) const {
        return begin == item.begin && end == item.end &&
               foreground == item.foreground && background == item.background &&
               comment == item.comment;
    }
};

struct QHexMetadataItem {
    qsizetype line;
    qsizetype start, length;
    QColor foreground, background;
    QString comment;

    // added by wingsummer
    bool operator==(const QHexMetadataItem &item) const {
        return line == item.line && start == item.start &&
               foreground == item.foreground && background == item.background &&
               comment == item.comment;
    }
};

typedef std::list<QHexMetadataItem> QHexLineMetadata;

class QHexMetadata : public QObject {
    Q_OBJECT

public:
    explicit QHexMetadata(QUndoStack *undo, QObject *parent = nullptr);
    const QHexLineMetadata &get(qsizetype line) const;
    QString comments(qsizetype line, qsizetype column) const;
    bool lineHasMetadata(qsizetype line) const; // modified by wingsummer

    /*============================*/
    // added by wingsummer

    void ModifyMetadata(QHexMetadataAbsoluteItem newmeta,
                        QHexMetadataAbsoluteItem oldmeta);
    void RemoveMetadata(QHexMetadataAbsoluteItem item);
    void RemoveMetadata(qint64 offset);
    void Metadata(qint64 begin, qint64 end, const QColor &fgcolor,
                  const QColor &bgcolor, const QString &comment);
    void Clear();

    //---------------------------------------------------------

    void modifyMetadata(QHexMetadataAbsoluteItem newmeta,
                        QHexMetadataAbsoluteItem oldmeta);
    void removeMetadata(QHexMetadataAbsoluteItem item);
    void removeMetadata(qint64 offset);
    QList<QHexMetadataAbsoluteItem> gets(qint64 offset);
    void applyMetas(QList<QHexMetadataAbsoluteItem> metas);

    void redo();
    void undo();
    bool canRedo();
    bool canUndo();
    bool hasMetadata();

    /*============================*/

    // this is transient till next call to setLineWidth()
    void clear(qsizetype line);

    void clear();
    void setLineWidth(quint8 width);

public:
    // new interface with begin, end
    void metadata(qint64 begin, qint64 end, const QColor &fgcolor,
                  const QColor &bgcolor, const QString &comment);

    // old interface with line, start, length
    void metadata(quint64 line, int start, int length, const QColor &fgcolor,
                  const QColor &bgcolor, const QString &comment);

    void color(quint64 line, int start, int length, const QColor &fgcolor,
               const QColor &bgcolor);
    void foreground(quint64 line, int start, int length, const QColor &fgcolor);
    void background(quint64 line, int start, int length, const QColor &bgcolor);
    void comment(quint64 line, int start, int length, const QString &comment);

    QList<QHexMetadataAbsoluteItem>
    getallMetas(); // added by wingsummer to support workspace

private:
    void setMetadata(const QHexMetadataItem &mi);
    void setAbsoluteMetadata(const QHexMetadataAbsoluteItem &mi);

signals:
    void metadataChanged(qsizetype line);
    void metadataCleared();

private:
    quint8 m_lineWidth;
    QHash<qsizetype, QHexLineMetadata> m_metadata;
    QList<QHexMetadataAbsoluteItem> m_absoluteMetadata;

    QUndoStack *m_undo; // added by wingsummer
};

#endif // QHEXMETADATA_H
