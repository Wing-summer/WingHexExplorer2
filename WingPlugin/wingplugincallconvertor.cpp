#include "wingplugincallconvertor.h"

#include "WingPlugin/wingplugin_global.h"
#include "iwingpluginbase.h"

WingHex::WingPluginCallConvertor::WingPluginCallConvertor(QObject *caller) {
    _caller = reinterpret_cast<WingHex::IWingPluginBase *>(caller);
    Q_ASSERT(_caller);
}

QArgument<WingHex::IWingPluginBase *>
WingHex::WingPluginCallConvertor::getSenderObj() {
    return WINGAPI_ARG(_caller);
}
