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

#include "toast.h"

#include <QApplication>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTimerEvent>

static constexpr int TIMER_INTERVAL = 30;
static constexpr int PADDING = 10;
static constexpr int CORNER_RADIUS = 8;
static constexpr int TOAST_INTERVAL = 3000;
static constexpr int FADE_INTERVAL = 400;

static Toast *toastInstance = nullptr;

Toast::Toast(const QString &strContent, const QPixmap &icon, QWidget *parent)
    : QDialog(parent, Qt::Widget), m_strContent(strContent), m_icon(icon),
      m_parent(parent) {
    init();
}

bool Toast::eventFilter(QObject *watched, QEvent *event) {
    if (m_dismissing) {
        return false;
    }

    const auto type = event->type();
    auto widget = qobject_cast<QWidget *>(watched);
    if (type == QEvent::ApplicationDeactivate || type == QEvent::KeyPress) {
        dismiss();
    } else if (type == QEvent::MouseButtonPress && widget) {
        auto mouseEvent = static_cast<QMouseEvent *>(event);
        // Map within the widget tree; no compositor-provided global position.
        if (widget->window() != window() ||
            !rect().contains(
                widget->mapTo(window(), mouseEvent->position().toPoint()) -
                mapTo(window(), QPoint()))) {
            dismiss();
        }
    } else if (widget && m_parent &&
               (widget == m_parent || widget->isAncestorOf(m_parent))) {
        if (type == QEvent::Hide || type == QEvent::HideToParent ||
            (type == QEvent::WindowStateChange && widget->isMinimized())) {
            dismiss();
        } else if (widget == m_parent && (type == QEvent::Resize ||
                                          type == QEvent::ContentsRectChange)) {
            setToastPos(m_pos);
        }
    }
    return QDialog::eventFilter(watched, event);
}

void Toast::toast(QWidget *parent, const QPixmap &icon,
                  const QString &strContent) {
    if (!parent || !parent->isVisible() || parent->window()->isMinimized()) {
        return;
    }

    if (toastInstance) {
        toastInstance->dismiss();
    }

    auto content = strContent;
    content.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    content.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    toastInstance = new Toast(content, icon, parent);
    // Polish before measuring: application styles can change the display font.
    toastInstance->ensurePolished();
    toastInstance->setToastPos(toastInstance->lastToastPos());
    qApp->installEventFilter(toastInstance);
    toastInstance->show();
    toastInstance->raise();
}

Toast::~Toast() {
    if (toastInstance == this) {
        toastInstance = nullptr;
    }
}

void Toast::dismiss() {
    if (m_dismissing) {
        return;
    }
    m_dismissing = true;
    m_timer.stop();
    hide();
    if (toastInstance == this) {
        toastInstance = nullptr;
    }
    deleteLater();
}

QSize Toast::calculateTextSize() {
    const QFontMetrics metrics(displayFont());
    const int parentWidth = m_parent ? m_parent->contentsRect().width() : 0;
    const int sideMargin = qMin(50, parentWidth / 10);
    const int iconWidth =
        m_icon.isNull() ? 0 : metrics.height() + metrics.horizontalAdvance(' ');
    const int maxTextWidth =
        qMax(1, parentWidth - sideMargin * 2 - PADDING * 2 - iconWidth - 1);
    const int textWidth =
        qMin(maxTextWidth,
             qMax(1, metrics.size(Qt::TextExpandTabs, m_strContent).width()));

    // QTextLayout uses line separators for hard breaks within one layout.
    auto text = m_strContent;
    text.replace(QLatin1Char('\n'), QChar(QChar::LineSeparator));
    m_textLayout.setText(text);
    m_textLayout.setFont(displayFont());
    QTextOption option;
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    option.setTextDirection(layoutDirection());
    m_textLayout.setTextOption(option);
    m_textLayout.setCacheEnabled(true);
    m_textLayout.beginLayout();
    qreal height = 0;
    while (true) {
        auto line = m_textLayout.createLine();
        if (!line.isValid()) {
            break;
        }
        line.setLineWidth(textWidth);
        line.setPosition(QPointF(0, height));
        height += line.height();
    }
    m_textLayout.endLayout();
    return QSize(textWidth, qMax(metrics.height(), qCeil(height)));
}

void Toast::init() {
    // A child overlay needs neither a native popup nor global positioning.
    // QDialog's constructor adds Qt::Dialog even when passed Qt::Widget (0).
    setWindowFlags(Qt::Widget);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAutoFillBackground(false);
    setToastPos(m_pos);
}

