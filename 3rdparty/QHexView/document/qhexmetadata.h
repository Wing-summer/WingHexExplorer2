#ifndef QHEXMETADATA_H
#define QHEXMETADATA_H

#include <QObject>

#include <QColor>
#include <QHash>
#include <QMap>
#include <QUndoStack>
#include <QVector>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using qhash_result_t = uint;

// copying from QT6 source code for supporting QT5's qHashMulti
namespace QtPrivate {
template <typename T>
inline constexpr bool
    QNothrowHashableHelper_v = noexcept(qHash(std::declval<const T &>()));

template <typename T, typename Enable = void>
struct QNothrowHashable : std::false_type {};

template <typename T>
struct QNothrowHashable<T, std::enable_if_t<QNothrowHashableHelper_v<T>>>
    : std::true_type {};

template <typename T>
constexpr inline bool QNothrowHashable_v = QNothrowHashable<T>::value;

} // namespace QtPrivate

template <typename... T>
constexpr qhash_result_t qHashMulti(size_t seed, const T &...args) noexcept(
    std::conjunction_v<QtPrivate::QNothrowHashable<T>...>) {
    QtPrivate::QHashCombine hash;
    return ((seed = hash(seed, args)), ...), seed;
}
#else
using qhash_result_t = size_t;
#endif

struct QHexMetadataItem {
    qsizetype begin = -1;
    qsizetype end = -1;
    QColor foreground, background;
    QString comment;
    bool flag = false;

    // added by wingsummer
    bool operator==(const QHexMetadataItem &item) const {
        return begin == item.begin && end == item.end &&
               foreground == item.foreground && background == item.background &&
               comment == item.comment;
    }
};

inline qhash_result_t qHash(const QHexMetadataItem &c,
                            qhash_result_t seed) noexcept {
    return qHashMulti(seed, c.begin, c.end, c.foreground.rgba(),
                      c.background.rgba(), c.comment);
}

// only for rendering
struct QHexLineMetadataItem {
    qsizetype start = -1, length = 0;
    QColor foreground, background;
    QString comment;
    QHexMetadataItem *parent = nullptr;
};

typedef QList<QHexLineMetadataItem> QHexLineMetadata;

class QHexMetadata : public QObject {
    Q_OBJECT

public:
    explicit QHexMetadata(QUndoStack *undo, QObject *parent = nullptr);
    QHexLineMetadata get(qsizetype line) const;
    QString comments(qsizetype line, qsizetype column) const;
    bool lineHasMetadata(qsizetype line) const; // modified by wingsummer

    qsizetype size() const;

    /*============================*/
    // added by wingsummer

    void ModifyMetadata(QHexMetadataItem newmeta, QHexMetadataItem oldmeta);
    void RemoveMetadatas(const QList<QHexMetadataItem> &items);
    void RemoveMetadata(QHexMetadataItem item);
    void RemoveMetadata(qsizetype offset);
    void Metadata(qsizetype begin, qsizetype end, const QColor &fgcolor,
                  const QColor &bgcolor, const QString &comment);
    void Clear();

    //---------------------------------------------------------

    bool modifyMetadata(const QHexMetadataItem &newmeta,
                        const QHexMetadataItem &oldmeta);
    bool removeMetadata(const QHexMetadataItem &item);
    void removeMetadata(qsizetype offset);

    QVector<QHexMetadataItem> getAllMetadata() const;
    QVector<QHexMetadataItem> gets(qsizetype offset);
    void applyMetas(const QVector<QHexMetadataItem> &metas);

    bool hasMetadata();

    /*============================*/

    void clear();
    void setLineWidth(quint8 width);

    void insertAdjust(qsizetype offset, qsizetype length);
    void removeAdjust(qsizetype offset, qsizetype length);

public:
    // new interface with begin, end
    bool metadata(qsizetype begin, qsizetype end, const QColor &fgcolor,
                  const QColor &bgcolor, const QString &comment);

    bool color(qsizetype begin, qsizetype end, const QColor &fgcolor,
               const QColor &bgcolor);
    bool foreground(qsizetype begin, qsizetype end, const QColor &fgcolor);
    bool background(qsizetype begin, qsizetype end, const QColor &bgcolor);
    bool comment(qsizetype begin, qsizetype end, const QString &comment);

private:
    void addMetadata(const QHexMetadataItem &mi);

    void addMetaLines(const QHexMetadataItem &mi);

signals:
    void metadataChanged();

private:
    quint8 m_lineWidth;

    QMap<qsizetype, QHash<QHexMetadataItem, QHexLineMetadata>> m_linemeta;
    QVector<QHexMetadataItem> m_metadata;

    QUndoStack *m_undo = nullptr; // added by wingsummer
};

#endif // QHEXMETADATA_H
