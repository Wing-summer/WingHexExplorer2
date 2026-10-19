/*==============================================================================
 ** Copyright (C) 2026-2029 WingSummer
 **
 ** This program is free software: you can redistribute it and/or modify it
 ** under the terms of the GNU Affero General Public License as published by the
 ** Free Software Foundation, version 3.
 **
 ** This program is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 ** for more details.
 **
 ** You should have received a copy of the GNU Affero General Public License
 ** along with this program. If not, see <https://www.gnu.org/licenses/>.
 ** =============================================================================
 */
#include "objtreewidget.h"

#include "control/editor/codeinfotip.h"
#include "utilities.h"

#include <QHeaderView>

ObjTreeWidget::ObjTreeWidget(QWidget *parent) : QTreeWidget(parent) {
    Utilities::applyTreeViewProperty(this);
    setColumnCount(2);
    header()->setSortIndicator(0, Qt::AscendingOrder);
    setHeaderLabels({tr("Symbol"), tr("Declaration")});
    connect(&LuauLanguageServer::instance(), &LuauLanguageServer::initialized,
            this, &ObjTreeWidget::refresh);
    refresh();
}

void ObjTreeWidget::refresh() {
    const auto symbols = LuauLanguageServer::instance().globalSymbols();
    setSortingEnabled(false);
    clear();
    createObjNodes(symbols, invisibleRootItem());
    setSortingEnabled(true);
    resizeColumnToContents(0);
}

void ObjTreeWidget::createObjNodes(
    const QList<LuauLanguageServer::Symbol> &nodes, QTreeWidgetItem *parent) {
    for (const auto &node : nodes) {
        const auto &symbol = node.item;
        const auto detail = symbol.detail.value_or(QString());
        auto item = new QTreeWidgetItem(parent, {symbol.label, detail});
        const auto declaration =
            node.fullDeclaration.isEmpty() ? detail : node.fullDeclaration;
        const auto tooltip =
            symbol.documentation.value.isEmpty()
                ? declaration
                : declaration + '\n' + symbol.documentation.value;
        item->setToolTip(0, tooltip);
        item->setToolTip(1, tooltip);
        item->setIcon(0, CodeInfoTip::getDisplayIcon(symbol.kind.value_or(
                             lsp::CompletionItemKind::Missing)));
        if (symbol.deprecated) {
            auto font = item->font(0);
            font.setStrikeOut(true);
            item->setFont(0, font);
        }
        createObjNodes(node.children, item);
    }
}

void ObjTreeWidget::focusOutEvent(QFocusEvent *event) {
    clearSelection();
    QTreeWidget::focusOutEvent(event);
}
