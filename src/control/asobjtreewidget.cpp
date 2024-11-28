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

#include "asobjtreewidget.h"

#include "class/qasparser.h"
#include "class/qcodenode.h"

#include <QHeaderView>
#include <QTreeWidgetItem>

ASObjTreeWidget::ASObjTreeWidget(QWidget *parent) : QTreeWidget(parent) {
    setColumnCount(2);
    header()->setDefaultAlignment(Qt::AlignCenter);
    setHeaderLabels({tr("Symbol"), tr("Decl")});
}

void ASObjTreeWidget::setEngine(asIScriptEngine *engine) {
    Q_ASSERT(engine);
    if (engine == nullptr) {
        return;
    }

    QAsParser parser(engine);
    auto nodes = parser.headerNodes() + parser.classNodes();
    for (auto &node : nodes) {
        if (node->type() == QCodeNode::Group) {
            continue;
        }

        auto header = QString::fromUtf8(node->role(QCodeNode::Name));
        auto item = new QTreeWidgetItem({header});
        item->setData(0, Qt::UserRole, node->qualifiedBaseName(true));
        item->setToolTip(0, header);
        item->setIcon(0, node->data(Qt::DecorationRole).value<QIcon>());

        for (auto &n : node->children()) {
            QStringList contents{QString::fromUtf8(n->role(QCodeNode::Name)),
                                 n->data(Qt::ToolTipRole).toString()};
            auto c = new QTreeWidgetItem(contents);
            c->setData(0, Qt::UserRole, n->qualifiedBaseName(true));
            c->setToolTip(0, contents.at(0));
            c->setToolTip(1, contents.at(1));
            c->setIcon(0, n->data(Qt::DecorationRole).value<QIcon>());
            item->addChild(c);
        }

        addTopLevelItem(item);
    }
}
