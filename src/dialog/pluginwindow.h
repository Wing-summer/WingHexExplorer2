#ifndef PLUGINWINDOW_H
#define PLUGINWINDOW_H

#include "framelessdialogbase.h"
#include "plugin/pluginsystem.h"

#include <QListWidget>
#include <QTextBrowser>

class PluginWindow : public FramelessDialogBase {
    Q_OBJECT
public:
    PluginWindow(QWidget *parent = nullptr);
    void setPluginSystem(PluginSystem *pluginsys);
    virtual ~PluginWindow();

private:
    PluginSystem *m_pluginsys;
    QListWidget *plglist;
    QTextBrowser *txtb;
    QTextBrowser *txtc;

    void on_list_selchanged();
};

#endif // PLUGINWINDOW_H
