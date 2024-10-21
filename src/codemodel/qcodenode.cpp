/****************************************************************************
**
** Copyright (C) 2006-2009 fullmetalcoder <fullmetalcoder@hotmail.fr>
**
** This file is part of the Edyuk project <http://edyuk.org>
**
** This file may be used under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation and appearing in the
** file GPL.txt included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcodenode.h"

#include <QIcon>
#include <QVariant>

#include "qcodemodel.h"
#include "qsourcecodewatcher.h"

enum CacheIndex {
    ICON_ENUM,
    ICON_ENUMERATOR,
    ICON_UNION,
    ICON_CLASS,
    ICON_STRUCT,
    ICON_TYPEDEF,
    ICON_NAMESPACE,
    ICON_FUNCTION = ICON_NAMESPACE + 2,
    ICON_VARIABLE = ICON_FUNCTION + 5
};

static QHash<int, QIcon> q_icon_cache;

static QIcon icon(int cacheIndex) {
    static bool setup = false;

    if (!setup) {
        q_icon_cache[ICON_UNION] = QIcon(":/completion/CVunion.png");

        q_icon_cache[ICON_ENUM] = QIcon(":/completion/CVenum.png");
        q_icon_cache[ICON_ENUMERATOR] = QIcon(":/completion/CVenumerator.png");

        q_icon_cache[ICON_CLASS] = QIcon(":/completion/CVclass.png");

        q_icon_cache[ICON_STRUCT] = QIcon(":/completion/CVstruct.png");

        q_icon_cache[ICON_TYPEDEF] = QIcon(":/completion/CVtypedef.png");

        q_icon_cache[ICON_NAMESPACE] = QIcon(":/completion/CVnamespace.png");

        q_icon_cache[ICON_FUNCTION + QCodeNode::VISIBILITY_DEFAULT] =
            QIcon(":/completion/CVglobal_meth.png");

        q_icon_cache[ICON_FUNCTION + QCodeNode::VISIBILITY_PUBLIC] =
            QIcon(":/completion/CVpublic_meth.png");

        q_icon_cache[ICON_FUNCTION + QCodeNode::VISIBILITY_PROTECTED] =
            QIcon(":/completion/CVprotected_meth.png");

        q_icon_cache[ICON_FUNCTION + QCodeNode::VISIBILITY_PRIVATE] =
            QIcon(":/completion/CVprivate_meth.png");

        q_icon_cache[ICON_FUNCTION + QCodeNode::VISIBILITY_SIGNAL] =
            QIcon(":/completion/CVprotected_signal.png");

        q_icon_cache[ICON_VARIABLE + QCodeNode::VISIBILITY_DEFAULT] =
            QIcon(":/completion/CVglobal_var.png");

        q_icon_cache[ICON_VARIABLE + QCodeNode::VISIBILITY_PUBLIC] =
            QIcon(":/completion/CVpublic_var.png");

        q_icon_cache[ICON_VARIABLE + QCodeNode::VISIBILITY_PROTECTED] =
            QIcon(":/completion/CVprotected_var.png");

        q_icon_cache[ICON_VARIABLE + QCodeNode::VISIBILITY_PRIVATE] =
            QIcon(":/completion/CVprivate_var.png");

        setup = true;
    }

    return q_icon_cache.value(cacheIndex);
}

QByteArray section(const QByteArray &b, char c, int beg, int end = -1) {
    QList<QByteArray> l = b.split(c);

    // qDebug("split %s into %i parts...", b.constData(), l.length());
    // qDebug("parts %i to %i", beg, end);

    if (beg < 0)
        beg = l.length() + beg;

    if (end < 0)
        end = l.length() + end;

    int start = qMin(beg, end), stop = qMax(beg, end);

    // qDebug("parts %i to %i", start, stop);

    if ((start >= l.length()) || (stop < 0))
        return QByteArray();

    QByteArray sec = l.at(start);

    while (++start <= stop)
        sec.prepend(c).prepend(l.at(start));

    return sec;
}

void setSection(QByteArray &b, char c, int beg, int end = -1,
                const QByteArray &r = QByteArray()) {
    QList<QByteArray> l = b.split(c);

    if (beg < 0)
        beg = l.length() + beg;

    if (end < 0)
        end = l.length() + end;

    int offset = 0, length = 0;
    int start = qMin(beg, end), stop = qMax(beg, end);

    if ((start >= l.length()) || (stop < 0))
        return;

    for (int i = 0; i < start; ++i)
        offset += l.at(i).length() + 1;

    for (int i = start; i <= stop; ++i)
        length += l.at(i).length() + 1;

    --length;

    /*
    qDebug("set section [%i, %i]=>[%i, %i] of \"%s\" to \"%s\"",
                    beg, end,
                    offset, offset + length,
                    b.constData(),
                    r.constData());
    */

    b.replace(offset, length, r);
}

