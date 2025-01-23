#include "qhexmetadata.h"
#include "commands/meta/metaaddcommand.h"
#include "commands/meta/metaclearcommand.h"
#include "commands/meta/metaremovecommand.h"
#include "commands/meta/metaremoveposcommand.h"
#include "commands/meta/metareplacecommand.h"

#include <cmath>

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

void QHexMetadata::ModifyMetadata(const QHexMetadataItem &newmeta,
                                  const QHexMetadataItem &oldmeta) {
    auto cmd = MakeModifyMetadata(nullptr, newmeta, oldmeta);
    if (cmd) {
        m_undo->push(cmd);
    }
}

void QHexMetadata::RemoveMetadatas(const QList<QHexMetadataItem> &items) {
    m_undo->beginMacro("RemoveMetadatas");
    for (auto &item : items) {
        RemoveMetadata(item);
    }
    m_undo->endMacro();
}

void QHexMetadata::RemoveMetadata(const QHexMetadataItem &item) {
    auto cmd = MakeRemoveMetadata(nullptr, item);
    if (cmd) {
        m_undo->push(cmd);
    }
}

void QHexMetadata::RemoveMetadata(qsizetype offset) {
    auto cmd = MakeRemoveMetadata(nullptr, offset);
    if (cmd) {
        m_undo->push(cmd);
    }
}

void QHexMetadata::Metadata(qsizetype begin, qsizetype end,
                            const QColor &fgcolor, const QColor &bgcolor,
                            const QString &comment) {
    auto cmd = MakeMetadata(nullptr, begin, end, fgcolor, bgcolor, comment);
    if (cmd) {
        m_undo->push(cmd);
    }
}

void QHexMetadata::Clear() {
    auto cmd = MakeClear(nullptr);
    if (cmd) {
        m_undo->push(cmd);
    }
}

QUndoCommand *QHexMetadata::MakeColor(QUndoCommand *parent, qsizetype begin,
                                      qsizetype end, const QColor &fgcolor,
                                      const QColor &bgcolor) {
    return MakeMetadata(parent, begin, end, fgcolor, bgcolor, QString());
}

QUndoCommand *QHexMetadata::MakeForeground(QUndoCommand *parent,
                                           qsizetype begin, qsizetype end,
                                           const QColor &fgcolor) {
    return MakeMetadata(parent, begin, end, fgcolor, QColor(), QString());
}

QUndoCommand *QHexMetadata::MakeBackground(QUndoCommand *parent,
                                           qsizetype begin, qsizetype end,
                                           const QColor &bgcolor) {
    return MakeMetadata(parent, begin, end, QColor(), bgcolor, QString());
}

QUndoCommand *QHexMetadata::MakeComment(QUndoCommand *parent, qsizetype begin,
                                        qsizetype end, const QString &comment) {
    return MakeMetadata(parent, begin, end, QColor(), QColor(), comment);
}

QUndoCommand *
QHexMetadata::MakeModifyMetadata(QUndoCommand *parent,
                                 const QHexMetadataItem &newmeta,
                                 const QHexMetadataItem &oldmeta) {
    if (m_metadata.contains(oldmeta) && checkValidMetadata(newmeta)) {
        return new MetaReplaceCommand(this, newmeta, oldmeta, parent);
    }
    return nullptr;
}

QUndoCommand *QHexMetadata::MakeRemoveMetadata(QUndoCommand *parent,
                                               const QHexMetadataItem &item) {
    if (m_metadata.contains(item)) {
        return new MetaRemoveCommand(this, item, parent);
    }
    return nullptr;
}

QUndoCommand *QHexMetadata::MakeRemoveMetadata(QUndoCommand *parent,
                                               qsizetype offset) {
    if (get(offset)) {
        return new MetaRemovePosCommand(this, offset, parent);
    }
    return nullptr;
}

QUndoCommand *QHexMetadata::MakeMetadata(QUndoCommand *parent, qsizetype begin,
                                         qsizetype end, const QColor &fgcolor,
                                         const QColor &bgcolor,
                                         const QString &comment) {
    if (checkValidMetadata(begin, end, fgcolor, bgcolor, comment)) {
        QHexMetadataItem absi(begin, end, fgcolor, bgcolor, comment);
        return new MetaAddCommand(this, absi, parent);
    }
    return nullptr;
}

QUndoCommand *QHexMetadata::MakeClear(QUndoCommand *parent) {
    auto metas = this->getAllMetadata();
    if (metas.isEmpty()) {
        return nullptr;
    }
    return new MetaClearCommand(this, metas, parent);
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

    m_metadata.takeAt(index);
    auto ret = removeLineMetadata(item);
    return ret;
}

