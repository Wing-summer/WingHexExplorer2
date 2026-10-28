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

#include "asynctableproxymodel.h"

#include <QCollator>
#include <QElapsedTimer>
#include <QtConcurrentRun>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <optional>

namespace {

enum class KeyKind { Missing, SignedInteger, UnsignedInteger, Floating, Text };

struct SortKey {
    KeyKind kind = KeyKind::Missing;
    union {
        qint64 signedValue = 0;
        quint64 unsignedValue;
        double floatingValue;
    };
    std::optional<QCollatorSortKey> text;
};

template <typename T>
int compareValues(T a, T b) {
    if (a < b) {
        return -1;
    }
    if (a > b) {
        return 1;
    }
    return 0;
}

KeyKind keyKind(const QVariant &value) {
    if (!value.isValid()) {
        return KeyKind::Missing;
    }
    switch (value.metaType().id()) {
    case QMetaType::Bool:
    case QMetaType::Char:
    case QMetaType::SChar:
    case QMetaType::Short:
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
        return KeyKind::SignedInteger;
    case QMetaType::UChar:
    case QMetaType::UShort:
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
        return KeyKind::UnsignedInteger;
    case QMetaType::Float:
    case QMetaType::Double:
        return KeyKind::Floating;
    default:
        return KeyKind::Text;
    }
}

SortKey prepareKey(const QVariant &value, const QCollator &collator) {
    SortKey key;
    key.kind = keyKind(value);
    switch (key.kind) {
    case KeyKind::SignedInteger:
        key.signedValue = value.toLongLong();
        break;
    case KeyKind::UnsignedInteger:
        key.unsignedValue = value.toULongLong();
        break;
    case KeyKind::Floating:
        key.floatingValue = value.toDouble();
        break;
    case KeyKind::Text:
        key.text = collator.sortKey(value.toString());
        break;
    case KeyKind::Missing:
        break;
    }
    return key;
}

int compareFloatingToInteger(double value, const SortKey &integer) {
    if (std::isnan(value)) {
        return -1;
    }
    int integralComparison;
    double integralValue;
    if (integer.kind == KeyKind::UnsignedInteger) {
        if (value < 0) {
            return -1;
        }
        if (value >= std::ldexp(1.0, 64)) {
            return 1;
        }
        const auto truncated = quint64(value);
        integralComparison = compareValues(truncated, integer.unsignedValue);
        integralValue = double(truncated);
    } else {
        if (value < -std::ldexp(1.0, 63)) {
            return -1;
        }
        if (value >= std::ldexp(1.0, 63)) {
            return 1;
        }
        const auto truncated = qint64(value);
        integralComparison = compareValues(truncated, integer.signedValue);
        integralValue = double(truncated);
    }
    return integralComparison != 0 ? integralComparison
                                   : compareValues(value, integralValue);
}

int compareNumbers(const SortKey &a, const SortKey &b) {
    if (a.kind == KeyKind::Floating && b.kind == KeyKind::Floating) {
        if (std::isnan(a.floatingValue) != std::isnan(b.floatingValue)) {
            return std::isnan(a.floatingValue) ? -1 : 1;
        }
        return compareValues(a.floatingValue, b.floatingValue);
    }
    if (a.kind == KeyKind::Floating) {
        return compareFloatingToInteger(a.floatingValue, b);
    }
    if (b.kind == KeyKind::Floating) {
        return -compareFloatingToInteger(b.floatingValue, a);
    }
    // Preserve all 64 bits, including on platforms where long double is double.
    if (a.kind == KeyKind::SignedInteger && b.kind == KeyKind::SignedInteger) {
        return compareValues(a.signedValue, b.signedValue);
    }
    if (a.kind == KeyKind::UnsignedInteger &&
        b.kind == KeyKind::UnsignedInteger) {
        return compareValues(a.unsignedValue, b.unsignedValue);
    }
    if (a.kind == KeyKind::SignedInteger) {
        return a.signedValue < 0
                   ? -1
                   : compareValues(quint64(a.signedValue), b.unsignedValue);
    }
    return b.signedValue < 0
               ? 1
               : compareValues(a.unsignedValue, quint64(b.signedValue));
}

int compareKeys(const SortKey &a, const SortKey &b) {
    if (a.kind == KeyKind::Missing || b.kind == KeyKind::Missing) {
        return compareValues(a.kind != KeyKind::Missing,
                             b.kind != KeyKind::Missing);
    }
    if (a.kind == KeyKind::Text && b.kind == KeyKind::Text) {
        return a.text->compare(*b.text);
    }
    if (a.kind == KeyKind::Text || b.kind == KeyKind::Text) {
        return a.kind == KeyKind::Text ? 1 : -1;
    }
    return compareNumbers(a, b);
}

QList<int> sortedRows(const QList<QVariant> &values, Qt::SortOrder order,
                      const std::atomic<quint64> &generation,
                      quint64 expected) {
    QCollator collator;
    QList<SortKey> keys;
    keys.reserve(values.size());
    for (const auto &value : values) {
        if (generation.load(std::memory_order_relaxed) != expected) {
            return {};
        }
        keys.append(prepareKey(value, collator));
    }
    QList<int> rows(values.size());
    std::iota(rows.begin(), rows.end(), 0);
    // Sort only row numbers: no large key swaps or stable_sort scratch buffer.
    // Source row is the tie breaker, providing stable source order both ways.
    std::sort(rows.begin(), rows.end(), [&keys, order](int a, int b) {
        const int comparison = compareKeys(keys.at(a), keys.at(b));
        if (comparison == 0) {
            return a < b;
        }
        return order == Qt::AscendingOrder ? comparison < 0 : comparison > 0;
    });
    return generation.load(std::memory_order_relaxed) == expected
               ? rows
               : QList<int>();
}
} // namespace

