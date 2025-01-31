#ifndef DOCKWIDGETTAB_H
#define DOCKWIDGETTAB_H

#include "Qt-Advanced-Docking-System/src/DockWidgetTab.h"

class DockWidgetTab : public ads::CDockWidgetTab {
    Q_OBJECT

public:
    DockWidgetTab(ads::CDockWidget *DockWidget, QWidget *parent = nullptr);

    // CDockAreaTitleBar interface
public:
    virtual QMenu *buildContextMenu(QMenu *menu) override;

private:
    void initMenuItems(QMenu *menu, const QString &path);
};

#endif // DOCKWIDGETTAB_H
