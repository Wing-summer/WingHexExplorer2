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

#include "../class/eventfilter.h"

int Toast::LENGTH_LONG = 4000;
int Toast::LENGTH_SHORT = 1500;
static const int TIMER_INTERVAL = 50;
static const int PADDING = 10;

Toast::Toast(const QString &strContent, const QPixmap &icon, int nToastInterval,
             QWidget *parent)
    : QWidget(parent), m_strContent(strContent),
      m_nToastInterval(nToastInterval), m_nCurrentWindowOpacity(0),
      m_nCurrentStayTime(0), m_nStatus(0), m_icon(icon) {
    Q_ASSERT(parent);
    init();
}

void Toast::toast(QWidget *parent, const QPixmap &icon,
                  const QString &strContent, int fontPointSize,
                  int nToastInterval) {
    static Toast *toast = nullptr;

    if (toast) {
        toast->deleteLater();
    }

    toast = new Toast(strContent, icon, nToastInterval, parent);
    toast->m_drawFont.setPointSize(fontPointSize);

    connect(toast, &Toast::destroyed, [&] { toast = nullptr; });

    auto e0 = new EventFilter(QEvent::Move, parent);
    auto e1 = new EventFilter(QEvent::Resize, parent);

    auto callback = [&](QObject *, QEvent *) {
        toast->setToastPos(toast->lastToastPos());
    };
    connect(e0, &EventFilter::eventTriggered, toast, callback);
    connect(e1, &EventFilter::eventTriggered, toast, callback);

    parent->installEventFilter(e0);
    parent->installEventFilter(e1);

    toast->show();
}

Toast::~Toast() {}

void Toast::setTextFont(const QFont &font) { m_drawFont = font; }

const QFont &Toast::textFont() { return m_drawFont; }

void Toast::setFontPointSize(int fontSize) {
    m_drawFont.setPointSize(fontSize);
    setToastPos(m_pos);
}

int Toast::fontPointSize() const { return m_drawFont.pointSize(); }

QSize Toast::calculateTextSize() {
    QFontMetrics metrice(m_drawFont);
    QStringList ls = m_strContent.split(QRegularExpression("\r\n|\n"));
    int nWidthInPixel = 0;
    for (int i = 0; i < ls.size(); i++) {
        int tmpWidth = metrice.horizontalAdvance(ls.at(i));
        nWidthInPixel = tmpWidth > nWidthInPixel ? tmpWidth : nWidthInPixel;
    }
    int nHeightInPixel = metrice.height() * ls.length();
    return QSize(nWidthInPixel, nHeightInPixel);
}

void Toast::init() {
    setWindowFlags(Qt::FramelessWindowHint | Qt::SubWindow);
    setAttribute(Qt::WA_TranslucentBackground);
    m_drawFont.setPointSize(20);
    setToastPos(TOAST_POS::BOTTOM);
}

void Toast::setToastPos(TOAST_POS pos) {
    QRect screenRect = this->parentWidget()->geometry();
    QSize fontsize = calculateTextSize();
    QSizeF windowSize((fontsize.width() + m_drawFont.pointSize() + 5) +
                          PADDING * 2,
                      fontsize.height() + PADDING * 2);
    auto windowsX = (screenRect.width() - windowSize.width()) / 2;
    auto windowsY = 0.0;
    if (pos == TOAST_POS::TOP) {
        windowsY = screenRect.height() / 6;
    } else if (pos == TOAST_POS::BOTTOM) {
        windowsY = screenRect.height() * 4 / 5;
    } else {
        windowsY = (screenRect.height() - windowSize.height()) / 2;
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

    painter.setFont(m_drawFont);

    QPen textPen(Qt::PenStyle::SolidLine);

    textPen.setColor(m_textColor);
    textPen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);

    QPen emptyPen(Qt::PenStyle::NoPen);

    painter.setPen(emptyPen);
    painter.setBrush(m_backColor);
    painter.drawRoundedRect(0, 0, widgetSize.width(), widgetSize.height(), 10,
                            10);

    auto ICON_SIZE = m_drawFont.pointSize();
    auto ICON_PADDING = ICON_SIZE + 5;

    painter.drawPixmap(QRect(PADDING, int(widgetSize.height() - ICON_SIZE) / 2,
                             ICON_SIZE, ICON_SIZE),
                       m_icon);

    painter.setPen(textPen);
    painter.drawText(PADDING + ICON_PADDING, widgetSize.height() / 2 + PADDING,
                     m_strContent);
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

QColor Toast::backColor() const { return m_backColor; }

void Toast::setBackColor(const QColor &backColor) { m_backColor = backColor; }

QPixmap Toast::icon() const { return m_icon; }

void Toast::setIcon(const QPixmap &icon) { m_icon = icon; }
