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

#ifndef TABLECOLUMNWIDTHCONTROLLER_H
#define TABLECOLUMNWIDTHCONTROLLER_H

#include "qtableviewext.h"

class QTreeView;
class QHeaderView;

// Owns measurement, allocation and per-source width state. The view only
// forwards model/style/geometry changes and keeps its public configuration API.
class TableColumnWidthController : public QObject {
public:
    explicit TableColumnWidthController(QTableViewExt *view);
    explicit TableColumnWidthController(QTreeView *view);
    using ColumnWidthPolicy = QTableViewExt::ColumnWidthPolicy;
    using WidthSource = QTableViewExt::WidthSource;
    using WidthUpdate = QTableViewExt::WidthUpdate;
    using WidthUnit = QTableViewExt::WidthUnit;

public:
    void setSourceModel(QAbstractItemModel *source);
    void saveState();
    void watchDelegate(QAbstractItemDelegate *delegate);
    bool setPolicy(int column, const ColumnWidthPolicy &policy);
    std::optional<ColumnWidthPolicy> policy(int column) const;
    void clearPolicy(int column);
    void reset(int column);
    void schedule(bool contentsChanged = false);
    void synchronizeEmptyLayout();
    void invalidateContents(int firstColumn, int lastColumn);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    TableColumnWidthController(QAbstractItemView *view, QHeaderView *header,
                               QTableViewExt *table);
    void measure();
    void apply();

private:
    struct State {
        QHash<int, int> widths;
        QSet<int> initialized;
        QSet<int> manual;
        QHash<int, int> measured;
        void clear(int column);
    };
    struct Measurement {
        int column;
        int sample = 0;
        int width = 0;
    };
    QAbstractItemView *m_view;
    QTableViewExt *m_table;
    QPointer<QHeaderView> m_header;
    QList<QMetaObject::Connection> m_modelConnections;
    QPointer<QAbstractItemModel> m_source;
    QHash<QAbstractItemModel *, State> m_states;
    QHash<int, ColumnWidthPolicy> m_widthPolicies;
    QTimer m_widthTimer;
    QSet<QAbstractItemDelegate *> m_delegates;
    QList<Measurement> m_measurements;
    bool m_applyingWidths = false;
    bool m_headerPressed = false;
};
#endif
