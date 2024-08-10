#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include "framelessdialogbase.h"
#include "src/plugin/settingpage.h"
#include <QAbstractButton>

namespace Ui {
class SettingDialog;
}

class SettingDialog : public QWidget {
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog();

    void addPage(WingHex::SettingPage *page);
    void build(); // you can only call once
    void showConfig(int index = -1);

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

    void on_listWidget_currentRowChanged(int currentRow);

private:
    Ui::SettingDialog *ui;
    QList<WingHex::SettingPage *> m_pages;
    FramelessDialogBase *_dialog = nullptr;
};

#endif // SETTINGDIALOG_H
