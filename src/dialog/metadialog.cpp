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

#include "metadialog.h"
#include "control/toast.h"
#include "utilities.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>

MetaDialog::MetaDialog(QWidget *parent)
    : FramelessDialogBase(parent), _foreground(Qt::transparent),
      _background(Qt::transparent) {
    Q_ASSERT(parent);

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    cforeground = new QCheckBox(this);
    cforeground->setText(tr("Foreground"));
    layout->addWidget(cforeground);
    layout->addSpacing(2);

    iforeground = new HueColorPickerSlider(Qt::Horizontal, this);
    iforeground->setEnabled(false);
    layout->addWidget(iforeground);
    connect(iforeground, &HueColorPickerSlider::colorChanged, this,
            [=](QColor color) { _foreground = color.toRgb(); });
    _foreground = iforeground->color();

    layout->addSpacing(2);

    cbackground = new QCheckBox(this);
    cbackground->setText(tr("Background"));
    layout->addWidget(cbackground);

    layout->addSpacing(2);

    ibackground = new HueColorPickerSlider(Qt::Horizontal, this);
    ibackground->setEnabled(false);
    layout->addWidget(ibackground);
    connect(ibackground, &HueColorPickerSlider::colorChanged, this,
            [=](QColor color) { _background = color.toRgb(); });
    _background = ibackground->color();

    layout->addSpacing(2);

    ccomment = new QCheckBox(this);
    ccomment->setText(tr("Comment"));
    layout->addWidget(ccomment);
    layout->addSpacing(2);

    m_comment = new QLineEdit(this);
    layout->addWidget(m_comment);
    m_comment->setEnabled(false);

    layout->addSpacing(5);

    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(dbbox);
    layout->addSpacing(2);

    connect(cforeground, &QCheckBox::clicked, iforeground,
            &QPushButton::setEnabled);
    connect(cbackground, &QCheckBox::clicked, ibackground,
            &QPushButton::setEnabled);
    connect(ccomment, &QCheckBox::clicked, m_comment, &QPushButton::setEnabled);
    connect(dbbox, &QDialogButtonBox::accepted, this, &MetaDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this, &MetaDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &MetaDialog::on_accept);

    buildUpContent(widget);
    setWindowTitle(tr("Metadata"));
}

void MetaDialog::on_accept() {
    if ((!cforeground->isChecked() && !cbackground->isChecked() &&
         !ccomment->isChecked()) ||
        (cforeground->isChecked() &&
         (!_foreground.isValid() || _foreground.alpha() == 0)) ||
        (cbackground->isChecked() &&
         (!_background.isValid() || _background.alpha() == 0)) ||
        (ccomment->isChecked() && m_comment->text().trimmed().length() == 0)) {
        Toast::toast(this->parentWidget(),
                     NAMEICONRES(QStringLiteral("metadata")), tr("NoChoose"));
        return;
    }

    _comment = ccomment->text();
    done(1);
}

void MetaDialog::on_reject() { done(0); }

QString MetaDialog::comment() {
    if (ccomment->isChecked())
        return m_comment->text();
    else
        return QString();
}

QColor MetaDialog::foreGroundColor() {
    if (cforeground->isChecked())
        return _foreground;
    else
        return QColor(qRgba(0, 0, 0, 0));
}

QColor MetaDialog::backGroundColor() {
    if (cbackground->isChecked())
        return _background;
    else
        return QColor::fromRgba(qRgba(0, 0, 0, 0));
}

void MetaDialog::setComment(QString comment) {
    if (comment.length() > 0) {
        ccomment->setChecked(true);
        emit ccomment->clicked(true);
        _comment = comment;
        m_comment->setText(comment);
    }
}

void MetaDialog::setBackGroundColor(QColor color) {
    if (color.rgba()) {
        cbackground->setChecked(true);
        emit cbackground->clicked(true);
        _background = color;
        ibackground->setColor(color);
    }
}

void MetaDialog::setForeGroundColor(QColor color) {
    if (color.rgba()) {
        cforeground->setChecked(true);
        emit cforeground->clicked(true);
        _foreground = color;
        iforeground->setColor(color);
    }
}
