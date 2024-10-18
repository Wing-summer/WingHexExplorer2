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

#include "qcodeview.h"

/*!
        \file qcodeview.cpp
        \brief Implementation of the QCodeView class.
*/

#include "qcodemodel.h"
#include "qcodenode.h"

#include <QAbstractProxyModel>
#include <QContextMenuEvent>
#include <QFileInfo>

/*!
        \class QCodeView
        \brief Specialized tree view to display code models
*/

QCodeView::QCodeView(QWidget *p) : QTreeView(p) {
    setAutoScroll(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(this, SIGNAL(activated(QModelIndex)), this,
            SLOT(indexActivated(QModelIndex)));
}

QCodeView::QCodeView(QCodeModel *m, QWidget *p) : QTreeView(p), m_model(0) {
    setAutoScroll(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(this, SIGNAL(activated(QModelIndex)), this,
            SLOT(indexActivated(QModelIndex)));

    setModel(m);
}

QCodeView::~QCodeView() {}

void QCodeView::setModel(QAbstractItemModel *model) {
    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel *>(model);

    if (proxy) {
        m_model = qobject_cast<QCodeModel *>(proxy->sourceModel());
    } else {
        m_model = qobject_cast<QCodeModel *>(model);
    }

    if (!m_model)
        qFatal("QCodeView can only display a QCodeModel");

    QTreeView::setModel(model);
}

void QCodeView::contextMenuEvent(QContextMenuEvent *e) {
    e->accept();

    QModelIndex idx = indexAt(e->pos());

    if (!idx.isValid())
        return;
}

void QCodeView::indexActivated(const QModelIndex &idx) {
    static QStringList exts = QStringList() << "cpp"
                                            << "cxx"
                                            << "c"
                                            << "cc";

    if (!m_model)
        return;

    QCodeNode *n = 0;
    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel *>(model());

    if (proxy) {
        n = m_model->node(proxy->mapToSource(idx));
    } else {
        n = m_model->node(idx);
    }

    if (!n)
        return;

    QString cxt = n->context();

    if (n->type() == QCodeNode::Function) {
        // open the source file instead (after checking whether it lies in
        // header...)
        QFileInfo inf(cxt);

        foreach (const QString &ext, exts) {
            QString scxt = cxt;
            scxt.chop(inf.suffix().length());
            scxt += ext;

            if (QFile::exists(scxt)) {
                cxt = scxt;
                break;
            }
        }
    } else if (n->line != -1) {
        emit actionRequested("open", QStringList() << cxt << "-l"
                                                   << QString::number(n->line));
        return;
    }

    int i;
    QString ptp, rxp, qn = n->qualifiedName(false);

    // qDebug("%s [%s] {%s}", qPrintable(qn),
    // qPrintable(n->data(Qt::DisplayRole).toString()),
    // n->role(QCodeNode::NodeType).constData());

    // qn.remove(0, qn.indexOf("/") + 1);

    ptp = qn;
    rxp = QRegularExpression::escape(qn);

    rxp.replace(
        QRegularExpression("\\s*(::|\\\\[()\\[\\]{}|*$+.?^]|[,&<>])\\s*"),
        "\\s*\\1\\s*");
    rxp.replace(" ", "\\s+");

    i = rxp.indexOf("(");

    QString tmp = rxp.mid(i);
    tmp.replace(
        QRegularExpression("(\\\\s[+*])[\\w_]+\\\\s\\*(,|\\\\\\)\\\\s\\*$)"),
        "\\1[\\w_]*\\s*\\2");
    rxp = rxp.left(i) + tmp;

    ptp.replace(QRegularExpression(" (::|[()<>]) "), "\\1");
    i = ptp.indexOf("(");

    if (i != -1)
        ptp = ptp.left(i);

    emit actionRequested("open", QStringList()
                                     << cxt << "-s" << ptp << "-rx" << rxp);
}
