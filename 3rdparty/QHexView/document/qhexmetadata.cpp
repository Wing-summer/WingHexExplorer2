#include "qhexmetadata.h"
#include "commands/meta/metaaddcommand.h"
#include "commands/meta/metaclearcommand.h"
#include "commands/meta/metaremovecommand.h"
#include "commands/meta/metaremoveposcommand.h"
#include "commands/meta/metareplacecommand.h"

#include <QtAlgorithms>
#include <QtConcurrent/QtConcurrentMap>

QHexMetadata::QHexMetadata(QUndoStack *undo, QObject *parent)
    : QObject(parent), m_undo(undo) {}

QHexLineMetadata QHexMetadata::get(qsizetype line) const {
    if (!m_linemeta.contains(line)) {
        return {};
    }

    QHexLineMetadata ret;
    for (auto &item : m_linemeta[line]) {
        ret.append(item);
    }

    return ret;
}

/*==================================*/
// added by wingsummer

//----------undo redo wrapper----------

void QHexMetadata::ModifyMetadata(QHexMetadataItem newmeta,
                                  QHexMetadataItem oldmeta) {
    m_undo->push(new MetaReplaceCommand(this, newmeta, oldmeta));
}

void QHexMetadata::RemoveMetadatas(const QList<QHexMetadataItem> &items) {
    m_undo->beginMacro("RemoveMetadatas");
    for (auto &item : items) {
        RemoveMetadata(item);
    }
    m_undo->endMacro();
}

void QHexMetadata::RemoveMetadata(QHexMetadataItem item) {
    m_undo->push(new MetaRemoveCommand(this, item));
}

void QHexMetadata::RemoveMetadata(qsizetype offset) {
    m_undo->push(new MetaRemovePosCommand(this, offset));
}

void QHexMetadata::Metadata(qsizetype begin, qsizetype end,
                            const QColor &fgcolor, const QColor &bgcolor,
                            const QString &comment) {
    QHexMetadataItem absi{begin, end, fgcolor, bgcolor, comment};
    m_undo->push(new MetaAddCommand(this, absi));
}

void QHexMetadata::Clear() {
    m_undo->push(new MetaClearCommand(this, this->getAllMetadata()));
}

//-------- the real function-----------

bool QHexMetadata::modifyMetadata(const QHexMetadataItem &newmeta,
                                  const QHexMetadataItem &oldmeta) {
    if (removeMetadata(oldmeta)) {
        metadata(newmeta.begin, newmeta.end, newmeta.foreground,
                 newmeta.background, newmeta.comment);
        return true;
    }
    return false;
}

