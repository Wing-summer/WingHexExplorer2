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
      m_nToastInterval(nToastInterval), m_nCurrentWindowOpacity(0),
      m_nCurrentStayTime(0), m_nStatus(0), m_nFadeStep(0), m_icon(icon),
      _parent(parent) {
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
    setWindowOpacity(1.0);
}

void Toast::setToastPos(TOAST_POS pos) {
    Q_ASSERT(_parent);

    QSize fontsize = calculateTextSize();
    auto metric = QFontMetricsF(displayFont());
    qreal ICON_PADDING = metric.height() + metric.horizontalAdvance(' ');

    QSizeF windowSize;
    if (m_icon.isNull()) {
        windowSize = QSizeF(fontsize.width() + PADDING * 2 + 1,
                            fontsize.height() + PADDING * 2);
    } else {
        windowSize = QSizeF((fontsize.width() + ICON_PADDING) + PADDING * 2 + 1,
                            fontsize.height() + PADDING * 2);
    }

    double localX = (_parent->width() - windowSize.width()) / 2.0;
    double localY;
    if (pos == TOAST_POS::TOP) {
        localY = _parent->height() / 6.0;
    } else if (pos == TOAST_POS::BOTTOM) {
        localY = _parent->height() * 5 / 6.0 - windowSize.height();
    } else {
        localY = (_parent->height() - windowSize.height()) / 2.0;
    }

    m_pos = pos;

    QPoint globalPos =
        _parent->mapToGlobal(QPoint(qRound(localX), qRound(localY)));
    setGeometry(globalPos.x(), globalPos.y(), qCeil(windowSize.width()),
                qCeil(windowSize.height()));
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
        QRectF rect(PADDING, PADDING, widgetSize.width() - PADDING - PADDING,
                    widgetSize.height() - PADDING - PADDING);
        painter.drawText(rect, metric.elidedText(m_strContent,
                                                 Qt::TextElideMode::ElideRight,
                                                 rect.width()));
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
    if (windowHandle() && _parent && _parent->window() &&
        _parent->window()->windowHandle()) {
        windowHandle()->setTransientParent(_parent->window()->windowHandle());
    }
    if (m_nStatus == 0x00) {
        m_nCurrentStayTime = 0;
        m_nCurrentWindowOpacity = 100;
        startTimer(TIMER_INTERVAL);
        m_nStatus = 0x02;
    }
}

void Toast::timerEvent(QTimerEvent *e) {
    constexpr int FADE_STEP = 10;
    constexpr int FADE_MS = 400;

    if (m_nStatus == 0x02) {
        if (m_nCurrentStayTime < m_nToastInterval) {
            m_nCurrentStayTime += TIMER_INTERVAL;
            return;
        }

        int fadeSteps = (FADE_MS + TIMER_INTERVAL - 1) / TIMER_INTERVAL;
        fadeSteps = qMax(1, fadeSteps);
        m_nFadeStep = (100 + fadeSteps - 1) / fadeSteps;
        if (m_nFadeStep < 1)
            m_nFadeStep = 1;
        m_nStatus = 0x03;
        return;
    }

    if (m_nStatus == 0x03) {
        if (m_nCurrentWindowOpacity > 0) {
            m_nCurrentWindowOpacity -= m_nFadeStep;
            if (m_nCurrentWindowOpacity < 0)
                m_nCurrentWindowOpacity = 0;
            setWindowOpacity(qMax(0.0, m_nCurrentWindowOpacity / 100.0));
            update();
            return;
        }
        m_nStatus = 0x04;
    }

    if (m_nStatus == 0x04) {
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
