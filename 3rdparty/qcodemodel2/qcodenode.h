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

#ifndef _QCODE_NODE_H_
#define _QCODE_NODE_H_

#include "qcm-config.h"

#include <QByteArray>
#include <QList>

class QVariant;

class QCodeModel;
class QCodeNodePool;

struct QCM_EXPORT QCodeNode {
    enum RoleIndex {
        NodeType = 0,
        Name = 1,

        // common
        Visibility = Name + 2,
        Templates = Visibility + 1,

        // class/struct
        Ancestors = Name + 1,
        Friends = Ancestors + 1,

        // typedef
        Alias = Name + 1,

        // enumerator
        Value = Name + 1,

        // variables/members
        Type = Name + 1,
        Specifiers = Visibility + 1,

        // function/methods
        Return = Name + 1,
        Qualifiers = Templates + 1,
        Arguments = Qualifiers + 1,

        Context = -1
    };

    enum DefaultNodeTypes {
        Group = 'g',
        Language = 'l',

        Class = 'c',
        Struct = 's',

        Function = 'f',

        Variable = 'v',

        Enum = 'e',
        Enumerator = 'r',

        Union = 'u',

        Namespace = 'n',

        Typedef = 't'
    };

    enum NodeContent {
        CONTENT_NONE = 0,
        CONTENT_DEFINITION = 1,
        CONTENT_IMPLEMENTATION = 2,
    };

    enum NodeVisibility {
        VISIBILITY_DEFAULT = -1,
        VISIBILITY_PUBLIC,
        VISIBILITY_SIGNAL,
        VISIBILITY_PROTECTED,
        VISIBILITY_PRIVATE
    };

    enum Specifier {
        SPECIFIER_NONE = 0,
        SPECIFIER_CONST = 1,
        SPECIFIER_VOLATILE = 2,
        SPECIFIER_MUTABLE = 4,
        SPECIFIER_AUTO = 8,
        SPECIFIER_STATIC = 16,
        SPECIFIER_REGISTER = 32,
        SPECIFIER_EXTERN = 64
    };

    typedef QFlags<Specifier> TypeSpecifier;

    enum Qualifier {
        QUALIFIER_NONE = 0,
        QUALIFIER_CONST = 1,
        QUALIFIER_VOLATILE = 2,
        QUALIFIER_STATIC = 4,
        QUALIFIER_EXTERN = 8,
        QUALIFIER_VIRTUAL = 16,
        QUALIFIER_PURE_VIRTUAL = 32,
        QUALIFIER_INLINE = 64
    };

    typedef QFlags<Qualifier> FunctionQualifier;

    QCodeNode();
    QCodeNode(QCodeNodePool *p);
    virtual ~QCodeNode();

    void operator delete(void *p);

    virtual int type() const;
    virtual QByteArray context() const;
    virtual QByteArray qualifiedName(bool language = true) const;

    virtual QVariant data(int role) const;
    virtual void setData(int role, const QVariant &v);

    QByteArray role(RoleIndex r) const;
    void setRole(RoleIndex r, const QByteArray &b);

    virtual void clear();
    virtual void removeAll();

    virtual void attach(QCodeNode *p);
    virtual void detach();

    int line;
    QByteArray roles;
    QCodeNode *parent;
    QCodeModel *model;
    QCodeNodePool *m_pool;
    QList<QCodeNode *> children;
};

#endif // !_QCODE_NODE_H_