AsyncTableProxyModel::AsyncTableProxyModel(QObject *parent)
    : QAbstractProxyModel(parent),
      m_generation(std::make_shared<std::atomic<quint64>>(0)) {
    m_collect.setSingleShot(true);
    connect(&m_collect, &QTimer::timeout, this,
            &AsyncTableProxyModel::collectKeys);
    connect(&m_worker, &QFutureWatcher<Result>::finished, this, [this] {
        m_workerActive = false;
        const auto result = m_worker.result();
        if (result.generation == m_generation->load() && !m_resetting) {
            QElapsedTimer commit;
            commit.start();
            publish(result.rows, result.inverse);
            Q_EMIT sortingMeasured(m_snapshotMs, result.workerMs,
                                   commit.elapsed());
        }
        if (m_pending) {
            m_collect.start(0);
        } else {
            setBusy(false);
        }
    });
}

AsyncTableProxyModel::~AsyncTableProxyModel() { ++*m_generation; }

void AsyncTableProxyModel::setSourceModel(QAbstractItemModel *source) {
    for (const auto &connection : std::as_const(m_connections)) {
        disconnect(connection);
    }
    m_connections.clear();
    ++*m_generation;
    m_collect.stop();
    m_keys.clear();
    beginResetModel();
    QAbstractProxyModel::setSourceModel(source);
    resetRows();
    endResetModel();
    if (source) {
        const auto before = [this] {
            ++*m_generation;
            m_collect.stop();
            m_keys.clear();
            if (!m_resetting) {
                m_resetting = true;
                beginResetModel();
            }
        };
        const auto after = [this] {
            if (!m_resetting) {
                beginResetModel();
            }
            resetRows();
            m_resetting = false;
            endResetModel();
            invalidate();
        };
        const auto watchStructure = [this, source, before,
                                     after](auto aboutToChange, auto changed) {
            m_connections << connect(source, aboutToChange, this, before);
            m_connections << connect(source, changed, this, after);
        };
        watchStructure(&QAbstractItemModel::modelAboutToBeReset,
                       &QAbstractItemModel::modelReset);
        watchStructure(&QAbstractItemModel::rowsAboutToBeInserted,
                       &QAbstractItemModel::rowsInserted);
        watchStructure(&QAbstractItemModel::rowsAboutToBeRemoved,
                       &QAbstractItemModel::rowsRemoved);
        watchStructure(&QAbstractItemModel::rowsAboutToBeMoved,
                       &QAbstractItemModel::rowsMoved);
        watchStructure(&QAbstractItemModel::columnsAboutToBeInserted,
                       &QAbstractItemModel::columnsInserted);
        watchStructure(&QAbstractItemModel::columnsAboutToBeRemoved,
                       &QAbstractItemModel::columnsRemoved);
        watchStructure(&QAbstractItemModel::columnsAboutToBeMoved,
                       &QAbstractItemModel::columnsMoved);
        m_connections << connect(
            source, &QAbstractItemModel::layoutAboutToBeChanged, this, before);
        // Some application models issue layoutChanged alone when refreshing.
        m_connections << connect(source, &QAbstractItemModel::layoutChanged,
                                 this, after);
        m_connections << connect(
            source, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex &first, const QModelIndex &last,
                   const QList<int> &roles) {
                if (rowCount() && columnCount()) {
                    Q_EMIT dataChanged(index(0, first.column()),
                                       index(rowCount() - 1, last.column()),
                                       roles);
                }
                const int sortRole = m_roles.value(m_column, Qt::DisplayRole);
                if (first.column() <= m_column && last.column() >= m_column &&
                    (roles.isEmpty() || roles.contains(sortRole))) {
                    invalidate();
                }
            });
        m_connections << connect(
            source, &QAbstractItemModel::headerDataChanged, this,
            [this](Qt::Orientation o, int first, int last) {
                Q_EMIT headerDataChanged(o, o == Qt::Vertical ? 0 : first,
                                         o == Qt::Vertical ? rowCount() - 1
                                                           : last);
            });
        m_connections << connect(source, &QObject::destroyed, this, [this] {
            ++*m_generation;
            m_collect.stop();
            m_pending = false;
            beginResetModel();
            m_rows.clear();
            m_inverse.clear();
            endResetModel();
            setBusy(false);
        });
    }
    invalidate();
}

