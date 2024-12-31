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

FindDialog::FindDialog(const FindInfo &info, QWidget *parent)
    : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    m_string = new QRadioButton(this);
    m_string->setText(tr("findstring"));
    layout->addWidget(m_string);
    layout->addSpacing(3);

    m_encodings = new QComboBox(this);
    m_encodings->addItems(Utilities::getEncodings());
    m_encodings->setCurrentIndex(0);
    m_encodings->setEnabled(false);
    connect(m_string, &QRadioButton::toggled, m_encodings,
            &QComboBox::setEnabled);
    layout->addWidget(m_encodings);
    layout->addSpacing(3);

    m_lineeditor = new QLineEdit(this);
    m_lineeditor->setEnabled(false);
    connect(m_string, &QRadioButton::toggled, m_lineeditor,
            &QLineEdit::setEnabled);
    layout->addWidget(m_lineeditor);
    layout->addSpacing(3);

    m_hex = new QRadioButton(this);
    m_hex->setText(tr("findhex"));
    layout->addWidget(m_hex);
    layout->addSpacing(3);

    m_hexeditor = new QHexView(this);
    m_hexeditor->setAsciiVisible(false);
    m_hexeditor->setAddressVisible(false);
    m_hexeditor->setEnabled(true);
    connect(m_hex, &QRadioButton::toggled, m_hexeditor, &QHexView::setEnabled);
    layout->addWidget(m_hexeditor);
    layout->addSpacing(10);

    if (info.isStringFind) {
        m_string->setChecked(true);
        m_lineeditor->setEnabled(true);
        m_hex->setEnabled(false);
        if (!info.encoding.isEmpty()) {
            m_encodings->setCurrentText(info.encoding);
        }
    } else {
        m_hex->setChecked(true);
        m_lineeditor->setEnabled(false);
        m_hex->setEnabled(true);
    }

    m_lineeditor->setText(info.str);
    m_hexeditor->document()->_insert(0, info.buffer);

    auto regionw = new QWidget(this);
    auto regionLayout = new QHBoxLayout(regionw);

    regionLayout->addWidget(new QLabel(tr("Region:"), regionw));

    m_regionStart = new QtLongLongSpinBox(regionw);
    Q_ASSERT(info.stop >= info.start);
    m_regionStart->setRange(info.start, info.stop);
    m_regionStart->setEnabled(false);
    m_regionStart->setValue(info.start);
    m_regionStart->setDisplayIntegerBase(16);
    m_regionStart->setPrefix(QStringLiteral("0x"));
    regionLayout->addWidget(m_regionStart, 1);

    regionLayout->addWidget(new QLabel(QStringLiteral(" - "), regionw));

    m_regionStop = new QtLongLongSpinBox(regionw);
    m_regionStop->setRange(info.start, info.stop);
    connect(m_regionStart, &QtLongLongSpinBox::valueChanged, m_regionStop,
            &QtLongLongSpinBox::setMinimum);
    m_regionStop->setEnabled(false);
    m_regionStop->setValue(qMin(info.start + 1024 * 1024, info.stop));
    m_regionStop->setDisplayIntegerBase(16);
    m_regionStop->setPrefix(QStringLiteral("0x"));
    regionLayout->addWidget(m_regionStop, 1);

    layout->addWidget(regionw);

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
    b->setEnabled(!info.isBigFile);
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("Region"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        if (b) {
            _result.dir = SearchDirection::Region;
        }
        m_regionStart->setEnabled(b);
        m_regionStop->setEnabled(b);
    });
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
    group->button(info.isBigFile ? 1 : 0)->setChecked(true);

    layout->addWidget(btnBox);
    layout->addSpacing(20);
    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this, &FindDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this, &FindDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &FindDialog::on_accept);
    layout->addWidget(dbbox);

    buildUpContent(widget);
    this->setWindowTitle(tr("find"));
}

FindDialog::Result FindDialog::getResult() const { return _result; }

void FindDialog::on_accept() {
    _result.start = 0;
    _result.stop = 0;
    if (m_regionStart->isEnabled()) {
        _result.start = m_regionStart->value();
        _result.stop = m_regionStop->value();
    }
    _result.encoding = m_encodings->currentText();
    _result.isStringFind = m_string->isChecked();
    _result.buffer = m_hexeditor->document()->read(0);
    _result.str = m_lineeditor->text();
    done(1);
}

void FindDialog::on_reject() { done(0); }
