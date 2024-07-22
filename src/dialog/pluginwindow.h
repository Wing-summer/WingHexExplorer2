#ifndef PLUGINWINDOW_H
#define PLUGINWINDOW_H

#include "framelessdialog.h"
#include "src/plugin/pluginsystem.h"

#include <QListWidget>
#include <QTextBrowser>

class PluginWindow : public FramelessDialog {
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
