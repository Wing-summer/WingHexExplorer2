#include "qsearchreplacepanel.h"

#include "qeditor.h"

#include "class/qdocumentsearch.h"
#include "qdocument.h"
#include "qdocumentcursor.h"
#include "qdocumentline.h"

#include <QPaintEvent>
#include <QScrollBar>

#include "ui_qsearchreplacepanel.h"

static QString escapeCpp(const QString &s, bool rep) {
    if (!rep)
        return s;

    QString es;

    for (int i = 0; i < s.size(); ++i) {
        if ((s.at(i) == '\\') && ((i + 1) < s.size())) {
            QChar c = s.at(++i);

            if (c == '\\')
                es += QChar('\\');
            else if (c == 't')
                es += QChar('\t');
            else if (c == 'n')
                es += QChar('\n');
            else if (c == 'r')
                es += QChar('\r');
            else if (c == '0')
                es += QChar('\0');

        } else {
            es += s.at(i);
        }
    }

    // qDebug("\"%s\" => \"%s\"", qPrintable(s), qPrintable(es));

    return es;
}

/*!
        \ingroup widgets
        @{
*/

/*!
        \class QSearchReplacePanel
        \brief A panel that provide inline search/replace functionalities
*/

QCE_AUTO_REGISTER(QSearchReplacePanel)

/*!
        \brief Constructor
*/
QSearchReplacePanel::QSearchReplacePanel(QWidget *p)
    : QPanel(p), ui(new Ui::QSearchReplacePanel()), lastDirection(0),
      m_search(0) {
    ui->setupUi(this);
    setDefaultVisibility(false);

    ui->frameReplace->setAttribute(Qt::WA_TranslucentBackground);
    ui->frameSearch->setAttribute(Qt::WA_TranslucentBackground);

    ui->label->setAttribute(Qt::WA_TranslucentBackground);

    ui->leFind->installEventFilter(this);
    ui->leReplace->installEventFilter(this);
}

/*!
        \brief Empty destructor
*/
QSearchReplacePanel::~QSearchReplacePanel() {
    if (m_search)
        delete m_search;
    delete ui;
}

/*!

*/
QString QSearchReplacePanel::type() const { return QStringLiteral("Search"); }

/*!
        \brief
*/
void QSearchReplacePanel::editorChange(QEditor *e) {
    if (editor()) {
        disconnect(editor(), SIGNAL(cursorPositionChanged()), this,
                   SLOT(cursorPositionChanged()));
    }

    if (e) {
        connect(e, SIGNAL(cursorPositionChanged()), this,
                SLOT(cursorPositionChanged()));
    }
}

bool QSearchReplacePanel::forward(QMouseEvent *e) {
    Q_UNUSED(e)

    /*
            This panel does not need mouse events to be forwarded to the editor.
            Even more, it requires them not to be forwarded...
    */
    return false;
}

/*!

*/
void QSearchReplacePanel::display(int mode, bool replace) {
    // qDebug("display(%i)", replace);
    bool visible = true;

    if (mode < 0)
        visible = (replace != ui->cbReplace->isChecked()) || isHidden();
    else
        visible = mode;

    if (visible) {
        ui->cbReplace->setChecked(replace);
        // frameReplace->setVisible(replace);
        ui->leFind->setFocus();
        ui->leFind->selectAll();
        // show();
    }

    setVisible(visible);

    if (!visible)
        editor()->setFocus();
}

/*!

*/
void QSearchReplacePanel::find(int backward) {
    if (!m_search) {
        if (!isVisible()) {
            display(1, false);
            return;
        } else {
            init();
        }

        if (backward != -1)
            lastDirection = backward;
    }

    bool replaceAll =
        ui->cbReplace->isChecked() && ui->cbReplaceAll->isChecked();

    if (backward == -1) {
        backward = lastDirection;
    } else {
        if (lastDirection != backward && editor()->cursor().hasSelection() &&
            !replaceAll)
            m_search->next(backward, false); // the first hit is already
                                             // selected

        lastDirection = backward;
    }

    m_search->next(backward, replaceAll);

    if (!ui->leFind->hasFocus() && !ui->leReplace->hasFocus())
        ui->leFind->setFocus();
}

