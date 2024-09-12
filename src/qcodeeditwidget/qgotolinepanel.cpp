#include "qgotolinepanel.h"

#include "qeditor.h"

#include "qdocument.h"
#include "qdocumentcursor.h"
#include "qdocumentline.h"

#include <QKeyEvent>

#include "utilities.h"

#include "ui_qgotolinepanel.h"

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
QGotoLinePanel::QGotoLinePanel(QWidget *p)
    : QPanel(p), ui(new Ui::QGotoLinePanel()) {
    ui->setupUi(this);
    setDefaultVisibility(false);

    ui->label->setAttribute(Qt::WA_TranslucentBackground);
    ui->slLine->setAttribute(Qt::WA_TranslucentBackground);

    ui->bClose->setStyleSheet(
        QStringLiteral("border:none;background:transparent;"));
    ui->bClose->setIcon(ICONRES("closefile"));
}

/*!
        \brief Empty destructor
*/
QGotoLinePanel::~QGotoLinePanel() { delete ui; }

/*!

*/
QString QGotoLinePanel::type() const { return QStringLiteral("Goto"); }

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
        ui->spLine->setValue(e->cursor().lineNumber() + 1);
        ui->slLine->setValue(e->cursor().lineNumber() + 1);
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

    ui->spLine->setFocus();
    ui->spLine->selectAll();
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
        QDocumentCursor(editor()->document(), ui->spLine->value() - 1));
}

void QGotoLinePanel::on_spLine_valueChanged(int v) {
    if (ui->slLine->value() != v)
        ui->slLine->setValue(v);
}

void QGotoLinePanel::on_slLine_valueChanged(int v) {
    if (ui->spLine->value() != v)
        ui->spLine->setValue(v);
}

void QGotoLinePanel::lineCountChanged(int n) {
    ui->spLine->setMaximum(n);
    ui->slLine->setMaximum(n);
}

void QGotoLinePanel::cursorPositionChanged() {
    ui->spLine->setValue(editor()->cursor().lineNumber() + 1);
    ui->slLine->setValue(editor()->cursor().lineNumber() + 1);
}
