#include "wingangelapi.h"

#include "AngelScript/angelscript/include/angelscript.h"
#include "class/wingfiledialog.h"
#include "class/winginputdialog.h"

#if Q_PROCESSOR_WORDSIZE == 4
#define QSIZETYPE_WRAP(decl) "int " decl
#define QPTR_WRAP(decl) "uint " decl
#define QSIZETYPE "int"
#elif Q_PROCESSOR_WORDSIZE == 8
#define QSIZETYPE_WRAP(decl) "int64 " decl
#define QPTR_WRAP(decl) "uint64 " decl
#define QSIZETYPE "int64"
#else
#error "Processor with unexpected word size"
#endif

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

void WingAngelAPI::installAPI(asIScriptEngine *engine,
                              asITypeInfo *stringType) {
    installExtAPI(engine);
    installLogAPI(engine);
    installMsgboxAPI(engine);
    installInputboxAPI(engine, stringType->GetTypeId());
    installFileDialogAPI(engine);

    installHexBaseType(engine);
    installHexReaderAPI(engine);
    installHexControllerAPI(engine);
}

void WingAngelAPI::plugin2MessagePipe(WingHex::WingPluginMessage type,
                                      const QVariantList &msg) {}

void WingAngelAPI::installLogAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("Log");
    Q_ASSERT(r >= 0);

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

    registerAngelType<QMessageBox::StandardButtons>(engine, "StandardButtons");
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
            "MsgBox::StandardButtons = MsgBox::StandardButtons::Ok, "
            "MsgBox::StandardButtons = MsgBox::StandardButtons::NoButton)",
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
            "MsgBox::StandardButtons = MsgBox::StandardButtons::Yes | "
            "MsgBox::StandardButtons::No, "
            "MsgBox::StandardButtons = MsgBox::StandardButtons::NoButton)",
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
            "MsgBox::StandardButtons = MsgBox::StandardButtons::Ok, "
            "MsgBox::StandardButtons = MsgBox::StandardButtons::NoButton)",
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
            "MsgBox::StandardButtons = MsgBox::StandardButtons::Ok, "
            "MsgBox::StandardButtons = MsgBox::StandardButtons::NoButton)",
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
            "MsgBox::StandardButtons = MsgBox::StandardButtons::NoButton, "
            "MsgBox::StandardButtons = MsgBox::StandardButtons::NoButton)",
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

