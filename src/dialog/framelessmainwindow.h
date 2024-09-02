#ifndef FRAMELESSMAINWINDOW_H
#define FRAMELESSMAINWINDOW_H

#include "class/framelesshelper.h"
#include <QMainWindow>

class FramelessMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit FramelessMainWindow(QWidget *parent = nullptr);

    void buildUpContent(QWidget *content);

protected:
    void showEvent(QShowEvent *event) override;
    bool event(QEvent *event) override;

private:
    FramelessHelper *_helper = nullptr;
    bool _useFrameLess = true;
#ifdef QT_DEBUG
    bool m_isBuilt = false;
#endif
};

#endif // FRAMELESSMAINWINDOW_H
