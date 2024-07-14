#include "framelessmainwindow.h"

FramelessMainWindow::FramelessMainWindow(QWidget *parent)
    : FramelessWindowBase(parent) {
    this->setContentsMargins(0, 0, 0, 0);
    setResizeable(true, this->contentsMargins());
}

void FramelessMainWindow::buildUpContent(QWidget *content) {
    FramelessWindowBase::buildUpContent(TitleBar::TITLE_ALL, content);
}

void FramelessMainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        event->ignore();
        return;
    }
    return FramelessWindowBase::keyPressEvent(event);
}

void FramelessMainWindow::done(int code) {
    auto closeE = new QCloseEvent;
    this->closeEvent(closeE);
    if (closeE->isAccepted()) {
        FramelessWindowBase::done(code);
    }
}

void FramelessMainWindow::closeEvent(QCloseEvent *) {}
