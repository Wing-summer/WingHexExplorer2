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

#include "qsnippetedit.h"
#include "ui_qsnippetedit.h"

/*!
        \file qsnippetedit.cpp
        \brief Implementation of the QSnippetEdit widget
*/

#include "qsnippet.h"
#include "qsnippetmanager.h"

#include <QMessageBox>

/*!
        \class QSnippetEdit
        \brief A widget for snippets management


*/

QSnippetEdit::QSnippetEdit(QWidget *p)
    : WingHex::SettingPage(p), ui(new Ui::QSnippetEdit), m_editedSnippet(-1),
      m_manager(nullptr) {
    ui->setupUi(this);
    setEnabled(false);
}

QSnippetEdit::QSnippetEdit(QSnippetManager *mgr, QWidget *p)
    : WingHex::SettingPage(p), ui(new Ui::QSnippetEdit), m_editedSnippet(-1),
      m_manager(nullptr) {
    ui->setupUi(this);
    setSnippetManager(mgr);
}

QSnippetManager *QSnippetEdit::snippetManager() const { return m_manager; }

QIcon QSnippetEdit::categoryIcon() const { return QIcon(); }

QString QSnippetEdit::name() const { return tr("Snippets"); }

QString QSnippetEdit::id() const { return QStringLiteral("Snippets"); }

void QSnippetEdit::apply() {}

void QSnippetEdit::reset() {}

void QSnippetEdit::cancel() {}

void QSnippetEdit::setSnippetManager(QSnippetManager *mgr) {
    if (m_manager) {
        disconnect(m_manager, SIGNAL(snippetAdded(QSnippet *)), this,
                   SLOT(snippetAdded(QSnippet *)));

        disconnect(m_manager, SIGNAL(snippetRemoved(int)), this,
                   SLOT(snippetRemoved(int)));

        QListWidgetItem *empty = ui->lwSnippets->takeItem(0);
        ui->lwSnippets->clear();
        ui->lwSnippets->addItem(empty);
    }

    m_manager = mgr;
    setEnabled(mgr);

    if (m_manager) {
        connect(m_manager, &QSnippetManager::snippetAdded, this,
                &QSnippetEdit::snippetAdded);

        connect(m_manager, &QSnippetManager::snippetRemovedByIndex, this,
                &QSnippetEdit::snippetRemoved);

        for (int i = 0; i < m_manager->snippetCount(); ++i) {
            ui->lwSnippets->addItem(m_manager->snippet(i)->name());
        }
    }

    ui->lwSnippets->setCurrentItem(0);
}

void QSnippetEdit::snippetRemoved(int i) {
    delete ui->lwSnippets->takeItem(i + 1);
}

void QSnippetEdit::snippetAdded(QSnippet *s) {
    ui->lwSnippets->addItem(s->name());
}

static const QRegularExpression _cxt_splitter("\\s*,\\s*");

bool QSnippetEdit::maybeSave() {
    static const QRegularExpression nonTrivial("\\S");

    QString pattern = ui->eSnippet->text();

    if (pattern.endsWith('\n'))
        pattern.chop(1);

    // QString name = ui->leSnippetName->text();
    // QStringList contexts = ui->leSnippetScope->text().split(_cxt_splitter);
    bool nonTrivialPattern = pattern.contains(nonTrivial);

    if (m_editedSnippet >= 0) {
        QSnippet *snip = m_manager->snippet(m_editedSnippet);

        if (snip->pattern() != pattern) {
            int ret = QMessageBox::warning(
                this, tr("Unsaved changes"),
                tr("Do you want to save pattern changes to snippet %1 ?")
                    .arg(snip->name()),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                QMessageBox::Yes);

            if (ret == QMessageBox::Cancel) {
                return true;
            } else if (ret == QMessageBox::Yes) {
                snip->setPattern(pattern);
            }
        }
    } else if (nonTrivialPattern) {
        int ret = QMessageBox::warning(
            this, tr("Unsaved changes"),
            tr("The current snippet data will be discarded. Do you want to "
               "continue?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        if (ret == QMessageBox::No) {
            return true;
        }
    }

    return false;
}

void QSnippetEdit::on_lwSnippets_currentRowChanged(int idx) {
    if ((idx - 1) == m_editedSnippet)
        return;

    if (maybeSave()) {
        ui->lwSnippets->setCurrentRow(m_editedSnippet);
        return;
    }

    m_editedSnippet = idx - 1;

    if (idx <= 0) {
        ui->eSnippet->setText(QString());
        ui->leSnippetName->setText(QString());
        ui->leSnippetScope->setText(QString());
    } else {
        QSnippet *snip = m_manager->snippet(m_editedSnippet);

        ui->eSnippet->setText(snip->pattern());
        ui->leSnippetName->setText(snip->name());
        ui->leSnippetScope->setText(snip->contexts().join(","));
        // ui->eSnippet->highlight();
    }

    ui->eSnippet->setFocus();
}

void QSnippetEdit::on_leSnippetName_editingFinished() {
    if (m_editedSnippet < 0)
        return;

    QSnippet *snip = m_manager->snippet(m_editedSnippet);

    snip->setName(ui->leSnippetName->text());

    ui->lwSnippets->item(m_editedSnippet + 1)->setText(snip->name());
}

void QSnippetEdit::on_leSnippetScope_editingFinished() {
    if (m_editedSnippet < 0)
        return;

    QSnippet *snip = m_manager->snippet(m_editedSnippet);

    snip->setContexts(ui->leSnippetScope->text().split(_cxt_splitter));
}

void QSnippetEdit::on_tbCreateSnippet_clicked() {
    QString name = ui->leSnippetName->text();
    QString pattern = ui->eSnippet->text();
    QStringList contexts = ui->leSnippetScope->text().split(_cxt_splitter);

    if (pattern.endsWith('\n'))
        pattern.chop(1);

    if (name.isEmpty() || pattern.isEmpty()) {
        QMessageBox::information(
            0, tr("Missing data"),
            tr("Please provide a name and a content to create a new snippet"));
        return;
    }

    // TODO : allow pattern loader choice...
    bool ok = m_manager->loadSnippetFromString(name, pattern, "Simple");

    if (!ok) {
        QMessageBox::warning(0, tr("Error"), tr("Invalid snippet pattern."));
        return;
    }

    ui->eSnippet->setText(QString());
    ui->leSnippetScope->clear();
    ui->leSnippetName->clear();

    QSnippet *snip = m_manager->snippet(m_manager->snippetCount() - 1);
    // snip->setName(name);
    snip->setContexts(contexts);

    ui->lwSnippets->setCurrentRow(0);
}

void QSnippetEdit::on_tbDeleteSnippet_clicked() {
    int row = ui->lwSnippets->currentRow() - 1;

    if (row < 0) {
        QMessageBox::warning(0, tr("Error"),
                             tr("Please select a valid snippet to erase"));
        return;
    }

    m_manager->removeSnippet(row);
}
