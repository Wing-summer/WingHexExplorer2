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

#include "framelessdialogbase.h"

#ifdef WINGHEX_USE_FRAMELESS
#include <QWKWidgets/widgetwindowagent.h>

#include "class/settings/settingmanager.h"
#include "widgetframe/windowbutton.h"
#endif

#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

FramelessDialogBase::FramelessDialogBase(QWidget *parent) : QDialog(parent) {
    setWindowFlag(Qt::WindowMaximizeButtonHint, false);
    setWindowFlag(Qt::WindowMinimizeButtonHint, false);

#ifdef WINGHEX_USE_FRAMELESS
    m_useFrameLess = !SettingManager::instance().useNativeTitleBar();
    if (m_useFrameLess) {
        m_helper = new FramelessHelper(this, true);
    }
#endif
}

FramelessDialogBase::~FramelessDialogBase() {}

void FramelessDialogBase::buildUpContent(QWidget *content) {
#ifdef QT_DEBUG
    Q_ASSERT_X(!m_isBuilt, __FUNCTION__, "You can only can it once!");
    Q_ASSERT(content);
#endif

    auto winWidget = new QWidget(this);
    QVBoxLayout *vLayout = new QVBoxLayout(winWidget);
    vLayout->setContentsMargins(1, 1, 1, 1);

#ifdef WINGHEX_USE_FRAMELESS
    if (m_useFrameLess) {
        auto titlebar = m_helper->windowBar();
        auto iconBtn =
            qobject_cast<QWK::WindowButton *>(titlebar->iconButton());
        Q_ASSERT(iconBtn);
        iconBtn->setIconNormal(this->windowIcon());
        connect(this, &FramelessDialogBase::windowIconChanged, iconBtn,
                &QWK::WindowButton::setIconNormal);

        vLayout->addWidget(titlebar);
    }
#endif

    vLayout->addWidget(content, 1);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(winWidget);

    setWindowTitle(content->windowTitle());
    setWindowIcon(content->windowIcon());
    connect(content, &QWidget::windowTitleChanged, this,
            &FramelessDialogBase::setWindowTitle);

#ifdef QT_DEBUG
    m_isBuilt = true;
#endif

    adjustSize();
}

void FramelessDialogBase::showEvent(QShowEvent *event) {
#ifdef QT_DEBUG
    Q_ASSERT_X(m_isBuilt, __FUNCTION__,
               "You must call it when you construct it!");
#endif
    QDialog::showEvent(event);
}

bool FramelessDialogBase::event(QEvent *event) {
#ifdef WINGHEX_USE_FRAMELESS
    if (m_useFrameLess) {
        switch (event->type()) {
        case QEvent::WindowActivate: {
            if (m_helper->windowBar()) {
                m_helper->windowBar()->setProperty("bar-active", true);
                style()->polish(m_helper->windowBar());
            }
            break;
        }

        case QEvent::WindowDeactivate: {
            if (m_helper->windowBar()) {
                m_helper->windowBar()->setProperty("bar-active", false);
                style()->polish(m_helper->windowBar());
            }
            break;
        }

        default:
            break;
        }
    }
#endif
    return QDialog::event(event);
}
