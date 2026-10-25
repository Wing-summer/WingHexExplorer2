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

#include <QTreeWidget>

class ObjTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit ObjTreeWidget(QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    void populateChildren(QTreeWidgetItem *item);
    QHash<QString, LuauLanguageServer::Symbol> _symbols;
    QSet<QString> _loaded;
    QSet<QString> _restoreExpanded;
    QString _restoreSelected;

    void createObjNodes(const QList<LuauLanguageServer::Symbol> &nodes,
                        QTreeWidgetItem *parent);

protected:
    void focusOutEvent(QFocusEvent *event) override;
};

#endif // OBJTREEWIDGET_H
