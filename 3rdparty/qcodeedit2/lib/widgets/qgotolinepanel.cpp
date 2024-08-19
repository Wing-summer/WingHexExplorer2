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

#include "qgotolinepanel.h"

/*!
        \file qgotolinepanel.cpp
        \brief Implementation of the QGotoLinePanel class.

        \see QGotoLinePanel
*/

#include "qeditor.h"

#include "qdocument.h"
#include "qdocumentcursor.h"
#include "qdocumentline.h"

#include <QKeyEvent>

/*!
        \ingroup widgets
        @{
*/

/*!
        \class QGotoLinePanel
        \brief A panel that provide inline goto line functionalities
*/

QCE_AUTO_REGISTER(QGotoLinePanel)

/*!
        \brief Constructor
*/
QGotoLinePanel::QGotoLinePanel(QWidget *p) : QPanel(p) {
    setupUi(this);
    setDefaultVisibility(false);

    bClose->setIcon(QPixmap(":/closeall.png"));
}

/*!
        \brief Empty destructor
*/
QGotoLinePanel::~QGotoLinePanel() {}

/*!

*/
QString QGotoLinePanel::type() const { return "Goto"; }

/*!
        \brief
*/
void QGotoLinePanel::editorChange(QEditor *e) {
    if (editor()) {
        disconnect(editor(), SIGNAL(cursorPositionChanged()), this,
                   SLOT(cursorPositionChanged()));

        disconnect(editor()->document(), SIGNAL(lineCountChanged(int)), this,
                   SLOT(lineCountChanged(int)));
    }

    if (e) {
        connect(e, SIGNAL(cursorPositionChanged()), this,
                SLOT(cursorPositionChanged()));

        connect(e->document(), SIGNAL(lineCountChanged(int)), this,
                SLOT(lineCountChanged(int)));

        lineCountChanged(e->document()->lineCount());
        spLine->setValue(e->cursor().lineNumber() + 1);
        slLine->setValue(e->cursor().lineNumber() + 1);
    }
}

bool QGotoLinePanel::forward(QMouseEvent *e) {
    Q_UNUSED(e)

    /*
            This panel does not need mouse events to be forwarded to the editor.
            Even more, it requires them not to be forwarded...
    */
    return false;
}

void QGotoLinePanel::showEvent(QShowEvent *e) {
    Q_UNUSED(e)

    spLine->setFocus();
    spLine->selectAll();
}

void QGotoLinePanel::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape) {
        on_bClose_clicked();
    } else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        on_bGo_clicked();
    } else {
        QPanel::keyPressEvent(e);
    }
}

void QGotoLinePanel::on_bClose_clicked() {
    hide();
    editor()->setFocus();
}

void QGotoLinePanel::on_bGo_clicked() {
    editor()->setCursor(
        QDocumentCursor(editor()->document(), spLine->value() - 1));
}

void QGotoLinePanel::on_spLine_valueChanged(int v) {
    if (slLine->value() != v)
        slLine->setValue(v);
}

void QGotoLinePanel::on_slLine_valueChanged(int v) {
    if (spLine->value() != v)
        spLine->setValue(v);
}

void QGotoLinePanel::lineCountChanged(int n) {
    spLine->setMaximum(n);
    slLine->setMaximum(n);
}

void QGotoLinePanel::cursorPositionChanged() {
    spLine->setValue(editor()->cursor().lineNumber() + 1);
    slLine->setValue(editor()->cursor().lineNumber() + 1);
}

/*! @} */