void Toast::setToastPos(ToastPos pos) {
    m_pos = pos;
    if (!m_parent) {
        return;
    }

    const auto bounds = m_parent->contentsRect();
    const auto textSize = calculateTextSize();
    const QFontMetrics metrics(displayFont());
    const int iconWidth =
        m_icon.isNull() ? 0 : metrics.height() + metrics.horizontalAdvance(' ');
    // Keep side margins on large hosts, but also fit very small parents.
    const int sideMargin = qMin(50, bounds.width() / 10);
    const int maxWidth = qMax(1, bounds.width() - sideMargin * 2);
    const QSize toastSize(
        qMin(maxWidth, textSize.width() + iconWidth + PADDING * 2 + 1),
        qMin(qMax(1, bounds.height()), textSize.height() + PADDING * 2));

    const int x = bounds.x() + (bounds.width() - toastSize.width()) / 2;
    int y;
    if (pos == ToastPos::Top) {
        y = bounds.height() / 6;
    } else if (pos == ToastPos::Bottom) {
        y = bounds.height() * 5 / 6 - toastSize.height();
    } else {
        y = (bounds.height() - toastSize.height()) / 2;
    }
    y = bounds.y() +
        qBound(0, y, qMax(0, bounds.height() - toastSize.height()));
    setGeometry(QRect(QPoint(x, y), toastSize));
    update();
}

Toast::ToastPos Toast::lastToastPos() const { return m_pos; }

void Toast::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                           QPainter::SmoothPixmapTransform);
    // Painter opacity works on child widgets on every platform, including
    // Wayland, without window opacity or an offscreen graphics effect.
    painter.setOpacity(m_opacity);
    QPainterPath background;
    background.addRoundedRect(QRectF(rect()), CORNER_RADIUS, CORNER_RADIUS);
    painter.fillPath(background, m_backColor);
    // Also keep icons inside the rounded outline on very small hosts.
    painter.setClipPath(background);
    painter.setFont(displayFont());
    painter.setPen(m_textColor);

    const QFontMetrics metrics(displayFont());
    QRect textRect = rect().adjusted(PADDING, PADDING, -PADDING, -PADDING);
    if (!m_icon.isNull()) {
        const int iconSize = metrics.height();
        painter.drawPixmap(QRect(PADDING, PADDING, iconSize, iconSize), m_icon,
                           m_icon.rect());
        textRect.setLeft(textRect.left() + iconSize +
                         metrics.horizontalAdvance(' '));
    }
    if (textRect.width() > 0 && textRect.height() > 0) {
        painter.setClipRect(textRect, Qt::IntersectClip);
        m_textLayout.draw(&painter, textRect.topLeft());
    }
}

void Toast::changeEvent(QEvent *event) {
    QDialog::changeEvent(event);
    if (event->type() == QEvent::FontChange ||
        event->type() == QEvent::StyleChange ||
        event->type() == QEvent::ApplicationFontChange ||
        event->type() == QEvent::LayoutDirectionChange) {
        setToastPos(m_pos);
    }
}

void Toast::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    m_activeTime = 0;
    m_opacity = 1.0;
    m_pauseTimer = false;
    m_elapsed.start();
    m_timer.start(TIMER_INTERVAL, this);
}

void Toast::timerEvent(QTimerEvent *event) {
    if (event->timerId() != m_timer.timerId()) {
        QDialog::timerEvent(event);
        return;
    }
    if (m_pauseTimer) {
        return;
    }

    const qint64 elapsed = m_activeTime + m_elapsed.elapsed();
    if (elapsed >= TOAST_INTERVAL + FADE_INTERVAL) {
        dismiss();
    } else if (elapsed > TOAST_INTERVAL) {
        m_opacity = 1.0 - qreal(elapsed - TOAST_INTERVAL) / FADE_INTERVAL;
        update();
    }
}

void Toast::enterEvent(QEnterEvent *event) {
    if (!m_pauseTimer && m_elapsed.isValid()) {
        m_activeTime += m_elapsed.elapsed();
        m_pauseTimer = true;
        m_timer.stop();
    }
    QDialog::enterEvent(event);
}

void Toast::leaveEvent(QEvent *event) {
    if (m_pauseTimer && !m_dismissing) {
        m_elapsed.start();
        m_pauseTimer = false;
        m_timer.start(TIMER_INTERVAL, this);
    }
    QDialog::leaveEvent(event);
}

QColor Toast::textColor() const { return m_textColor; }

void Toast::setTextColor(const QColor &textColor) {
    if (m_textColor != textColor) {
        m_textColor = textColor;
        update();
    }
}

QFont Toast::displayFont() const {
    auto font = this->font();
    if (font.pointSizeF() > 0) {
        font.setPointSizeF(font.pointSizeF() * 1.8);
    } else if (font.pixelSize() > 0) {
        font.setPixelSize(qMax(1, qRound(font.pixelSize() * 1.8)));
    }
    return font;
}

QColor Toast::backColor() const { return m_backColor; }

void Toast::setBackColor(const QColor &backColor) {
    if (m_backColor != backColor) {
        m_backColor = backColor;
        update();
    }
}

QPixmap Toast::icon() const { return m_icon; }

void Toast::setIcon(const QPixmap &icon) {
    m_icon = icon;
    setToastPos(m_pos);
}
