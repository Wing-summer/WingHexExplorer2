#include "wingangelapi.h"

#include "AngelScript/angelscript/include/angelscript.h"
#include "src/class/winginputdialog.h"

// a helper function to register Qt enums to AngelScript
template <typename T>
void registerAngelType(asIScriptEngine *engine, const char *enumName) {
    auto e = QMetaEnum::fromType<T>();
    auto r = engine->RegisterEnum(enumName);
    Q_ASSERT(r >= 0);

    for (int i = 0; i < e.keyCount(); ++i) {
        r = engine->RegisterEnumValue(enumName, e.key(i), e.value(i));
        Q_ASSERT(r >= 0);
    }
}

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
    installExtAPI(engine);
    installLogAPI(engine);
    installMsgboxAPI(engine);
    installInputboxAPI(engine);
    installFileDialogAPI(engine);
}

void WingAngelAPI::plugin2MessagePipe(WingHex::WingPluginMessage type,
                                      const QVariantList &msg) {}

void WingAngelAPI::installLogAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("Log");
    assert(r >= 0);

    {

        Q_ASSERT(this);
        static std::function<void(const QString &)> fn =
            std::bind(&WingAngelAPI::info, this, std::placeholders::_1);
        r = engine->RegisterGlobalFunction("void info(const string &in)",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<void(const QString &)> fn =
            std::bind(&WingAngelAPI::debug, this, std::placeholders::_1);
        r = engine->RegisterGlobalFunction("void debug(const string &in)",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<void(const QString &)> fn =
            std::bind(&WingAngelAPI::warn, this, std::placeholders::_1);
        r = engine->RegisterGlobalFunction("void warn(const string &in)",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<void(const QString &)> fn =
            std::bind(&WingAngelAPI::error, this, std::placeholders::_1);
        r = engine->RegisterGlobalFunction("void error(const string &in)",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installExtAPI(asIScriptEngine *engine) {
    // toast(message, iconPath)
    static std::function<void(const QString &, const QString &)> fn =
        std::bind(&WingAngelAPI::toast, this, std::placeholders::_2,
                  std::placeholders::_1);
    auto r = engine->RegisterGlobalFunction(
        "void toast(const string &in, const string &in =\"\")",
        asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
    Q_ASSERT(r >= 0);
}

void WingAngelAPI::installMsgboxAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("MsgBox");
    Q_ASSERT(r >= 0);

    registerAngelType<QMessageBox::StandardButtons>(engine, "StandardButton");
    registerAngelType<QMessageBox::Icon>(engine, "Icon");
    auto msgbox = &this->msgbox;

    {
        static std::function<void(const QString &)> fn =
            std::bind(&WingHex::WingPlugin::MessageBox::aboutQt, msgbox,
                      nullptr, std::placeholders::_1);
        r = engine->RegisterGlobalFunction(
            "void aboutQt(const string &in=\"\")",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<QMessageBox::StandardButton(
            const QString &, const QString &, QMessageBox::StandardButtons,
            QMessageBox::StandardButton)>
            fn =
                std::bind(&WingHex::WingPlugin::MessageBox::information, msgbox,
                          nullptr, std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3, std::placeholders::_4);
        r = engine->RegisterGlobalFunction(
            "void information(const string &in, const string &in, "
            "MsgBox::StandardButton = MsgBox::StandardButton::Ok, "
            "MsgBox::StandardButton = MsgBox::StandardButton::NoButton)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<QMessageBox::StandardButton(
            const QString &, const QString &, QMessageBox::StandardButtons,
            QMessageBox::StandardButton)>
            fn =
                std::bind(&WingHex::WingPlugin::MessageBox::question, msgbox,
                          nullptr, std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3, std::placeholders::_4);
        r = engine->RegisterGlobalFunction(
            "void question(const string &in, const string &in, "
            "MsgBox::StandardButton = MsgBox::StandardButton::Yes | "
            "MsgBox::StandardButton::No, "
            "MsgBox::StandardButton = MsgBox::StandardButton::NoButton)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<QMessageBox::StandardButton(
            const QString &, const QString &, QMessageBox::StandardButtons,
            QMessageBox::StandardButton)>
            fn =
                std::bind(&WingHex::WingPlugin::MessageBox::warning, msgbox,
                          nullptr, std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3, std::placeholders::_4);
        r = engine->RegisterGlobalFunction(
            "void warning(const string &in, const string &in, "
            "MsgBox::StandardButton = MsgBox::StandardButton::Ok, "
            "MsgBox::StandardButton = MsgBox::StandardButton::NoButton)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<QMessageBox::StandardButton(
            const QString &, const QString &, QMessageBox::StandardButtons,
            QMessageBox::StandardButton)>
            fn =
                std::bind(&WingHex::WingPlugin::MessageBox::critical, msgbox,
                          nullptr, std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3, std::placeholders::_4);
        r = engine->RegisterGlobalFunction(
            "void critical(const string &in, const string &in, "
            "MsgBox::StandardButton = MsgBox::StandardButton::Ok, "
            "MsgBox::StandardButton = MsgBox::StandardButton::NoButton)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<QMessageBox::StandardButton(
            QMessageBox::Icon, const QString &, const QString &,
            QMessageBox::StandardButtons, QMessageBox::StandardButton)>
            fn = std::bind(&WingHex::WingPlugin::MessageBox::msgbox, msgbox,
                           nullptr, std::placeholders::_1,
                           std::placeholders::_2, std::placeholders::_3,
                           std::placeholders::_4, std::placeholders::_5);
        r = engine->RegisterGlobalFunction(
            "void msgbox(MsgBox::Icon, const string &in, const string &in, "
            "MsgBox::StandardButton = MsgBox::StandardButton::NoButton, "
            "MsgBox::StandardButton = MsgBox::StandardButton::NoButton)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<void(const QString &, const QString &)> fn =
            std::bind(&WingHex::WingPlugin::MessageBox::about, msgbox, nullptr,
                      std::placeholders::_1, std::placeholders::_2);
        r = engine->RegisterGlobalFunction(
            "void about(const string &in, const string &in)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installInputboxAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("InputBox");
    Q_ASSERT(r >= 0);

    registerAngelType<QLineEdit::EchoMode>(engine, "EchoMode");
    registerAngelType<Qt::InputMethodHints>(engine, "InputMethodHints");

    auto inputbox = &this->inputbox;

    {
        static std::function<QString(
            const QString &, const QString &, QLineEdit::EchoMode,
            const QString &, bool *, Qt::InputMethodHints inputMethodHints)>
            fn =
                std::bind(&WingHex::WingPlugin::InputBox::getText, inputbox,
                          nullptr, std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3, std::placeholders::_4,
                          std::placeholders::_5, std::placeholders::_6);

        r = engine->RegisterGlobalFunction(
            "string getText(const string &in, const string &in, "
            "InputBox::EchoMode = InputBox::Normal, const string &in = \"\", "
            "bool &out = false, "
            "InputBox::InputMethodHints = InputBox::ImhNone)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<QString(const QString &, const QString &,
                                     const QString &, bool *,
                                     Qt::InputMethodHints inputMethodHints)>
            fn = std::bind(&WingHex::WingPlugin::InputBox::getMultiLineText,
                           inputbox, nullptr, std::placeholders::_1,
                           std::placeholders::_2, std::placeholders::_3,
                           std::placeholders::_4, std::placeholders::_5);

        r = engine->RegisterGlobalFunction(
            "string getMultiLineText(const string &in, const string &in, "
            "const string &in = \"\", bool &out = false, "
            "InputBox::InputMethodHints = InputBox::ImhNone)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<int(const QString &, const QString &, int, int,
                                 int, int, bool *)>
            fn = std::bind(&WingHex::WingPlugin::InputBox::getInt, inputbox,
                           nullptr, std::placeholders::_1,
                           std::placeholders::_2, std::placeholders::_3,
                           std::placeholders::_4, std::placeholders::_5,
                           std::placeholders::_6, std::placeholders::_7);
        r = engine->RegisterGlobalFunction(
            "int getInt(const string &in, const string &in, "
            "int &in = 0, int &in = -2147483647, int &in = 2147483647, "
            "int &in = 1, bool &out = false)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<int(const QString &, const QString &, double,
                                 double, double, int, bool *, double)>
            fn =
                std::bind(&WingHex::WingPlugin::InputBox::getDouble, inputbox,
                          nullptr, std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3, std::placeholders::_4,
                          std::placeholders::_5, std::placeholders::_6,
                          std::placeholders::_7, std::placeholders::_8);
        r = engine->RegisterGlobalFunction(
            "double getDouble(const string &in, const string &in, "
            "double &in = 0, double &in = -2147483647, "
            "double &in = 2147483647, int &in = 1, "
            "bool &out = false, double &in = 1)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<QString(const QString &, const QString &,
                                     const CScriptArray &, int, bool, bool *,
                                     Qt::InputMethodHints)>
            fn = std::bind(&WingAngelAPI::_InputBox_getItem, this,
                           std::placeholders::_1, std::placeholders::_2,
                           std::placeholders::_3, std::placeholders::_4,
                           std::placeholders::_5, std::placeholders::_6,
                           std::placeholders::_7);

        r = engine->RegisterGlobalFunction(
            "string getItem(const string &in, const string &in, "
            "const array<string> &in, int = 0, bool = true, bool &out = false, "
            "InputBox::InputMethodHints = InputBox::ImhNone)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installFileDialogAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("FileDialog");
    Q_ASSERT(r >= 0);

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installColorDialogAPI(asIScriptEngine *engine) {}

void WingAngelAPI::installHexReaderAPI(asIScriptEngine *engine) {}

void WingAngelAPI::installHexControllerAPI(asIScriptEngine *engine) {}

QString WingAngelAPI::_InputBox_getItem(const QString &title,
                                        const QString &label,
                                        const CScriptArray &items, int current,
                                        bool editable, bool *ok,
                                        Qt::InputMethodHints inputMethodHints) {
    QStringList buffer;

    items.AddRef();
    for (int i = 0; i < items.GetSize(); ++i) {
        auto item = reinterpret_cast<const QString *>(items.At(i));
        buffer.append(*item);
    }
    items.Release();

    return WingInputDialog::getItem(nullptr, title, label, buffer, current,
                                    editable, ok, inputMethodHints);
}