void AsyncTableProxyModel::resetRows() {
    m_rows.resize(sourceModel() ? sourceModel()->rowCount() : 0);
    std::iota(m_rows.begin(), m_rows.end(), 0);
    m_inverse = m_rows;
}

QModelIndex AsyncTableProxyModel::mapToSource(const QModelIndex &i) const {
    if (!sourceModel() || i.model() != this || i.row() < 0 ||
        i.row() >= m_rows.size()) {
        return {};
    }
    return sourceModel()->index(m_rows.at(i.row()), i.column());
}

QModelIndex AsyncTableProxyModel::mapFromSource(const QModelIndex &i) const {
    if (!i.isValid() || i.model() != sourceModel() || i.parent().isValid() ||
        i.row() >= m_inverse.size()) {
        return {};
    }
    return index(m_inverse.at(i.row()), i.column());
}

QModelIndex AsyncTableProxyModel::index(int r, int c,
                                        const QModelIndex &p) const {
    return !p.isValid() && r >= 0 && r < rowCount() && c >= 0 &&
                   c < columnCount()
               ? createIndex(r, c)
               : QModelIndex();
}

QModelIndex AsyncTableProxyModel::parent(const QModelIndex &) const {
    return {};
}

int AsyncTableProxyModel::rowCount(const QModelIndex &p) const {
    return p.isValid() ? 0 : m_rows.size();
}

int AsyncTableProxyModel::columnCount(const QModelIndex &p) const {
    return !p.isValid() && sourceModel() ? sourceModel()->columnCount() : 0;
}

QVariant AsyncTableProxyModel::headerData(int section, Qt::Orientation o,
                                          int role) const {
    if (!sourceModel()) {
        return {};
    }
    if (o == Qt::Horizontal && role == Qt::ToolTipRole &&
        !isColumnSortable(section)) {
        return tr("Sorting is disabled for this column.");
    }
    if (o == Qt::Vertical) {
        if (section < 0 || section >= m_rows.size()) {
            return {};
        }
        section = m_rows.at(section);
    }
    return sourceModel()->headerData(section, o, role);
}

bool AsyncTableProxyModel::setHeaderData(int section, Qt::Orientation o,
                                         const QVariant &value, int role) {
    if (!sourceModel()) {
        return false;
    }
    if (o == Qt::Vertical) {
        if (section < 0 || section >= m_rows.size()) {
            return false;
        }
        section = m_rows.at(section);
    }
    return sourceModel()->setHeaderData(section, o, value, role);
}