void QCodeNode::operator delete(void *p) {
    QCodeNode *n = static_cast<QCodeNode *>(p);

    if (!n)
        return;
    else
        ::operator delete(p);
}

QCodeNode::QCodeNode() : line(-1), parent(0), model(0) {}

QCodeNode::~QCodeNode() {
    detach();

    model = 0;
    parent = 0;

    clear();

    QSourceCodeWatcher *w = QSourceCodeWatcher::watcher(this, 0);

    if (w)
        delete w;
}

void QCodeNode::attach(QCodeNode *p) {
    detach();

    if (!p || p->children.contains(this))
        return;

    bool modelChange = model != p->model;

    if (modelChange) {
        QStack<QCodeNode *> tree;

        tree.push(this);

        while (tree.length()) {
            QCodeNode *n = tree.pop();

            n->model = p->model;

            foreach (QCodeNode *c, n->children)
                tree.push(c);
        }
    }

    int row = p->children.length();

    if (model)
        model->beginInsertRows(model->index(p), row, row);

    parent = p;
    p->children.insert(row, this);

    if (model)
        model->endInsertRows();
}

void QCodeNode::detach() {
    if (!parent)
        return;

    int row = parent->children.indexOf(this);

    if (row < 0)
        return;

    if (model)
        model->beginRemoveRows(model->index(parent), row, row);

    parent->children.removeAt(row);
    parent = 0;

    if (model)
        model->endRemoveRows();

    if (model) {
        QStack<QCodeNode *> tree;

        tree.push(this);

        while (tree.length()) {
            QCodeNode *n = tree.pop();

            n->model = 0;

            foreach (QCodeNode *c, n->children)
                tree.push(c);
        }
    }
}

void QCodeNode::clear() {
    QList<QCodeNode *> c = children;

    removeAll();

    qDeleteAll(c);
}

void QCodeNode::removeAll() {
    if (children.isEmpty())
        return;

    if (model)
        model->beginRemoveRows(model->index(this), 0, children.length() - 1);

    foreach (QCodeNode *n, children) {
        n->model = 0;
        n->parent = 0;
    }

    children.clear();

    if (model)
        model->endRemoveRows();
}

int QCodeNode::type() const {
    return roles.length() ? role(NodeType).at(0) : 0;
}

QByteArray QCodeNode::context() const {
    int t = type();

    if ((t == Group) || (t == Language) || (t == Namespace))
        return QByteArray();

    const QCodeNode *p = this;

    while (p->parent) {
        int t = p->parent->type();

        if ((t == Group) || (t == Language) || (t == Namespace))
            break;

        p = p->parent;
    }

    return p ? p->role(Context) : role(Context);
}

QByteArray QCodeNode::qualifiedName(bool language) const {
    int t = type();

    if (t == Group)
        return QByteArray();
    else if (t == Language)
        return language ? role(Name) : QByteArray();

    QByteArray cxt = parent ? parent->qualifiedName(language) : QByteArray();

    if (cxt.length()) {
        // if ( parent->type() == Language )
        //	cxt += "/";
        // else
        cxt += "::";
    }

    // cxt += role(Name);

    cxt += role(Name);

    if (t == Function) {
        cxt += "(";
        cxt += role(Arguments);
        cxt += ")";
    }

    return cxt;
}