void WingAngelAPI::installInputboxAPI(asIScriptEngine *engine, int stringID) {
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
            fn = std::bind(&WingAngelAPI::_InputBox_getItem, this, stringID,
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

    registerAngelType<QFileDialog::Options>(engine, "Options");

    auto filedlg = &this->filedlg;

    {
        static std::function<QString(const QString &, const QString &,
                                     QFileDialog::Options)>
            fn = std::bind(
                &WingHex::WingPlugin::FileDialog::getExistingDirectory, filedlg,
                nullptr, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3);

        r = engine->RegisterGlobalFunction(
            "string getExistingDirectory(const string &in = \"\", "
            "const string &in = \"\", "
            "FileDialog::Options &in = FileDialog::Options::ShowDirsOnly)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<QString(const QString &, const QString &,
                                     const QString &, QString *,
                                     QFileDialog::Options)>
            fn = std::bind(&WingHex::WingPlugin::FileDialog::getOpenFileName,
                           filedlg, nullptr, std::placeholders::_1,
                           std::placeholders::_2, std::placeholders::_3,
                           std::placeholders::_4, std::placeholders::_5);

        r = engine->RegisterGlobalFunction(
            "string getOpenFileName(const string &in = \"\", "
            "const string &in = \"\", const string &in = \"\", "
            "string &out = \"\", FileDialog::Options &in = 0)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<QString(const QString &, const QString &,
                                     const QString &, QString *,
                                     QFileDialog::Options)>
            fn = std::bind(&WingHex::WingPlugin::FileDialog::getSaveFileName,
                           filedlg, nullptr, std::placeholders::_1,
                           std::placeholders::_2, std::placeholders::_3,
                           std::placeholders::_4, std::placeholders::_5);

        r = engine->RegisterGlobalFunction(
            "string getSaveFileName(const string &in = \"\", "
            "const string &in = \"\", const string &in = \"\", "
            "string &out = \"\", FileDialog::Options &in = 0)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<CScriptArray *(const QString &, const QString &,
                                            const QString &, QString *,
                                            QFileDialog::Options)>
            fn = std::bind(&WingAngelAPI::_FileDialog_getOpenFileNames, this,
                           std::placeholders::_1, std::placeholders::_2,
                           std::placeholders::_3, std::placeholders::_4,
                           std::placeholders::_5);

        r = engine->RegisterGlobalFunction(
            "array<string>@ getOpenFileNames(const string &in = \"\", "
            "const string &in = \"\", const string &in = \"\", "
            "string &out = \"\", FileDialog::Options &in = 0)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installColorDialogAPI(asIScriptEngine *engine) {}

void WingAngelAPI::installHexBaseType(asIScriptEngine *engine) {
    registerAngelType<WingHex::ErrFile>(engine, "ErrFile");

    int r = engine->RegisterTypedef("byte", "uint8");
    Q_ASSERT(r >= 0);

    // FindResult
    r = engine->RegisterObjectType("FindResult", sizeof(WingHex::FindResult),
                                   asOBJ_VALUE | asOBJ_POD |
                                       asGetTypeTraits<WingHex::FindResult>());
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty("FindResult", QSIZETYPE_WRAP("offset"),
                                       asOFFSET(WingHex::FindResult, offset));
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectProperty("FindResult", QSIZETYPE_WRAP("line"),
                                       asOFFSET(WingHex::FindResult, line));
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectProperty("FindResult", QSIZETYPE_WRAP("col"),
                                       asOFFSET(WingHex::FindResult, col));
    Q_ASSERT(r >= 0);

    // BookMark
    r = engine->RegisterObjectType("BookMark", sizeof(WingHex::BookMark),
                                   asOBJ_VALUE | asOBJ_POD |
                                       asGetTypeTraits<WingHex::BookMark>());
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty("BookMark", QSIZETYPE_WRAP("pos"),
                                       asOFFSET(WingHex::BookMark, pos));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty("BookMark", "string comment",
                                       asOFFSET(WingHex::BookMark, comment));
    Q_ASSERT(r >= 0);

    // HexPosition
    r = engine->RegisterObjectType("HexPosition", sizeof(WingHex::HexPosition),
                                   asOBJ_VALUE | asOBJ_POD |
                                       asGetTypeTraits<WingHex::HexPosition>());
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty("HexPosition", QSIZETYPE_WRAP("line"),
                                       asOFFSET(WingHex::HexPosition, line));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty("HexPosition", "int column",
                                       asOFFSET(WingHex::HexPosition, column));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexPosition", "uint8 lineWidth",
        asOFFSET(WingHex::HexPosition, lineWidth));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexPosition", "int nibbleindex",
        asOFFSET(WingHex::HexPosition, nibbleindex));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("HexPosition", QSIZETYPE_WRAP("offset()"),
                                     asMETHOD(WingHex::HexPosition, offset),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "HexPosition", "int opSub(const HexPosition &in) const",
        asMETHODPR(WingHex::HexPosition, operator-,
                   (const WingHex::HexPosition &) const, qsizetype),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "HexPosition", "bool opEquals(const HexPosition &in) const",
        asMETHODPR(WingHex::HexPosition, operator==,
                   (const WingHex::HexPosition &) const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);

    // HexMetadataAbsoluteItem
    r = engine->RegisterObjectType(
        "HexMetadataAbsoluteItem", sizeof(WingHex::HexMetadataAbsoluteItem),
        asOBJ_VALUE | asOBJ_POD |
            asGetTypeTraits<WingHex::HexMetadataAbsoluteItem>());
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexMetadataAbsoluteItem", QSIZETYPE_WRAP("begin"),
        asOFFSET(WingHex::HexMetadataAbsoluteItem, begin));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexMetadataAbsoluteItem", QSIZETYPE_WRAP("end"),
        asOFFSET(WingHex::HexMetadataAbsoluteItem, end));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexMetadataAbsoluteItem", "Color foreground",
        asOFFSET(WingHex::HexMetadataAbsoluteItem, foreground));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexMetadataAbsoluteItem", "Color background",
        asOFFSET(WingHex::HexMetadataAbsoluteItem, background));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexMetadataAbsoluteItem", "string comment",
        asOFFSET(WingHex::HexMetadataAbsoluteItem, comment));
    Q_ASSERT(r >= 0);

    // HexMetadataItem
    r = engine->RegisterObjectType(
        "HexMetadataItem", sizeof(WingHex::HexMetadataItem),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<WingHex::HexMetadataItem>());
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexMetadataItem", QSIZETYPE_WRAP("line"),
        asOFFSET(WingHex::HexMetadataItem, line));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexMetadataItem", "int start",
        asOFFSET(WingHex::HexMetadataItem, start));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexMetadataItem", "int length",
        asOFFSET(WingHex::HexMetadataItem, length));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexMetadataItem", "Color foreground",
        asOFFSET(WingHex::HexMetadataItem, foreground));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexMetadataItem", "Color background",
        asOFFSET(WingHex::HexMetadataItem, background));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexMetadataItem", "string comment",
        asOFFSET(WingHex::HexMetadataItem, comment));
    Q_ASSERT(r >= 0);
}

