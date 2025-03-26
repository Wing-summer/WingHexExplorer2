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

#include "class/asdatabase.h"

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

    ASDataBase db(engine);
    auto &nodes = db.headerNodes();
    for (auto node = nodes.constKeyValueBegin();
         node != nodes.constKeyValueEnd(); node++) {
        QString header = node->first;

        QTreeWidgetItem *item = nullptr;

        if (header.isEmpty()) {
            item = new QTreeWidgetItem({tr("[Global]")});
        } else {
            item = new QTreeWidgetItem({header});
        }

        item->setToolTip(0, header);
        item->setIcon(0, CodeInfoTip::getDisplayIcon(CodeInfoTip::Type::Group));
        createObjNodes(node->second, item);
        addTopLevelItem(item);
    }

    setSortingEnabled(true);
    resizeColumnToContents(0);
}

QTreeWidgetItem *ASObjTreeWidget::createObjNode(const CodeInfoTip &node,
                                                QTreeWidgetItem *parent) {
    Q_ASSERT(node.type != CodeInfoTip::Type::Unknown && parent);
    QStringList contents{node.name, node.getTooltip()};
    auto c = new QTreeWidgetItem(contents);
    c->setToolTip(0, contents.at(0));
    c->setToolTip(1, contents.at(1));
    c->setIcon(0, CodeInfoTip::getDisplayIcon(node.type));
    parent->addChild(c);
    return c;
}

void ASObjTreeWidget::createObjNodes(const QList<CodeInfoTip> &nodes,
                                     QTreeWidgetItem *parent) {

    for (auto &n : nodes) {
        // only for code namespace completion
        auto c = createObjNode(n, parent);
        if (!n.children.isEmpty()) {
            createObjNodes(n.children, c);
        }
    }
}
