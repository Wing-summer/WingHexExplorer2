%{Cpp:LicenseTemplate}\
#include "%{JS: Util.relativeFilePath('%{Path}/%{HdrFileName}', '%{Path}' + '/' + Util.path('%{SrcFileName}'))}"

%{CN}::%{CN}() : WingHex::IWingPlugin(){
}

%{CN}::~%{CN}() {}

int %{CN}::sdkVersion() const { return WingHex::SDKVERSION; }

const QString %{CN}::signature() const { return WingHex::WINGSUMMER; }

bool %{CN}::init(const std::unique_ptr<QSettings> &set) {
    // Your codes there
    return true;
}

void %{CN}::unload(std::unique_ptr<QSettings> &set) {
    // Your unloading codes here
}

const QString %{CN}::pluginName() const { return tr("%{CN}"); }

const QString %{CN}::pluginComment() const {
    return tr("%{Description}%");
}
