#ifndef PLUGINSETTINGDIALOG_H
#define PLUGINSETTINGDIALOG_H

#include <QWidget>

namespace Ui {
class PluginSettingDialog;
}

class PluginSettingDialog : public QWidget {
    Q_OBJECT

public:
    explicit PluginSettingDialog(QWidget *parent = nullptr);
    ~PluginSettingDialog();

private:
    Ui::PluginSettingDialog *ui;
};

#endif // PLUGINSETTINGDIALOG_H
