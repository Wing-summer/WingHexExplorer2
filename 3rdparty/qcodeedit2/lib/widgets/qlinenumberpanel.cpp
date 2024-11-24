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

#include "qlinenumberpanel.h"

/*!
        \file qlinenumberpanel.cpp
        \brief Implementation of the QLineNumberPanel class

        \see QLineNumberPanel
*/

#include <QPainter>
#include <QScrollBar>

#include "qdocument.h"
#include "qdocument_p.h"
#include "qdocumentline.h"
#include "qeditor.h"

/*!
        \ingroup widgets
        @{
*/

/*!
        \class QLineNumberPanel
        \brief A specific panel in charge of drawing line numbers of an editor
*/

QCE_AUTO_REGISTER(QLineNumberPanel)

/*!
        \brief Constructor
*/
QLineNumberPanel::QLineNumberPanel(QWidget *p) : QPanel(p), m_verbose(false) {
    setFixedWidth(20);
    setObjectName("lineNumberPanel");
}

/*!
        \brief Empty destructor
*/
QLineNumberPanel::~QLineNumberPanel() {}

/*!

*/
QString QLineNumberPanel::type() const {
    return QStringLiteral("Line numbers");
}

QString QLineNumberPanel::name() const { return tr("Line Number Panel"); }

/*!

*/
bool QLineNumberPanel::isVerboseMode() const { return m_verbose; }

/*!

*/
void QLineNumberPanel::setVerboseMode(bool y) {
    m_verbose = y;
    update();
}

/*!

*/
void QLineNumberPanel::editorChange(QEditor *e) {
    if (editor()) {
        disconnect(editor(), SIGNAL(cursorPositionChanged()), this,
                   SLOT(update()));
        disconnect(editor()->document(), SIGNAL(fontChanged(QFont)), this,
                   SLOT(setFont_slot(QFont)));
    }

    if (e) {
        setFixedWidth(fontMetrics().horizontalAdvance(
                          QString::number(e->document()->lines())) +
                      5);

        connect(e, &QEditor::cursorPositionChanged, this,
                QOverload<>::of(&QLineNumberPanel::update));

        connect(e->document(), &QDocument::fontChanged, this,
                &QLineNumberPanel::setFont);
    }
}

/*!

*/
void QLineNumberPanel::paint(QPainter *p, QEditor *e) {
    /*
                    possible Unicode caracter for wrapping arrow :
                            0x21B3
                            0x2937
            */

    QFont f(font());
    // f.setWeight(QFont::Bold);
    const QFontMetrics sfm(f);
    bool specialFontUsage = false;
    QFont specialFont(font());

#ifndef WIN32
    static const QChar wrappingArrow(0x2937);
    const QFontMetrics specialSfm(sfm);
#if QT_VERSION >= 0x050000 && defined Q_OS_MAC && (QT_VERSION < 0x050200)
    if (!specialSfm.inFont(wrappingArrow)) {
        specialFontUsage = true;
        specialFont.setFamily("Gothic Regular");
        // specialSfm(specialFont);
    }
#endif
#else
    // 0xC4 gives a decent wrapping arrow in Wingdings fonts, availables on all
    // windows systems
    // this is a hackish fallback to workaround Windows issues with Unicode...
    static const QChar wrappingArrow(0xC4);
    specialFont.setFamily("Wingdings");
    const QFontMetrics specialSfm(specialFont);
    specialFontUsage = true;
#endif

    int max = e->document()->lines();
    if (max < 100)
        max = 100; // always reserve 3 line number columns to avoid ugly jumping
                   // of width
    QString s_width = QString::number(max);
    s_width.fill('6');
    const int panelWidth = sfm.horizontalAdvance(s_width) + 5;
    setFixedWidth(panelWidth);

    int n, posY, as = QFontMetrics(e->document()->font()).ascent(),
                 ls = e->document()->lineSpacing(),
                 pageBottom = e->viewport()->height(),
                 contentsY = e->verticalOffset();

    QString txt;
    QDocument *d = e->document();
    const int cursorLine = e->cursor().lineNumber();

    n = d->lineNumber(contentsY);
    posY = as + 2 + d->y(n) - contentsY;

    // qDebug("first = %i; last = %i", first, last);
    // qDebug("beg pos : %i", posY);

    for (;; ++n) {
        // qDebug("n = %i; pos = %i", n, posY);
        QDocumentLine line = d->line(n);

        if (line.isNull() || ((posY - as) > pageBottom))
            break;

        if (line.isHidden())
            continue;

        bool draw = true;

        if (!m_verbose) {
            draw = !((n + 1) % 10) || !n || !line.marks().empty();
        }

        txt = QString::number(n + 1);

        if (n == cursorLine) {
            draw = true;

            p->save();
            QFont f = p->font();
            f.setBold(true);
            f.setUnderline(true);
            p->setFont(f);
        }

        if (draw) {
            p->drawText(width() - 2 - sfm.horizontalAdvance(txt), posY, txt);
            if (specialFontUsage) {
                if (line.lineSpan() > 1) {
                    p->save();
                    specialFont.setBold(n ==
                                        cursorLine); // todo: only get bold on
                                                     // the current wrapped line
                    p->setFont(specialFont);
                }
            }

            for (int i = 1; i < line.lineSpan(); ++i)
                p->drawText(width() - 2 -
                                specialSfm.horizontalAdvance(wrappingArrow),
                            posY + i * ls, wrappingArrow);

            if (specialFontUsage) {
                if (line.lineSpan() > 1)
                    p->restore();
            }
        } else {
            int yOff = posY - (as + 1) + ls / 2;

            if ((n + 1) % 5)
                p->drawPoint(width() - 5, yOff);
            else
                p->drawLine(width() - 7, yOff, width() - 2, yOff);
        }

        if (n == cursorLine) {
            p->restore();
        }

        posY += ls * line.lineSpan();
    }

    // p->setPen(Qt::DotLine);
    // p->drawLine(width()-1, 0, width()-1, pageBottom);

    // setFixedWidth(sfm.width(txt) + 5);
}

/*! @} */

QSize QLineNumberPanel::sizeHint() const { return QSize(_fixWidth, 0); }
