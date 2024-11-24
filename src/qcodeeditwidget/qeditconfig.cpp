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

#include "qeditconfig.h"

/*!
        \file qeditconfig.cpp
        \brief Implementation of the QEditConfig class.

        \see QEditConfig
*/

#include "class/langservice.h"
#include "class/settingmanager.h"
#include "qdocument.h"
#include "qdocument_p.h"
#include "qeditor.h"
#include "ui_qeditconfig.h"
#include "utilities.h"

#include <QDebug>

/*!
        \ingroup dialogs
        @{

        \class QEditConfig
        \brief A minimalistic, easy to embed, settings widget.

*/

QEditConfig::QEditConfig(QWidget *w)
    : WingHex::SettingPage(w), ui(new Ui::QEditConfig()) {
    ui->setupUi(this);

    QFile code(QStringLiteral(":/com.wingsummer.winghex/src/TESTCODE.as"));
    auto ret = code.open(QFile::ReadOnly);
    Q_ASSERT(ret);
    Q_UNUSED(ret);

    auto cbuffer = code.readAll();
    _edit = new QEditor(this);
    _edit->setText(QString::fromUtf8(cbuffer));
    LangService::instance().applyLanguageSerivce(_edit);

    _edit->setFlag(QEditor::ReadOnly, true);
    _edit->setUndoRedoEnabled(false);
    ui->layoutFont->addWidget(_edit, ui->layoutFont->rowCount(), 0, 1,
                              ui->layoutFont->columnCount());
    reload();
}

QEditConfig::~QEditConfig() { delete ui; }

/*!
        \brief Apply changes
*/
void QEditConfig::apply() {
    QFont font = ui->cbFont->currentFont();
    font.setPointSize(ui->spnFontSize->value());

    QEditor::setDefaultFont(font);
    QEditor::setDefaultTabStop(ui->spnTabWidth->value());

    if (ui->chkDetectLE->isChecked()) {
        QEditor::setDefaultLineEnding(QDocument::Conservative);
    } else {
        QEditor::setDefaultLineEnding(
            QDocument::LineEnding(ui->cbLineEndings->currentIndex() + 1));
    }

    QDocument::WhiteSpaceMode ws = QDocument::ShowNone;

    if (ui->chkShowLeadingWhitespace->isChecked())
        ws |= QDocument::ShowLeading;

    if (ui->chkShowTrailingWhitespace->isChecked())
        ws |= QDocument::ShowTrailing;

    if (ui->chkShowTabsInText->isChecked())
        ws |= QDocument::ShowTabs;

    QEditor::setDefaultShowSpaces(ws);

    int flags = QEditor::defaultFlags();

    if (ui->chkReplaceTabs->isChecked())
        flags |= QEditor::ReplaceTabs;
    else
        flags &= ~QEditor::ReplaceTabs;

    if (ui->chkAutoRemoveTrailingWhitespace->isChecked())
        flags |= QEditor::RemoveTrailing;
    else
        flags &= ~QEditor::RemoveTrailing;

    if (ui->chkPreserveTrailingIndent->isChecked())
        flags |= QEditor::PreserveTrailingIndent;
    else
        flags &= ~QEditor::PreserveTrailingIndent;

    QEditor::setDefaultFlags(flags);

    SettingManager::instance().save(SettingManager::CODEEDIT);
}

/*!
        \brief Reset the subcontrols to reflect the current settings

        The name can be a bit misleading at first, it has been chosen
        because it directly maps to the effect a "cancel" button would
        have on the widget
*/
void QEditConfig::cancel() { reload(); }

/*!
        \brief Restore default values for all subcontrols

        \note The widgets are changed but these changes are NOT applied.
*/
void QEditConfig::reset() {
    // restore default configuration
    auto font = qApp->font();
    ui->cbFont->setFont(font);
    ui->spnFontSize->setValue(font.pointSize());

    ui->spnTabWidth->setValue(4);

    ui->chkShowTabsInText->setChecked(true);
    ui->chkShowLeadingWhitespace->setChecked(true);
    ui->chkShowTrailingWhitespace->setChecked(true);

    ui->chkDetectLE->setChecked(true);
    ui->cbLineEndings->setCurrentIndex(0);

    ui->chkReplaceTabs->setChecked(true);
    ui->chkAutoRemoveTrailingWhitespace->setChecked(true);
    ui->chkPreserveTrailingIndent->setChecked(false);

    apply();
}

