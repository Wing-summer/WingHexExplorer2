#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include "framelessdialog.h"
#include <QAbstractButton>
#include <QDialog>

namespace Ui {
class SettingDialog;
}

class SettingDialog : public FramelessDialog {
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog() override;

    void addPage(QWidget *page);
    void build(); // you can only call once
    void showConfig(int index = -1);
    void reload();
    void setBlocked(bool blocked);

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::SettingDialog *ui;
    QList<QWidget *> m_pages;
};

#endif // SETTINGDIALOG_H
