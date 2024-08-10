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

void WingAngelAPI::installAPI(asIScriptEngine *engine) {
    // TODO: install void toast(const QPixmap &icon, const QString &message);
    installLogAPI(engine);
}

void WingAngelAPI::plugin2MessagePipe(WingHex::WingPluginMessage type,
                                      const QVariantList &msg) {}

void WingAngelAPI::installLogAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("Log");
    assert(r >= 0);

    {
        static std::function<void(const QString &)> fn =
            std::bind(&WingAngelAPI::info, this, std::placeholders::_1);
        r = engine->RegisterGlobalFunction("void info(string &in)",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        assert(r >= 0);
    }

    {
        static std::function<void(const QString &)> fn =
            std::bind(&WingAngelAPI::debug, this, std::placeholders::_1);
        r = engine->RegisterGlobalFunction("void debug(string &in)",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        assert(r >= 0);
    }

    {
        static std::function<void(const QString &)> fn =
            std::bind(&WingAngelAPI::warn, this, std::placeholders::_1);
        r = engine->RegisterGlobalFunction("void warn(string &in)",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        assert(r >= 0);
    }

    {
        static std::function<void(const QString &)> fn =
            std::bind(&WingAngelAPI::error, this, std::placeholders::_1);
        r = engine->RegisterGlobalFunction("void error(string &in)",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        assert(r >= 0);
    }

    engine->SetDefaultNamespace("");
}
