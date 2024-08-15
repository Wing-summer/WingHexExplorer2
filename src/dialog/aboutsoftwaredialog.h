#ifndef ABOUTSOFTWAREDIALOG_H
#define ABOUTSOFTWAREDIALOG_H

#include "framelessdialogbase.h"

namespace Ui {
class AboutSoftwareDialog;
}

class AboutSoftwareDialog : public QWidget {
    Q_OBJECT

public:
    explicit AboutSoftwareDialog(QWidget *parent = nullptr);
    ~AboutSoftwareDialog();

public slots:
    int exec();

private:
    Ui::AboutSoftwareDialog *ui;

    FramelessDialogBase *_dialog;
};

#endif // ABOUTSOFTWAREDIALOG_H
