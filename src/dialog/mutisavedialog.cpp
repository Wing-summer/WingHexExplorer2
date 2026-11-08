/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#include "mutisavedialog.h"
#include "ui_mutisavedialog.h"

#include <QMenu>
#include <QPushButton>

#include "class/settings/thememanager.h"
#include "control/delegate/mutisavebuttondelegate.h"
#include "utilities.h"

MutiSaveDialog::MutiSaveDialog(const QVector<EditorInfo *> &editors,
                               EditorsCtl *edctl, QWidget *parent)
    : QWidget(parent), ui(new Ui::MutiSaveDialog), m_editors(editors),
      m_esctl(edctl) {
    ui->setupUi(this);
    setWindowIcon(ThemeManager::instance().standardIcon(
        ThemeManager::StandardIcon::Unsaved));

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this,
            [this](QAbstractButton *button) {
                auto sbtn = ui->buttonBox->standardButton(button);
                if (sbtn == QDialogButtonBox::Discard) {
                    m_dialog->done(SAVE_DISCARD);
                } else if (sbtn == QDialogButtonBox::Cancel) {
                    m_dialog->done(SAVE_CANCEL);
                } else if (sbtn == QDialogButtonBox::SaveAll) {
                    m_dialog->done(SAVE_SAVE);
                }
            });

    auto delegate = new MutiSaveButtonDelegate(this);
    connect(delegate, &MutiSaveButtonDelegate::sigOnButtonClicked, this,
            &MutiSaveDialog::onClickFileButton);
    ui->listWidget->setItemDelegate(delegate);

    Q_ASSERT(edctl);
    for (const auto &e : editors) {
        auto item = new QListWidgetItem(e->editorIcon(), e->infoFileName());
        item->setToolTip(e->infoTooltip());
        ui->listWidget->addItem(item);
    }

    m_dialog = new FramelessDialogBase(parent);
    m_dialog->buildUpContent(this);
    m_dialog->setWindowTitle(this->windowTitle());
    m_dialog->setMinimumSize(400, 350);

    Utilities::moveToCenter(m_dialog);
}

MutiSaveDialog::~MutiSaveDialog() {
    delete ui;
    m_dialog->deleteLater();
}

int MutiSaveDialog::exec() { return m_dialog->exec(); }

void MutiSaveDialog::onClickFileButton(const QModelIndex &index) {
    QMenu menu;
    auto &theme = ThemeManager::instance();
    auto a = menu.addAction(
        theme.standardIcon(ThemeManager::StandardIcon::Saved), tr("Save"));
    auto row = index.row();
    connect(a, &QAction::triggered, this, [this, row]() {
        auto e = m_editors.at(row);
        Q_ASSERT(m_esctl);
        if (m_esctl->save(e)) {
            m_editors.removeAt(row);
            auto le = ui->listWidget->takeItem(row);
            if (le) {
                delete le;
            }
        }

        if (m_editors.isEmpty()) {
            m_dialog->done(SAVE_NOOP);
        }
    });
    a = menu.addAction(theme.standardIcon(ThemeManager::StandardIcon::Unsaved),
                       tr("Discard"));
    connect(a, &QAction::triggered, this, [this, row]() {
        auto e = m_editors.at(row);
        Q_ASSERT(m_esctl);
        m_esctl->discard(e);
        m_editors.removeAt(row);
        auto le = ui->listWidget->takeItem(row);
        if (le) {
            delete le;
        }

        if (m_editors.isEmpty()) {
            m_dialog->done(SAVE_NOOP);
        }
    });
    menu.exec(QCursor::pos());
}
