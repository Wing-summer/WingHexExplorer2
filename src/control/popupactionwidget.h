#ifndef POPUPACTIONWIDGET_H
#define POPUPACTIONWIDGET_H

#include <QWidgetAction>

class PopupActionWidget : public QWidgetAction {
    Q_OBJECT
public:
    explicit PopupActionWidget(QObject *parent = nullptr);

protected:
    void closePopUpWidget();
};

#endif // POPUPACTIONWIDGET_H
