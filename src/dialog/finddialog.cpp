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

#include "finddialog.h"
#include "utilities.h"

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>

#include "control/toast.h"

FindDialog::FindDialog(const FindInfo &info, QWidget *parent)
    : FramelessDialogBase(parent) {

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    layout->addWidget(new QLabel(tr("Mode:"), this));
    m_findMode = new QComboBox(this);
    m_findMode->addItem(QStringLiteral("HEX"));
    m_findMode->addItems(Utilities::getEncodings());
    m_findMode->setCurrentIndex(1);

    layout->addWidget(m_findMode);
    layout->addSpacing(3);

    layout->addWidget(new QLabel(tr("Content:"), this));
    m_lineeditor = new HexLineEdit(this);
    m_lineeditor->setFixedHeight(m_findMode->height());
    layout->addWidget(m_lineeditor);
    layout->addSpacing(3);

    connect(m_findMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                auto newva = index == 0;
                m_lineeditor->setMode(newva ? HexLineEdit::HexMode
                                            : HexLineEdit::TextMode);
            });

    if (info.isStringFind) {
        m_lineeditor->setMode(HexLineEdit::TextMode);
        m_findMode->setCurrentText(info.encoding);
        m_lineeditor->setText(info.findValue);
    } else {
        m_lineeditor->setMode(HexLineEdit::HexMode);
        m_findMode->setCurrentIndex(0);
        m_lineeditor->setHexText(info.findValue);
    }

    auto group = new QButtonGroup(this);
    group->setExclusive(true);

    auto btnBox = new QWidget(this);
    auto buttonLayout = new QHBoxLayout(btnBox);
    buttonLayout->setSpacing(0);

    int id = 0;

    auto b = new QPushButton(tr("None"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        if (b) {
            _result.dir = SearchDirection::None;
        }
    });
    group->addButton(b, id++);
    b->setChecked(true);
    buttonLayout->addWidget(b);

    group->addButton(b, id++);
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("BeforeCursor"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        if (b) {
            _result.dir = SearchDirection::Foreword;
        }
    });
    group->addButton(b, id++);
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("AfterCursor"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        if (b) {
            _result.dir = SearchDirection::Backword;
        }
    });
    group->addButton(b, id++);
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("Selection"), this);
    b->setCheckable(true);
    if (info.isSel) {
        connect(b, &QPushButton::toggled, this, [=](bool b) {
            if (b) {
                _result.dir = SearchDirection::Selection;
            }
        });
    } else {
        b->setEnabled(false);
    }

    group->addButton(b, id++);
    buttonLayout->addWidget(b);

    layout->addWidget(btnBox);

    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this, &FindDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this, &FindDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &FindDialog::on_accept);

    layout->addSpacing(20);
    layout->addWidget(dbbox);

    buildUpContent(widget);
    setWindowTitle(tr("find"));
    setWindowIcon(ICONRES("find"));

    m_lineeditor->setFocus();
}

FindDialog::Result FindDialog::getResult() const { return _result; }

void FindDialog::on_accept() {
    // check the last byte nibbles
    if (!m_lineeditor->isValid()) {
        Toast::toast(this, NAMEICONRES("find"), tr("InvalidSeq"));
        return;
    }

    _result.isStringFind = m_findMode->currentIndex() > 0;

    if (_result.isStringFind) {
        _result.value = m_lineeditor->text();
    } else {
        _result.value = m_lineeditor->rawHexText();
    }

    _result.encoding = m_findMode->currentText();
    done(1);
}

void FindDialog::on_reject() { done(0); }
