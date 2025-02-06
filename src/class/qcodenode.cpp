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

enum CacheIndex {
    ICON_ENUM,
    ICON_ENUMERATOR,
    // ICON_UNION,
    ICON_CLASS,
    // ICON_STRUCT,
    ICON_KEYWORD,
    ICON_TYPEDEF,
    ICON_NAMESPACE,
    ICON_FUNCTION = ICON_NAMESPACE + 2,
    ICON_VARIABLE = ICON_FUNCTION + 5
};

using IconHash = QHash<int, QIcon>;
Q_GLOBAL_STATIC(IconHash, q_icon_cache)

inline static QIcon getIcon(const QString &name) {
    return QIcon(QStringLiteral(":/completion/images/completion/") + name +
                 QStringLiteral(".png"));
}

static QIcon icon(int cacheIndex) {
    static bool setup = false;

    if (!setup) {
        // q_icon_cache[ICON_UNION] = QIcon(":/completion/CVunion.png");

        (*q_icon_cache)[ICON_ENUM] = getIcon(QStringLiteral("CVenum"));
        (*q_icon_cache)[ICON_ENUMERATOR] =
            getIcon(QStringLiteral("CVenumerator"));

        (*q_icon_cache)[ICON_CLASS] = getIcon(QStringLiteral("CVclass"));

        // q_icon_cache[ICON_STRUCT] = QIcon(":/completion/CVstruct.png");

        (*q_icon_cache)[ICON_TYPEDEF] = getIcon(QStringLiteral("CVtypedef"));

        (*q_icon_cache)[ICON_NAMESPACE] =
            getIcon(QStringLiteral("CVnamespace"));

        (*q_icon_cache)[ICON_FUNCTION + QCodeNode::VISIBILITY_DEFAULT] =
            getIcon(QStringLiteral("CVglobal_meth"));

        (*q_icon_cache)[ICON_FUNCTION + QCodeNode::VISIBILITY_PUBLIC] =
            getIcon(QStringLiteral("CVpublic_meth"));

        (*q_icon_cache)[ICON_FUNCTION + QCodeNode::VISIBILITY_PROTECTED] =
            getIcon(QStringLiteral("CVprotected_meth"));

        (*q_icon_cache)[ICON_FUNCTION + QCodeNode::VISIBILITY_PRIVATE] =
            getIcon(QStringLiteral("CVprivate_meth"));

        (*q_icon_cache)[ICON_VARIABLE + QCodeNode::VISIBILITY_DEFAULT] =
            getIcon(QStringLiteral("CVglobal_var"));

        (*q_icon_cache)[ICON_VARIABLE + QCodeNode::VISIBILITY_PUBLIC] =
            getIcon(QStringLiteral("CVpublic_var"));

        (*q_icon_cache)[ICON_VARIABLE + QCodeNode::VISIBILITY_PROTECTED] =
            getIcon(QStringLiteral("CVprotected_var"));

        (*q_icon_cache)[ICON_VARIABLE + QCodeNode::VISIBILITY_PRIVATE] =
            getIcon(QStringLiteral("CVprivate_var"));

        setup = true;
    }

    return q_icon_cache->value(cacheIndex);
}

QCodeNode::QCodeNode() : line(-1), _parent(nullptr) {}

QCodeNode::~QCodeNode() {
    QCodeNode::detach();
    _parent = nullptr;
    clear();
}

void QCodeNode::attach(QCodeNode *p) {
    detach();

    if (!p || p->_children.contains(this))
        return;

    _parent = p;
    p->_children.append(this);
}

void QCodeNode::detach() {
    if (!_parent)
        return;

    int row = _parent->_children.indexOf(this);

    if (row < 0)
        return;

    _parent->_children.removeAt(row);
    _parent = 0;
}

QCodeNode *QCodeNode::parent() const { return _parent; }

void QCodeNode::setParent(QCodeNode *newParent) { _parent = newParent; }

int QCodeNode::getLine() const { return line; }

void QCodeNode::setLine(int newLine) { line = newLine; }

