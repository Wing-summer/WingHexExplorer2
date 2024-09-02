#include "framelessdialogbase.h"
#include "widgetframe/windowbutton.h"

#include "class/settingmanager.h"
#include <QWKWidgets/widgetwindowagent.h>

#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

FramelessDialogBase::FramelessDialogBase(QWidget *parent) : QDialog(parent) {
    _useFrameLess = !SettingManager::instance().useNativeTitleBar();
    if (_useFrameLess) {
        _helper = new FramelessHelper(this, true);
    }
}

FramelessDialogBase::~FramelessDialogBase() {}

void FramelessDialogBase::buildUpContent(QWidget *content) {
#ifdef QT_DEBUG
    Q_ASSERT_X(!m_isBuilt, __FUNCTION__, "You can only can it once!");
    Q_ASSERT(content);
#endif

    auto WIN_WIDGET = new QWidget(this);
    QVBoxLayout *vLayout = new QVBoxLayout(WIN_WIDGET);
    vLayout->setContentsMargins(1, 1, 1, 1);

    if (_useFrameLess) {
        auto titlebar = _helper->windowBar();
        auto iconBtn =
            qobject_cast<QWK::WindowButton *>(titlebar->iconButton());
        Q_ASSERT(iconBtn);
        iconBtn->setIconNormal(this->windowIcon());
        connect(this, &FramelessDialogBase::windowIconChanged, iconBtn,
                &QWK::WindowButton::setIconNormal);

        vLayout->addWidget(titlebar);
    }

    vLayout->addWidget(content, 1);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(WIN_WIDGET);

#ifdef QT_DEBUG
    m_isBuilt = true;
#endif
}

void FramelessDialogBase::showEvent(QShowEvent *event) {
#ifdef QT_DEBUG
    Q_ASSERT_X(m_isBuilt, __FUNCTION__,
               "You must call it when you construct it!");
#endif
    QDialog::showEvent(event);
}

bool FramelessDialogBase::event(QEvent *event) {
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
    return QDialog::event(event);
}
