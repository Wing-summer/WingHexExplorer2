#ifndef SCRIPTUISETTINGDIALOG_H
#define SCRIPTUISETTINGDIALOG_H

#include <QWidget>

namespace Ui {
class ScriptUISettingDialog;
}

class ScriptUISettingDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ScriptUISettingDialog(QWidget *parent = nullptr);
    ~ScriptUISettingDialog();

private:
    Ui::ScriptUISettingDialog *ui;
};

#endif // SCRIPTUISETTINGDIALOG_H
