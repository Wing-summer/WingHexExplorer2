#include "wingangelapi.h"

#include "AngelScript/angelscript/include/angelscript.h"

int WingAngelAPI::sdkVersion() const { return WingHex::SDKVERSION; }

const QString WingAngelAPI::signature() const { return WingHex::WINGSUMMER; }

bool WingAngelAPI::init(const QList<WingHex::WingPluginInfo> &loadedplugin) {
    Q_UNUSED(loadedplugin);
    return true;
}

void WingAngelAPI::unload() { this->disconnect(); }

const QString WingAngelAPI::pluginName() const {
    return tr("AngelScriptService");
}

const QString WingAngelAPI::pluginAuthor() const { return WingHex::WINGSUMMER; }

uint WingAngelAPI::pluginVersion() const { return WingHex::SDKVERSION; }

const QString WingAngelAPI::pluginComment() const {
    return tr("A internal plugin that provides AngelScript scripts with the "
              "ability to call the host API.");
}

void WingAngelAPI::installAPI(asIScriptEngine *engine) {}

void WingAngelAPI::plugin2MessagePipe(WingHex::WingPluginMessage type,
                                      const QVariantList &msg) {}
