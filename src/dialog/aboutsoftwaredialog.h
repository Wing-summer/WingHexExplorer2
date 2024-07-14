#ifndef ABOUTSOFTWAREDIALOG_H
#define ABOUTSOFTWAREDIALOG_H

#include "framelessdialog.h"

class AboutSoftwareDialog : public FramelessDialog {

    Q_OBJECT
public:
    explicit AboutSoftwareDialog(QWidget *parent = nullptr);
};

#endif // ABOUTSOFTWAREDIALOG_H
