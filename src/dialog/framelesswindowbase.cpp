#include "framelesswindowbase.h"
#include "src/class/eventfilter.h"

#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>

FramelessWindowBase::FramelessWindowBase(QWidget *parent)
    : FramelessWindow<QDialog>(true, parent, true) {
    WIN_WIDGET = new QWidget(this);
    WIN_WIDGET->setObjectName(QStringLiteral("WIN_WIDGET"));
}

FramelessWindowBase::~FramelessWindowBase() {}

void FramelessWindowBase::buildUpContent(TitleBar::TitleItems title,
                                         QWidget *content) {
    Q_ASSERT_X(!m_isBuilt, __FUNCTION__, "You can only can it once!");
    Q_ASSERT(content);

    m_titlebar = new TitleBar(this->windowTitle(), title);

    setTitlebar({m_titlebar->titleHolder()});
    connect(m_titlebar, &TitleBar::titleCloseButtonClicked, this,
            [this] { done(DialogCode::Rejected); });
    connect(m_titlebar, &TitleBar::titleMinButtonClicked, this,
            &FramelessWindowBase::showMinimized);
    connect(m_titlebar, &TitleBar::titleMaxButtonClicked, this,
            [this](bool needMax) {
                if (needMax) {
                    this->show(WindowState::Maximized);
                } else {
                    this->show(WindowState::Normal);
                }
            });

    connect(this, &FramelessWindowBase::windowTitleChanged, m_titlebar,
            &TitleBar::setTitle);
    connect(this, &FramelessWindowBase::windowIconChanged, m_titlebar,
            &TitleBar::setWindowIcon);

    auto painte = new EventFilter(QEvent::Paint, WIN_WIDGET);
    connect(painte, &EventFilter::eventTriggered, this,
            [this](QObject *obj, QEvent *event) {
                auto e = static_cast<QPaintEvent *>(event);
                Q_ASSERT(e->type() == QEvent::Paint);
                auto widget = qobject_cast<QWidget *>(obj);
                if (widget != WIN_WIDGET) {
                    return;
                }
                QPainter p(widget);
                p.setRenderHint(QPainter::Antialiasing);
                QPainterPath path;
                path.setFillRule(Qt::FillRule::WindingFill);
                auto w = widget->width() - 2;
                auto h = this->m_titlebar->titleHeight() + 1;
                auto radius = 8;
                if (this->isMaximized() || this->isFullScreen()) {
                    path.addRect(QRectF(1, 1, w, h));
                } else {
                    path.addRoundedRect(QRectF(1, 1, w, h), radius, radius);
                    path.addRect(QRectF(1, h - radius - 1, w, radius + 2));
                }
                p.fillPath(path, this->titleColor());
            });

    WIN_WIDGET->installEventFilter(painte);

    QVBoxLayout *vLayout = new QVBoxLayout(WIN_WIDGET);
    vLayout->setContentsMargins(1, 1, 1, 1);
    vLayout->addWidget(m_titlebar);
    content->setObjectName(QStringLiteral("WIN_WIDGET_CONTENT"));
    vLayout->addWidget(content, 1);

    auto hMain = new QHBoxLayout(this);
    hMain->setContentsMargins(0, 0, 0, 0);
    hMain->addWidget(WIN_WIDGET);

    // init mouse tracking
    initMouseTracking();

    m_isBuilt = true;
}

bool FramelessWindowBase::event(QEvent *event) {
    if (event->type() == QEvent::WindowStateChange) {
        if (this->isMaximized() || this->isFullScreen()) {
            WIN_WIDGET->setStyleSheet(
                QStringLiteral("#WIN_WIDGET{border-radius:0px;}"));
            m_titlebar->setIsMaximizedFlag(true);
        } else {
            WIN_WIDGET->setStyleSheet(QString());
            m_titlebar->setIsMaximizedFlag(false);
        }
        WIN_WIDGET->update();
    } else if (event->type() == QEvent::Resize) {
        QRect rect = this->screen()->availableGeometry();
        bool equalWidth = this->width() == rect.width();
        bool equalHeight = this->height() == rect.height();
        if (equalWidth || equalHeight) {
            this->setContentsMargins(0, 0, 0, 0);
            WIN_WIDGET->setStyleSheet(
                QStringLiteral("#WIN_WIDGET{border-radius:0px;}"));
            onDealedWithBorder(true);
        } else {
            this->setContentsMargins(m_transparentMargsins);
            onDealedWithBorder(false);
        }
        WIN_WIDGET->update();
    }
    return QDialog::event(event);
}

void FramelessWindowBase::showEvent(QShowEvent *event) {
    Q_ASSERT_X(m_isBuilt, __FUNCTION__,
               "You must call it when you construct it!");
    QDialog::showEvent(event);
}

bool FramelessWindowBase::onTitleMovingBegin(const QPoint &pt,
                                             bool &needProcess) {
    Q_UNUSED(pt);
    needProcess = false;
    if (m_titlebar->testIsMaximized()) {
        needProcess = true;
#ifdef Q_OS_LINUX
        // may be some fix on dde
        // 这时候不使用 SystemMove ，要不然容易出问题，
        // 有更好的解决方案再说
        return false;
#endif
    }
    return true;
}

void FramelessWindowBase::onTitleMoving(const QPoint &pt, bool needProcess) {
    Q_UNUSED(pt);
    if (needProcess) {
        m_titlebar->setIsMaximizedFlag(false);
        this->show(WindowState::Normal);
        WIN_WIDGET->repaint();
    }
}

void FramelessWindowBase::onDealedWithBorder(bool needHide) {
    Q_UNUSED(needHide);
}

QColor FramelessWindowBase::titleColor() const { return m_titleColor; }

void FramelessWindowBase::setTitleColor(const QColor &newTitleColor) {
    if (m_titleColor == newTitleColor)
        return;
    m_titleColor = newTitleColor;
    emit titleColorChanged();
}

int FramelessWindowBase::titleHeight() const {
    return m_titlebar->titleHeight();
}
