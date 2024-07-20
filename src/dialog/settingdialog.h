#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include "../plugin/settingpage.h"
#include "framelessdialog.h"
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

private:
    Ui::SettingDialog *ui;
    QList<WingHex::SettingPage *> m_pages;
    FramelessDialog *_dialog = nullptr;
};

#endif // SETTINGDIALOG_H
