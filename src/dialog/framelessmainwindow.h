#ifndef FRAMELESSMAINWINDOW_H
#define FRAMELESSMAINWINDOW_H

#ifdef WINGHEX_USE_FRAMELESS
#include "class/framelesshelper.h"
#endif

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
#ifdef WINGHEX_USE_FRAMELESS
    FramelessHelper *_helper = nullptr;
    bool _useFrameLess = true;
#endif
#ifdef QT_DEBUG
    bool m_isBuilt = false;
#endif
};

#endif // FRAMELESSMAINWINDOW_H
