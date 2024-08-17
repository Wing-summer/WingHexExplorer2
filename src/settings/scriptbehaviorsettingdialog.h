#ifndef SCRIPTBEHAVIORSETTINGDIALOG_H
#define SCRIPTBEHAVIORSETTINGDIALOG_H

#include <QWidget>

namespace Ui {
class ScriptBehaviorSettingDialog;
}

class ScriptBehaviorSettingDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ScriptBehaviorSettingDialog(QWidget *parent = nullptr);
    ~ScriptBehaviorSettingDialog();

private:
    Ui::ScriptBehaviorSettingDialog *ui;
};

#endif // SCRIPTBEHAVIORSETTINGDIALOG_H
