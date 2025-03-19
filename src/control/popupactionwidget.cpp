#include "popupactionwidget.h"

#include <QApplication>
#include <QTimer>
#include <QWidget>

PopupActionWidget::PopupActionWidget(QObject *parent) : QWidgetAction(parent) {}

void PopupActionWidget::closePopUpWidget() {
    QTimer::singleShot(100, qApp, []() {
        auto popupWidget = QApplication::activePopupWidget();
        if (popupWidget) {
            popupWidget->close();
        }
    });
}
