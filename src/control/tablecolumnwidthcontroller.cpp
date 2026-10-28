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

#include "tablecolumnwidthcontroller.h"

#include <QAbstractItemDelegate>
#include <QElapsedTimer>
#include <QHeaderView>
#include <QMouseEvent>
#include <QStyleOptionViewItem>
#include <QTreeView>
#include <algorithm>
#include <cmath>
#include <limits>

namespace {
using WidthPolicy = QTableViewExt::ColumnWidthPolicy;

struct WidthBounds {
    int minimum;
    int maximum;
};
struct WeightedColumn {
    int column;
    WidthBounds bounds;
    qreal weight;
};

WidthBounds widthBounds(const WidthPolicy &policy, int available,
                        const QHeaderView *header) {
    int minimum = std::max(header->minimumSectionSize(),
                           policy.minimumPixels.value_or(0));
    if (policy.minimumRatio) {
        minimum =
            std::max(minimum, int(std::ceil(available * *policy.minimumRatio)));
    }
    // Qt's absolute section-size limit cannot be exceeded by any policy.
    minimum = std::min(minimum, header->maximumSectionSize());
    int maximum =
        std::min(header->maximumSectionSize(),
                 policy.maximumPixels.value_or(header->maximumSectionSize()));
    if (policy.maximumRatio) {
        maximum = std::min(maximum,
                           int(std::floor(available * *policy.maximumRatio)));
    }
    return {minimum, std::max(minimum, maximum)};
}

void allocateWeights(const QList<WeightedColumn> &columns, qreal available,
                     QHash<int, int> &widths) {
    if (columns.isEmpty()) {
        return;
    }
    if (columns.size() == 1) {
        const auto &column = columns.first();
        widths[column.column] =
            int(std::clamp(available, qreal(column.bounds.minimum),
                           qreal(column.bounds.maximum)));
        return;
    }
    // Find one common scale satisfying sum(clamp(scale * weight)) = space.
    // A logarithmic scale keeps all finite positive weights valid, even when
    // direct maximum/weight division or summing weights would overflow.
    QList<qreal> logWeights;
    logWeights.reserve(columns.size());
    qreal low = std::numeric_limits<qreal>::max();
    qreal high = std::numeric_limits<qreal>::lowest();
    for (const auto &column : columns) {
        const qreal logWeight = std::log(column.weight);
        logWeights.append(logWeight);
        low =
            std::min(low, std::log(qreal(std::max(1, column.bounds.minimum))) -
                              logWeight - 40);
        high =
            std::max(high, std::log(qreal(std::max(1, column.bounds.maximum))) -
                               logWeight);
    }
    const auto scaledWidth = [&](int index, qreal logScale) {
        const auto bounds = columns.at(index).bounds;
        const qreal exponent = logScale + logWeights.at(index);
        if (exponent >= std::log(qreal(std::max(1, bounds.maximum)))) {
            return qreal(bounds.maximum);
        }
        return std::clamp(std::exp(exponent), qreal(bounds.minimum),
                          qreal(bounds.maximum));
    };
    for (int iteration = 0; iteration < 64; ++iteration) {
        const qreal middle = low + (high - low) / 2;
        qreal total = 0;
        for (int i = 0; i < columns.size(); ++i) {
            total += scaledWidth(i, middle);
        }
        if (total < available) {
            low = middle;
        } else {
            high = middle;
        }
    }
    qreal allocated = 0;
    for (int i = 0; i < columns.size(); ++i) {
        const int width = int(scaledWidth(i, high));
        widths[columns.at(i).column] = width;
        allocated += width;
    }
    // Deterministic rounding: equal weights differ by at most one logical
    // pixel.
    qint64 remainder = qint64(std::max(qreal(0), available - allocated));
    for (const auto &column : columns) {
        if (remainder > 0 && widths[column.column] < column.bounds.maximum) {
            ++widths[column.column];
            --remainder;
        }
    }
}
} // namespace

TableColumnWidthController::TableColumnWidthController(QTableViewExt *view)
    : TableColumnWidthController(view, view->horizontalHeader(), view) {}

TableColumnWidthController::TableColumnWidthController(QTreeView *view)
    : TableColumnWidthController(view, view->header(), nullptr) {
    setSourceModel(view->model());
}