void QEditConfig::reload() {
    // reload the current config
    auto dfont = QEditor::defaultFont();
    ui->cbFont->setFont(dfont);
    ui->spnFontSize->setValue(dfont.pointSize());

    ui->spnTabWidth->setValue(QEditor::defaultTabStop());

    auto ss = QEditor::defaultShowSpaces();
    ui->chkShowTabsInText->setChecked(ss.testFlag(QDocument::ShowTabs));
    ui->chkShowLeadingWhitespace->setChecked(
        ss.testFlag(QDocument::ShowLeading));
    ui->chkShowTrailingWhitespace->setChecked(
        ss.testFlag(QDocument::ShowTrailing));

    auto le = QEditor::defaultLineEnding();
    ui->chkDetectLE->setChecked(le == QDocument::Conservative);
    ui->cbLineEndings->setCurrentIndex(le ? le - 1 : 0);

    int flag = QEditor::defaultFlags();
    ui->chkReplaceTabs->setChecked(flag & QEditor::ReplaceTabs);
    ui->chkAutoRemoveTrailingWhitespace->setChecked(flag &
                                                    QEditor::RemoveTrailing);
    ui->chkPreserveTrailingIndent->setChecked(flag &
                                              QEditor::PreserveTrailingIndent);
}

QIcon QEditConfig::categoryIcon() const { return ICONRES("file"); }

QString QEditConfig::name() const { return tr("Edit"); }

QString QEditConfig::id() const { return QStringLiteral("Edit"); }

/*!
        \brief Slot used to apply font size settings
*/
void QEditConfig::on_spnFontSize_valueChanged(int size) {
    QFont font = ui->cbFont->currentFont();
    font.setPointSize(size);
    _edit->setFont(font);
}

/*!
        \brief Slot used to apply font family settings
*/
void QEditConfig::on_cbFont_currentFontChanged(QFont font) {
    font.setPointSize(ui->spnFontSize->value());
    _edit->setFont(font);
}

/*!
        \brief Slot used to apply tab width settings
*/
void QEditConfig::on_spnTabWidth_valueChanged(int n) {
    _edit->document()->setTabStop(n);
}

/*!
        \brief Slot used to apply tabs replacement settings
*/
void QEditConfig::on_chkReplaceTabs_toggled(bool y) {
    _edit->setFlag(QEditor::ReplaceTabs, y);
}

/*!
        \brief Slot used to apply tabs display settings
*/
void QEditConfig::on_chkShowTabsInText_toggled(bool y) {
    auto &doc_ps = QDocumentPrivate::m_documents;
    if (y) {
        for (auto &pdoc : doc_ps) {
            auto doc = pdoc->m_doc;
            doc->setShowSpaces(doc->showSpaces() | QDocument::ShowTabs);
        }
    } else {
        for (auto &pdoc : doc_ps) {
            auto doc = pdoc->m_doc;
            doc->setShowSpaces(doc->showSpaces() & ~QDocument::ShowTabs);
        }
    }
}

/*!
        \brief Slot used to apply trailing whitespace display settings
*/
void QEditConfig::on_chkShowLeadingWhitespace_toggled(bool y) {
    auto &doc_ps = QDocumentPrivate::m_documents;
    if (y) {
        for (auto &pdoc : doc_ps) {
            auto doc = pdoc->m_doc;
            doc->setShowSpaces(doc->showSpaces() | QDocument::ShowLeading);
        }
    } else {
        for (auto &pdoc : doc_ps) {
            auto doc = pdoc->m_doc;
            doc->setShowSpaces(doc->showSpaces() & ~QDocument::ShowLeading);
        }
    }
}

/*!
        \brief Slot used to apply leading whitespace display settings
*/
void QEditConfig::on_chkShowTrailingWhitespace_toggled(bool y) {
    auto &doc_ps = QDocumentPrivate::m_documents;
    if (y) {
        for (auto &pdoc : doc_ps) {
            auto doc = pdoc->m_doc;
            doc->setShowSpaces(doc->showSpaces() | QDocument::ShowTrailing);
        }
    } else {
        for (auto &pdoc : doc_ps) {
            auto doc = pdoc->m_doc;
            doc->setShowSpaces(doc->showSpaces() & ~QDocument::ShowTrailing);
        }
    }
}

void QEditConfig::on_cbLineEndings_currentIndexChanged(int idx) {
    QDocument::LineEnding le = QDocument::LineEnding(idx + 1);
    _edit->document()->setLineEnding(le);
}

/*!
        \brief Slot used to apply line endings auto detectionl settings
*/
void QEditConfig::on_chkDetectLE_toggled(bool y) {
    QDocument::LineEnding le = QDocument::Conservative;

    if (!y) {
        le = QDocument::LineEnding(ui->cbLineEndings->currentIndex() + 1);
    }

    _edit->document()->setLineEnding(le);
}

/*!
        \brief Slot used to apply trailing space removal settings
*/
void QEditConfig::on_chkAutoRemoveTrailingWhitespace_toggled(bool y) {
    _edit->setFlag(QEditor::RemoveTrailing, y);
}

/*!
        \brief Slot used to indent preservation settings
*/
void QEditConfig::on_chkPreserveTrailingIndent_toggled(bool y) {
    _edit->setFlag(QEditor::PreserveTrailingIndent, y);
}

/*! @} */
