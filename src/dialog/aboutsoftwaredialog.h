#ifndef ABOUTSOFTWAREDIALOG_H
#define ABOUTSOFTWAREDIALOG_H

#include "framelessdialogbase.h"

class AboutSoftwareDialog : public FramelessDialogBase {

    Q_OBJECT
public:
    explicit AboutSoftwareDialog(QWidget *parent = nullptr);
};

#endif // ABOUTSOFTWAREDIALOG_H