bool QHexMetadata::removeMetadata(const QHexMetadataItem &item) {
    auto index = m_metadata.indexOf(item);
    if (index < 0) {
        return false;
    }
    m_metadata.removeAt(index);
    for (auto &l : m_linemeta) {
        l.remove(item);
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_linemeta.erase(std::remove_if(
        m_linemeta.begin(), m_linemeta.end(),
        [](const QHash<QHexMetadataItem, QHexLineMetadata> &item) {
            return item.isEmpty();
        }));
#else
    m_linemeta.removeIf(
        [](const QPair<qsizetype, QHash<QHexMetadataItem, QHexLineMetadata>>
               &item) { return item.second.isEmpty(); });
#endif

    emit metadataChanged();
    return true;
}

void QHexMetadata::removeMetadata(qsizetype offset) {
    auto rmfn = [offset, this](const QHexMetadataItem &item) {
        auto r = offset >= item.begin && offset <= item.end;
        if (r) {
            for (auto &l : m_linemeta) {
                l.remove(item);
            }
        }
        return r;
    };

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_metadata.erase(
        std::remove_if(m_metadata.begin(), m_metadata.end(), rmfn));
#else
    m_metadata.removeIf(rmfn);
#endif

    emit metadataChanged();
}

QVector<QHexMetadataItem> QHexMetadata::getAllMetadata() const {
    return m_metadata;
}

QVector<QHexMetadataItem> QHexMetadata::gets(qsizetype offset) {
    QVector<QHexMetadataItem> ret;

    std::copy_if(
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        m_metadata.begin(), m_metadata.end(),
#else
        m_metadata.constBegin(), m_metadata.constEnd(),
#endif
        std::back_inserter(ret), [offset](const QHexMetadataItem &item) {
            return offset >= item.begin && offset <= item.end;
        });

    return ret;
}

void QHexMetadata::applyMetas(const QVector<QHexMetadataItem> &metas) {
    m_metadata = metas;
}

bool QHexMetadata::hasMetadata() { return m_metadata.count() > 0; }

/*==================================*/

QString QHexMetadata::comments(qsizetype line, qsizetype column) const {
    if (!this->lineHasMetadata(line))
        return QString();

    QString s;

    const auto &linemetadata = this->get(line);

    for (auto &mi : linemetadata) {
        if (!(mi.start <= column && column < mi.start + mi.length))
            continue;
        if (mi.comment.isEmpty())
            continue;

        if (!s.isEmpty())
            s += "\n";

        s += mi.comment;
    }

    return s;
}

bool QHexMetadata::lineHasMetadata(qsizetype line) const {
    return m_linemeta.contains(line);
}

qsizetype QHexMetadata::size() const { return m_metadata.size(); }

void QHexMetadata::clear() {
    m_linemeta.clear();
    m_metadata.clear();
    emit metadataChanged();
}

bool QHexMetadata::metadata(qsizetype begin, qsizetype end,
                            const QColor &fgcolor, const QColor &bgcolor,
                            const QString &comment) {
    if (begin < end ||
        ((!fgcolor.isValid() || fgcolor.alpha() == 0) &&
         (!bgcolor.isValid() || bgcolor.alpha() == 0) && comment.isEmpty()))
        return false;
    QHexMetadataItem absi{begin, end, fgcolor, bgcolor, comment};
    addMetadata(absi);
    emit metadataChanged();
    return true;
}

void QHexMetadata::setLineWidth(quint8 width) {
    if (width != m_lineWidth) {
        m_lineWidth = width;

        m_linemeta.clear();
        for (auto &item : m_metadata) {
            addMetadata(item);
        }

        emit metadataChanged();
    }
}

void QHexMetadata::insertAdjust(qsizetype offset, qsizetype length) {
    m_linemeta.clear();

    QtConcurrent::blockingMap(
        m_metadata, [offset, length](QHexMetadataItem &meta) {
            if (meta.end < offset) {
                return;
            }
            if (meta.begin <= offset && meta.end > offset) {
                meta.end += length;
            } else {
                meta.begin += length;
                meta.end += length;
            }
        });

    for (auto &meta : m_metadata) {
        addMetaLines(meta);
    }
}

void QHexMetadata::removeAdjust(qsizetype offset, qsizetype length) {
    m_linemeta.clear();

    QtConcurrent::blockingMap(
        m_metadata, [offset, length](QHexMetadataItem &meta) {
            if (meta.end < offset) {
                return;
            }
            if (meta.begin <= offset && meta.end > offset) {
                meta.end -= length;
                if (meta.begin > meta.end) {
                    meta.flag = true;
                }
            } else {
                meta.begin -= length;
                meta.end -= length;
            }
        });

    auto rmfn = [](const QHexMetadataItem &meta) { return meta.flag; };

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_metadata.erase(
        std::remove_if(m_metadata.begin(), m_metadata.end(), rmfn));
#else
    m_metadata.removeIf(rmfn);
#endif

    for (auto &meta : m_metadata) {
        addMetaLines(meta);
    }
}

bool QHexMetadata::color(qsizetype begin, qsizetype end, const QColor &fgcolor,
                         const QColor &bgcolor) {
    return this->metadata(begin, end, fgcolor, bgcolor, QString());
}

bool QHexMetadata::foreground(qsizetype begin, qsizetype end,
                              const QColor &fgcolor) {
    return this->color(begin, end, fgcolor, QColor());
}

bool QHexMetadata::background(qsizetype begin, qsizetype end,
                              const QColor &bgcolor) {
    return this->color(begin, end, QColor(), bgcolor);
}

bool QHexMetadata::comment(qsizetype begin, qsizetype end,
                           const QString &comment) {
    return this->metadata(begin, end, QColor(), QColor(), comment);
}

void QHexMetadata::addMetadata(const QHexMetadataItem &mi) {
    addMetaLines(mi);
    m_metadata << mi;
}

void QHexMetadata::addMetaLines(const QHexMetadataItem &mi) {
    const auto firstRow = mi.begin / m_lineWidth;
    const auto lastRow = mi.end / m_lineWidth;

    for (auto row = firstRow; row <= lastRow; ++row) {
        qsizetype start, length;
        if (row == firstRow) {
            Q_ASSERT(m_lineWidth > 0);
            start = mi.begin % m_lineWidth;
        } else {
            start = 0;
        }
        if (row == lastRow) {
            Q_ASSERT(m_lineWidth > 0);
            const int lastChar = mi.end % m_lineWidth;
            length = lastChar - start;
        } else {
            // fix the bug by wingsummer
            if (firstRow != lastRow)
                length = m_lineWidth - start;
            else
                length = m_lineWidth;
        }

        if (length > 0) {
            m_linemeta[row][mi].append(
                {start, length, mi.foreground, mi.background, mi.comment});
        }
    }
}