TableColumnWidthController::TableColumnWidthController(QAbstractItemView *view,
                                                       QHeaderView *header,
                                                       QTableViewExt *table)
    : QObject(view), m_view(view), m_table(table), m_header(header) {
    watchDelegate(view->itemDelegate());
    m_widthTimer.setSingleShot(true);
    connect(&m_widthTimer, &QTimer::timeout, this,
            &TableColumnWidthController::measure);
    header->viewport()->installEventFilter(this);
    if (!m_table) {
        view->installEventFilter(this);
        view->viewport()->installEventFilter(this);
    }
    connect(header, &QHeaderView::sectionResized, this,
            [this](int column, int, int width) {
                if (m_applyingWidths || !m_source) {
                    return;
                }
                if (m_headerPressed && m_widthPolicies.contains(column)) {
                    auto &state = m_states[m_source];
                    state.manual.insert(column);
                    state.widths[column] = width;
                    m_measurements.clear();
                }
                // Unconfigured columns also affect the space available to
                // weights.
                schedule();
            });
    connect(header, &QHeaderView::geometriesChanged, this,
            [this] { schedule(); });
}

void TableColumnWidthController::watchDelegate(
    QAbstractItemDelegate *delegate) {
    if (!delegate || m_delegates.contains(delegate)) {
        return;
    }
    m_delegates.insert(delegate);
    connect(delegate, &QAbstractItemDelegate::sizeHintChanged, this,
            [this](const QModelIndex &index) {
                if (index.isValid()) {
                    invalidateContents(index.column(), index.column());
                } else {
                    schedule(true);
                }
            });
    connect(delegate, &QObject::destroyed, this, [this, delegate] {
        m_delegates.remove(delegate);
        schedule(true);
    });
}

void TableColumnWidthController::State::clear(int column) {
    initialized.remove(column);
    manual.remove(column);
    measured.remove(column);
    widths.remove(column);
}

void TableColumnWidthController::saveState() {
    if (!m_source) {
        return;
    }
    auto &state = m_states[m_source];
    for (auto it = m_widthPolicies.cbegin(); it != m_widthPolicies.cend();
         ++it) {
        if (state.initialized.contains(it.key()) ||
            state.manual.contains(it.key())) {
            state.widths[it.key()] = m_header->sectionSize(it.key());
        }
    }
}

void TableColumnWidthController::setSourceModel(QAbstractItemModel *source) {
    for (const auto &connection : std::as_const(m_modelConnections)) {
        disconnect(connection);
    }
    m_modelConnections.clear();
    m_source = source;
    m_measurements.clear();
    if (!source) {
        return;
    }
    if (!m_states.contains(source)) {
        m_states.insert(source, {});
        connect(source, &QObject::destroyed, this,
                [this, source] { m_states.remove(source); });
    }
    if (!m_table) {
        const auto changed = [this] { schedule(true); };
        m_modelConnections << connect(source, &QAbstractItemModel::modelReset,
                                      this, changed);
        m_modelConnections << connect(
            source, &QAbstractItemModel::layoutChanged, this, changed);
        m_modelConnections << connect(source, &QAbstractItemModel::rowsInserted,
                                      this, changed);
        m_modelConnections << connect(source, &QAbstractItemModel::rowsRemoved,
                                      this, changed);
        m_modelConnections << connect(
            source, &QAbstractItemModel::columnsInserted, this, changed);
        m_modelConnections << connect(
            source, &QAbstractItemModel::columnsRemoved, this, changed);
        m_modelConnections << connect(
            source, &QAbstractItemModel::headerDataChanged, this, changed);
        m_modelConnections << connect(
            source, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex &first, const QModelIndex &last) {
                invalidateContents(first.column(), last.column());
            });
    }
    auto header = m_header;
    for (auto it = m_widthPolicies.cbegin(); it != m_widthPolicies.cend();
         ++it) {
        if (it->source == WidthSource::Weight ||
            it.key() == header->count() - 1) {
            header->setStretchLastSection(false);
        }
    }
    m_applyingWidths = true;
    const auto &state = m_states[source];
    for (auto it = state.widths.cbegin(); it != state.widths.cend(); ++it) {
        if (it.key() < m_header->count()) {
            header->setSectionResizeMode(it.key(), QHeaderView::Interactive);
            m_header->resizeSection(it.key(), it.value());
        }
    }
    m_applyingWidths = false;
    schedule(true);
}

