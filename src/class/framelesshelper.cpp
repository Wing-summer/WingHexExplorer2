#include "framelesshelper.h"

#include "widgetframe/windowbutton.h"
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWKWidgets/widgetwindowagent.h>

FramelessHelper::FramelessHelper(QWidget *parent, bool isDialog)
    : QObject(parent) {
    Q_ASSERT(parent);

    auto windowAgent = new QWK::WidgetWindowAgent(parent);
    windowAgent->setup(parent);

#ifndef Q_OS_MAC
    auto iconButton = new QWK::WindowButton();
    iconButton->setObjectName(QStringLiteral("icon-button"));
    iconButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    auto minButton = new QWK::WindowButton();
    minButton->setObjectName(QStringLiteral("min-button"));
    minButton->setProperty("system-button", true);
    minButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    auto maxButton = new QWK::WindowButton();
    maxButton->setCheckable(true);
    maxButton->setObjectName(QStringLiteral("max-button"));
    maxButton->setProperty("system-button", true);
    maxButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    auto closeButton = new QWK::WindowButton();
    closeButton->setObjectName(QStringLiteral("close-button"));
    closeButton->setProperty("system-button", true);
    closeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
#endif

    m_windowBar = new QWK::WindowBar();

#ifndef Q_OS_MAC
    m_windowBar->setIconButton(iconButton);
    m_windowBar->setMinButton(minButton);
    m_windowBar->setMaxButton(maxButton);
    m_windowBar->setCloseButton(closeButton);
#endif
    m_windowBar->setHostWidget(parent);

    windowAgent->setTitleBar(m_windowBar);

#ifndef Q_OS_MAC
    windowAgent->setSystemButton(QWK::WindowAgentBase::WindowIcon, iconButton);
    windowAgent->setSystemButton(QWK::WindowAgentBase::Minimize, minButton);
    windowAgent->setSystemButton(QWK::WindowAgentBase::Maximize, maxButton);
    windowAgent->setSystemButton(QWK::WindowAgentBase::Close, closeButton);
#endif

    if (isDialog) {
        minButton->hide();
        maxButton->hide();
    }

    windowAgent->setHitTestVisible(m_windowBar, false);

#ifndef Q_OS_MAC
    connect(m_windowBar, &QWK::WindowBar::minimizeRequested, parent,
            &QWidget::showMinimized);
    connect(m_windowBar, &QWK::WindowBar::maximizeRequested, parent,
            [parent, maxButton](bool max) {
                if (max) {
                    parent->showMaximized();
                } else {
                    parent->showNormal();
                }

                // It's a Qt issue that if a QAbstractButton::clicked triggers a
                // window's maximization, the button remains to be hovered until
                // the mouse move. As a result, we need to manually send leave
                // events to the button.
                emulateLeaveEvent(maxButton);
            });
    connect(m_windowBar, &QWK::WindowBar::closeRequested, parent,
            &QWidget::close);
#endif
}

void FramelessHelper::emulateLeaveEvent(QWidget *widget) {
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    QTimer::singleShot(0, widget, [widget]() {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        const QScreen *screen = widget->screen();
#else
        const QScreen *screen = widget->windowHandle()->screen();
#endif
        const QPoint globalPos = QCursor::pos(screen);
        if (!QRect(widget->mapToGlobal(QPoint{0, 0}), widget->size())
                 .contains(globalPos)) {
            QCoreApplication::postEvent(widget, new QEvent(QEvent::Leave));
            if (widget->testAttribute(Qt::WA_Hover)) {
                const QPoint localPos = widget->mapFromGlobal(globalPos);
                const QPoint scenePos =
                    widget->window()->mapFromGlobal(globalPos);
                static constexpr const auto oldPos = QPoint{};
                const Qt::KeyboardModifiers modifiers =
                    QGuiApplication::keyboardModifiers();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
                const auto event = new QHoverEvent(
                    QEvent::HoverLeave, scenePos, globalPos, oldPos, modifiers);
                Q_UNUSED(localPos);
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
                const auto event =  new QHoverEvent(QEvent::HoverLeave, localPos, globalPos, oldPos, modifiers);
                Q_UNUSED(scenePos);
#else
                const auto event =  new QHoverEvent(QEvent::HoverLeave, localPos, oldPos, modifiers);
                Q_UNUSED(scenePos);
#endif
                QCoreApplication::postEvent(widget, event);
            }
        }
    });
}

QWK::WindowBar *FramelessHelper::windowBar() const { return m_windowBar; }
