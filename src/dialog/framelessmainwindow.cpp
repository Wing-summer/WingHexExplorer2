#include "framelessmainwindow.h"

#include <src/widgetframe/windowbutton.h>

FramelessMainWindow::FramelessMainWindow(QWidget *parent)
    : QMainWindow(parent) {
    _helper = new FramelessHelper(this, false);
}

void FramelessMainWindow::buildUpContent(QWidget *content) {
    auto titlebar = _helper->windowBar();
    auto iconBtn = qobject_cast<QWK::WindowButton *>(titlebar->iconButton());
    Q_ASSERT(iconBtn);
    iconBtn->setIconNormal(this->windowIcon());
    connect(this, &FramelessMainWindow::windowIconChanged, iconBtn,
            &QWK::WindowButton::setIconNormal);
    setMenuWidget(titlebar);
    setCentralWidget(content);
}
