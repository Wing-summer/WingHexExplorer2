#ifndef WINGPLUGINCALLCONVERTOR_H
#define WINGPLUGINCALLCONVERTOR_H

#include <QArgument>

namespace WingHex {

class IWingPluginBase;

class WingPluginCallConvertor {
public:
    WingPluginCallConvertor(QObject *caller);

protected:
    QArgument<IWingPluginBase *> getSenderObj();

private:
    IWingPluginBase *_caller;
};
} // namespace WingHex

#endif // WINGPLUGINCALLCONVERTOR_H