void AsyncTableProxyModel::setBusy(bool busy) {
    if (m_busy != busy) {
        m_busy = busy;
        Q_EMIT sortingChanged(busy);
    }
}

bool AsyncTableProxyModel::isSorting() const { return m_busy; }

bool AsyncTableProxyModel::isColumnSortable(int c) const {
    return m_sortable.value(c, true);
}

void AsyncTableProxyModel::setColumnSortable(int c, bool enabled) {
    m_sortable[c] = enabled;
    if (!enabled && c == m_column) {
        sort(-1);
    }
    Q_EMIT headerDataChanged(Qt::Horizontal, c, c);
}

void AsyncTableProxyModel::setColumnSortRole(int c, int role) {
    m_roles[c] = role;
    if (c == m_column) {
        invalidate();
    }
}

void AsyncTableProxyModel::sort(int c, Qt::SortOrder order) {
    if (c >= 0 && !isColumnSortable(c)) {
        return;
    }
    m_column = c;
    m_order = order;
    invalidate();
    if (c < 0) {
        QList<int> rows(rowCount());
        std::iota(rows.begin(), rows.end(), 0);
        publish(rows, rows);
        Q_EMIT sortCleared();
    }
}

void AsyncTableProxyModel::invalidate() {
    ++*m_generation;
    m_collect.stop();
    m_keys.clear();
    m_pending = sourceModel() && m_column >= 0 && m_column < columnCount();
    setBusy(m_pending);
    if (m_pending && !m_workerActive) {
        m_collect.start(0);
    }
}

void AsyncTableProxyModel::collectKeys() {
    if (!m_pending || !sourceModel() || m_resetting) {
        return;
    }
    if (m_keys.isEmpty()) {
        m_collectGeneration = m_generation->load();
        m_snapshotClock.start();
        m_keys.reserve(sourceModel()->rowCount());
    }
    QElapsedTimer budget;
    budget.start();
    const int total = sourceModel()->rowCount();
    const int role = m_roles.value(m_column, Qt::DisplayRole);
    while (m_keys.size() < total) {
        const int row = m_keys.size();
        auto value =
            sourceModel()->data(sourceModel()->index(row, m_column), role);
        // Freeze custom display conversions here too: a registered converter
        // can depend on GUI objects even though QVariant itself is copyable.
        if (keyKind(value) == KeyKind::Text) {
            value = value.toString();
        }
        m_keys.append(std::move(value));
        if (budget.elapsed() >= 4) {
            m_collect.start(0);
            return;
        }
    }
    m_snapshotMs = m_snapshotClock.elapsed();
    m_pending = false;
    auto keys = std::move(m_keys);
    m_keys.clear();
    const auto generation = m_collectGeneration;
    const auto token = m_generation;
    const auto order = m_order;
    m_workerActive = true;
    m_worker.setFuture(QtConcurrent::run(
        [keys = std::move(keys), generation, token, order]() mutable {
            QElapsedTimer workerClock;
            workerClock.start();
            Result result;
            result.generation = generation;
            result.rows = sortedRows(keys, order, *token, generation);
            result.inverse.resize(result.rows.size());
            for (int row = 0; row < result.rows.size(); ++row) {
                result.inverse[result.rows.at(row)] = row;
            }
            result.workerMs = workerClock.elapsed();
            return result;
        }));
}

void AsyncTableProxyModel::publish(const QList<int> &rows,
                                   const QList<int> &inverse) {
    if (rows.size() != m_rows.size()) {
        return;
    }
    Q_EMIT layoutAboutToBeChanged({}, QAbstractItemModel::VerticalSortHint);
    const auto oldIndexes = persistentIndexList();
    QModelIndexList sources;
    sources.reserve(oldIndexes.size());
    for (const auto &i : oldIndexes) {
        sources.append(mapToSource(i));
    }
    m_rows = rows;
    m_inverse = inverse;
    QModelIndexList newIndexes;
    newIndexes.reserve(sources.size());
    for (const auto &i : sources) {
        newIndexes.append(mapFromSource(i));
    }
    changePersistentIndexList(oldIndexes, newIndexes);
    Q_EMIT layoutChanged({}, QAbstractItemModel::VerticalSortHint);
}
