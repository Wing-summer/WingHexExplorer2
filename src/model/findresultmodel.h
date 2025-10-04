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
** =============================================================================
*/

#ifndef FINDRESULTMODEL_H
#define FINDRESULTMODEL_H

#include <QAbstractTableModel>

#include "utilities.h"

class FindResultModel : public QAbstractTableModel {
    Q_OBJECT

public:
    struct FindInfo {
        QByteArray cheader;
        QByteArray hbuffer;
        QByteArray tbuffer;
        QByteArray ctailer;

        inline QString generateRange() const {
            QString buffer = cheader.toHex(' ').toUpper();
            if (!hbuffer.isEmpty()) {
                buffer += hbuffer.toHex(' ').toUpper();
                if (!tbuffer.isEmpty()) {
                    buffer += QStringLiteral(" .. ");
                }
            }
            buffer +=
                tbuffer.toHex(' ').toUpper() + ctailer.toHex(' ').toUpper();
            return buffer;
        }

        inline QString generateDecoding(const QString &encoding) const {
            QString buffer = Utilities::decodingString(cheader, encoding);
            if (!hbuffer.isEmpty()) {
                buffer += Utilities::decodingString(hbuffer);
                if (!tbuffer.isEmpty()) {
                    buffer += QStringLiteral(" ... ");
                }
            }

            buffer += Utilities::decodingString(tbuffer) +
                      Utilities::decodingString(ctailer);

            return buffer;
        }
    };

    struct FindResult {
        qsizetype offset = -1;
        qsizetype line = -1;
        qsizetype col = -1;
    };

    struct FindData {
        QList<FindResult> results;
        QList<FindInfo> findData;
        QPair<QString, qsizetype> lastFindData;
        QString encoding = QStringLiteral("ASCII");

        inline void clear() {
            results.clear();
            findData.clear();
            lastFindData = {};
        }

        inline QString generateRange(qsizetype idx) const {
            return findData.at(idx).generateRange();
        }

        inline QString generateDecoding(qsizetype idx) const {
            return findData.at(idx).generateDecoding(encoding);
        }
    };

public:
    explicit FindResultModel(QObject *parent = nullptr);

    bool isValid() const;
    void updateData(FindData &data);
    void reset();
    void clear();
    void refresh();

    QString copyContent(const QModelIndex &index) const;

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

    QString encoding() const;
    void setEncoding(const QString &newEncoding);

private:
    FindData *m_data = nullptr;
};

#endif // FINDRESULTMODEL_H
