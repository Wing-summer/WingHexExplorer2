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

#include "qcodeserializer.h"

/*!
        \file qcodeserializer.cpp
        \brief Implementation of the QCodeSerializer class.
*/

#include "qcodemodel.h"
#include "qcodenode.h"
#include "qcodenodepool.h"

#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QRegularExpression>
#include <QTextStream>

/*
        File format :
         * header :  '{' MIME type (text/x-tag) ',' version ',' source '}'
         * each line represents on node as a at ('@') separated list of roles
         * each space at the begining of the line represents a parenting level

*/

#define Q_VERSION(maj, min, patch) ((maj << 8) | (min << 8) | patch)

static int _version = Q_VERSION(2, 0, 0);
static int _version_min = Q_VERSION(2, 0, 0), _version_max = Q_VERSION(2, 0, 0);
static QString header_pattern("{text/x-tag,%1,%2}");
static QRegularExpression
    header_matcher("^\\{text/x-tag,([0-9a-fA-F]+),([^}]+)\\}\n$");

/*!
        \class QCodeSerializer
        \brief Serializer/deserializer for code trees
*/

QCodeSerializer::QCodeSerializer() : m_model(0), m_pool(0) {}

QCodeSerializer::~QCodeSerializer() {}

QCodeModel *QCodeSerializer::targetModel() const { return m_model; }

void QCodeSerializer::setTargetModel(QCodeModel *m) { m_model = m; }

QCodeNodePool *QCodeSerializer::nodePool() const { return m_pool; }

void QCodeSerializer::setNodePool(QCodeNodePool *p) { m_pool = p; }

void QCodeSerializer::serialize(QCodeNode *n, QTextStream &out, int indent) {
    if (!n)
        return;

    if (indent > 0)
        out << QByteArray(indent, ' ');
    else
        out << header_pattern.arg(QString::number(_version))
                   .arg(QString::fromLocal8Bit(n->role(QCodeNode::Context)))
            << Qt::endl;

    out << n->roles << "\n";

    foreach (QCodeNode *c, n->children)
        serialize(c, out, indent + 1);
}

void QCodeSerializer::deserialize(const QString &file, bool *ok,
                                  QString *source, const QDateTime &t) {
    QFile f(file);

    if (f.open(QFile::Text | QFile::ReadOnly))
        deserialize(&f, ok, source, t);
}

void QCodeSerializer::deserialize(QIODevice *device, bool *ok, QString *source,
                                  const QDateTime &t) {
    if (!device)
        return;

    QByteArray line;
    int indent = 0, spaces;
    QCodeNode *n, *previous = 0, *parent = 0;

    line = device->readLine();
    auto match = header_matcher.match(line);
    if (!match.hasMatch()) {
        qWarning("Header mismatch : %s", line.constData());

        if (ok)
            *ok = false;

        return;
    }

    int version = match.captured(1).toInt(0, 0);

    if ((version < _version_min) || (version > _version_max)) {
        qWarning("Version mismatch : %i !E [%i, %i]", version, _version_min,
                 _version_max);

        if (ok)
            *ok = false;

        return;
    }

    if (source)
        *source = match.captured(2);

    if (t.isValid() && ok) {
        // check time stamps if someone pay attention to success/failure
        QFileInfo info(*source);

        if (info.lastModified() > t) {
            // source modified after tag file... must reload...

            *ok = false;
        }
    }

    while (!device->atEnd()) {
        spaces = 0;
        char c;
        bool ok = device->getChar(&c);

        while (ok && isspace(c) && (c != '\n')) {
            ++spaces;
            ok = device->getChar(&c);
        }

        if (!ok || c == '\n')
            continue;

        if (spaces > indent) {
            // down one level
            parent = previous;

            // if ( parent )
            //	qDebug("down a level : parent = %s",
            // parent->role(QCodeNode::Name).constData());
        } else if (spaces < indent) {

            // up one (or more) level
            for (int i = 0; (i < (indent - spaces)) && parent && parent->parent;
                 ++i)
                parent = parent ? parent->parent : 0;

            // if ( parent )
            //	qDebug("up one level : parent = %s",
            // parent->role(QCodeNode::Name).constData());
        } else {
            // qDebug("adding child %s", line.constData());
        }

        n = m_pool ? m_pool->alloc() : new QCodeNode;

        while (ok && c != '\n') {
            n->roles += c;
            ok = device->getChar(&c);
        }

        // n->roles = device->readLine();
        // n->roles.prepend(c);

        n->roles.squeeze();

        if (parent)
            n->attach(parent);

        indent = spaces;
        previous = n;
    }

    while (parent && parent->parent)
        parent = parent->parent;

    if (parent && m_model)
        m_model->appendTopLevelNode(parent);

    if (ok)
        *ok = true;
}
