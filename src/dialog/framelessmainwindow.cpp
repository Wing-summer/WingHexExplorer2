#include "framelessmainwindow.h"
#include <QEvent>
#include <QStyle>

#include "class/settingmanager.h"
#include <widgetframe/windowbutton.h>

FramelessMainWindow::FramelessMainWindow(QWidget *parent)
    : QMainWindow(parent) {
    _useFrameLess = !SettingManager::instance().useNativeTitleBar();
    if (_useFrameLess) {
        _helper = new FramelessHelper(this, false);
    }
}

void FramelessMainWindow::buildUpContent(QWidget *content) {
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
    return QMainWindow::event(event);
}
