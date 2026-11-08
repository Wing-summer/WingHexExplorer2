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

#include "consolehighlighanim.h"

#include <QApplication>
#include <QPalette>
#include <QWidget>

ConsoleHighlighAnim::ConsoleHighlighAnim(QObject *parent)
    : QObject(parent), m_highlightColor(qApp->palette().highlight().color()),
      m_alpha(0), m_w(nullptr), m_anim(nullptr) {
    m_anim = new QPropertyAnimation(this, QByteArrayLiteral("alpha"), this);
    m_anim->setStartValue(10);
    m_anim->setEndValue(100);
    m_anim->setDuration(800);
    m_anim->setEasingCurve(QEasingCurve::InOutQuad);
    m_anim->setLoopCount(-1);
    connect(m_anim, &QPropertyAnimation::finished, this, [this]() {
        if (m_w) {
            m_w->setStyleSheet({});
        }
    });
}

int ConsoleHighlighAnim::getAlpha() const { return m_alpha; }

void ConsoleHighlighAnim::setAlpha(int newAlpha) {
    if (m_alpha == newAlpha) {
        return;
    }
    m_alpha = newAlpha;

    auto clsname = m_w->metaObject()->className();
    auto ss = QStringLiteral("background-color: rgba(%1, %2, %3, %4);")
                  .arg(m_highlightColor.red())
                  .arg(m_highlightColor.green())
                  .arg(m_highlightColor.blue())
                  .arg(m_alpha);
    if (clsname) {
        ss.prepend('{').prepend(clsname).append('}');
    }

    if (m_w) {
        m_w->setStyleSheet(ss);
    }

    Q_EMIT sigOnAlphaChanged();
}

QWidget *ConsoleHighlighAnim::widget() const { return m_w; }

void ConsoleHighlighAnim::setWidget(QWidget *newW) {
    if (m_w) {
        m_w->setStyleSheet({});
    }
    m_w = newW;
}

void ConsoleHighlighAnim::start() {
    if (m_w == nullptr) {
        return;
    }
    if (m_anim->state() == QPropertyAnimation::Stopped) {
        m_anim->start();
    }
}

void ConsoleHighlighAnim::stop() {
    if (m_anim->state() == QPropertyAnimation::Running) {
        m_anim->stop();

        if (m_w) {
            m_w->setStyleSheet({});
        }
    }
}