QVariant QCodeNode::data(int r) const {
    const int t = type();

    switch (r) {
    case Qt::DisplayRole: {
        if (t == Function)
            return role(Name) + "(" + role(Arguments) + ")";

        // if ( t == Enumerator )
        //	;

        return role(Name);
    }

    case Qt::ToolTipRole:
    case Qt::StatusTipRole: {
        switch (t) {
        case Class: {
            QByteArray d("class ");
            d += role(Name);

            QByteArray a = role(Ancestors);

            if (a.length())
                d += " : " + a;

            return d;
        }

        case Struct: {
            QByteArray d("struct ");
            d += role(Name);

            QByteArray a = role(Ancestors);

            if (a.length())
                d += " : " + a;

            return d;
        }

        case Enum:
            return QByteArray("enum ") + role(Name);

        case Enumerator:
            return role(Name) + " = " + role(Value);

        case Union:
            return QByteArray("union ") + role(Name);

        case Namespace:
            return QByteArray("namespace ") + role(Name);

        case Typedef:
            return QByteArray("typedef ") + role(Alias) + " " + role(Name);

        case Variable: {
            QByteArray signature, specifier;

            signature += role(Type);
            signature += " ";
            signature += role(Name);

            int m_visibility = role(Visibility).toInt();
            int m_specifiers = role(Specifiers).toInt();

            // visibility (for class members)
            if (m_visibility == QCodeNode::VISIBILITY_PUBLIC)
                specifier = " public ";
            else if (m_visibility == QCodeNode::VISIBILITY_PROTECTED)
                specifier = " protected ";
            else
                specifier = " private ";

            // storage class
            if (m_specifiers & QCodeNode::SPECIFIER_AUTO)
                specifier += " auto ";
            else if (m_specifiers & QCodeNode::SPECIFIER_REGISTER)
                specifier += " register ";
            else if (m_specifiers & QCodeNode::SPECIFIER_STATIC)
                specifier += " static ";
            else if (m_specifiers & QCodeNode::SPECIFIER_EXTERN)
                specifier += " extern ";
            else if (m_specifiers & QCodeNode::SPECIFIER_MUTABLE)
                specifier += " mutable ";

            // cv qualifier (for class members)
            if (m_specifiers & QCodeNode::SPECIFIER_CONST)
                specifier += " const ";
            else if (m_specifiers & QCodeNode::SPECIFIER_VOLATILE)
                specifier += " volatile ";

            if (specifier.length())
                signature += " [" + specifier.simplified() + "]";

            return signature;
            // return role(Type) + " " + role(Name);
        }

        case Function: {
            QByteArray signature, qualifier, ret = role(Return);

            if (ret.length())
                signature += ret + " ";

            signature += role(Name);

            signature += "(";
            signature += role(Arguments);
            signature += ")";

            int m_qualifiers = role(Qualifiers).toInt();

            if (m_qualifiers & QCodeNode::QUALIFIER_CONST)
                qualifier += " const ";
            else if (m_qualifiers & QCodeNode::QUALIFIER_VOLATILE)
                qualifier += " volatile ";
            else if (m_qualifiers & QCodeNode::QUALIFIER_STATIC)
                qualifier += " static ";

            if (m_qualifiers & QCodeNode::QUALIFIER_PURE_VIRTUAL)
                qualifier.prepend(" pure virtual ");
            else if (m_qualifiers & QCodeNode::QUALIFIER_INLINE)
                qualifier.prepend(" inline ");
            else if (m_qualifiers & QCodeNode::QUALIFIER_VIRTUAL)
                qualifier.prepend(" virtual ");

            int m_visibility = role(Visibility).toInt();

            if (m_visibility == QCodeNode::VISIBILITY_PUBLIC)
                qualifier.prepend(" public ");
            else if (m_visibility == QCodeNode::VISIBILITY_PROTECTED)
                qualifier.prepend(" protected ");
            else if (m_visibility == QCodeNode::VISIBILITY_SIGNAL)
                qualifier.prepend(" signal ");
            else if (m_visibility == QCodeNode::VISIBILITY_PRIVATE)
                qualifier.prepend(" private ");
            else
                qualifier.prepend(" global ");

            if (ret.isEmpty()) {
                if (role(Name).startsWith("~"))
                    qualifier += " destructor ";
                else
                    qualifier += " constructor ";
            }

            if (qualifier.length())
                signature += " [" + qualifier.simplified() + "]";

            // return role(Name) + " " + role(Name);
            return signature;
        }

        default:
            break;
        };

        return QVariant();
    }

    case Qt::DecorationRole: {
        switch (t) {
        case Class:
            return icon(ICON_CLASS);

        case Struct:
            return icon(ICON_STRUCT);

        case Enum:
            return icon(ICON_ENUM);

        case Enumerator:
            return icon(ICON_ENUMERATOR);

        case Union:
            return icon(ICON_UNION);

        case Namespace:
            return icon(ICON_NAMESPACE);

        case Typedef:
            return icon(ICON_TYPEDEF);

        case Variable:
            return icon(ICON_VARIABLE + role(Visibility).toInt());

        case Function:
            return icon(ICON_FUNCTION + role(Visibility).toInt());

        default:
            break;
        };

        return QVariant();
    }

    case QCodeModel::TypeRole:
        return type();

    case QCodeModel::VisibilityRole:
        return role(Visibility).toInt();

    default:
        break;
    }

    return QVariant();
}

void QCodeNode::setData(int role, const QVariant &v) {
    Q_UNUSED(v)
    Q_UNUSED(role)
}

QByteArray QCodeNode::role(RoleIndex r) const {
    return section(roles, '@', (int)r, (int)r);
}

void QCodeNode::setRole(RoleIndex r, const QByteArray &b) {
    setSection(roles, '@', (int)r, (int)r, b);
}
