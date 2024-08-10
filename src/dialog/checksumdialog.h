#ifndef CHECKSUMDIALOG_H
#define CHECKSUMDIALOG_H

#include "framelessdialogbase.h"

#include <QListWidget>

class CheckSumDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    explicit CheckSumDialog(QWidget *parent = nullptr);

    const QVector<int> &getResults();

private slots:
    void on_accept();
    void on_reject();

private:
    QListWidget *hashlist = nullptr;
    QVector<int> _result;
};

#endif // CHECKSUMDIALOG_H
