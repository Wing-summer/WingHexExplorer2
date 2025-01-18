// Copyright (C) 2023-2024 Stdware Collections (https://www.github.com/stdware)
// Copyright (C) 2021-2023 wangwenx190 (Yuhang Zhao)
// SPDX-License-Identifier: Apache-2.0

#include "windowbar.h"
#include "windowbar_p.h"

#include <QApplication>
#include <QStyle>
#include <QtCore/QDebug>
#include <QtCore/QLocale>
#include <QtGui/QtEvents>

namespace QWK {

WindowBarPrivate::WindowBarPrivate() {
    w = nullptr;
    autoTitle = true;
    autoIcon = false;
}

WindowBarPrivate::~WindowBarPrivate() = default;

void WindowBarPrivate::init() {
    Q_Q(WindowBar);
    layout = new QHBoxLayout();
    if (QLocale::system().textDirection() == Qt::RightToLeft) {
        layout->setDirection(QBoxLayout::RightToLeft);
    }

    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);
    for (int i = IconButton; i <= CloseButton; ++i) {
        insertDefaultSpace(i);
    }
    q->setLayout(layout);
}

void WindowBarPrivate::setWidgetAt(int index, QWidget *widget) {
    auto item = layout->takeAt(index);
    auto orgWidget = item->widget();
    if (orgWidget) {
        orgWidget->deleteLater();
    }
    delete item;
    if (!widget) {
        insertDefaultSpace(index);
    } else {
        layout->insertWidget(index, widget);
    }
}

QWidget *WindowBarPrivate::takeWidgetAt(int index) {
    auto item = layout->itemAt(index);
    auto orgWidget = item->widget();
    if (orgWidget) {
        item = layout->takeAt(index);
        delete item;
        insertDefaultSpace(index);
    }
    return orgWidget;
}

WindowBar::WindowBar(QWidget *parent)
    : WindowBar(*new WindowBarPrivate(), parent) {
    setAlignment(Qt::AlignCenter);
    setObjectName(QStringLiteral("win-title-label"));

    Q_D(WindowBar);
    auto w = new QWidget(this);
    w->setAttribute(Qt::WA_TranslucentBackground);
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->setWidgetAt(WindowBarPrivate::Holder, w);

    // default
    setFixedHeight(
        QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight));
}

WindowBar::~WindowBar() = default;

QAbstractButton *WindowBar::iconButton() const {
    Q_D(const WindowBar);
    return static_cast<QAbstractButton *>(
        d->widgetAt(WindowBarPrivate::IconButton));
}

QAbstractButton *WindowBar::minButton() const {
    Q_D(const WindowBar);
    return static_cast<QAbstractButton *>(
        d->widgetAt(WindowBarPrivate::MinimumButton));
}

QAbstractButton *WindowBar::maxButton() const {
    Q_D(const WindowBar);
    return static_cast<QAbstractButton *>(
        d->widgetAt(WindowBarPrivate::MaximumButton));
}

QAbstractButton *WindowBar::closeButton() const {
    Q_D(const WindowBar);
    return static_cast<QAbstractButton *>(
        d->widgetAt(WindowBarPrivate::CloseButton));
}

QWidget *WindowBar::holder() const {
    Q_D(const WindowBar);
    return static_cast<QAbstractButton *>(
        d->widgetAt(WindowBarPrivate::Holder));
}

void WindowBar::setIconButton(QAbstractButton *btn) {
    Q_D(WindowBar);
    auto org = takeIconButton();
    if (org)
        org->deleteLater();
    if (!btn)
        return;
    d->setWidgetAt(WindowBarPrivate::IconButton, btn);
    if (d->autoIcon && d->w)
        btn->setIcon(d->w->windowIcon());
    btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
}

void WindowBar::setMinButton(QAbstractButton *btn) {
    Q_D(WindowBar);
    auto org = takeMinButton();
    if (org)
        org->deleteLater();
    if (!btn)
        return;
    d->setWidgetAt(WindowBarPrivate::MinimumButton, btn);
    connect(btn, &QAbstractButton::clicked, this,
            &WindowBar::minimizeRequested);
}

void WindowBar::setMaxButton(QAbstractButton *btn) {
    Q_D(WindowBar);
    auto org = takeMaxButton();
    if (org)
        org->deleteLater();
    if (!btn)
        return;
    d->setWidgetAt(WindowBarPrivate::MaximumButton, btn);
    connect(btn, &QAbstractButton::clicked, this,
            &WindowBar::maximizeRequested);
}

