/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#ifndef ASYNCTABLEPROXYMODEL_H
#define ASYNCTABLEPROXYMODEL_H

#include <QAbstractProxyModel>
#include <QElapsedTimer>
#include <QFutureWatcher>
#include <QHash>
#include <QTimer>
#include <atomic>
#include <memory>

// Flat table proxy. Source access and mapping publication stay on the GUI
// thread.
class AsyncTableProxyModel : public QAbstractProxyModel {
    Q_OBJECT
public:
    explicit AsyncTableProxyModel(QObject *parent = nullptr);
    ~AsyncTableProxyModel() override;

public:
    void setSourceModel(QAbstractItemModel *source) override;
    QModelIndex mapToSource(const QModelIndex &index) const override;
    QModelIndex mapFromSource(const QModelIndex &index) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role) override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    void setColumnSortable(int column, bool enabled);
    bool isColumnSortable(int column) const;
    void setColumnSortRole(int column, int role);
    bool isSorting() const;

signals:
    void sortingChanged(bool busy);
    void sortCleared();
    void sortingMeasured(qint64 snapshotMs, qint64 workerMs, qint64 publishMs);

private:
    void invalidate();
    void collectKeys();
    void publish(const QList<int> &rows, const QList<int> &inverse);
    void resetRows();
    void setBusy(bool busy);

private:
    struct Result {
        QList<int> rows;
        QList<int> inverse;
        quint64 generation = 0;
        qint64 workerMs = 0;
    };
    QList<int> m_rows;
    QList<int> m_inverse;
    QList<QVariant> m_keys;
    QList<QMetaObject::Connection> m_connections;
    QHash<int, bool> m_sortable;
    QHash<int, int> m_roles;
    QTimer m_collect;
    QElapsedTimer m_snapshotClock;
    qint64 m_snapshotMs = 0;
    QFutureWatcher<Result> m_worker;
    std::shared_ptr<std::atomic<quint64>> m_generation;
    quint64 m_collectGeneration = 0;
    int m_column = -1;
    Qt::SortOrder m_order = Qt::AscendingOrder;
    bool m_pending = false;
    bool m_busy = false;
    bool m_workerActive = false;
    bool m_resetting = false;
};
#endif
