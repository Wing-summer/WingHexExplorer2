#ifndef FRAMELESSMAINWINDOW_H
#define FRAMELESSMAINWINDOW_H

#include "src/class/framelesshelper.h"
#include <QMainWindow>

class FramelessMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit FramelessMainWindow(QWidget *parent = nullptr);

    void buildUpContent(QWidget *content);

private:
    FramelessHelper *_helper = nullptr;
};

#endif // FRAMELESSMAINWINDOW_H