void WingAngelAPI::installHexReaderAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("Reader");
    Q_ASSERT(r >= 0);
    auto reader = &this->reader;

    {
        static std::function<bool(void)> fn = std::bind(
            &WingHex::WingPlugin::Reader::isCurrentDocEditing, reader);
        r = engine->RegisterGlobalFunction("bool isCurrentDocEditing()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<QString(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::currentDocFilename, reader);
        r = engine->RegisterGlobalFunction("string currentDocFilename()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::isReadOnly, reader);
        r = engine->RegisterGlobalFunction("bool isReadOnly()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::isKeepSize, reader);
        r = engine->RegisterGlobalFunction("bool isKeepSize()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::isLocked, reader);
        r = engine->RegisterGlobalFunction("bool isLocked()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<WingHex::HexPosition(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::currentPos, reader);
        r = engine->RegisterGlobalFunction("HexPosition currentPos()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<WingHex::HexPosition(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::selectionPos, reader);
        r = engine->RegisterGlobalFunction("HexPosition selectionPos()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::stringVisible, reader);
        r = engine->RegisterGlobalFunction("bool stringVisible()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::addressVisible, reader);
        r = engine->RegisterGlobalFunction("bool addressVisible()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::headerVisible, reader);
        r = engine->RegisterGlobalFunction("bool headerVisible()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::isModified, reader);
        r = engine->RegisterGlobalFunction("bool isModified()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::isEmpty, reader);
        r = engine->RegisterGlobalFunction("bool isEmpty()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::atEnd, reader);
        r = engine->RegisterGlobalFunction("bool atEnd()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::canUndo, reader);
        r = engine->RegisterGlobalFunction("bool canUndo()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::canRedo, reader);
        r = engine->RegisterGlobalFunction("bool canRedo()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(bool)> fn = std::bind(
            &WingHex::WingPlugin::Reader::copy, reader, std::placeholders::_1);
        r = engine->RegisterGlobalFunction("bool copy(bool hex = false)",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<qsizetype(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::documentLines, reader);
        r = engine->RegisterGlobalFunction(QSIZETYPE_WRAP("documentLines()"),
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<qsizetype(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::documentBytes, reader);
        r = engine->RegisterGlobalFunction(QSIZETYPE_WRAP("documentBytes()"),
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<qsizetype(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::currentRow, reader);
        r = engine->RegisterGlobalFunction(QSIZETYPE_WRAP("currentRow()"),
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<qsizetype(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::currentColumn, reader);
        r = engine->RegisterGlobalFunction(QSIZETYPE_WRAP("currentColumn()"),
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<qsizetype(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::currentOffset, reader);
        r = engine->RegisterGlobalFunction(QSIZETYPE_WRAP("currentOffset()"),
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<qsizetype(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::selectedLength, reader);
        r = engine->RegisterGlobalFunction(QSIZETYPE_WRAP("selectedLength()"),
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<CScriptArray *(void)> fn =
            std::bind(&WingAngelAPI::_HexReader_selectedBytes, this);
        r = engine->RegisterGlobalFunction("array<byte>@ selectedBytes()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<quintptr(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::addressBase, reader);
        r = engine->RegisterGlobalFunction(QPTR_WRAP("addressBase()"),
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(void *ref, int typeId)> fn =
            std::bind(&WingAngelAPI::_HexReader_read, this,
                      std::placeholders::_1, std::placeholders::_2);
        r = engine->RegisterGlobalFunction("bool read(? &in)",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<CScriptArray *(qsizetype, qsizetype)> fn =
            std::bind(&WingAngelAPI::_HexReader_readBytes, this,
                      std::placeholders::_1, std::placeholders::_2);
        r = engine->RegisterGlobalFunction(
            "array<byte>@ readBytes(" QSIZETYPE_WRAP(
                "offset") "," QSIZETYPE_WRAP("len") ")",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<qint8(qsizetype)> fn =
            std::bind(&WingHex::WingPlugin::Reader::readInt8, reader,
                      std::placeholders::_1);
        r = engine->RegisterGlobalFunction(
            "int8 readInt8(" QSIZETYPE_WRAP("offset") ")",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<qint16(qsizetype)> fn =
            std::bind(&WingHex::WingPlugin::Reader::readInt16, reader,
                      std::placeholders::_1);
        r = engine->RegisterGlobalFunction(
            "int16 readInt16(" QSIZETYPE_WRAP("offset") ")",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }
    engine->SetDefaultNamespace("");

    {
        static std::function<qint32(qsizetype)> fn =
            std::bind(&WingHex::WingPlugin::Reader::readInt32, reader,
                      std::placeholders::_1);
        r = engine->RegisterGlobalFunction(
            "int readInt32(" QSIZETYPE_WRAP("offset") ")",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<qint64(qsizetype)> fn =
            std::bind(&WingHex::WingPlugin::Reader::readInt64, reader,
                      std::placeholders::_1);
        r = engine->RegisterGlobalFunction(
            "int64 readInt64(" QSIZETYPE_WRAP("offset") ")",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<QString(qsizetype, QString)> fn =
            std::bind(&WingHex::WingPlugin::Reader::readString, reader,
                      std::placeholders::_1, std::placeholders::_2);
        r = engine->RegisterGlobalFunction(
            "string readInt64(" QSIZETYPE_WRAP("offset") ", string &in = \"\")",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<qsizetype(qsizetype, const CScriptArray &)> fn =
            std::bind(&WingAngelAPI::_HexReader_searchForward, this,
                      std::placeholders::_1, std::placeholders::_2);
        r = engine->RegisterGlobalFunction(
            QSIZETYPE_WRAP(
                "searchForward(" QSIZETYPE_WRAP("begin") ", array<byte> &in)"),
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<qsizetype(qsizetype, const CScriptArray &)> fn =
            std::bind(&WingAngelAPI::_HexReader_searchBackward, this,
                      std::placeholders::_1, std::placeholders::_2);
        r = engine->RegisterGlobalFunction(
            QSIZETYPE_WRAP(
                "searchBackward(" QSIZETYPE_WRAP("begin") ", array<byte> &in)"),
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<CScriptArray *(qsizetype, qsizetype,
                                            const CScriptArray &)>
            fn = std::bind(&WingAngelAPI::_HexReader_findAllBytes, this,
                           std::placeholders::_1, std::placeholders::_2,
                           std::placeholders::_3);
        r = engine->RegisterGlobalFunction(
            "array<" QSIZETYPE ">@ findAllBytes(" QSIZETYPE_WRAP(
                "begin") ", " QSIZETYPE_WRAP("end") ", array<byte> &in)",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<qsizetype(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::documentLastLine, reader);
        r = engine->RegisterGlobalFunction(QSIZETYPE_WRAP("documentLastLine()"),
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<qsizetype(void)> fn =
            std::bind(&WingHex::WingPlugin::Reader::documentLastColumn, reader);
        r = engine->RegisterGlobalFunction(
            QSIZETYPE_WRAP("documentLastColumn()"),
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(qsizetype)> fn =
            std::bind(&WingHex::WingPlugin::Reader::lineHasMetadata, reader,
                      std::placeholders::_1);
        r = engine->RegisterGlobalFunction(
            "bool lineHasMetadata(" QSIZETYPE_WRAP("line") ")",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<CScriptArray *(qsizetype)> fn =
            std::bind(&WingAngelAPI::_HexReader_getMetadatas, this,
                      std::placeholders::_1);
        r = engine->RegisterGlobalFunction(
            "array<HexMetadataAbsoluteItem>@ getMetadatas(" QSIZETYPE_WRAP(
                "offset") ")",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<CScriptArray *(qsizetype)> fn = std::bind(
            &WingAngelAPI::_HexReader_getMetaLine, this, std::placeholders::_1);
        r = engine->RegisterGlobalFunction(
            "array<HexMetadataItem>@ getMetaLine(" QSIZETYPE_WRAP("offset") ")",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(qsizetype)> fn =
            std::bind(&WingHex::WingPlugin::Reader::lineHasBookMark, reader,
                      std::placeholders::_1);
        r = engine->RegisterGlobalFunction(
            "bool lineHasBookMark(" QSIZETYPE_WRAP("line") ")",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<CScriptArray *(qsizetype)> fn =
            std::bind(&WingAngelAPI::_HexReader_getsBookmarkPos, this,
                      std::placeholders::_1);
        r = engine->RegisterGlobalFunction(
            "array<" QSIZETYPE
            ">@ getsBookmarkPos(" QSIZETYPE_WRAP("offset") ")",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<WingHex::BookMark(qsizetype)> fn =
            std::bind(&WingHex::WingPlugin::Reader::bookMark, reader,
                      std::placeholders::_1);
        r = engine->RegisterGlobalFunction(
            "BookMark bookMark(" QSIZETYPE_WRAP("pos") ")",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<QString(qsizetype)> fn =
            std::bind(&WingHex::WingPlugin::Reader::bookMarkComment, reader,
                      std::placeholders::_1);
        r = engine->RegisterGlobalFunction(
            "string bookMarkComment(" QSIZETYPE_WRAP("pos") ")",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<CScriptArray *()> fn =
            std::bind(&WingAngelAPI::_HexReader_getBookMarks, this);
        r = engine->RegisterGlobalFunction("array<BookMark>@ getBookMarks()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<bool(qsizetype)> fn =
            std::bind(&WingHex::WingPlugin::Reader::existBookMark, reader,
                      std::placeholders::_1);
        r = engine->RegisterGlobalFunction(
            "bool existBookMark(" QSIZETYPE_WRAP("pos") ")",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<CScriptArray *()> fn =
            std::bind(&WingAngelAPI::_HexReader_getOpenFiles, this);
        r = engine->RegisterGlobalFunction("array<string>@ getOpenFiles()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<CScriptArray *()> fn =
            std::bind(&WingAngelAPI::_HexReader_getSupportedEncodings, this);
        r = engine->RegisterGlobalFunction(
            "array<string>@ getSupportedEncodings()",
            asMETHOD(decltype(fn), operator()), asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }

    {
        static std::function<QString()> fn =
            std::bind(&WingHex::WingPlugin::Reader::currentEncoding, reader);
        r = engine->RegisterGlobalFunction("string currentEncoding()",
                                           asMETHOD(decltype(fn), operator()),
                                           asCALL_THISCALL_ASGLOBAL, &fn);
        Q_ASSERT(r >= 0);
    }
}

void WingAngelAPI::installHexControllerAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("Controller");
    Q_ASSERT(r >= 0);

    engine->SetDefaultNamespace("");
}

QStringList WingAngelAPI::cArray2QStringList(const CScriptArray &array,
                                             int stringID, bool *ok) {
    bool b = array.GetElementTypeId() == stringID;
    if (ok) {
        *ok = b;
    }
    if (!b) {
        return {};
    }

    QStringList buffer;
    buffer.reserve(array.GetSize());
    array.AddRef();
    for (asUINT i = 0; i < array.GetSize(); ++i) {
        auto item = reinterpret_cast<const QString *>(array.At(i));
        buffer.append(*item);
    }
    array.Release();
    return buffer;
}

QByteArray WingAngelAPI::cArray2ByteArray(const CScriptArray &array, int byteID,
                                          bool *ok) {
    bool b = array.GetElementTypeId() == byteID;
    if (ok) {
        *ok = b;
    }
    if (!b) {
        return {};
    }

    QByteArray buffer;
    buffer.reserve(array.GetSize());
    array.AddRef();
    for (asUINT i = 0; i < array.GetSize(); ++i) {
        auto item = reinterpret_cast<const asBYTE *>(array.At(i));
        buffer.append(*item);
    }
    array.Release();
    return buffer;
}

QString WingAngelAPI::_InputBox_getItem(int stringID, const QString &title,
                                        const QString &label,
                                        const CScriptArray &items, int current,
                                        bool editable, bool *ok,
                                        Qt::InputMethodHints inputMethodHints) {
    bool o = false;
    auto ret = cArray2QStringList(items, stringID, &o);
    if (o) {
        return WingInputDialog::getItem(nullptr, title, label, ret, current,
                                        editable, ok, inputMethodHints);
    } else {
        *ok = false;
        return {};
    }
}

CScriptArray *WingAngelAPI::_FileDialog_getOpenFileNames(
    const QString &caption, const QString &dir, const QString &filter,
    QString *selectedFilter, QFileDialog::Options options) {
    return retarrayWrapperFunction(
        [=]() -> QStringList {
            return WingFileDialog::getOpenFileNames(
                nullptr, caption, dir, filter, selectedFilter, options);
        },
        "array<string>");
}

CScriptArray *WingAngelAPI::_HexReader_selectedBytes() {
    return byteArrayWrapperFunction(
        [this]() -> QByteArray { return emit reader.selectedBytes(); });
}

CScriptArray *WingAngelAPI::_HexReader_readBytes(qsizetype offset,
                                                 qsizetype len) {
    return byteArrayWrapperFunction([this, offset, len]() -> QByteArray {
        return emit reader.readBytes(offset, len);
    });
}

bool WingAngelAPI::_HexReader_read(void *ref, int typeId) { return false; }

qsizetype WingAngelAPI::_HexReader_searchForward(qsizetype begin,
                                                 const CScriptArray &ba) {
    // If called from the script, there will always be an active
    // context, which can be used to obtain a pointer to the engine.
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();

        bool ok = false;
        auto byteID = engine->GetTypeIdByDecl("byte");
        Q_ASSERT(byteID);
        auto bab = cArray2ByteArray(ba, byteID, &ok);
        return emit reader.searchForward(begin, bab);
    } else {
        return qsizetype(-1);
    }
}

qsizetype WingAngelAPI::_HexReader_searchBackward(qsizetype begin,
                                                  const CScriptArray &ba) {
    // If called from the script, there will always be an active
    // context, which can be used to obtain a pointer to the engine.
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();

        bool ok = false;
        auto byteID = engine->GetTypeIdByDecl("byte");
        auto bab = cArray2ByteArray(ba, byteID, &ok);
        if (ok) {
            return emit reader.searchBackward(begin, bab);
        } else {
            return qsizetype(-1);
        }
    } else {
        return qsizetype(-1);
    }
}

CScriptArray *WingAngelAPI::_HexReader_findAllBytes(qsizetype begin,
                                                    qsizetype end,
                                                    const CScriptArray &ba) {
    // If called from the script, there will always be an active
    // context, which can be used to obtain a pointer to the engine.
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();

        bool ok;
        auto byteID = engine->GetTypeIdByDecl("byte");
        auto bab = cArray2ByteArray(ba, byteID, &ok);

        if (!ok) {
            return nullptr;
        }

        auto ret = emit reader.findAllBytes(begin, end, bab);

        // The script array needs to know its type to properly handle the
        // elements. Note that the object type should be cached to avoid
        // performance issues if the function is called frequently.
        asITypeInfo *t = engine->GetTypeInfoByDecl("array<" QSIZETYPE ">");

        auto array = CScriptArray::Create(t, ret.size());
        for (decltype(ret)::size_type i = 0; i < ret.size(); ++i) {
            array->SetValue(i, &ret[i]);
        }
        return array;
    } else {
        return nullptr;
    }
}

CScriptArray *WingAngelAPI::_HexReader_getMetadatas(qsizetype offset) {
    return retarrayWrapperFunction(
        [this, offset]() -> QList<WingHex::HexMetadataAbsoluteItem> {
            return emit reader.getMetadatas(offset);
        },
        "array<HexMetadataAbsoluteItem>");
}

CScriptArray *WingAngelAPI::_HexReader_getMetaLine(qsizetype line) {
    return retarrayWrapperFunction(
        [this, line]() -> WingHex::HexLineMetadata {
            return emit reader.getMetaLine(line);
        },
        "array<HexMetadataItem>");
}

CScriptArray *WingAngelAPI::_HexReader_getsBookmarkPos(qsizetype line) {
    return retarrayWrapperFunction(
        [this, line]() -> QList<qsizetype> {
            return emit reader.getsBookmarkPos(line);
        },
        "array<" QSIZETYPE ">");
}

CScriptArray *WingAngelAPI::_HexReader_getBookMarks() {
    return retarrayWrapperFunction(
        [this]() -> QList<WingHex::BookMark> {
            return emit reader.getBookMarks();
        },
        "array<BookMark>");
}

CScriptArray *WingAngelAPI::_HexReader_getOpenFiles() {
    return retarrayWrapperFunction(
        [this]() -> QStringList { return emit reader.getOpenFiles(); },
        "array<string>");
}

CScriptArray *WingAngelAPI::_HexReader_getSupportedEncodings() {
    return retarrayWrapperFunction(
        [this]() -> QStringList { return emit reader.getSupportedEncodings(); },
        "array<string>");
}
