#ifndef WINGANGELAPI_H
#define WINGANGELAPI_H

#include "src/plugin/iwingplugin.h"

class asIScriptEngine;

class WingAngelAPI : public WingHex::IWingPlugin {
    Q_OBJECT

    // IWingPlugin interface
public:
    virtual int sdkVersion() const override;
    virtual const QString signature() const override;
    virtual bool
    init(const QList<WingHex::WingPluginInfo> &loadedplugin) override;
    virtual void unload() override;
    virtual const QString pluginName() const override;
    virtual const QString pluginAuthor() const override;
    virtual uint pluginVersion() const override;
    virtual const QString pluginComment() const override;

    void installAPI(asIScriptEngine *engine);

public slots:
    virtual void plugin2MessagePipe(WingHex::WingPluginMessage type,
                                    const QVariantList &msg) override;

private:
    void installLogAPI(asIScriptEngine *engine);
};

#endif // WINGANGELAPI_H
