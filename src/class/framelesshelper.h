#ifndef FRAMELESSHELPER_H
#define FRAMELESSHELPER_H

#include "src/widgetframe/windowbar.h"
#include <QObject>

class FramelessHelper : public QObject {
    Q_OBJECT
public:
    FramelessHelper(QWidget *parent, bool isDialog);

    QWK::WindowBar *windowBar() const;

private:
    static void emulateLeaveEvent(QWidget *widget);

private:
    QWK::WindowBar *m_windowBar = nullptr;
};

#endif // FRAMELESSHELPER_H