/*!

*/
void QSearchReplacePanel::hideEvent(QHideEvent *) {
    /*
    if ( m_search )
            delete m_search;

    m_search = 0;
    */
}

bool QSearchReplacePanel::eventFilter(QObject *o, QEvent *e) {
    int kc;

    if (o == ui->leFind || o == ui->leReplace) {
        switch (e->type()) {
            /*
            case QEvent::FocusIn :
                    leFind->grabKeyboard();
                    break;

            case QEvent::FocusOut :
                    leFind->releaseKeyboard();
                    break;
                    */

        case QEvent::KeyPress:

            kc = static_cast<QKeyEvent *>(e)->key();

            if ((kc == Qt::Key_Enter) || (kc == Qt::Key_Return)) {
                // on_leFind_returnPressed();
                on_leFind_returnPressed(
                    Qt::ShiftModifier &
                    static_cast<QKeyEvent *>(e)->modifiers());
                return true;
            } else if (kc == Qt::Key_Escape) {
                if (ui->cbReplace->isChecked())
                    display(1, false);
                else
                    display(0, false);
                return true;
            } else if (kc == Qt::Key_Tab || kc == Qt::Key_Backtab) {
                if (ui->cbReplace->isChecked()) {
                    if (ui->leFind->hasFocus())
                        ui->leReplace->setFocus();
                    else
                        ui->leFind->setFocus();
                }
                return true;
            }
            break;

        default:
            break;
        }
    }

    return QWidget::eventFilter(o, e);
}

void QSearchReplacePanel::on_leFind_textEdited(const QString &text) {
    if (ui->cbReplace->isChecked()) {
        // do not perfrom incremental search when replacing

        if (m_search)
            m_search->setSearchText(text);

        ui->leFind->setStyleSheet(QString());
        return;
    }

    bool hadSearch = m_search;
    QDocumentCursor cur = editor()->cursor();

    if (m_search) {
        cur = m_search->cursor();

        m_search->setSearchText(text);

        if (ui->cbCursor->isChecked()) {
            QDocumentCursor c = cur;
            c.setColumnNumber(qMin(c.anchorColumnNumber(), c.columnNumber()));

            m_search->setCursor(c);
        }
    } else {
        // TODO : make incremental search optional
        init();
    }

    if (text.isEmpty()) {
        ui->leFind->setStyleSheet(QString());
        return;
    }

    m_search->setOption(QDocumentSearch::Silent, true);

    find(0);

    m_search->setOption(QDocumentSearch::Silent, false);

    if (m_search->cursor().isNull()) {
        ui->leFind->setStyleSheet(
            "QLineEdit { background: red; color : white; }");

        if (hadSearch) {
            m_search->setCursor(cur);

            // figure out whether other matches are availables
            QDocumentSearch::Options opts = m_search->options();
            opts &= ~QDocumentSearch::HighlightAll;
            opts |= QDocumentSearch::Silent;

            QDocumentSearch temp(editor(), text, opts);
            temp.setOrigin(QDocumentCursor());
            temp.setScope(m_search->scope());
            temp.next(true);

            if (temp.cursor().isValid()) {
                // other match found from doc start
                ui->leFind->setStyleSheet(
                    "QLineEdit { background: yellow; color : black; }");
                m_search->setCursor(cur.document()->cursor(0, 0));
                find(0);
            }
        }
    } else {
        ui->leFind->setStyleSheet(QString());
        editor()->setCursor(m_search->cursor());
    }
}

void QSearchReplacePanel::on_leFind_returnPressed(bool backward) {
    ui->leFind->setStyleSheet(QString());

    if (backward)
        find(1);
    else
        find(0);
}

void QSearchReplacePanel::on_leReplace_textEdited(const QString &text) {
    if (m_search)
        m_search->setReplaceText(text);
}

void QSearchReplacePanel::on_cbReplace_toggled(bool on) {
    if (m_search)
        m_search->setOption(QDocumentSearch::Replace, on);

    if (ui->leFind->isVisible())
        ui->leFind->setFocus();
}

