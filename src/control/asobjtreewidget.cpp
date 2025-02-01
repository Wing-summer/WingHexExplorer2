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

#include <QHeaderView>

ASObjTreeWidget::ASObjTreeWidget(QWidget *parent) : QTreeWidget(parent) {
    setColumnCount(2);
    auto header = this->header();
    header->setDefaultAlignment(Qt::AlignCenter);
    header->setSortIndicator(0, Qt::SortOrder::AscendingOrder);
    setHeaderLabels({tr("Symbol"), tr("Decl")});
}

void ASObjTreeWidget::setEngine(asIScriptEngine *engine) {
    Q_ASSERT(engine);
    if (engine == nullptr) {
        return;
    }

    QAsParser parser(engine);
    auto &nodes = parser.headerNodes();
    for (auto &node : nodes) {
        auto header = QString::fromUtf8(node->role(QCodeNode::Name));

        QTreeWidgetItem *item = nullptr;

        if (header.isEmpty()) {
            item = new QTreeWidgetItem({tr("[Global]")});
        } else {
            item = new QTreeWidgetItem({header});
        }

        item->setToolTip(0, node->qualifiedBaseName(true));
        item->setIcon(0, node->data(Qt::DecorationRole).value<QIcon>());

        createObjNodes(node->children(), item);
        addTopLevelItem(item);
    }

    setSortingEnabled(true);
}

QTreeWidgetItem *ASObjTreeWidget::createObjNode(QCodeNode *node,
                                                QTreeWidgetItem *parent) {
    Q_ASSERT(node && parent);
    QStringList contents{QString::fromUtf8(node->role(QCodeNode::Name)),
                         node->data(Qt::ToolTipRole).toString()};
    auto c = new QTreeWidgetItem(contents);
    c->setToolTip(0, contents.at(0));
    c->setToolTip(1, contents.at(1));
    c->setIcon(0, node->data(Qt::DecorationRole).value<QIcon>());
    parent->addChild(c);
    return c;
}

void ASObjTreeWidget::createObjNodes(const QList<QCodeNode *> &children,
                                     QTreeWidgetItem *parent) {
    for (auto &n : children) {
        // only for code namespace completion
        if (n->role(QCodeNode::NodeType).at(0) == QCodeNode::Namespace) {
            if (n->children().isEmpty()) {
                continue;
            }
        }
        auto c = createObjNode(n, parent);
        if (!n->children().isEmpty()) {
            createObjNodes(n->children(), c);
        }
    }
}
