#ifndef FRAMELESSDIALOG_H
#define FRAMELESSDIALOG_H

#include "framelesswindowbase.h"

#include <QGraphicsDropShadowEffect>

class FramelessDialog : public FramelessWindowBase {
    Q_OBJECT
public:
    FramelessDialog(QWidget *parent = nullptr);
    virtual ~FramelessDialog();

    void buildUpContent(QWidget *content);

protected:
    void onDealedWithBorder(bool needHide) override;

private:
    QGraphicsDropShadowEffect *m_winEffect = nullptr;
};

#endif // FRAMELESSDIALOG_H
