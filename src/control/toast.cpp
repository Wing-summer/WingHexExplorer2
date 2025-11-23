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

#include <QColor>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QPainter>
#include <QPen>
#include <QRegularExpression>
#include <QScreen>
#include <QTimerEvent>
#include <QWindow>

int Toast::LENGTH_LONG = 4000;
int Toast::LENGTH_SHORT = 1500;
static const int TIMER_INTERVAL = 50;
static const int PADDING = 10;

Toast::Toast(const QString &strContent, const QPixmap &icon, int nToastInterval,
             QWidget *parent)
    : QDialog(parent), m_strContent(strContent),
      m_nToastInterval(nToastInterval * TIMER_INTERVAL / 1000),
      m_nCurrentWindowOpacity(0), m_nCurrentStayTime(0), m_nStatus(0),
      m_icon(icon), _parent(parent) {
    Q_ASSERT(parent);
    init();
}

void Toast::toast(QWidget *parent, const QPixmap &icon,
                  const QString &strContent, int nToastInterval) {
    Q_ASSERT(parent);
    static Toast *toast = nullptr;

    if (toast) {
        toast->hide();
        toast->destroy();
        toast->deleteLater();
    }

    static QRegularExpression regex(QStringLiteral("\r\n|\n"));
    auto str = strContent;
    str.remove(regex);
    toast = new Toast(str, icon, nToastInterval, parent);
    connect(toast, &Toast::destroyed, toast, [&] { toast = nullptr; });

    toast->show();
}

Toast::~Toast() {}

QSize Toast::calculateTextSize() {
    QFontMetrics metrice(displayFont());
    int nWidthInPixel = metrice.horizontalAdvance(m_strContent);
    int nHeightInPixel = metrice.height();
    return QSize(nWidthInPixel, nHeightInPixel);
}

void Toast::init() {
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setToastPos(TOAST_POS::BOTTOM);
    auto w = _parent->width();
    constexpr auto PADDING = 100;
    setMaximumWidth(qMax(PADDING, w - PADDING));
}

void Toast::setToastPos(TOAST_POS pos) {
    Q_ASSERT(_parent);
    QRect screenRect = _parent->geometry();
    QSize fontsize = calculateTextSize();

    auto font = this->displayFont();
    auto metric = QFontMetricsF(font);
    auto ICON_PADDING = fontsize.height() + metric.horizontalAdvance(' ');

    QSizeF windowSize;
    if (m_icon.isNull()) {
        windowSize = QSizeF(fontsize.width() + PADDING * 2,
                            fontsize.height() + PADDING * 2);
    } else {
        windowSize = QSizeF((fontsize.width() + ICON_PADDING) + PADDING * 2,
                            fontsize.height() + PADDING * 2);
    }
    auto windowsX =
        screenRect.left() + (screenRect.width() - windowSize.width()) / 2;
    auto windowsY = 0.0;
    if (pos == TOAST_POS::TOP) {
        windowsY = screenRect.top() + screenRect.height() / 6;
    } else if (pos == TOAST_POS::BOTTOM) {
        windowsY = screenRect.top() + screenRect.height() * 4 / 5;
    } else {
        windowsY =
            screenRect.top() + (screenRect.height() - windowSize.height()) / 2;
    }

    m_pos = pos;

    setGeometry(int(windowsX), int(windowsY), int(windowSize.width()),
                int(windowSize.height()));
}

Toast::TOAST_POS Toast::lastToastPos() const { return m_pos; }

void Toast::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                           QPainter::SmoothPixmapTransform);

    QSize widgetSize = size();

    QPen textPen(Qt::PenStyle::SolidLine);
    textPen.setColor(m_textColor);
    textPen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
    painter.fillRect(this->rect(), m_backColor);

    auto font = this->displayFont();
    painter.setFont(font);
    auto metric = QFontMetricsF(font);
    auto ICON_SIZE = metric.height();
    auto ICON_PADDING = metric.horizontalAdvance(' ');
    if (m_icon.isNull()) {
        painter.setPen(textPen);
        QRectF rect(PADDING, PADDING,
                    widgetSize.width() - PADDING - PADDING / 2,
                    widgetSize.height() - PADDING - PADDING);
        painter.drawText(rect, metric.elidedText(m_strContent,
                                                 Qt::TextElideMode::ElideRight,
                                                 rect.width() + PADDING / 2));
    } else {
        painter.drawPixmap(QRectF(PADDING, PADDING, ICON_SIZE, ICON_SIZE),
                           m_icon, {});

        painter.setPen(textPen);
        QRectF rect(PADDING + ICON_SIZE + ICON_PADDING, PADDING,
                    widgetSize.width() - PADDING - PADDING - ICON_SIZE -
                        ICON_PADDING,
                    widgetSize.height() - PADDING - PADDING);
        painter.drawText(rect, metric.elidedText(m_strContent,
                                                 Qt::TextElideMode::ElideRight,
                                                 rect.width()));
    }
}

void Toast::showEvent(QShowEvent *) {
    if (m_nStatus == 0x00) {
        m_nCurrentStayTime = 0;
        m_nCurrentWindowOpacity = 0;
        startTimer(TIMER_INTERVAL);
        m_nStatus = 0x01;
    }
}

void Toast::timerEvent(QTimerEvent *e) {
    if (m_nStatus == 0x01) {
        if (m_nCurrentWindowOpacity < 100) {
            m_nCurrentWindowOpacity += 10;
            update();
        } else {
            m_nStatus = 0x02;
        }
    } else if (m_nStatus == 0x02) {
        if (m_nCurrentStayTime < m_nToastInterval) {
            m_nCurrentStayTime += TIMER_INTERVAL;
        } else {
            m_nStatus = 0x03;
        }
    } else if (m_nStatus == 0x03) {
        if (m_nCurrentWindowOpacity > 0) {
            m_nCurrentWindowOpacity -= 10;
            update();
        } else {
            m_nStatus = 0x04;
        }
    } else if (m_nStatus == 0x04) {
        m_nStatus = 0x00;
        killTimer(e->timerId());
        this->deleteLater();
    }
}

QColor Toast::textColor() const { return m_textColor; }

void Toast::setTextColor(const QColor &textColor) { m_textColor = textColor; }

QFont Toast::displayFont() const {
    auto font = this->font();
    font.setPointSizeF(font.pointSizeF() * 1.8);
    return font;
}

QColor Toast::backColor() const { return m_backColor; }

void Toast::setBackColor(const QColor &backColor) { m_backColor = backColor; }

QPixmap Toast::icon() const { return m_icon; }

void Toast::setIcon(const QPixmap &icon) { m_icon = icon; }
