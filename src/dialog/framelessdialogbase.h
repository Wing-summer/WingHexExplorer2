#ifndef FRAMELESSDIALOGBASE_H
#define FRAMELESSDIALOGBASE_H

#ifdef WINGHEX_USE_FRAMELESS
#include "class/framelesshelper.h"
#endif

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
#ifdef WINGHEX_USE_FRAMELESS
    FramelessHelper *_helper = nullptr;
    bool _useFrameLess = true;
#endif
#ifdef QT_DEBUG
    bool m_isBuilt = false;
#endif
};

#endif // FRAMELESSMAINWINDOW_H
