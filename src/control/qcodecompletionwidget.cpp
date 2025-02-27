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

#include "qcodecompletionwidget.h"
#include "qcodecompletionwidget_p.h"

#include "qdocument.h"
#include "qdocumentcursor.h"
#include "qdocumentline.h"
#include "qeditor.h"

#include "class/qcodenode.h"

#include "class/ascompletion.h"

#include <QApplication>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QScrollBar>

constexpr auto PADDING = 3;

QCodeCompletionWidget::QCodeCompletionWidget(QEditor *p)
    : QListView(nullptr), offset(PADDING) {
    // setWindowFlags(Qt::FramelessWindowHint | Qt::SubWindow);
    setBatchSize(10);
    setMovement(Static);
    setFlow(TopToBottom);
    setLayoutMode(Batched);
    setUniformItemSizes(true);
    setEditTriggers(NoEditTriggers);
    setSelectionMode(SingleSelection);
    QWidget::setCursor(Qt::ArrowCursor);

    // just to get rid of warnings...
    qRegisterMetaType<QItemSelection>("QItemSelection");

    hide();
    setEditor(p);

    pModel = new QCodeCompletionModel(this);
    setModel(pModel);

    connect(pModel, &QCodeCompletionModel::changed, this,
            &QCodeCompletionWidget::changed);

    connect(this, &QCodeCompletionWidget::activated, this,
            &QCodeCompletionWidget::complete);
}

void QCodeCompletionWidget::changed() {
    if (!isVisible())
        return;

    verticalScrollBar()->setSliderPosition(0);
    horizontalScrollBar()->setSliderPosition(0);

    reset();
    adjustGeometry();

    show();
    setFocus();
    setCurrentIndex(pModel->index(0, 0));
}

void QCodeCompletionWidget::adjustGeometry() {
    QEditor *e = editor();
    QWidget *o = e->viewport();

    QDocumentCursor cursor = e->cursor();
    QDocumentLine line = cursor.line();
    const QRect lrect = e->lineRect(cursor.lineNumber());
    const QFontMetrics fm = e->document()->fontMetrics();

    int h = 0, w = 300, ls = fm.lineSpacing(), y = lrect.y(),
        x = line.cursorToX(cursor.columnNumber() + offset);

    int off = (cursor.columnNumber() + offset) - line.length();

    if (off > 0)
        x += off * fm.horizontalAdvance(' ');

    x -= e->horizontalScrollBar()->value();
    // qDebug("%i items", length()) ;

    h = qMin(pModel->rowCount() * ls + 20, qMin(250, 2 * o->height() / 5));
    // h = qMin(250, 2 * o->height() / 5);

    // qDebug("y = %i; height = %i;", crect.y(), o->height());

    if (lrect.y() <= (o->height() / 2)) {
        if (x >= (o->width() / 2))
            setGeometry(x - w, y + ls, w, h);
        else
            setGeometry(x, y, w, h);
    } else {
        if (x >= (o->width() / 2))
            setGeometry(x - w, y + lrect.height() - h - ls, w, h);
        else
            setGeometry(x, y + lrect.height() - h, w, h);
    }

    offset = PADDING;
}

QDocumentCursor QCodeCompletionWidget::cursor() const { return _cur; }

void QCodeCompletionWidget::setCursor(const QDocumentCursor &newCur) {
    _cur = newCur;
}

QEditor *QCodeCompletionWidget::editor() const {
    return dynamic_cast<QEditor *>((QObject *)pEditor);
}

void QCodeCompletionWidget::setEditor(QEditor *e) {
    pEditor = e;
    if (e) {
        setParent(e->viewport());
    }
}

bool QCodeCompletionWidget::hasEntries() const { return pModel->rowCount(); }

QCodeCompletionWidget::Filter QCodeCompletionWidget::filter() const {
    return pModel->filter();
}

void QCodeCompletionWidget::setFilter(Filter f) { pModel->setFilter(f); }

QString QCodeCompletionWidget::prefix() const { return pModel->prefix(); }

