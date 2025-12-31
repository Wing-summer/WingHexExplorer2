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
#include "class/mutisavebuttondelegate.h"
#include "ui_mutisavedialog.h"

#include "utilities.h"

#include <QPushButton>

MutiSaveDialog::MutiSaveDialog(const QVector<EditorInfo *> &editors,
                               EditorsCtl *edctl, QWidget *parent)
    : QWidget(parent), ui(new Ui::MutiSaveDialog), _editors(editors),
      _esctl(edctl) {
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this,
            [this](QAbstractButton *button) {
                auto sbtn = ui->buttonBox->standardButton(button);
                if (sbtn == QDialogButtonBox::Discard) {
                    _dialog->done(SAVE_DISCARD);
                } else if (sbtn == QDialogButtonBox::Cancel) {
                    _dialog->done(SAVE_CANCEL);
                } else if (sbtn == QDialogButtonBox::SaveAll) {
                    _dialog->done(SAVE_SAVE);
                }
            });

    auto delegate = new MutiSaveButtonDelegate(this);
    connect(delegate, &MutiSaveButtonDelegate::buttonClicked, this,
            &MutiSaveDialog::onClickFileButton);
    ui->listWidget->setItemDelegate(delegate);

    Q_ASSERT(edctl);
    for (auto &e : editors) {
        auto item = new QListWidgetItem(e->editorIcon(), e->infoFileName());
        item->setToolTip(e->infoTooltip());
        ui->listWidget->addItem(item);
    }

    _dialog = new FramelessDialogBase(parent);
    _dialog->buildUpContent(this);
    _dialog->setWindowTitle(this->windowTitle());
    _dialog->setMinimumSize(400, 350);

    Utilities::moveToCenter(_dialog);
}

MutiSaveDialog::~MutiSaveDialog() {
    delete ui;
    _dialog->deleteLater();
}

int MutiSaveDialog::exec() { return _dialog->exec(); }

void MutiSaveDialog::onClickFileButton(const QModelIndex &index) {
    QMenu menu;
    auto a = menu.addAction(ICONRES(QStringLiteral("saved")), tr("Save"));
    auto row = index.row();
    connect(a, &QAction::triggered, this, [this, row]() {
        auto e = _editors.at(row);
        Q_ASSERT(_esctl);
        if (_esctl->save(e)) {
            _editors.removeAt(row);
            auto le = ui->listWidget->takeItem(row);
            if (le) {
                delete le;
            }
        }

        if (_editors.isEmpty()) {
            _dialog->done(SAVE_NOOP);
        }
    });
    a = menu.addAction(ICONRES(QStringLiteral("unsaved")), tr("Discard"));
    connect(a, &QAction::triggered, this, [this, row]() {
        auto e = _editors.at(row);
        Q_ASSERT(_esctl);
        _esctl->discard(e);
        _editors.removeAt(row);
        auto le = ui->listWidget->takeItem(row);
        if (le) {
            delete le;
        }

        if (_editors.isEmpty()) {
            _dialog->done(SAVE_NOOP);
        }
    });
    menu.exec(QCursor::pos());
}
