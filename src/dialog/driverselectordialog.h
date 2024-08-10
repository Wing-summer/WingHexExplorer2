#ifndef DRIVERSELECTORDIALOG_H
#define DRIVERSELECTORDIALOG_H

#include "framelessdialogbase.h"

#include <QDialog>
#include <QList>
#include <QListWidget>
#include <QMainWindow>
#include <QObject>
#include <QStorageInfo>
#include <QTextBrowser>

class DriverSelectorDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    DriverSelectorDialog(QWidget *parent = nullptr);
    QStorageInfo GetResult();

private:
    QListWidget *drivers;
    QTextBrowser *infob;
    QList<QStorageInfo> m_infos;
    QStorageInfo m_si;

    void on_list_selectionChanged();

    void on_accepted();
    void on_rejected();
};

#endif // DRIVERSELECTORDIALOG_H
