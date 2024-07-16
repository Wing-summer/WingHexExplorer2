#ifndef WINGPROGRESSDIALOG_H
#define WINGPROGRESSDIALOG_H

#include <QProgressDialog>

#include "../dialog/framelessdialog.h"

class WingProgressDialog {
public:
    WingProgressDialog(const QString &labelText,
                       const QString &cancelButtonText, int minimum,
                       int maximum, QWidget *parent = nullptr);
    ~WingProgressDialog();

    FramelessDialog *pdialog() const;
    QProgressDialog *dialog() const;

private:
    QProgressDialog *m_dialog;
    FramelessDialog *m_d;
};

#endif // WINGPROGRESSDIALOG_H
