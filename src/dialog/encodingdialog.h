#ifndef ENCODINGDIALOG_H
#define ENCODINGDIALOG_H

#include "framelessdialogbase.h"

#include <QDialogButtonBox>
#include <QListWidget>
#include <QMainWindow>

class EncodingDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    EncodingDialog(QWidget *parent = nullptr);
    QString getResult();

private:
    void on_accept();
    void on_reject();

private:
    QListWidget *enclist;
    QString result;
};

#endif // ENCODINGDIALOG_H
