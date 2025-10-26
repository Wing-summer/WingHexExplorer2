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
      m_alpha(0), _w(nullptr), _anim(nullptr) {
    _anim = new QPropertyAnimation(this, QByteArrayLiteral("alpha"), this);
    _anim->setStartValue(10);
    _anim->setEndValue(100);
    _anim->setDuration(800);
    _anim->setEasingCurve(QEasingCurve::InOutQuad);
    _anim->setLoopCount(-1);
    connect(_anim, &QPropertyAnimation::finished, this, [this]() {
        if (_w) {
            _w->setStyleSheet({});
        }
    });
}

int ConsoleHighlighAnim::getAlpha() const { return m_alpha; }

void ConsoleHighlighAnim::setAlpha(int newAlpha) {
    if (m_alpha == newAlpha)
        return;
    m_alpha = newAlpha;

    auto clsname = _w->metaObject()->className();
    auto ss = QStringLiteral("background-color: rgba(%1, %2, %3, %4);")
                  .arg(m_highlightColor.red())
                  .arg(m_highlightColor.green())
                  .arg(m_highlightColor.blue())
                  .arg(m_alpha);
    if (clsname) {
        ss.prepend('{').prepend(clsname).append('}');
    }

    if (_w) {
        _w->setStyleSheet(ss);
    }

    Q_EMIT alphaChanged();
}

QWidget *ConsoleHighlighAnim::widget() const { return _w; }

void ConsoleHighlighAnim::setWidget(QWidget *newW) {
    if (_w) {
        _w->setStyleSheet({});
    }
    _w = newW;
}

void ConsoleHighlighAnim::start() {
    if (_w == nullptr) {
        return;
    }
    if (_anim->state() == QPropertyAnimation::Stopped) {
        _anim->start();
    }
}

void ConsoleHighlighAnim::stop() {
    if (_anim->state() == QPropertyAnimation::Running) {
        _anim->stop();

        if (_w) {
            _w->setStyleSheet({});
        }
    }
}
