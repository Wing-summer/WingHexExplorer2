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

#include <QDialogButtonBox>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>

#include "class/settings/thememanager.h"
#include "control/toast.h"

MetaDialog::MetaDialog(QWidget *parent)
    : FramelessDialogBase(parent), m_foreground(Qt::transparent),
      m_background(Qt::transparent) {
    Q_ASSERT(parent);

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    m_cforeground = new QCheckBox(this);
    m_cforeground->setText(tr("Foreground"));
    layout->addWidget(m_cforeground);
    layout->addSpacing(2);

    m_iforeground = new HueColorPickerSlider(Qt::Horizontal, this);
    m_iforeground->setEnabled(false);
    layout->addWidget(m_iforeground);
    connect(m_iforeground, &HueColorPickerSlider::sigOnColorChanged, this,
            [this](QColor color) { m_foreground = color.toRgb(); });
    m_foreground = m_iforeground->color();

    layout->addSpacing(2);

    m_cbackground = new QCheckBox(this);
    m_cbackground->setText(tr("Background"));
    layout->addWidget(m_cbackground);

    layout->addSpacing(2);

    m_ibackground = new HueColorPickerSlider(Qt::Horizontal, this);
    m_ibackground->setEnabled(false);
    layout->addWidget(m_ibackground);
    connect(m_ibackground, &HueColorPickerSlider::sigOnColorChanged, this,
            [this](QColor color) { m_background = color.toRgb(); });
    m_background = m_ibackground->color();

    layout->addSpacing(2);

    m_ccomment = new QCheckBox(this);
    m_ccomment->setText(tr("Comment"));
    layout->addWidget(m_ccomment);
    layout->addSpacing(2);

    m_comment = new QLineEdit(this);
    m_comment->setMaxLength(QHEXVIEW_COMMENT_LIMIT);
    layout->addWidget(m_comment);
    m_comment->setEnabled(false);

    layout->addSpacing(5);

    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(dbbox);
    layout->addSpacing(2);

    connect(m_cforeground, &QCheckBox::clicked, m_iforeground,
            &QPushButton::setEnabled);
    connect(m_cbackground, &QCheckBox::clicked, m_ibackground,
            &QPushButton::setEnabled);
    connect(m_ccomment, &QCheckBox::clicked, m_comment,
            &QPushButton::setEnabled);
    connect(dbbox, &QDialogButtonBox::accepted, this, &MetaDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this, &MetaDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &MetaDialog::on_accept);

    buildUpContent(widget);
    setWindowTitle(tr("Annotation"));
    setWindowIcon(ThemeManager::instance().standardIcon(
        ThemeManager::StandardIcon::Metadata));
}

void MetaDialog::on_accept() {
    if ((!m_cforeground->isChecked() && !m_cbackground->isChecked() &&
         !m_ccomment->isChecked()) ||
        (m_cforeground->isChecked() &&
         (!m_foreground.isValid() || m_foreground.alpha() == 0)) ||
        (m_cbackground->isChecked() &&
         (!m_background.isValid() || m_background.alpha() == 0)) ||
        (m_ccomment->isChecked() &&
         m_comment->text().trimmed().length() == 0)) {
        Toast::toast(
            this->parentWidget(),
            ThemeManager::instance().standardPixmap(
                ThemeManager::StandardIcon::Metadata),
            tr("Configure at least one enabled option, or disable the option"));
        return;
    }

    m_commentValue = m_ccomment->text();
    done(1);
}

void MetaDialog::on_reject() { done(0); }

QString MetaDialog::comment() {
    if (m_ccomment->isChecked()) {
        return m_comment->text();
    } else {
        return QString();
    }
}

QColor MetaDialog::foreGroundColor() {
    if (m_cforeground->isChecked()) {
        return m_foreground;
    } else {
        return QColor();
    }
}

QColor MetaDialog::backGroundColor() {
    if (m_cbackground->isChecked()) {
        return m_background;
    } else {
        return QColor();
    }
}

void MetaDialog::setComment(QString comment) {
    if (comment.length() > 0) {
        m_ccomment->setChecked(true);
        Q_EMIT m_ccomment->clicked(true);
        m_commentValue = comment;
        m_comment->setText(comment);
    }
}

void MetaDialog::setBackGroundColor(QColor color) {
    if (color.isValid()) {
        m_cbackground->setChecked(true);
        Q_EMIT m_cbackground->clicked(true);
        m_background = color;
        m_ibackground->setColor(color);
    }
}

void MetaDialog::setForeGroundColor(QColor color) {
    if (color.isValid()) {
        m_cforeground->setChecked(true);
        Q_EMIT m_cforeground->clicked(true);
        m_foreground = color;
        m_iforeground->setColor(color);
    }
}