void QSearchReplacePanel::on_cbWords_toggled(bool on) {
    if (m_search)
        m_search->setOption(QDocumentSearch::WholeWords, on);

    ui->leFind->setFocus();
}

void QSearchReplacePanel::on_cbRegExp_toggled(bool on) {
    if (m_search)
        m_search->setOption(QDocumentSearch::RegExp, on);

    ui->leFind->setFocus();
}

void QSearchReplacePanel::on_cbCase_toggled(bool on) {
    if (m_search)
        m_search->setOption(QDocumentSearch::CaseSensitive, on);

    ui->leFind->setFocus();
}

void QSearchReplacePanel::on_cbCursor_toggled(bool on) {
    if (m_search) {
        m_search->setOrigin(on ? editor()->cursor() : QDocumentCursor());

        if (ui->cbHighlight->isChecked())
            m_search->next(false);
    }

    ui->leFind->setFocus();
}

void QSearchReplacePanel::on_cbHighlight_toggled(bool on) {
    if (!m_search)
        init();

    if (m_search) {
        m_search->setOption(QDocumentSearch::HighlightAll, on);

        if (on && !m_search->indexedMatchCount())
            m_search->next(false);
    }

    ui->leFind->setFocus();
}

void QSearchReplacePanel::on_cbSelection_toggled(bool on) {
    if (m_search)
        m_search->setScope(on ? editor()->cursor() : QDocumentCursor());

    ui->leFind->setFocus();
}

void QSearchReplacePanel::on_cbPrompt_toggled(bool on) {
    if (m_search)
        m_search->setOption(QDocumentSearch::Prompt, on);

    ui->leFind->setFocus();
}

void QSearchReplacePanel::on_cbEscapeSeq_toggled(bool on) {
    if (m_search)
        m_search->setReplaceText(escapeCpp(ui->leReplace->text(), on));
}

void QSearchReplacePanel::on_bNext_clicked() {
    if (!m_search)
        init();

    ui->leFind->setStyleSheet(QString());
    find(0);
}

void QSearchReplacePanel::on_bPrevious_clicked() {
    if (!m_search)
        init();

    ui->leFind->setStyleSheet(QString());
    find(1);
}

void QSearchReplacePanel::on_bRefresh_clicked() { init(); }

void QSearchReplacePanel::init() {
    if (m_search) {
        delete m_search;
        m_search = 0;
    }

    QDocumentSearch::Options opt;

    if (ui->cbRegExp->isChecked())
        opt |= QDocumentSearch::RegExp;

    if (ui->cbCase->isChecked())
        opt |= QDocumentSearch::CaseSensitive;

    if (ui->cbWords->isChecked())
        opt |= QDocumentSearch::WholeWords;

    if (ui->cbHighlight->isChecked() && !ui->cbReplace->isVisible())
        opt |= QDocumentSearch::HighlightAll;

    if (ui->cbReplace->isChecked() && ui->cbReplace->isVisible())
        opt |= QDocumentSearch::Replace;

    if (ui->cbPrompt->isChecked())
        opt |= QDocumentSearch::Prompt;

    m_search = new QDocumentSearch(
        editor(), ui->leFind->text(), opt,
        ui->cbReplace->isChecked()
            ? escapeCpp(ui->leReplace->text(), ui->cbEscapeSeq->isChecked())
            : QString());

    if (ui->cbCursor->isChecked())
        m_search->setCursor(editor()->cursor());

    if (ui->cbSelection->isChecked())
        m_search->setScope(editor()->cursor());
}

void QSearchReplacePanel::cursorPositionChanged() {
    if (m_search) {
        if (editor()->cursor() == m_search->cursor() ||
            ui->cbHighlight->isChecked())
            return;

        if (ui->cbCursor->isChecked())
            m_search->setOrigin(editor()->cursor());

        m_search->setCursor(editor()->cursor());
    }
}

void QSearchReplacePanel::on_bClose_clicked() {
    hide();
    editor()->setFocus();
}