void TableColumnWidthController::invalidateContents(int first, int last) {
    if (!m_source) {
        return;
    }
    auto &state = m_states[m_source];
    for (auto it = m_widthPolicies.cbegin(); it != m_widthPolicies.cend();
         ++it) {
        if (it.key() >= first && it.key() <= last &&
            (it->update == WidthUpdate::Continuous ||
             !state.initialized.contains(it.key()))) {
            state.measured.remove(it.key());
        }
    }
    m_measurements.clear();
    schedule();
}

bool TableColumnWidthController::setPolicy(int column,
                                           const ColumnWidthPolicy &p) {
    const auto validRatio = [](const std::optional<qreal> &r) {
        return !r || (std::isfinite(*r) && *r >= 0 && *r <= 1);
    };
    if (column < 0 || !std::isfinite(p.value) || p.value < 0 ||
        (p.unit == WidthUnit::ViewportRatio &&
         p.source == WidthSource::Specified && p.value > 1) ||
        !std::isfinite(p.weight) || p.weight <= 0 ||
        (p.minimumPixels && *p.minimumPixels < 0) ||
        (p.maximumPixels && *p.maximumPixels < 0) ||
        !validRatio(p.minimumRatio) || !validRatio(p.maximumRatio)) {
        return false;
    }
    m_widthPolicies[column] = p;
    for (auto it = m_states.begin(); it != m_states.end(); ++it) {
        it->clear(column);
    }
    if (column < m_header->count()) {
        m_header->setSectionResizeMode(column, QHeaderView::Interactive);
    }
    if (p.source == WidthSource::Weight || column == m_header->count() - 1) {
        m_header->setStretchLastSection(false);
    }
    schedule(true);
    return true;
}

std::optional<QTableViewExt::ColumnWidthPolicy>
TableColumnWidthController::policy(int c) const {
    const auto it = m_widthPolicies.constFind(c);
    return it == m_widthPolicies.cend() ? std::nullopt
                                        : std::optional<ColumnWidthPolicy>(*it);
}

void TableColumnWidthController::clearPolicy(int c) {
    m_widthPolicies.remove(c);
    for (auto it = m_states.begin(); it != m_states.end(); ++it) {
        it->clear(c);
    }
    schedule(true);
}

void TableColumnWidthController::reset(int c) {
    if (!m_source) {
        return;
    }
    auto &state = m_states[m_source];
    if (c < 0) {
        state.initialized.clear();
        state.manual.clear();
        state.measured.clear();
        state.widths.clear();
    } else {
        state.clear(c);
    }
    schedule(true);
}

void TableColumnWidthController::schedule(bool contentsChanged) {
    if (m_applyingWidths || m_widthPolicies.isEmpty()) {
        return;
    }
    if (contentsChanged && m_source) {
        auto &state = m_states[m_source];
        for (auto it = m_widthPolicies.cbegin(); it != m_widthPolicies.cend();
             ++it) {
            if ((it->update == WidthUpdate::Continuous ||
                 !state.initialized.contains(it.key()))) {
                state.measured.remove(it.key());
            }
        }
        m_measurements.clear();
    }
    if (!m_widthTimer.isActive()) {
        m_widthTimer.start(0);
    }
}

void TableColumnWidthController::measure() {
    if (!m_table && m_source != m_view->model()) {
        setSourceModel(m_view->model());
        return;
    }
    if (!m_header || !m_source || !m_view->isVisible() ||
        m_view->viewport()->width() <= 0) {
        return;
    }
    if (m_source->rowCount() == 0) {
        // Empty tables still need a layout, but have no contents to sample.
        m_measurements.clear();
        apply();
        return;
    }
    auto &state = m_states[m_source];
    if (m_measurements.isEmpty()) {
        for (auto it = m_widthPolicies.cbegin(); it != m_widthPolicies.cend();
             ++it) {
            const int c = it.key();
            if (c >= m_view->model()->columnCount() ||
                m_header->isSectionHidden(c) || state.manual.contains(c) ||
                (it->update == WidthUpdate::Initial &&
                 state.initialized.contains(c))) {
                continue;
            }
            if (it->source == WidthSource::Contents &&
                !state.measured.contains(c)) {
                m_measurements.append({c, 0, m_header->sectionSizeHint(c)});
            }
        }
    }
    QElapsedTimer budget;
    budget.start();
    const int rows = m_view->model()->rowCount();
    const int samples = std::min(1000, rows);
    QStyleOptionViewItem option;
    if (m_table) {
        m_table->initViewItemOption(&option);
    } else {
        option.initFrom(m_view);
        option.widget = m_view;
        option.decorationSize = m_view->iconSize();
    }
    // Request intrinsic width, including delegates backed by QTextDocument.
    // A zero-width rect would wrap rich text and underestimate its width.
    option.rect = QRect(0, 0, -1, -1);
    while (!m_measurements.isEmpty()) {
        auto &work = m_measurements.first();
        while (work.sample < samples) {
            const int row =
                samples <= 1 ? 0
                             : qint64(work.sample) * (rows - 1) / (samples - 1);
            const auto idx =
                m_table
                    ? m_table->mapFromSource(m_source->index(row, work.column))
                    : m_source->index(row, work.column);
            auto delegate = m_view->itemDelegateForIndex(idx);
            if (delegate) {
                work.width = std::max(
                    work.width, delegate->sizeHint(option, idx).width() + 2);
            }
            ++work.sample;
            if (budget.elapsed() >= 4) {
                m_widthTimer.start(0);
                return;
            }
        }
        state.measured[work.column] = work.width;
        m_measurements.removeFirst();
    }
    apply();
}

