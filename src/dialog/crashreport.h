#ifndef CRASHREPORT_H
#define CRASHREPORT_H

#include "dialog/framelessdialogbase.h"

#include <QAbstractButton>
#include <QWidget>

namespace Ui {
class CrashReport;
}

class CrashReport : public QWidget {
    Q_OBJECT

public:
    explicit CrashReport(QWidget *parent = nullptr);
    ~CrashReport();

public slots:
    void setInfomation(const QString &info);
    int exec();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::CrashReport *ui;

    FramelessDialogBase *_dialog = nullptr;
};

#endif // CRASHREPORT_H
