#include "dockcomponentsfactory.h"

#include "DockWidget.h"
#include "control/dockwidgettab.h"

ads::CDockWidgetTab *
DockComponentsFactory::createDockWidgetTab(ads::CDockWidget *DockWidget) const {
    return new DockWidgetTab(DockWidget);
}