void QCodeCompletionWidget::setPrefix(const QString &prefix) {
    pModel->setPrefix(prefix);
}

QStringList QCodeCompletionWidget::completions() const {
    QString name;
    QStringList l;

    foreach (QCodeNode *n, pModel->focusNodes()) {
        name = n->role(QCodeNode::Name);

        if (n->type() ==
            QCodeNode::Function) // qobject_cast<QFunctionNode*>(n) )
            name += "()";

        if (!l.contains(name))
            l << name;
    }

    return l;
}

void QCodeCompletionWidget::setCompletions(const QList<QCodeNode *> &nodes) {
    pModel->setFocusNodes(nodes);
}

void QCodeCompletionWidget::setTemporaryNodes(const QList<QCodeNode *> &l) {
    m_temps = l;
}

bool QCodeCompletionWidget::isCompleting() const { return _completing; }

void QCodeCompletionWidget::clear() { pModel->clear(); }

void QCodeCompletionWidget::popup() {
    // qDebug("popping up...");

    if (!editor() || !hasEntries()) {
        // qDebug("popup not needed... [0x%x, %i]", (long)editor(),
        // pModel->rowlength());
        return;
    }

    adjustGeometry();

    // qDebug("showing...");
    show();

    // qDebug("focusing...");
    setFocus();

    // qDebug("selecting...");
    setCurrentIndex(pModel->index(0, 0, QModelIndex()));

    // qDebug("popped up");
}

void QCodeCompletionWidget::complete(const QModelIndex &index) {
    _completing = true;

    QEditor *e = editor();

    if (!index.isValid() || !e)
        return;

    bool back = false;
    QDocumentCursor c = e->cursor();
    QString prefix = pModel->prefix(),
            txt = pModel->data(index, Qt::UserRole).toString();

    // must obtain data before hiding... might be a temporary node
    hide();

    int pb = txt.indexOf('('), pe = txt.lastIndexOf(')');

    if (pb < (pe - 1)) {
        back = true;
        txt.remove(pb + 1, pe - pb - 1);
    }

    static QRegularExpression re("(\\bconst\\s*)?(=\\s*0)?$");
    txt.remove(re);

    QStringView view(txt);
    if (prefix.length() &&
        prefix.compare(view.left(prefix.length()), Qt::CaseInsensitive) == 0) {
        if (_cur.isValid()) {
            _cur.movePosition(prefix.length(),
                              QDocumentCursor::PreviousCharacter,
                              QDocumentCursor::KeepAnchor);
            _cur.removeSelectedText();
        }
    }

    e->write(txt);

    if (back) {
        auto c = e->cursor();
        c.movePosition(1, QDocumentCursor::PreviousCharacter);
        e->setCursor(c);
        auto cc = c;
        cc.movePosition(1, QDocumentCursor::PreviousCharacter);
        e->completionEngine()->complete(cc, QStringLiteral("("));
    }

    e->setFocus();

    _completing = false;
}

void QCodeCompletionWidget::showEvent(QShowEvent *e) {
    QListView::showEvent(e);

    verticalScrollBar()->setValue(0);
    horizontalScrollBar()->setValue(0);
}

void QCodeCompletionWidget::hideEvent(QHideEvent *e) {
    QListView::hideEvent(e);

    qDeleteAll(m_temps);
    m_temps.clear();
}

void QCodeCompletionWidget::focusInEvent(QFocusEvent *e) {
    QListView::focusInEvent(e);
}

void QCodeCompletionWidget::focusOutEvent(QFocusEvent *e) {
    QListView::focusOutEvent(e);

    hide();

    if (editor())
        editor()->setFocus();
}