void QCodeNode::clear() {
    if (_children.isEmpty())
        return;

    for (auto &n : _children) {
        n->_parent = nullptr;
        n->clear();
    }

    qDeleteAll(_children);
    _children.clear();
}

int QCodeNode::type() const {
    return roles.value(NodeType, QByteArray(1, 0)).at(0);
}

QByteArray QCodeNode::context() const {
    int t = type();

    if ((t == Group) || (t == Namespace))
        return QByteArray();

    const QCodeNode *p = this;

    while (p->_parent) {
        int t = p->_parent->type();

        if ((t == Group) || (t == Namespace))
            break;

        p = p->_parent;
    }

    return p ? p->role(Context) : role(Context);
}

QByteArray QCodeNode::qualifiedBaseName(bool ext) const {
    int t = type();

    if (t == Group)
        return QByteArray();

    QByteArray cxt;
    if (ext) {
        if (_parent && _parent->type() == Namespace) {
            cxt += _parent->role(Name);
            cxt += "::";
        }
    }

    cxt += role(Name);
    return cxt;
}

QByteArray QCodeNode::qualifiedName(bool ext) const {
    int t = type();

    if (t == Group)
        return QByteArray();

    auto cxt = qualifiedBaseName(ext);

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

            // case Struct: {
            //     QByteArray d("struct ");
            //     d += role(Name);

            //     QByteArray a = role(Ancestors);

            //     if (a.length())
            //         d += " : " + a;

            //     return d;
            // }

        case Enum:
            return QByteArray("enum ") + role(Name);

        case Enumerator:
            return role(Name) + " = " + role(Value);

            // case Union:
            //     return QByteArray("union ") + role(Name);

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
            // else if (m_specifiers & QCodeNode::SPECIFIER_REGISTER)
            //     specifier += " register ";
            else if (m_specifiers & QCodeNode::SPECIFIER_STATIC)
                specifier += " static ";
            else if (m_specifiers & QCodeNode::SPECIFIER_EXTERN)
                specifier += " extern ";
            // else if (m_specifiers & QCodeNode::SPECIFIER_MUTABLE)
            //     specifier += " mutable ";

            // cv qualifier (for class members)
            if (m_specifiers & QCodeNode::SPECIFIER_CONST)
                specifier += " const ";
            // else if (m_specifiers & QCodeNode::SPECIFIER_VOLATILE)
            //     specifier += " volatile ";

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
            // else if (m_qualifiers & QCodeNode::QUALIFIER_VOLATILE)
            //     qualifier += " volatile ";
            // else if (m_qualifiers & QCodeNode::QUALIFIER_STATIC)
            //     qualifier += " static ";

            /*   if (m_qualifiers & QCodeNode::QUALIFIER_PURE_VIRTUAL)
                   qualifier.prepend(" pure virtual ");
               else */
            if (m_qualifiers & QCodeNode::QUALIFIER_INLINE)
                qualifier.prepend(" inline ");
            // else if (m_qualifiers & QCodeNode::QUALIFIER_VIRTUAL)
            //     qualifier.prepend(" virtual ");

            int m_visibility = role(Visibility).toInt();

            if (m_visibility == QCodeNode::VISIBILITY_PUBLIC)
                qualifier.prepend(" public ");
            else if (m_visibility == QCodeNode::VISIBILITY_PROTECTED)
                qualifier.prepend(" protected ");
            // else if (m_visibility == QCodeNode::VISIBILITY_SIGNAL)
            //     qualifier.prepend(" signal ");
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

            // case Struct:
            //     return icon(ICON_STRUCT);

        case Enum:
            return icon(ICON_ENUM);

        case Enumerator:
            return icon(ICON_ENUMERATOR);

            // case Union:
            //     return icon(ICON_UNION);

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

    default:
        break;
    }

    return QVariant();
}

QByteArray QCodeNode::role(RoleIndex r) const { return roles.value(r); }

void QCodeNode::setRole(RoleIndex r, const QByteArray &b) { roles[r] = b; }

QList<QCodeNode *> &QCodeNode::children() { return _children; }

void QCodeNode::setNodeType(DefaultNodeTypes t) {
    setRole(NodeType, QByteArray(1, t));
}
