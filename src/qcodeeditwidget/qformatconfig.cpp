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

#include "qformatconfig.h"

/*!
        \file qformatconfig.cpp
        \brief Implementation of the QFormatConfig class.

        \see QFormatConfig
*/

#include "qformatscheme.h"

#include "qdocument.h"
#include "qeditor.h"

#include "ui_qformatconfig.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QTableWidget>
#include <QVBoxLayout>

#include "utilities.h"

/*!
        \ingroup dialogs
        @{

        \class QFormatConfig
        \brief A minimalistic, easy to embed, format settings widget.

*/

QFormatConfig::QFormatConfig(const QHash<QString, QFormatScheme *> &schemes,
                             const QString &defaultScheme, QWidget *w)
    : WingHex::SettingPage(w), ui(new Ui::QFormatConfig()), m_autonomous(false),
      m_currentScheme(nullptr) {
    ui->setupUi(this);

    m_model = new FormatConfigModel(this);
    ui->m_table->verticalHeader()->hide();
    ui->m_table->setModel(m_model);

    Q_ASSERT(schemes.contains(defaultScheme));
    for (auto p = schemes.constKeyValueBegin(); p != schemes.constKeyValueEnd();
         p++) {
        addScheme(p->first, p->second);
    }
    setCurrentScheme(schemes.value(defaultScheme));

    connect(ui->m_table, SIGNAL(itemSelectionChanged()), ui->m_table,
            SLOT(clearSelection()));
}

QFormatConfig::~QFormatConfig() { delete ui; }

QIcon QFormatConfig::categoryIcon() const { return ICONRES("scheme"); }

QString QFormatConfig::name() const { return tr("Format"); }

/*!
        \brief Add a format scheme to the config widget

        Users will be able to edit that scheme (switching among schemes
   using a combobox if several schemes are added to the widget)
*/
void QFormatConfig::addScheme(const QString &name, QFormatScheme *scheme) {
    m_schemes << scheme;
    ui->m_selector->addItem(name);
}

void QFormatConfig::setCurrentScheme(QFormatScheme *s) {
    int idx = m_schemes.indexOf(s);

    if (idx != -1) {
        m_currentScheme = s;

        // update table widget
        m_model->setScheme(m_currentScheme);
    } else {
        ui->m_selector->setCurrentIndex(idx);
    }
}

/*!
        \brief Apply changes made to the currently edited scheme, if any
*/
void QFormatConfig::apply() {
    if (m_currentScheme) {
        auto editors = QEditor::editors();
        for (auto &e : editors) {
            if (e->document()->formatScheme() == m_currentScheme)
                e->viewport()->update();
        }
    }
}

/*!
        \brief Reset the subcontrols to reflect the data of the format
   scheme being edited

        The name can be a bit misleading at first, it has been chosen
        because it directly maps to the effect a "cancel" button would
        have on the widget
*/
void QFormatConfig::cancel() { ui->m_table->resizeColumnsToContents(); }

/*!
        \brief Restore default values for all subcontrols

        \note The widgets are changed but these changes are NOT applied.
*/
void QFormatConfig::reset() {}

/*! @} */

QString QFormatConfig::id() const { return QStringLiteral("Scheme"); }
