#ifndef WINGPROGRESSDIALOG_H
#define WINGPROGRESSDIALOG_H

#include <QProgressDialog>

#include "../dialog/framelessdialogbase.h"

class WingProgressDialog {
public:
    WingProgressDialog(const QString &labelText,
                       const QString &cancelButtonText, int minimum,
                       int maximum, QWidget *parent = nullptr);
    ~WingProgressDialog();

    FramelessDialogBase *pdialog() const;
    QProgressDialog *dialog() const;

private:
    QProgressDialog *m_dialog;
    FramelessDialogBase *m_d;
};

#endif // WINGPROGRESSDIALOG_H
