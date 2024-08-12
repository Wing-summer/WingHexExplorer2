#ifndef FRAMELESSDIALOGBASE_H
#define FRAMELESSDIALOGBASE_H

#include "class/framelesshelper.h"
#include <QDialog>

class FramelessDialogBase : public QDialog {
    Q_OBJECT

public:
    explicit FramelessDialogBase(QWidget *parent = nullptr);

    virtual ~FramelessDialogBase();

    void buildUpContent(QWidget *content);

protected:
    void showEvent(QShowEvent *event) override;
    bool event(QEvent *event) override;

private:
    FramelessHelper *_helper = nullptr;
#ifdef QT_DEBUG
    bool m_isBuilt = false;
#endif
};

#endif // FRAMELESSMAINWINDOW_H
