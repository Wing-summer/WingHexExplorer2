#ifndef FRAMELESSMAINWINDOW_H
#define FRAMELESSMAINWINDOW_H

#include "framelesswindowbase.h"

class FramelessMainWindow : public FramelessWindowBase {
    Q_OBJECT
public:
    FramelessMainWindow(QWidget *parent = nullptr);

    void buildUpContent(QWidget *content);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void done(int code) override;
    void closeEvent(QCloseEvent *) override;
};

#endif // FRAMELESSMAINWINDOW_H