void QHexMetadata::removeMetadata(qsizetype begin, qsizetype end) {
    auto broken = mayBrokenMetaData(begin, end);
    for (auto &item : broken) {
        removeMetadata(item);
    }
}

bool QHexMetadata::removeLineMetadata(const QHexMetadataItem &item) {
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

std::optional<QHexMetadataItem> QHexMetadata::get(qsizetype offset) {
    auto r = std::find_if(m_metadata.begin(), m_metadata.end(),
                          [offset](const QHexMetadataItem &item) {
                              return offset >= item.begin && offset <= item.end;
                          });
    if (r == m_metadata.end()) {
        return {};
    }
    return *r;
}

QHexLineMetadata QHexMetadata::gets(qsizetype line) {
    QHexLineMetadata ret;

    if (!m_linemeta.contains(line)) {
        return {};
    }

    for (auto &lms : m_linemeta[line]) {
        ret.append(lms);
    }

    return ret;
}

QVector<QHexMetadata::MetaInfo> QHexMetadata::getRealMetaRange(qsizetype begin,
                                                               qsizetype end) {
    Q_ASSERT(begin <= end);

    QVector<MetaInfo> ret;
    MetaInfo g(begin, end);
    for (auto &meta : m_metadata) {
        if (!(end < meta.begin || begin > meta.end)) {
            auto m = MetaInfo(meta.begin, meta.end, meta.foreground,
                              meta.background, meta.comment);
            if (m.intersect(g)) {
                ret.insert(std::distance(ret.constBegin(),
                                         std::upper_bound(ret.constBegin(),
                                                          ret.constEnd(), m)),
                           m);
            }
        }
    }

    return ret;
}

void QHexMetadata::applyMetas(const QVector<QHexMetadataItem> &metas) {
    for (auto &meta : metas) {
        m_metadata.mergeAdd(meta);
    }
    for (auto &meta : m_metadata) {
        addMetaLines(meta);
    }
    emit metadataChanged();
}

bool QHexMetadata::hasMetadata() { return m_metadata.count() > 0; }

QColor QHexMetadata::generateContrastingColor(const QColor &backgroundColor) {
    // Invert RGB values
    QColor invertedColor(255 - backgroundColor.red(),
                         255 - backgroundColor.green(),
                         255 - backgroundColor.blue());

    // Ensure it meets contrast ratio criteria
    double contrastRatio =
        calculateContrastRatio(backgroundColor, invertedColor);
    if (contrastRatio >= 4.5) {
        return invertedColor;
    }

    // If contrast is still too low, adjust brightness or saturation
    int luminance = (backgroundColor.red() + backgroundColor.green() +
                     backgroundColor.blue()) /
                    3;
    return (luminance < 128) ? QColor(255, 255, 255)
                             : QColor(0, 0, 0); // Use black or white
}

/*==================================*/

bool QHexMetadata::lineHasMetadata(qsizetype line) const {
    return m_linemeta.contains(line);
}

qsizetype QHexMetadata::size() const { return m_metadata.size(); }

void QHexMetadata::beginMarco(const QString &text) { m_undo->beginMacro(text); }

void QHexMetadata::endMarco() { m_undo->endMacro(); }

void QHexMetadata::clear() {
    m_linemeta.clear();
    m_metadata.clear();
    emit metadataChanged();
}

bool QHexMetadata::metadata(qsizetype begin, qsizetype end,
                            const QColor &fgcolor, const QColor &bgcolor,
                            const QString &comment) {
    if (checkValidMetadata(begin, end, fgcolor, bgcolor, comment)) {
        QHexMetadataItem absi(begin, end, fgcolor, bgcolor, comment);
        addMetadata(absi);
        emit metadataChanged();
        return true;
    }
    return false;
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
            if (meta.begin <= offset && meta.end >= offset) {
                meta.end += length;
            } else {
                meta.begin += length;
                meta.end += length;
            }
        });

    for (auto &meta : m_metadata) {
        addMetaLines(meta);
    }

    emit metadataChanged();
}

void QHexMetadata::insertAdjustRevert(qsizetype offset, qsizetype length) {
    m_linemeta.clear();

    QtConcurrent::blockingMap(
        m_metadata, [offset, length](QHexMetadataItem &meta) {
            if (meta.end < offset) {
                return;
            }
            if (meta.begin <= offset && meta.end >= offset) {
                meta.end -= length;
            } else {
                meta.begin -= length;
                meta.end -= length;
            }
        });

    for (auto &meta : m_metadata) {
        addMetaLines(meta);
    }

    emit metadataChanged();
}

