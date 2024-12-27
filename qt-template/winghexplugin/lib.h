%{Cpp:LicenseTemplate}\
@if '%{Cpp:PragmaOnce}'
#pragma once
@else
#ifndef %{GUARD}
#define %{GUARD}
@endif

#include "iwingplugin.h"

class %{CN} final : public WingHex::IWingPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.wingsummer.iwingplugin" FILE "%{PluginJsonFile}")
    Q_INTERFACES(WingHex::IWingPlugin)

public:
    explicit %{CN}();
    virtual ~%{CN}();

// If you are writing codes with QtCreator,
// you can press Alt+Enter in a empty line or line with spaces only to
// trigger "Insert Virtual Functions of Base Classes" to select optional
// virtual functions you want to implement.

public:
    virtual int sdkVersion() const override;
    virtual const QString signature() const override;
    virtual bool init(const std::unique_ptr<QSettings> &set) override;
    virtual void unload(std::unique_ptr<QSettings> &set) override;
    virtual const QString pluginName() const override;
    virtual const QString pluginAuthor() const override;
    virtual uint pluginVersion() const override;
    virtual const QString pluginComment() const override;

private:
    // Your private members
};

@if ! '%{Cpp:PragmaOnce}'
#endif // %{GUARD}
@endif
