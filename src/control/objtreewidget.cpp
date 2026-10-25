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

#include <QCryptographicHash>
#include <QHeaderView>
#include <QTreeWidgetItemIterator>

ObjTreeWidget::ObjTreeWidget(QWidget *parent) : QTreeWidget(parent) {
    Utilities::applyTreeViewProperty(this);
    setColumnCount(2);
    header()->setSortIndicator(0, Qt::AscendingOrder);
    setHeaderLabels({tr("Symbol"), tr("Declaration")});
    connect(&LuauLanguageServer::instance(), &LuauLanguageServer::initialized,
            this, &ObjTreeWidget::refresh);
    connect(this, &QTreeWidget::itemExpanded, this,
            &ObjTreeWidget::populateChildren);
    refresh();
}

void ObjTreeWidget::refresh() {
    _restoreExpanded.clear();
    _restoreSelected = currentItem()
                           ? currentItem()->data(0, Qt::UserRole).toString()
                           : QString();
    for (QTreeWidgetItemIterator it(this); *it; ++it) {
        if ((*it)->isExpanded()) {
            _restoreExpanded.insert((*it)->data(0, Qt::UserRole).toString());
        }
    }
    const auto symbols = LuauLanguageServer::instance().builtinSymbols();
    setSortingEnabled(false);
    clear();
    _symbols.clear();
    _loaded.clear();
    using Category = LuauLanguageServer::SymbolCategory;
    struct CategoryEntry {
        Category kind;
        QString id;
        QString title;
    };
    const QList<CategoryEntry> categories = {
        {Category::Standard, QStringLiteral("standard"),
         tr("Standard library")},
        {Category::Document, QStringLiteral("document"), tr("Document access")},
        {Category::Logging, QStringLiteral("logging"),
         tr("Logging and output")},
        {Category::Values, QStringLiteral("values"), tr("Value types")},
        {Category::Data, QStringLiteral("data"), tr("Data processing")},
        {Category::Services, QStringLiteral("services"), tr("Host services")},
        {Category::Qt, QStringLiteral("qt"), tr("Qt")},
        {Category::Ffi, QStringLiteral("ffi"), tr("FFI")},
        {Category::Plugins, QStringLiteral("plugins"), tr("Plugin extensions")},
    };
    for (const auto &category : categories) {
        QList<LuauLanguageServer::Symbol> members;
        for (const auto &symbol : symbols) {
            if (symbol.category == category.kind) {
                members.append(symbol);
            }
        }
        if (members.isEmpty()) {
            continue;
        }
        auto *item = new QTreeWidgetItem(this, {category.title});
        const QString identity = QStringLiteral("category:") + category.id;
        item->setData(0, Qt::UserRole, identity);
        createObjNodes(members, item);
        item->setExpanded(_restoreExpanded.contains(identity));
    }
    setSortingEnabled(true);
    resizeColumnToContents(0);
}

void ObjTreeWidget::populateChildren(QTreeWidgetItem *item) {
    const auto identity = item->data(0, Qt::UserRole).toString();
    if (!_symbols.contains(identity) || _loaded.contains(identity)) {
        return;
    }
    _loaded.insert(identity);
    const auto symbol = _symbols.value(identity);
    const auto children = LuauLanguageServer::instance().symbolChildren(symbol);
    createObjNodes(children, item);
    item->setChildIndicatorPolicy(children.isEmpty()
                                      ? QTreeWidgetItem::DontShowIndicator
                                      : QTreeWidgetItem::ShowIndicator);
}

void ObjTreeWidget::createObjNodes(
    const QList<LuauLanguageServer::Symbol> &nodes, QTreeWidgetItem *parent) {
    QList<LuauLanguageServer::Symbol> rows;
    for (const auto &node : nodes) {
        if (node.overloads.size() > 1) {
            for (const auto &signature : node.overloads) {
                auto overload = node;
                overload.id +=
                    QStringLiteral("/overload:") +
                    QString::fromLatin1(
                        QCryptographicHash::hash(signature.toUtf8(),
                                                 QCryptographicHash::Sha256)
                            .toHex());
                overload.item.detail = signature;
                overload.fullDeclaration = signature;
                overload.overloads.clear();
                rows.append(std::move(overload));
            }
        } else {
            rows.append(node);
        }
    }
    for (const auto &node : rows) {
        const auto &symbol = node.item;
        const auto detail = symbol.detail.value_or(QString());
        const bool root = node.moduleRoot;
        auto item = new QTreeWidgetItem(
            parent,
            {symbol.label,
             root ? (node.requiresImport
                         ? QStringLiteral("require(\"@%1\")").arg(symbol.label)
                         : tr("Global"))
                  : detail});
        item->setData(0, Qt::UserRole, node.id);
        _symbols.insert(node.id, node);
        if (node.hasChildren) {
            item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }
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
        if (_restoreExpanded.contains(node.id)) {
            item->setExpanded(true);
        }
        if (_restoreSelected == node.id) {
            setCurrentItem(item);
        }
    }
}

void ObjTreeWidget::focusOutEvent(QFocusEvent *event) {
    clearSelection();
    QTreeWidget::focusOutEvent(event);
}