void QCodeCompletionWidget::keyPressEvent(QKeyEvent *e) {
    if (!e)
        return;

    if (e->modifiers() &
        (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) {
        e->ignore();
        hide();

        if (editor())
            editor()->setFocus();

        return;
    }

    const QModelIndex &index = currentIndex();
    const int row = index.isValid() ? index.row() : -1;
    QString prefix = pModel->prefix(), text = e->text();

    switch (e->key()) {
    case Qt::Key_Escape:
        hide();

        if (editor())
            editor()->setFocus();

        e->accept();
        break;

    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Tab:
        // hide();

        if (pModel->rowCount()) {
            complete(currentIndex());
            e->accept();
        } else {
            e->ignore();

            if (editor())
                editor()->setFocus();
        }
        break;

    case Qt::Key_Up:

        if (row >= 0)
            setCurrentIndex(pModel->index(row - 1, 0));

        e->accept();

        break;

    case Qt::Key_Down:

        if (row < (pModel->rowCount(QModelIndex()) - 1))
            setCurrentIndex(pModel->index(row + 1, 0));

        e->accept();

        break;

    case Qt::Key_Backspace:

        if (prefix.length()) {
            prefix.chop(1);
            pModel->setPrefix(prefix);
            offset = -1 + PADDING;
            changed();
        } else {
            hide();

            if (editor())
                editor()->setFocus();
        }

        e->ignore();

        break;

    case Qt::Key_Shift:
    case Qt::Key_Alt:
    case Qt::Key_Control:
        e->ignore();
        break;

    default:

        if (text.length() && text.at(0).isPrint() && pModel->rowCount()) {
            pModel->setPrefix(prefix + text);
            offset = text.length() + PADDING;
            changed();
        } else {
            hide();

            if (editor())
                editor()->setFocus();
        }

        e->ignore();

        break;
    }
}

////////////////////////////////

QCodeCompletionModel::QCodeCompletionModel(QObject *p)
    : QAbstractListModel(p), bUpdate(false) {}

void QCodeCompletionModel::clear() {
    beginResetModel();
    setPrefix(QString());
    setFocusNodes(QList<QCodeNode *>());
    setFilter(QCodeCompletionWidget::KeepAll);

    m_visibles.clear();
    endResetModel();
}

QString QCodeCompletionModel::prefix() const {
    return QString::fromUtf8(m_prefix);
}

void QCodeCompletionModel::setPrefix(const QString &prefix) {
    m_prefix = prefix.toUtf8();
    emit prefixChanged(prefix);
    update();
}

QCodeCompletionWidget::Filter QCodeCompletionModel::filter() const {
    return m_filter;
}

void QCodeCompletionModel::setFilter(QCodeCompletionWidget::Filter filter) {
    m_filter = filter;
    emit filterChanged(filter);
    update();
}

void QCodeCompletionModel::update() { bUpdate = true; }

void QCodeCompletionModel::forceUpdate() const {
    m_visibles.clear();

    foreach (QCodeNode *n, m_nodes) {
        foreach (QCodeNode *c, n->children()) {
            if (match(c, m_filter, m_prefix)) {
                m_visibles << c;
            }

            if (c->type() == QCodeNode::Enum) {
                if (match(c, m_filter))
                    for (auto &ev : c->children())
                        if (match(ev, m_filter, m_prefix))
                            m_visibles << ev;
            }
        }
    }

    emit const_cast<QCodeCompletionModel *>(this)->layoutChanged();
    bUpdate = false;
    emit const_cast<QCodeCompletionModel *>(this)->changed();
}

QList<QCodeNode *> QCodeCompletionModel::focusNodes() const { return m_nodes; }

void QCodeCompletionModel::setFocusNodes(const QList<QCodeNode *> &l) {
    m_nodes.clear();

    foreach (QCodeNode *n, l)
        if (n && !m_nodes.contains(n))
            m_nodes << n;

    update();
}

QVariant QCodeCompletionModel::data(const QModelIndex &index, int role) const {
    if (bUpdate)
        forceUpdate();

    int row = index.row();

    if (!index.isValid() || row < 0 || row >= m_visibles.length()) {
        // qDebug("invalid input (row=%i)", row);

        return QVariant();
    }

    QCodeNode *n = m_visibles.at(row);
    int type = n->type();

    if (role == Qt::DisplayRole) {
        if (type == QCodeNode::Enumerator) {
            return n->parent()->data(role).toString() +
                   "::" + n->data(role).toString();
        }

        if (type == QCodeNode::Function || type == QCodeNode::Variable) {
            auto p = n->parent();
            if (p) {
                if (p->type() == QCodeNode::Class) {
                    return p->role(QCodeNode::Name) + QStringLiteral("::") +
                           n->data(role).toString();
                }
            }
        }
    }

    if (role == Qt::UserRole)
        role = Qt::DisplayRole;

    return n->data(role);
}

Qt::ItemFlags QCodeCompletionModel::flags(const QModelIndex &) const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant QCodeCompletionModel::headerData(int, Qt::Orientation, int) const {
    return QVariant();
}

int QCodeCompletionModel::rowCount(const QModelIndex &parent) const {
    if (bUpdate)
        forceUpdate();

    return parent.isValid() ? 0 : m_visibles.length();
}

bool QCodeCompletionModel::match(QCodeNode *n,
                                 QCodeCompletionWidget::Filter filter,
                                 const QByteArray &prefix) {
    QByteArray bcxt = n->parent()->qualifiedName(),
               bnn = n->role(QCodeNode::Name);

    if (!n ||
        (!prefix.isEmpty() && (prefix.compare(bnn.left(prefix.length()),
                                              Qt::CaseInsensitive) != 0))) {
        return false;
    }

    int type = n->type(), visibility = n->role(QCodeNode::Visibility).toInt(),
        specifiers = (type == QCodeNode::Variable)
                         ? n->role(QCodeNode::Specifiers).toInt()
                         : 0,
        qualifiers = (type == QCodeNode::Function)
                         ? n->role(QCodeNode::Qualifiers).toInt()
                         : 0;

    const char *ctxt = bcxt.constData(), *name = bnn.constData();

    int cxt_off = qMax(0, bcxt.lastIndexOf("::"));

    if ((((type == QCodeNode::Class) || (type == QCodeNode::Typedef)) &&
         !(filter & QCodeCompletionWidget::KeepSubTypes)) ||
        ((type == QCodeNode::Enum) &&
         !(filter & QCodeCompletionWidget::KeepEnums)) ||
        ((visibility == QCodeNode::VISIBILITY_PRIVATE) &&
         !(filter & QCodeCompletionWidget::Private)) ||
        ((visibility == QCodeNode::VISIBILITY_PROTECTED) &&
         !(filter & QCodeCompletionWidget::Protected)) ||
        ((visibility == QCodeNode::VISIBILITY_PUBLIC) &&
         !(filter & QCodeCompletionWidget::Public)) ||
        ((type == QCodeNode::Variable) &&
         ((!(filter & QCodeCompletionWidget::KeepStatic)) ||
          ((specifiers & QCodeNode::SPECIFIER_CONST) &&
           !(filter & QCodeCompletionWidget::KeepConst)) ||
          ((filter & QCodeCompletionWidget::IsStatic))
          //		||
          //			(!(specifiers & QCodeNode::SPECIFIER_CONST) &&
          //(filter & QCodeCompletionWidget::IsConst))
          )) ||
        ((type == QCodeNode::Function) &&
         ((!(filter & QCodeCompletionWidget::KeepStatic)) ||
          ((qualifiers & QCodeNode::QUALIFIER_CONST) &&
           !(filter & QCodeCompletionWidget::KeepConst)) ||
          ((filter & QCodeCompletionWidget::IsStatic) &&
           (n->parent()->type() != QCodeNode::Namespace)) ||
          (!(qualifiers & QCodeNode::QUALIFIER_CONST) &&
           (filter & QCodeCompletionWidget::IsConst)) ||
          (!qstrcmp(name, ctxt + cxt_off) &&
           !(filter & QCodeCompletionWidget::KeepCtor)) ||
          ((*name == '~') && !qstrcmp(name + 1, ctxt) &&
           !(filter & QCodeCompletionWidget::KeepDtor))))) {
        return false;
    }

    return true;
}
