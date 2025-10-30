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
** The original License is MIT from Dax89/QHexView. I have modified a lot so I
** decide to change the Open Source License. You can use the original library
** under MIT. Thanks for Dax89's efforts.
** =============================================================================
*/

#ifndef QHEXMETADATA_H
#define QHEXMETADATA_H

#include "qhexregionobject.h"

#include <QObject>

#include <QColor>
#include <QHash>
#include <QMap>
#include <QUndoStack>
#include <QVector>

using qhash_result_t = size_t;

struct QHexMetadataItem : QHexRegionObject<qsizetype, QHexMetadataItem> {
    QColor foreground, background;
    QString comment;
    bool flag = false;

    QHexMetadataItem() {
        this->begin = -1;
        this->end = -1;
    }

    explicit QHexMetadataItem(qsizetype begin, qsizetype end, QColor foreground,
                              QColor background, const QString comment)
        : foreground(foreground), background(background), comment(comment) {
        this->begin = begin;
        this->end = end;
    }

    // added by wingsummer
    bool operator==(const QHexMetadataItem &item) const {
        return begin == item.begin && end == item.end &&
               foreground == item.foreground && background == item.background &&
               comment == item.comment;
    }

    // QHexRegionObject interface
public:
    // true: merged successfully
    // false: not merge yet
    std::variant<bool, QHexMetadataItem>
    mergeRegion(const QHexMetadataItem &sel,
                QMutex *locker = nullptr) override {
        if (sel.foreground == this->foreground &&
            sel.background == this->background &&
            sel.comment == this->comment) {
            if (mergeRegionWithoutMetaCheck(sel, locker)) {
                return true;
            }
            return false;
        } else {
            auto ret = removeRegion(sel, locker);
            if (std::holds_alternative<QHexMetadataItem>(ret)) {
                return std::get<QHexMetadataItem>(ret);
            } else {
                return false;
            }
        }
    }

    bool mergeRegionWithoutMetaCheck(const QHexMetadataItem &sel,
                                     QMutex *locker = nullptr) {
        Q_ASSERT(sel.foreground == this->foreground &&
                 sel.background == this->background &&
                 sel.comment == this->comment);
        auto ret = Super::mergeRegion(sel, locker);
        if (std::holds_alternative<bool>(ret)) {
            return std::get<bool>(ret);
        }
        return false;
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
    struct MetaInfo : QHexRegionObject<qsizetype, MetaInfo> {
        QColor foreground;
        QColor background;
        QString comment;

        explicit MetaInfo() {
            this->begin = -1;
            this->end = -1;
        }

        explicit MetaInfo(qsizetype begin, qsizetype end) {
            this->begin = begin;
            this->end = end;
        }

        explicit MetaInfo(qsizetype begin, qsizetype end, QColor foreground,
                          QColor background, QString comment)
            : foreground(foreground), background(background), comment(comment) {
            this->begin = begin;
            this->end = end;
        }
    };

public:
    explicit QHexMetadata(QUndoStack *undo, QObject *parent = nullptr);
    QHexLineMetadata get(qsizetype line) const;
    bool lineHasMetadata(qsizetype line) const; // modified by wingsummer

    qsizetype size() const;

    /*============================*/
    // added by wingsummer

    void beginMarco(const QString &text);
    void endMarco();

    bool ModifyMetadata(const QHexMetadataItem &newmeta,
                        const QHexMetadataItem &oldmeta);
    void RemoveMetadatas(const QList<QHexMetadataItem> &items);
    bool RemoveMetadata(const QHexMetadataItem &item);
    bool RemoveMetadata(qsizetype offset);
    void Metadata(qsizetype begin, qsizetype end, const QColor &fgcolor,
                  const QColor &bgcolor, const QString &comment);
    void Clear();

    QUndoCommand *MakeModifyMetadata(QUndoCommand *parent,
                                     const QHexMetadataItem &newmeta,
                                     const QHexMetadataItem &oldmeta);
    QUndoCommand *MakeRemoveMetadata(QUndoCommand *parent,
                                     const QHexMetadataItem &item);
    QUndoCommand *MakeRemoveMetadata(QUndoCommand *parent, qsizetype offset);
    QUndoCommand *MakeMetadata(QUndoCommand *parent, qsizetype begin,
                               qsizetype end, const QColor &fgcolor,
                               const QColor &bgcolor, const QString &comment);
    QUndoCommand *MakeClear(QUndoCommand *parent);

    //---------------------------------------------------------

    bool modifyMetadata(const QHexMetadataItem &newmeta,
                        const QHexMetadataItem &oldmeta);
    bool removeMetadata(const QHexMetadataItem &item);
    void removeMetadata(qsizetype begin, qsizetype end);
    bool removeLineMetadata(const QHexMetadataItem &item);
    void removeMetadata(qsizetype offset);

    QVector<QHexMetadataItem> getAllMetadata() const;

    std::optional<QHexMetadataItem> get(qsizetype offset);
    QHexLineMetadata gets(qsizetype line);
    QVector<MetaInfo> getRealMetaRange(qsizetype begin, qsizetype end);

    void applyMetas(const QVector<QHexMetadataItem> &metas);

    bool hasMetadata();

public:
    static QColor generateContrastingColor(const QColor &backgroundColor);

    static bool areColorsContrast(const QColor &color1, const QColor &color2);

    // Function to calculate relative luminance
    static double calculateLuminance(const QColor &color);

    // Function to calculate contrast ratio
    static double calculateContrastRatio(const QColor &color1,
                                         const QColor &color2);

    /*============================*/

public:
    void clear();
    void setLineWidth(quint8 width);

    void insertAdjust(qsizetype offset, qsizetype length);
    void insertAdjustRevert(qsizetype offset, qsizetype length);
    QVector<QHexMetadataItem> removeAdjust(qsizetype offset, qsizetype length);
    void removeAdjustRevert(const QVector<QHexMetadataItem> &metas,
                            qsizetype offset, qsizetype length);

public:
    // new interface with begin, end
    bool metadata(qsizetype begin, qsizetype end, const QColor &fgcolor,
                  const QColor &bgcolor, const QString &comment);

    bool color(qsizetype begin, qsizetype end, const QColor &fgcolor,
               const QColor &bgcolor);
    bool foreground(qsizetype begin, qsizetype end, const QColor &fgcolor);
    bool background(qsizetype begin, qsizetype end, const QColor &bgcolor);
    bool comment(qsizetype begin, qsizetype end, const QString &comment);

    QVector<QHexMetadataItem> mayBrokenMetaData(qsizetype begin, qsizetype end);

private:
    void addMetadata(const QHexMetadataItem &mi);

    void addMetaLines(const QHexMetadataItem &mi);

    bool checkValidMetadata(const QHexMetadataItem &mi);
    bool checkValidMetadata(qsizetype begin, qsizetype end,
                            const QColor &fgcolor, const QColor &bgcolor,
                            const QString &comment);

signals:
    void metadataChanged();

private:
    quint8 m_lineWidth;

    QMap<qsizetype, QHash<QHexMetadataItem, QHexLineMetadata>> m_linemeta;

    QHexRegionObjectList<qsizetype, QHexMetadataItem> m_metadata;

    QUndoStack *m_undo = nullptr; // added by wingsummer
};

#endif // QHEXMETADATA_H