void TableColumnWidthController::synchronizeEmptyLayout() {
    // Empty views need no delegate measurement. Settle their provisional widths
    // before Qt computes scrollbar ranges, rather than one timer turn later.
    if (!m_applyingWidths && m_source && m_source->rowCount() == 0 &&
        !m_widthPolicies.isEmpty()) {
        apply();
    }
}

void TableColumnWidthController::apply() {
    if (!m_header || !m_source) {
        return;
    }
    auto &state = m_states[m_source];
    const int available = m_view->viewport()->width();
    const bool hasRows = m_source->rowCount() > 0;
    QList<WeightedColumn> weighted;
    QHash<int, int> widths;
    qreal remaining = available;
    for (int c = 0; c < m_view->model()->columnCount(); ++c) {
        if (m_header->isSectionHidden(c)) {
            continue;
        }
        const auto it = m_widthPolicies.constFind(c);
        if (it == m_widthPolicies.cend() || state.manual.contains(c) ||
            (it->update == WidthUpdate::Initial &&
             (state.initialized.contains(c) ||
              (!hasRows && it->source != WidthSource::Contents)))) {
            remaining -= m_header->sectionSize(c);
            continue;
        }
        const auto bounds = widthBounds(*it, available, m_header);
        if (it->source == WidthSource::Weight) {
            weighted.append({c, bounds, it->weight});
            continue;
        }
        const qreal requested =
            it->source == WidthSource::Contents
                ? (hasRows ? state.measured.value(c, bounds.minimum)
                           : m_header->sectionSizeHint(c))
                : (it->unit == WidthUnit::Pixels ? it->value
                                                 : it->value * available);
        const int width = int(std::clamp(requested, qreal(bounds.minimum),
                                         qreal(bounds.maximum)));
        widths[c] = width;
        remaining -= width;
    }
    allocateWeights(weighted, remaining, widths);
    m_applyingWidths = true;
    for (auto it = widths.cbegin(); it != widths.cend(); ++it) {
        m_header->setSectionResizeMode(it.key(), QHeaderView::Interactive);
        m_header->resizeSection(it.key(), it.value());
        state.widths[it.key()] = m_header->sectionSize(it.key());
        // Header-only estimates must not consume initial content sizing.
        if (hasRows) {
            state.initialized.insert(it.key());
        }
    }
    m_applyingWidths = false;
    if (m_table) {
        Q_EMIT m_table->columnWidthsAdjusted();
    }
}

bool TableColumnWidthController::eventFilter(QObject *watched, QEvent *event) {
    if (!m_header) {
        return false;
    }
    if (!m_table && (watched == m_view || watched == m_view->viewport())) {
        switch (event->type()) {
        case QEvent::Show:
        case QEvent::Resize:
            schedule();
            break;
        case QEvent::StyleChange:
        case QEvent::FontChange:
            schedule(true);
            break;
        default:
            break;
        }
    }
    if (watched == m_header->viewport()) {
        auto e = event->type();
        if (e == QEvent::MouseButtonPress || e == QEvent::MouseButtonDblClick) {
            m_headerPressed =
                static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton;
        } else if (e == QEvent::MouseButtonRelease) {
            m_headerPressed = false;
        }
    }
    return QObject::eventFilter(watched, event);
}
