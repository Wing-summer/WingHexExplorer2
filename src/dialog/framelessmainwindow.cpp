#include "framelessmainwindow.h"
#include <QEvent>
#include <QStyle>

#ifdef WINGHEX_USE_FRAMELESS
#include "class/settingmanager.h"
#include <widgetframe/windowbutton.h>
#endif

FramelessMainWindow::FramelessMainWindow(QWidget *parent)
    : QMainWindow(parent) {
#ifdef WINGHEX_USE_FRAMELESS
    _useFrameLess = !SettingManager::instance().useNativeTitleBar();
    if (_useFrameLess) {
        _helper = new FramelessHelper(this, false);
    }
#endif
}

void FramelessMainWindow::buildUpContent(QWidget *content) {
#ifdef WINGHEX_USE_FRAMELESS
    if (_useFrameLess) {
        auto titlebar = _helper->windowBar();
        auto iconBtn =
            qobject_cast<QWK::WindowButton *>(titlebar->iconButton());
        Q_ASSERT(iconBtn);
        iconBtn->setIconNormal(this->windowIcon());
        connect(this, &FramelessMainWindow::windowIconChanged, iconBtn,
                &QWK::WindowButton::setIconNormal);

        setMenuWidget(titlebar);
    }
#endif
    setCentralWidget(content);
#ifdef QT_DEBUG
    m_isBuilt = true;
#endif
}

void FramelessMainWindow::showEvent(QShowEvent *event) {
#ifdef QT_DEBUG
    Q_ASSERT_X(m_isBuilt, __FUNCTION__,
               "You must call it when you construct it!");
#endif
    QMainWindow::showEvent(event);
}

bool FramelessMainWindow::event(QEvent *event) {
#ifdef WINGHEX_USE_FRAMELESS
    if (_useFrameLess) {
        switch (event->type()) {
        case QEvent::WindowActivate: {
            if (_helper->windowBar()) {
                _helper->windowBar()->setProperty("bar-active", true);
                style()->polish(_helper->windowBar());
            }
            break;
        }

        case QEvent::WindowDeactivate: {
            if (_helper->windowBar()) {
                _helper->windowBar()->setProperty("bar-active", false);
                style()->polish(_helper->windowBar());
            }
            break;
        }

        default:
            break;
        }
    }
#endif
    return QMainWindow::event(event);
}
