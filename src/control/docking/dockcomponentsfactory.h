#ifndef DOCKCOMPONENTSFACTORY_H
#define DOCKCOMPONENTSFACTORY_H

#include "Qt-Advanced-Docking-System/src/DockComponentsFactory.h"

class DockComponentsFactory : public ads::CDockComponentsFactory {
public:
    // CDockComponentsFactory interface
public:
    virtual ads::CDockWidgetTab *
    createDockWidgetTab(ads::CDockWidget *DockWidget) const override;
};

#endif // DOCKCOMPONENTSFACTORY_H
