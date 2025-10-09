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

#include "utilities.h"

#include "AngelScript/sdk/angelscript/source/as_objecttype.h"
#include "AngelScript/sdk/angelscript/source/as_scriptengine.h"
#include "AngelScript/sdk/angelscript/source/as_scriptfunction.h"

#include <QHeaderView>

ASObjTreeWidget::ASObjTreeWidget(QWidget *parent) : QTreeWidget(parent) {
    Utilities::applyTreeViewProperty(this);
    setColumnCount(2);
    auto header = this->header();
    header->setSortIndicator(0, Qt::SortOrder::AscendingOrder);
    setHeaderLabels({tr("Symbol"), tr("Decl")});
}

void ASObjTreeWidget::setEngine(asIScriptEngine *engine) {
    Q_ASSERT(engine);
    if (engine == nullptr) {
        return;
    }

    QHash<QString, QList<CodeInfoTip>> headerNodes;
    addGlobalFunctionCompletion(engine, headerNodes);
    addClassCompletion(engine, headerNodes);
    addEnumCompletion(engine, headerNodes);

    for (auto node = headerNodes.constKeyValueBegin();
         node != headerNodes.constKeyValueEnd(); node++) {
        QString header = node->first;

        QTreeWidgetItem *item = nullptr;

        if (header.isEmpty()) {
            item = new QTreeWidgetItem({tr("[Global]")});
        } else {
            item = new QTreeWidgetItem({header});
        }

        item->setToolTip(0, header);
        item->setIcon(
            0, CodeInfoTip::getDisplayIcon(LSP::CompletionItemKind::Module));
        createObjNodes(node->second, item);
        addTopLevelItem(item);
    }

    setSortingEnabled(true);
    resizeColumnToContents(0);
}

QTreeWidgetItem *ASObjTreeWidget::createObjNode(const CodeInfoTip &node,
                                                QTreeWidgetItem *parent) {
    Q_ASSERT(parent);
    QStringList contents{node.name, node.comment()};
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

QByteArray ASObjTreeWidget::getFnRealName(asIScriptFunction *fn) {
    auto fun = reinterpret_cast<asCScriptFunction *>(fn);
    if (fun == nullptr) {
        return {};
    }

    asCString str;
    asCString name = fun->GetName();

    if (name.GetLength() == 0)
        str += "_unnamed_function_";
    else if (name.SubString(0, 4) == "$beh" && name.GetLength() == 5) {
        if (name[4] == '0' + asBEHAVE_CONSTRUCT)
            str += fun->objectType->name;
        else if (name[4] == '0' + asBEHAVE_FACTORY)
            str += fun->returnType.GetTypeInfo()->name;
        else if (name[4] == '0' + asBEHAVE_DESTRUCT)
            str += "~" + fun->objectType->name;
        else
            str += name;
    } else {
        str = name;
    }

    return QByteArray(str.AddressOf(), QByteArray::size_type(str.GetLength()));
}

void ASObjTreeWidget::addGlobalFunctionCompletion(
    asIScriptEngine *engine, QHash<QString, QList<CodeInfoTip>> &c) {
    Q_ASSERT(engine);

    for (asUINT i = 0; i < engine->GetGlobalFunctionCount(); ++i) {
        auto fn = engine->GetGlobalFunctionByIndex(i);
        CodeInfoTip fnInfo;
        auto ns = fn->GetNamespace();
        fnInfo.name = fn->GetName();
        fnInfo.type = LSP::CompletionItemKind::Function;
        fnInfo.setComment(fn->GetDeclaration(true, true, true));
        c[ns].append(fnInfo);
    }
}

void ASObjTreeWidget::addEnumCompletion(asIScriptEngine *engine,
                                        QHash<QString, QList<CodeInfoTip>> &c) {
    Q_ASSERT(engine);

    for (asUINT i = 0; i < engine->GetEnumCount(); ++i) {
        auto etype = engine->GetEnumByIndex(i);
        etype->AddRef();

        CodeInfoTip einfo;
        QString ns = etype->GetNamespace();
        einfo.name = etype->GetName();
        einfo.type = LSP::CompletionItemKind::Enum;

        for (asUINT i = 0; i < etype->GetEnumValueCount(); ++i) {
            int v;
            auto e = etype->GetEnumValueByIndex(i, &v);

            CodeInfoTip en;
            en.type = LSP::CompletionItemKind::EnumMember;
            en.name = QString::fromLatin1(e);
            en.setComment(en.name + QStringLiteral(" = ") + QString::number(v));
            einfo.children.append(en);
        }

        etype->Release();
        c[ns].append(einfo);
    }
}

void ASObjTreeWidget::addClassCompletion(
    asIScriptEngine *engine, QHash<QString, QList<CodeInfoTip>> &c) {
    auto eng = reinterpret_cast<asCScriptEngine *>(engine);
    Q_ASSERT(eng);

    for (asUINT i = 0; i < engine->GetObjectTypeCount(); ++i) {
        auto obj = eng->registeredObjTypes[i];
        obj->AddRef();

        CodeInfoTip cls;
        cls.type = LSP::CompletionItemKind::Class;
        cls.name = obj->GetName();
        QString ns = obj->GetNamespace();

        for (asUINT i = 0; i < obj->GetBehaviourCount(); ++i) {
            asEBehaviours bv;
            auto b = obj->GetBehaviourByIndex(i, &bv);

            switch (bv) {
            case asBEHAVE_CONSTRUCT: {
                // only these are supported
                b->AddRef();
                CodeInfoTip fn;
                fn.type = LSP::CompletionItemKind::Function;
                fn.name = getFnRealName(b);
                fn.setComment(b->GetDeclaration(true, true, true));
                cls.children.append(fn);
                b->Release();
            }
            default:
                continue;
            }
        }

        for (asUINT i = 0; i < obj->GetMethodCount(); ++i) {
            auto m = obj->GetMethodByIndex(i, true);

            m->AddRef();
            CodeInfoTip fn;
            fn.type = LSP::CompletionItemKind::Function;
            fn.name = getFnRealName(m);
            fn.setComment(m->GetDeclaration(true, true, true));
            cls.children.append(fn);
            m->Release();
        }

        for (asUINT i = 0; i < obj->GetPropertyCount(); ++i) {
            auto p = obj->properties[i];

            CodeInfoTip pi;
            pi.type = LSP::CompletionItemKind::Property;
            pi.name =
                QString::fromLatin1(p->name.AddressOf(),
                                    QByteArray::size_type(p->name.GetLength()));
            auto tn = p->type.Format(obj->nameSpace);
            auto type = QString::fromLatin1(
                tn.AddressOf(), QByteArray::size_type(tn.GetLength()));

            QString prefix;
            if (p->isPrivate) {
                prefix = QStringLiteral("(private) ");
            } else if (p->isProtected) {
                prefix = QStringLiteral("(protected) ");
            } else {
                prefix = QStringLiteral("(public) ");
            }

            pi.setComment(prefix + type + QChar(' ') + pi.name);
            cls.children.append(pi);
        }

        obj->Release();

        c[ns].append(cls);
    }
}