QVector<QHexMetadataItem> QHexMetadata::removeAdjust(qsizetype offset,
                                                     qsizetype length) {
    QVector<QHexMetadataItem> rmmetas;

    if (!m_metadata.isEmpty()) {
        m_linemeta.clear();

        QtConcurrent::blockingMap(
            m_metadata, [offset, length](QHexMetadataItem &meta) {
                if (meta.end < offset) {
                    return;
                }
                if (meta.begin <= offset && meta.end > offset) {
                    if (meta.begin > meta.end - length) {
                        meta.flag = true;
                    } else {
                        meta.end -= length;
                    }
                } else {
                    meta.begin -= length;
                    meta.end -= length;
                }
            });

        auto rmfn = [](const QHexMetadataItem &meta) { return meta.flag; };

        std::copy_if(
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            m_metadata.begin(), m_metadata.end(),
#else
            m_metadata.constBegin(), m_metadata.constEnd(),
#endif
            std::back_inserter(rmmetas), rmfn);

        m_metadata.erase(
            std::remove_if(m_metadata.begin(), m_metadata.end(), rmfn));

        for (auto &meta : m_metadata) {
            addMetaLines(meta);
        }

        emit metadataChanged();

        QtConcurrent::blockingMap(
            rmmetas, [](QHexMetadataItem &meta) { meta.flag = false; });
    }

    return rmmetas;
}

void QHexMetadata::removeAdjustRevert(const QVector<QHexMetadataItem> &metas,
                                      qsizetype offset, qsizetype length) {
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

    for (auto &meta : metas) {
        m_metadata.insert(
            std::distance(m_metadata.constBegin(),
                          std::upper_bound(m_metadata.constBegin(),
                                           m_metadata.constEnd(), meta)),
            meta);
    }

    for (auto &meta : m_metadata) {
        addMetaLines(meta);
    }

    emit metadataChanged();
}

bool QHexMetadata::areColorsContrast(const QColor &color1,
                                     const QColor &color2) {
    return calculateContrastRatio(color1, color2) >= 4.5;
}

double QHexMetadata::calculateLuminance(const QColor &color) {
    // Normalize RGB values to [0, 1]
    double r = color.redF();
    double g = color.greenF();
    double b = color.blueF();

    // Apply gamma correction
    auto gammaCorrect = [](double value) {
        return (value <= 0.03928) ? (value / 12.92)
                                  : std::pow((value + 0.055) / 1.055, 2.4);
    };

    r = gammaCorrect(r);
    g = gammaCorrect(g);
    b = gammaCorrect(b);

    // Calculate relative luminance
    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

double QHexMetadata::calculateContrastRatio(const QColor &color1,
                                            const QColor &color2) {
    double luminance1 = calculateLuminance(color1);
    double luminance2 = calculateLuminance(color2);

    // Ensure luminance1 is the lighter one
    if (luminance1 < luminance2)
        std::swap(luminance1, luminance2);

    return (luminance1 + 0.05) / (luminance2 + 0.05);
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

QVector<QHexMetadataItem> QHexMetadata::mayBrokenMetaData(qsizetype begin,
                                                          qsizetype end) {
    QVector<QHexMetadataItem> ret;
    std::copy_if(
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        m_metadata.begin(), m_metadata.end(),
#else
        m_metadata.constBegin(), m_metadata.constEnd(),
#endif
        std::back_inserter(ret), [begin, end](const QHexMetadataItem &item) {
            return !(end < item.begin || begin > item.end);
        });
    return ret;
}

void QHexMetadata::addMetadata(const QHexMetadataItem &mi) {
    auto old = m_metadata;
    auto idx = m_metadata.mergeAdd(mi);
    if (idx >= 0) {
        auto meta = m_metadata.at(idx);
        auto lastMeta = m_metadata.last();
        removeLineMetadata(old.at(idx));
        addMetaLines(meta);
        addMetaLines(lastMeta);
    }

    addMetaLines(mi);
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
            length = lastChar - start + 1;
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

bool QHexMetadata::checkValidMetadata(const QHexMetadataItem &mi) {
    return checkValidMetadata(mi.begin, mi.end, mi.foreground, mi.background,
                              mi.comment);
}

bool QHexMetadata::checkValidMetadata(qsizetype begin, qsizetype end,
                                      const QColor &fgcolor,
                                      const QColor &bgcolor,
                                      const QString &comment) {
    if (begin > end)
        return false;

    if (!fgcolor.isValid() || fgcolor.alpha() == 0) {
        if (!bgcolor.isValid() || bgcolor.alpha() == 0) {
            if (comment.isEmpty()) {
                return false;
            }
        }
    }
    return true;
}
