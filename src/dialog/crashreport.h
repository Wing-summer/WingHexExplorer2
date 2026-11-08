#ifndef CRASHREPORT_H
#define CRASHREPORT_H

#include <QAbstractButton>
#include <QWidget>

#include "dialog/framelessdialogbase.h"

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

    FramelessDialogBase *m_dialog = nullptr;
};

#endif // CRASHREPORT_H