void WindowBar::setCloseButton(QAbstractButton *btn) {
    Q_D(WindowBar);
    auto org = takeCloseButton();
    if (org)
        org->deleteLater();
    if (!btn)
        return;
    d->setWidgetAt(WindowBarPrivate::CloseButton, btn);
    connect(btn, &QAbstractButton::clicked, this, &WindowBar::closeRequested);
}

QAbstractButton *WindowBar::takeIconButton() {
    Q_D(WindowBar);
    return static_cast<QAbstractButton *>(
        d->takeWidgetAt(WindowBarPrivate::IconButton));
}

QAbstractButton *WindowBar::takeMinButton() {
    Q_D(WindowBar);
    auto btn = static_cast<QAbstractButton *>(
        d->takeWidgetAt(WindowBarPrivate::MinimumButton));
    if (!btn) {
        return nullptr;
    }
    disconnect(btn, &QAbstractButton::clicked, this,
               &WindowBar::minimizeRequested);
    return btn;
}

QAbstractButton *WindowBar::takeMaxButton() {
    Q_D(WindowBar);
    auto btn = static_cast<QAbstractButton *>(
        d->takeWidgetAt(WindowBarPrivate::MaximumButton));
    if (!btn) {
        return nullptr;
    }
    disconnect(btn, &QAbstractButton::clicked, this,
               &WindowBar::maximizeRequested);
    return btn;
}

QAbstractButton *WindowBar::takeCloseButton() {
    Q_D(WindowBar);
    auto btn = static_cast<QAbstractButton *>(
        d->takeWidgetAt(WindowBarPrivate::CloseButton));
    if (!btn) {
        return nullptr;
    }
    disconnect(btn, &QAbstractButton::clicked, this,
               &WindowBar::closeRequested);
    return btn;
}

QWidget *WindowBar::hostWidget() const {
    Q_D(const WindowBar);
    return d->w;
}

void WindowBar::setHostWidget(QWidget *w) {
    Q_D(WindowBar);

    QWidget *org = d->w;
    if (org) {
        org->removeEventFilter(this);
    }
    d_ptr->w = w;
    if (w) {
        w->installEventFilter(this);
    }
}

bool WindowBar::titleFollowWindow() const {
    Q_D(const WindowBar);
    return d->autoTitle;
}

void WindowBar::setTitleFollowWindow(bool value) {
    Q_D(WindowBar);
    d->autoTitle = value;
}

bool WindowBar::iconFollowWindow() const {
    Q_D(const WindowBar);
    return d->autoIcon;
}

void WindowBar::setIconFollowWindow(bool value) {
    Q_D(WindowBar);
    d->autoIcon = value;
}

bool WindowBar::eventFilter(QObject *obj, QEvent *event) {
    Q_D(WindowBar);
    auto w = d->w;
    if (obj == w) {
        QAbstractButton *iconBtn = iconButton();
        QLabel *label = this;
        QAbstractButton *maxBtn = maxButton();
        switch (event->type()) {
        case QEvent::WindowIconChange: {
            if (d_ptr->autoIcon && iconBtn) {
                iconBtn->setIcon(w->windowIcon());
            }
            break;
        }
        case QEvent::WindowTitleChange: {
            if (d_ptr->autoTitle && label) {
                label->setText(w->windowTitle());
                constexpr auto padding = 5;
                auto width =
                    QFontMetrics(font()).horizontalAdvance(w->windowTitle()) +
                    padding + padding;
                if (iconBtn) {
                    width += iconBtn->iconSize().width();
                }
                if (maxBtn) {
                    width += maxBtn->sizeHint().width();
                }
                if (minButton()) {
                    width += minButton()->sizeHint().width();
                }
                if (closeButton()) {
                    width += closeButton()->sizeHint().width();
                }
                this->setMinimumWidth(width);
            }
            break;
        }
        case QEvent::WindowStateChange: {
            if (maxBtn) {
                maxBtn->setChecked(w->isMaximized());
            }
            break;
        }
        default:
            break;
        }
    }
    return QWidget::eventFilter(obj, event);
}

WindowBar::WindowBar(WindowBarPrivate &d, QWidget *parent)
    : QLabel(parent), d_ptr(&d) {
    d.q_ptr = this;

    d.init();
}

} // namespace QWK
