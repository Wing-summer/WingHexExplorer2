#ifndef GENERALSETTINGDIALOG_H
#define GENERALSETTINGDIALOG_H

#include <QWidget>

namespace Ui {
class GeneralSettingDialog;
}

class GeneralSettingDialog : public QWidget {
    Q_OBJECT

public:
    explicit GeneralSettingDialog(QWidget *parent = nullptr);
    ~GeneralSettingDialog();

private:
    Ui::GeneralSettingDialog *ui;
};

#endif // GENERALSETTINGDIALOG_H
