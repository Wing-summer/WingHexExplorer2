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

#ifndef OBJTREEWIDGET_H
#define OBJTREEWIDGET_H

#include "luau/lsp/luaulanguageserver.h"

#include "searchabletreewidget.h"

class ObjTreeWidget : public SearchableTreeWidget {
    Q_OBJECT
public:
    explicit ObjTreeWidget(QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    struct SymbolCache {
        QHash<QString, LuauLanguageServer::Symbol> entries;
        QSet<QString> loaded;
    } _symbols;
    struct RestoreState {
        QSet<QString> expanded;
        QString selected;
    } _restore;

private:
    void populateChildren(QTreeWidgetItem *item);
    void createObjNodes(const QList<LuauLanguageServer::Symbol> &nodes,
                        QTreeWidgetItem *parent);

protected:
    void prepareSearchItem(QTreeWidgetItem *item) override;
    bool isSearchableItem(const QTreeWidgetItem *item) const override;
};

#endif // OBJTREEWIDGET_H
