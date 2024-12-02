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

#ifndef ASOBJTREEWIDGET_H
#define ASOBJTREEWIDGET_H

#include "angelscript.h"
#include "class/qcodenode.h"

#include <QHash>
#include <QTreeWidget>
#include <QTreeWidgetItem>

class ASObjTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit ASObjTreeWidget(QWidget *parent = nullptr);

    void setEngine(asIScriptEngine *engine);

private:
    QTreeWidgetItem *createObjNode(QCodeNode *node, QTreeWidgetItem *parent);

    void createObjNodes(const QList<QCodeNode *> &children,
                        QTreeWidgetItem *parent);
};

#endif // ASOBJTREEWIDGET_H
