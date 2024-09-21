#include "wingangelapi.h"

#include "AngelScript/sdk/angelscript/include/angelscript.h"
#include "class/angelscripthelper.h"
#include "class/wingfiledialog.h"
#include "class/winginputdialog.h"

#include <QJsonDocument>

WingAngelAPI::WingAngelAPI() {
    qsizetype signalCount = 0;
    const QMetaObject *objs[]{this->metaObject(),
                              this->reader.metaObject(),
                              this->controller.metaObject(),
                              this->msgbox.metaObject(),
                              this->inputbox.metaObject(),
                              this->filedlg.metaObject(),
                              this->colordlg.metaObject(),
                              this->visual.metaObject()};
    for (auto &obj : objs) {
        for (auto i = obj->methodOffset(); i < obj->methodCount(); ++i) {
            if (obj->method(i).methodType() == QMetaMethod::Signal) {
                signalCount++;
            }
        }
    }

    _fnbuffer.reserve(signalCount);
}

WingAngelAPI::~WingAngelAPI() {
    for (auto &p : _fnbuffer) {
        auto f = reinterpret_cast<std::function<void(void *)> *>(p);
        delete f;
    }
    _fnbuffer.clear();
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
    installColorDialogAPI(engine);

    installHexBaseType(engine);
    installHexReaderAPI(engine);
    installHexControllerAPI(engine);
    installDataVisualAPI(engine, stringType->GetTypeId());
}

void WingAngelAPI::installLogAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("log");
    Q_ASSERT(r >= 0);

    registerAPI<void(const QString &)>(
        engine, std::bind(&WingAngelAPI::info, this, std::placeholders::_1),
        "void info(const string &in)");

    registerAPI<void(const QString &)>(
        engine, std::bind(&WingAngelAPI::trace, this, std::placeholders::_1),
        "void trace(const string &in)");

    registerAPI<void(const QString &)>(
        engine, std::bind(&WingAngelAPI::debug, this, std::placeholders::_1),
        "void debug(const string &in)");

    registerAPI<void(const QString &)>(
        engine, std::bind(&WingAngelAPI::warn, this, std::placeholders::_1),
        "void warn(const string &in)");

    registerAPI<void(const QString &)>(
        engine, std::bind(&WingAngelAPI::error, this, std::placeholders::_1),
        "void error(const string &in)");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installExtAPI(asIScriptEngine *engine) {
    // toast(message, iconPath)
    registerAPI<void(const QString &, const QString &)>(
        engine,
        std::bind(&WingAngelAPI::toast, this, std::placeholders::_2,
                  std::placeholders::_1),
        "void toast(const string &in, const string &in =\"\")");
}

void WingAngelAPI::installMsgboxAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("msgbox");
    Q_ASSERT(r >= 0);

    registerAngelType<QMessageBox::StandardButtons>(engine, "buttons");
    registerAngelType<QMessageBox::Icon>(engine, "icon");
    auto msgbox = &this->msgbox;

    registerAPI<void(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::MessageBox::aboutQt, msgbox, nullptr,
                  std::placeholders::_1),
        "void aboutQt(const string &in=\"\")");

    registerAPI<QMessageBox::StandardButton(const QString &, const QString &,
                                            QMessageBox::StandardButtons,
                                            QMessageBox::StandardButton)>(
        engine,
        std::bind(&WingHex::WingPlugin::MessageBox::information, msgbox,
                  nullptr, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "void information(const string &in, const string &in, "
        "msgbox::buttons = msgbox::buttons::Ok, "
        "msgbox::buttons = msgbox::buttons::NoButton)");

    registerAPI<QMessageBox::StandardButton(const QString &, const QString &,
                                            QMessageBox::StandardButtons,
                                            QMessageBox::StandardButton)>(
        engine,
        std::bind(&WingHex::WingPlugin::MessageBox::question, msgbox, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "void question(const string &in, const string &in, "
        "msgbox::buttons = msgbox::buttons::Yes | "
        "msgbox::buttons::No, "
        "msgbox::buttons = msgbox::buttons::NoButton)");

    registerAPI<QMessageBox::StandardButton(const QString &, const QString &,
                                            QMessageBox::StandardButtons,
                                            QMessageBox::StandardButton)>(
        engine,
        std::bind(&WingHex::WingPlugin::MessageBox::warning, msgbox, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "void warning(const string &in, const string &in, "
        "msgbox::buttons = msgbox::buttons::Ok, "
        "msgbox::buttons = msgbox::buttons::NoButton)");

    registerAPI<QMessageBox::StandardButton(const QString &, const QString &,
                                            QMessageBox::StandardButtons,
                                            QMessageBox::StandardButton)>(
        engine,
        std::bind(&WingHex::WingPlugin::MessageBox::critical, msgbox, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "void critical(const string &in, const string &in, "
        "msgbox::buttons = msgbox::buttons::Ok, "
        "msgbox::buttons = msgbox::buttons::NoButton)");

    registerAPI<QMessageBox::StandardButton(
        QMessageBox::Icon, const QString &, const QString &,
        QMessageBox::StandardButtons, QMessageBox::StandardButton)>(
        engine,
        std::bind(&WingHex::WingPlugin::MessageBox::msgbox, msgbox, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5),
        "void msgbox(msgbox::icon, const string &in, const string &in, "
        "msgbox::buttons = msgbox::buttons::NoButton, "
        "msgbox::buttons = msgbox::buttons::NoButton)");

    registerAPI<void(const QString &, const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::MessageBox::about, msgbox, nullptr,
                  std::placeholders::_1, std::placeholders::_2),
        "void about(const string &in, const string &in)");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installInputboxAPI(asIScriptEngine *engine, int stringID) {
    int r = engine->SetDefaultNamespace("inputbox");
    Q_ASSERT(r >= 0);

    registerAngelType<QLineEdit::EchoMode>(engine, "EchoMode");
    registerAngelType<Qt::InputMethodHints>(engine, "InputMethodHints");

    auto inputbox = &this->inputbox;

    registerAPI<QString(const QString &, const QString &, QLineEdit::EchoMode,
                        const QString &, bool *,
                        Qt::InputMethodHints inputMethodHints)>(
        engine,
        std::bind(&WingHex::WingPlugin::InputBox::getText, inputbox, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5, std::placeholders::_6),
        "string getText(const string &in, const string &in, "
        "inputbox::EchoMode = inputbox::Normal, const string &in = \"\", "
        "bool &out = false, "
        "inputbox::InputMethodHints = inputbox::ImhNone)");

    registerAPI<QString(const QString &, const QString &, const QString &,
                        bool *, Qt::InputMethodHints inputMethodHints)>(
        engine,
        std::bind(&WingHex::WingPlugin::InputBox::getMultiLineText, inputbox,
                  nullptr, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5),
        "string getMultiLineText(const string &in, const string &in, "
        "const string &in = \"\", bool &out = false, "
        "inputbox::InputMethodHints = inputbox::ImhNone)");

    registerAPI<int(const QString &, const QString &, int, int, int, int,
                    bool *)>(
        engine,
        std::bind(&WingHex::WingPlugin::InputBox::getInt, inputbox, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5, std::placeholders::_6,
                  std::placeholders::_7),
        "int getInt(const string &in, const string &in, "
        "int &in = 0, int &in = -2147483647, int &in = 2147483647, "
        "int &in = 1, bool &out = false)");

    registerAPI<int(const QString &, const QString &, double, double, double,
                    int, bool *, double)>(
        engine,
        std::bind(&WingHex::WingPlugin::InputBox::getDouble, inputbox, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5, std::placeholders::_6,
                  std::placeholders::_7, std::placeholders::_8),
        "double getDouble(const string &in, const string &in, "
        "double &in = 0, double &in = -2147483647, "
        "double &in = 2147483647, int &in = 1, "
        "bool &out = false, double &in = 1)");

    registerAPI<QString(const QString &, const QString &, const CScriptArray &,
                        int, bool, bool *, Qt::InputMethodHints)>(
        engine,
        std::bind(&WingAngelAPI::_InputBox_getItem, this, stringID,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5, std::placeholders::_6,
                  std::placeholders::_7),
        "string getItem(const string &in, const string &in, "
        "const array<string> &in, int = 0, bool = true, bool &out = false, "
        "inputbox::InputMethodHints = inputbox::ImhNone)");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installFileDialogAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("filedlg");
    Q_ASSERT(r >= 0);

    registerAngelType<QFileDialog::Options>(engine, "options");

    auto filedlg = &this->filedlg;

    registerAPI<QString(const QString &, const QString &,
                        QFileDialog::Options)>(
        engine,
        std::bind(&WingHex::WingPlugin::FileDialog::getExistingDirectory,
                  filedlg, nullptr, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3),
        "string getExistingDirectory(const string &in = \"\", "
        "const string &in = \"\", "
        "filedlg::options &in = filedlg::options::ShowDirsOnly)");

    registerAPI<QString(const QString &, const QString &, const QString &,
                        QString *, QFileDialog::Options)>(
        engine,
        std::bind(&WingHex::WingPlugin::FileDialog::getOpenFileName, filedlg,
                  nullptr, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5),
        "string getOpenFileName(const string &in = \"\", "
        "const string &in = \"\", const string &in = \"\", "
        "string &out = \"\", filedlg::options &in = 0)");

    registerAPI<QString(const QString &, const QString &, const QString &,
                        QString *, QFileDialog::Options)>(
        engine,
        std::bind(&WingHex::WingPlugin::FileDialog::getSaveFileName, filedlg,
                  nullptr, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5),
        "string getSaveFileName(const string &in = \"\", "
        "const string &in = \"\", const string &in = \"\", "
        "string &out = \"\", filedlg::options &in = 0)");

    registerAPI<CScriptArray *(const QString &, const QString &,
                               const QString &, QString *,
                               QFileDialog::Options)>(
        engine,
        std::bind(&WingAngelAPI::_FileDialog_getOpenFileNames, this,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5),
        "array<string>@ getOpenFileNames(const string &in = \"\", "
        "const string &in = \"\", const string &in = \"\", "
        "string &out = \"\", filedlg::options &in = 0)");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installColorDialogAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("colordlg");
    Q_ASSERT(r >= 0);

    auto colordlg = &this->colordlg;

    registerAPI<QColor(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::ColorDialog::getColor, colordlg,
                  std::placeholders::_1, nullptr),
        "color getColor(const string &in)");

    engine->SetDefaultNamespace("");
}

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
        "HexMetadataAbsoluteItem", "color foreground",
        asOFFSET(WingHex::HexMetadataAbsoluteItem, foreground));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexMetadataAbsoluteItem", "color background",
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
        "HexMetadataItem", "color foreground",
        asOFFSET(WingHex::HexMetadataItem, foreground));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexMetadataItem", "color background",
        asOFFSET(WingHex::HexMetadataItem, background));
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectProperty(
        "HexMetadataItem", "string comment",
        asOFFSET(WingHex::HexMetadataItem, comment));
    Q_ASSERT(r >= 0);
}

void WingAngelAPI::installHexReaderAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("reader");
    Q_ASSERT(r >= 0);
    auto reader = &this->reader;

    registerAPI<bool(void)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::isCurrentDocEditing, reader),
        "bool isCurrentDocEditing()");

    registerAPI<QString(void)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::currentDocFilename, reader),
        "string currentDocFilename()");

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::isReadOnly, reader),
        "bool isReadOnly()");

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::isKeepSize, reader),
        "bool isKeepSize()");

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::isLocked, reader),
        "bool isLocked()");

    registerAPI<WingHex::HexPosition(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::currentPos, reader),
        "HexPosition currentPos()");

    registerAPI<WingHex::HexPosition(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::selectionPos, reader),
        "HexPosition selectionPos()");

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::stringVisible, reader),
        "bool stringVisible()");

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::addressVisible, reader),
        "bool addressVisible()");

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::headerVisible, reader),
        "bool headerVisible()");

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::isModified, reader),
        "bool isModified()");

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::isEmpty, reader),
        "bool isEmpty()");

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::atEnd, reader),
        "bool atEnd()");

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::canUndo, reader),
        "bool canUndo()");

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::canRedo, reader),
        "bool canRedo()");

    registerAPI<bool(bool)>(engine,
                            std::bind(&WingHex::WingPlugin::Reader::copy,
                                      reader, std::placeholders::_1),
                            "bool copy(bool hex = false)");

    registerAPI<qsizetype(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::documentLines, reader),
        QSIZETYPE_WRAP("documentLines()"));

    registerAPI<qsizetype(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::documentBytes, reader),
        QSIZETYPE_WRAP("documentBytes()"));

    registerAPI<qsizetype(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::currentRow, reader),
        QSIZETYPE_WRAP("currentRow()"));

    registerAPI<qsizetype(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::currentColumn, reader),
        QSIZETYPE_WRAP("currentColumn()"));

    registerAPI<qsizetype(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::currentOffset, reader),
        QSIZETYPE_WRAP("currentOffset()"));

    registerAPI<qsizetype(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::selectedLength, reader),
        QSIZETYPE_WRAP("selectedLength()"));

    registerAPI<CScriptArray *(void)>(
        engine, std::bind(&WingAngelAPI::_HexReader_selectedBytes, this),
        "array<byte>@ selectedBytes()");

    registerAPI<quintptr(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::addressBase, reader),
        QPTR_WRAP("addressBase()"));

    registerAPI<bool(qsizetype, void *, int)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_read, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3),
        "bool read(" QSIZETYPE ", ? &out)");

    registerAPI<CScriptArray *(qsizetype, qsizetype)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_readBytes, this,
                  std::placeholders::_1, std::placeholders::_2),
        "array<byte>@ readBytes(" QSIZETYPE "," QSIZETYPE ")");

    registerAPI<qint8(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::readInt8, reader,
                  std::placeholders::_1),
        "int8 readInt8(" QSIZETYPE ")");

    registerAPI<qint16(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::readInt16, reader,
                  std::placeholders::_1),
        "int16 readInt16(" QSIZETYPE ")");

    registerAPI<qint32(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::readInt32, reader,
                  std::placeholders::_1),
        "int readInt32(" QSIZETYPE ")");

    registerAPI<qint64(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::readInt64, reader,
                  std::placeholders::_1),
        "int64 readInt64(" QSIZETYPE ")");

    registerAPI<float(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::readFloat, reader,
                  std::placeholders::_1),
        "float readFloat(" QSIZETYPE ")");

    registerAPI<double(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::readDouble, reader,
                  std::placeholders::_1),
        "double readDouble(" QSIZETYPE ")");

    registerAPI<QString(qsizetype, QString)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::readString, reader,
                  std::placeholders::_1, std::placeholders::_2),
        "string readInt64(" QSIZETYPE ", string &in = \"\")");

    registerAPI<qsizetype(qsizetype, const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_searchForward, this,
                  std::placeholders::_1, std::placeholders::_2),
        QSIZETYPE_WRAP("searchForward(" QSIZETYPE ", array<byte> &in)"));

    registerAPI<qsizetype(qsizetype, const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_searchBackward, this,
                  std::placeholders::_1, std::placeholders::_2),
        QSIZETYPE_WRAP("searchBackward(" QSIZETYPE ", array<byte> &in)"));

    registerAPI<CScriptArray *(qsizetype, qsizetype, const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_findAllBytes, this,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "array<" QSIZETYPE ">@ findAllBytes(" QSIZETYPE ", " QSIZETYPE
        ", array<byte> &in)");

    registerAPI<qsizetype(void)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::documentLastLine, reader),
        QSIZETYPE_WRAP("documentLastLine()"));

    registerAPI<qsizetype(void)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::documentLastColumn, reader),
        QSIZETYPE_WRAP("documentLastColumn()"));

    registerAPI<bool(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::lineHasMetadata, reader,
                  std::placeholders::_1),
        "bool lineHasMetadata(" QSIZETYPE ")");

    registerAPI<CScriptArray *(qsizetype)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_getMetadatas, this,
                  std::placeholders::_1),
        "array<HexMetadataAbsoluteItem>@ getMetadatas(" QSIZETYPE ")");

    registerAPI<CScriptArray *(qsizetype)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_getMetaLine, this,
                  std::placeholders::_1),
        "array<HexMetadataItem>@ getMetaLine(" QSIZETYPE ")");

    registerAPI<bool(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::lineHasBookMark, reader,
                  std::placeholders::_1),
        "bool lineHasBookMark(" QSIZETYPE ")");

    registerAPI<CScriptArray *(qsizetype)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_getsBookmarkPos, this,
                  std::placeholders::_1),
        "array<" QSIZETYPE ">@ getsBookmarkPos(" QSIZETYPE ")");

    registerAPI<WingHex::BookMark(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::bookMark, reader,
                  std::placeholders::_1),
        "BookMark bookMark(" QSIZETYPE ")");

    registerAPI<QString(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::bookMarkComment, reader,
                  std::placeholders::_1),
        "string bookMarkComment(" QSIZETYPE ")");

    registerAPI<CScriptArray *()>(
        engine, std::bind(&WingAngelAPI::_HexReader_getBookMarks, this),
        "array<BookMark>@ getBookMarks()");

    registerAPI<bool(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::existBookMark, reader,
                  std::placeholders::_1),
        "bool existBookMark(" QSIZETYPE ")");

    registerAPI<CScriptArray *()>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_getSupportedEncodings, this),
        "array<string>@ getSupportedEncodings()");

    registerAPI<QString()>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::currentEncoding, reader),
        "string currentEncoding()");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installHexControllerAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("ctl");
    Q_ASSERT(r >= 0);

    auto ctl = &this->controller;

    registerAPI<bool(int)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::switchDocument, ctl,
                  std::placeholders::_1),
        "bool switchDocument(int)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setLockedFile, ctl,
                  std::placeholders::_1),
        "bool setLockedFile(bool)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setKeepSize, ctl,
                  std::placeholders::_1),
        "bool setKeepSize(bool)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setStringVisible, ctl,
                  std::placeholders::_1),
        "bool setStringVisible(bool)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setAddressVisible, ctl,
                  std::placeholders::_1),
        "bool setAddressVisible(bool)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setHeaderVisible, ctl,
                  std::placeholders::_1),
        "bool setHeaderVisible(bool)");

    registerAPI<bool(quintptr)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setAddressBase, ctl,
                  std::placeholders::_1),
        "bool setAddressBase(" QPTR ")");

    registerAPI<bool(qsizetype, void *, int)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_write, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3),
        "bool write(" QSIZETYPE ", ? &out)");

    registerAPI<bool(qsizetype, void *, int)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_insert, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3),
        "bool insert(" QSIZETYPE ", ? &out)");

    registerAPI<bool(qsizetype, void *, int)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_append, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3),
        "bool append(" QSIZETYPE ", ? &out)");

    registerAPI<bool()>(engine,
                        std::bind(&WingHex::WingPlugin::Controller::undo, ctl),
                        "bool undo()");

    registerAPI<bool()>(engine,
                        std::bind(&WingHex::WingPlugin::Controller::redo, ctl),
                        "bool redo()");

    registerAPI<bool(bool)>(engine,
                            std::bind(&WingHex::WingPlugin::Controller::cut,
                                      ctl, std::placeholders::_1),
                            "bool cut(bool = false)");

    registerAPI<bool(bool)>(engine,
                            std::bind(&WingHex::WingPlugin::Controller::paste,
                                      ctl, std::placeholders::_1),
                            "bool paste(bool = false)");

    registerAPI<bool(qsizetype, qint8)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::writeInt8, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeInt8(" QSIZETYPE ", int8)");

    registerAPI<bool(qsizetype, qint16)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::writeInt16, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeInt16(" QSIZETYPE ", int16)");

    registerAPI<bool(qsizetype, qint32)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::writeInt32, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeInt32(" QSIZETYPE ", int)");

    registerAPI<bool(qsizetype, qint64)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::writeInt64, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeInt64(" QSIZETYPE ", int64)");

    registerAPI<bool(qsizetype, float)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::writeFloat, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeFloat(" QSIZETYPE ", float)");

    registerAPI<bool(qsizetype, double)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::writeDouble, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeDouble(" QSIZETYPE ", double)");

    registerAPI<bool(qsizetype, const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexController_writeBytes, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeBytes(" QSIZETYPE ", array<byte>&in)");

    registerAPI<bool(qsizetype, const QString &, const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::writeString, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "bool writeString(" QSIZETYPE ", string &in, string &in = \"\")");

    registerAPI<bool(qsizetype, qint8)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::insertInt8, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertInt8(" QSIZETYPE ", int8)");

    registerAPI<bool(qsizetype, qint16)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::insertInt16, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertInt16(" QSIZETYPE ", int16)");

    registerAPI<bool(qsizetype, qint32)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::insertInt32, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertInt32(" QSIZETYPE ", int)");

    registerAPI<bool(qsizetype, qint32)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::insertInt64, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertInt64(" QSIZETYPE ", int64)");

    registerAPI<bool(qsizetype, float)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::insertFloat, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertFloat(" QSIZETYPE ", float)");

    registerAPI<bool(qsizetype, double)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::insertDouble, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertDouble(" QSIZETYPE ", double)");

    registerAPI<bool(qsizetype, const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexController_insertBytes, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertBytes(" QSIZETYPE ", array<byte>&in)");

    registerAPI<bool(qsizetype, const QString &, const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::insertString, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "bool insertString(" QSIZETYPE ", string &in, string &in = \"\")");

    registerAPI<bool(qint8)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::appendInt8, ctl,
                  std::placeholders::_1),
        "bool appendInt8(int8)");

    registerAPI<bool(qint16)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::appendInt16, ctl,
                  std::placeholders::_1),
        "bool appendInt16(int16)");

    registerAPI<bool(qint32)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::appendInt32, ctl,
                  std::placeholders::_1),
        "bool appendInt32(int)");

    registerAPI<bool(qint32)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::appendInt64, ctl,
                  std::placeholders::_1),
        "bool appendInt64(int64)");

    registerAPI<bool(float)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::appendFloat, ctl,
                  std::placeholders::_1),
        "bool appendFloat(float)");

    registerAPI<bool(double)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::appendDouble, ctl,
                  std::placeholders::_1),
        "bool appendDouble(double)");

    registerAPI<bool(const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexController_appendBytes, this,
                  std::placeholders::_1),
        "bool appendBytes(array<byte>&in)");

    registerAPI<bool(const QString &, const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::appendString, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool appendString(string &in, string &in = \"\")");

    registerAPI<bool(qsizetype, qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::remove, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool remove(" QSIZETYPE ", " QSIZETYPE ")");

    registerAPI<bool()>(
        engine, std::bind(&WingHex::WingPlugin::Controller::removeAll, ctl),
        "bool removeAll()");

    registerAPI<bool(qsizetype, qsizetype, int)>(
        engine,
        std::bind(QOverload<qsizetype, qsizetype, int>::of(
                      &WingHex::WingPlugin::Controller::moveTo),
                  ctl, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "bool moveTo(" QSIZETYPE ", " QSIZETYPE ", int = -1)");

    registerAPI<bool(qsizetype)>(
        engine,
        std::bind(
            QOverload<qsizetype>::of(&WingHex::WingPlugin::Controller::moveTo),
            ctl, std::placeholders::_1),
        "bool moveTo(" QSIZETYPE ")");

    registerAPI<bool(qsizetype, qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::select, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool select(" QSIZETYPE ", " QSIZETYPE ")");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setInsertionMode, ctl,
                  std::placeholders::_1),
        "bool setInsertionMode(bool)");

    registerAPI<bool(qsizetype, qsizetype, const QColor &, const QColor &,
                     const QString &)>(
        engine,
        std::bind(QOverload<qsizetype, qsizetype, const QColor &,
                            const QColor &, const QString &>::
                      of(&WingHex::WingPlugin::Controller::metadata),
                  ctl, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5),
        "bool metadata(" QSIZETYPE ", " QSIZETYPE
        ", color &in, color &in, string &in)");

    registerAPI<bool(qsizetype, qsizetype, qsizetype, const QColor &,
                     const QColor &, const QString &)>(
        engine,
        std::bind(QOverload<qsizetype, qsizetype, qsizetype, const QColor &,
                            const QColor &, const QString &>::
                      of(&WingHex::WingPlugin::Controller::metadata),
                  ctl, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5, std::placeholders::_6),
        "bool metadata(" QSIZETYPE ", " QSIZETYPE ", " QSIZETYPE
        ", color &in, color &in, string &in)");

    registerAPI<bool(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::removeMetadata, ctl,
                  std::placeholders::_1),
        "bool removeMetadata(" QSIZETYPE ")");

    registerAPI<bool()>(
        engine, std::bind(&WingHex::WingPlugin::Controller::clearMetadata, ctl),
        "bool clearMetadata()");

    registerAPI<bool(qsizetype, qsizetype, qsizetype, const QColor &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::foreground, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "bool foreground(" QSIZETYPE ", " QSIZETYPE ", " QSIZETYPE
        ", color &in)");

    registerAPI<bool(qsizetype, qsizetype, qsizetype, const QColor &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::background, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "bool background(" QSIZETYPE ", " QSIZETYPE ", " QSIZETYPE
        ", color &in)");

    registerAPI<bool(qsizetype, qsizetype, qsizetype, const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::comment, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "bool comment(" QSIZETYPE ", " QSIZETYPE ", " QSIZETYPE
        ", string &in)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setMetaVisible, ctl,
                  std::placeholders::_1),
        "bool setMetaVisible(bool)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setMetafgVisible, ctl,
                  std::placeholders::_1),
        "bool setMetafgVisible(bool)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setMetabgVisible, ctl,
                  std::placeholders::_1),
        "bool setMetabgVisible(bool)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setMetaCommentVisible, ctl,
                  std::placeholders::_1),
        "bool setMetaCommentVisible(bool)");

    registerAPI<bool()>(
        engine, std::bind(&WingHex::WingPlugin::Controller::newFile, ctl),
        "bool newFile()");

    registerAPI<WingHex::ErrFile(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::openFile, ctl,
                  std::placeholders::_1),
        "ErrFile openFile(string &in)");

    registerAPI<WingHex::ErrFile(const QString &, qsizetype, qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::openRegionFile, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "ErrFile openRegionFile(string &in, " QSIZETYPE " = 0, " QSIZETYPE
        " = 1024)");

    registerAPI<WingHex::ErrFile(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::openDriver, ctl,
                  std::placeholders::_1),
        "ErrFile openDriver(string &in)");

    registerAPI<WingHex::ErrFile(const QString &, bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::closeFile, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "ErrFile closeFile(string &in, bool = false)");

    registerAPI<WingHex::ErrFile(const QString &, bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::saveFile, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "ErrFile saveFile(string &in, bool = false)");

    registerAPI<WingHex::ErrFile(const QString &, const QString &, bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::exportFile, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "ErrFile exportFile(string &in, string &in, bool = false)");

    registerAPI<WingHex::ErrFile(const QString &, const QString &, bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::saveAsFile, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "ErrFile saveasFile(string &in, string &in, bool = false)");

    registerAPI<WingHex::ErrFile(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::closeCurrentFile, ctl,
                  std::placeholders::_1),
        "bool closeCurrentFile(bool = false)");

    registerAPI<WingHex::ErrFile(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::saveCurrentFile, ctl,
                  std::placeholders::_1),
        "bool saveCurrentFile(bool = false)");

    registerAPI<bool()>(
        engine, std::bind(&WingHex::WingPlugin::Controller::exportFileGUI, ctl),
        "bool exportFileGUI()");

    registerAPI<bool()>(
        engine, std::bind(&WingHex::WingPlugin::Controller::saveAsFileGUI, ctl),
        "bool saveAsFileGUI()");

    registerAPI<bool()>(
        engine, std::bind(&WingHex::WingPlugin::Controller::openFileGUI, ctl),
        "bool openFileGUI()");

    registerAPI<bool()>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::openRegionFileGUI, ctl),
        "bool openRegionFileGUI()");

    registerAPI<bool()>(
        engine, std::bind(&WingHex::WingPlugin::Controller::openDriverGUI, ctl),
        "bool openDriverGUI()");

    registerAPI<bool()>(
        engine, std::bind(&WingHex::WingPlugin::Controller::findGUI, ctl),
        "bool findGUI()");

    registerAPI<bool()>(
        engine, std::bind(&WingHex::WingPlugin::Controller::gotoGUI, ctl),
        "bool gotoGUI()");

    registerAPI<bool()>(
        engine, std::bind(&WingHex::WingPlugin::Controller::fillGUI, ctl),
        "bool fillGUI()");

    registerAPI<bool()>(
        engine, std::bind(&WingHex::WingPlugin::Controller::fillZeroGUI, ctl),
        "bool fillZeroGUI()");

    registerAPI<bool(qsizetype, const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::addBookMark, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool addBookMark(" QSIZETYPE ", string &in)");

    registerAPI<bool(qsizetype, const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::modBookMark, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool modBookMark(" QSIZETYPE ", string &in)");

    registerAPI<bool(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::removeBookMark, ctl,
                  std::placeholders::_1),
        "bool removeBookMark(" QSIZETYPE ")");

    registerAPI<bool()>(
        engine, std::bind(&WingHex::WingPlugin::Controller::clearBookMark, ctl),
        "bool clearBookMark()");

    registerAPI<bool(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::openWorkSpace, ctl,
                  std::placeholders::_1),
        "bool openWorkSpace(string &in)");

    registerAPI<bool(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setCurrentEncoding, ctl,
                  std::placeholders::_1),
        "bool setCurrentEncoding(string &in)");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installDataVisualAPI(asIScriptEngine *engine, int stringID) {
    int r = engine->SetDefaultNamespace("visual");
    Q_ASSERT(r >= 0);

    auto datavis = &this->visual;

    registerAPI<bool(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::DataVisual::updateText, datavis,
                  std::placeholders::_1),
        "bool updateText(string &in)");

    registerAPI<bool(const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_DataVisual_updateTextList, this, stringID,
                  std::placeholders::_1),
        "bool updateTextList(array<string> &in)");

    registerAPI<bool(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::DataVisual::updateTextTree, datavis,
                  std::placeholders::_1),
        "bool updateTextTree(string &in)");

    registerAPI<bool(const QString &, const CScriptArray &,
                     const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_DataVisual_updateTextTable, this, stringID,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "bool updateTextTable(string &in, array<string> &in, "
        "array<string> &in = array<string>())");

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

bool WingAngelAPI::read2Ref(qsizetype offset, void *ref, int typeId) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();

        if (typeId == asTYPEID_VOID)
            return false;
        else if (typeId == asTYPEID_BOOL)
            *reinterpret_cast<bool *>(ref) = (reader.readInt8(offset) != 0);
        else if (typeId == asTYPEID_INT8)
            *reinterpret_cast<qint8 *>(ref) = reader.readInt8(offset);
        else if (typeId == asTYPEID_INT16)
            *reinterpret_cast<qint16 *>(ref) = reader.readInt16(offset);
        else if (typeId == asTYPEID_INT32)
            *reinterpret_cast<qint32 *>(ref) = reader.readInt32(offset);
        else if (typeId == asTYPEID_INT64)
            *reinterpret_cast<qint64 *>(ref) = reader.readInt64(offset);
        else if (typeId == asTYPEID_UINT8)
            *reinterpret_cast<quint8 *>(ref) = quint8(reader.readInt8(offset));
        else if (typeId == asTYPEID_UINT16)
            *reinterpret_cast<quint16 *>(ref) =
                quint16(reader.readInt16(offset));
        else if (typeId == asTYPEID_UINT32)
            *reinterpret_cast<quint32 *>(ref) =
                quint32(reader.readInt32(offset));
        else if (typeId == asTYPEID_UINT64)
            *reinterpret_cast<quint64 *>(ref) =
                quint64(reader.readInt64(offset));
        else if (typeId == asTYPEID_FLOAT)
            *reinterpret_cast<float *>(ref) = reader.readFloat(offset);
        else if (typeId == asTYPEID_DOUBLE)
            *reinterpret_cast<double *>(ref) = reader.readDouble(offset);
        else if ((typeId & asTYPEID_MASK_OBJECT) == 0) {
            bool ok = false;
            // Check if the value matches one of the defined enums
            if (engine) {
                asITypeInfo *t = engine->GetTypeInfoById(typeId);
                for (int n = t->GetEnumValueCount(); n-- > 0;) {
                    int enumVal;
                    t->GetEnumValueByIndex(n, &enumVal);
                    if (enumVal == *(int *)ref) {
                        *reinterpret_cast<int *>(ref) = enumVal;
                        ok = true;
                        break;
                    }
                }
            }
            if (!ok) {
                return false;
            }
        } else if (typeId & asTYPEID_SCRIPTOBJECT) {
            void *value = ref;

            // Dereference handles, so we can see what it points to
            if (typeId & asTYPEID_OBJHANDLE)
                value = *(void **)value;

            asIScriptObject *obj = (asIScriptObject *)value;

            if (obj) {
                for (asUINT n = 0; n < obj->GetPropertyCount(); n++) {
                    auto id = obj->GetPropertyTypeId(n);
                    auto data = obj->GetAddressOfProperty(n);
                    auto size = getAsTypeSize(id, data);
                    read2Ref(offset, data, id);
                    if (size > 0) {
                        offset += size;
                    }
                }
            }
        } else {
            // Dereference handles, so we can see what it points to
            void *value = ref;

            if (typeId & asTYPEID_OBJHANDLE)
                value = *(void **)value;

            if (engine) {
                asITypeInfo *type = engine->GetTypeInfoByName("string");
                if (value) {
                    // TODO support other type, now only string
                    if (type->GetTypeId() == (typeId & ~asTYPEID_OBJHANDLE)) {
                        *reinterpret_cast<QString *>(value) =
                            reader.readString(offset);
                    }
                }
            }
        }
        return true;
    } else {
        return false;
    }
}

qsizetype WingAngelAPI::getAsTypeSize(int typeId, void *data) {
    if (typeId == asTYPEID_VOID)
        return false;
    else if (typeId == asTYPEID_BOOL)
        return 1;
    else if (typeId == asTYPEID_INT8)
        return sizeof(qint8);
    else if (typeId == asTYPEID_INT16)
        return sizeof(qint16);
    else if (typeId == asTYPEID_INT32)
        return sizeof(qint32);
    else if (typeId == asTYPEID_INT64)
        return sizeof(qint64);
    else if (typeId == asTYPEID_UINT8)
        return sizeof(quint8);
    else if (typeId == asTYPEID_UINT16)
        return sizeof(quint16);
    else if (typeId == asTYPEID_UINT32)
        return sizeof(quint32);
    else if (typeId == asTYPEID_UINT64)
        return sizeof(quint64);
    else if (typeId == asTYPEID_FLOAT)
        return sizeof(float);
    else if (typeId == asTYPEID_DOUBLE)
        return sizeof(double);
    else {
        typeId &= ~asTYPEID_OBJHANDLE;
        asIScriptContext *ctx = asGetActiveContext();
        auto engine = ctx->GetEngine();
        asITypeInfo *type = engine->GetTypeInfoByName("string");

        // TODO support other type, now only string
        if (type->GetTypeId() == (typeId & ~asTYPEID_OBJHANDLE)) {
            return reinterpret_cast<QString *>(data)->length() + 1;
        }
    }
    return -1;
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
        [&]() -> QStringList {
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

bool WingAngelAPI::_HexReader_read(qsizetype offset, void *ref, int typeId) {
    return read2Ref(offset, ref, typeId);
}

bool WingAngelAPI::_HexReader_write(qsizetype offset, void *ref, int typeId) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();

        if (typeId == asTYPEID_VOID)
            return false;
        else if (typeId == asTYPEID_BOOL)
            controller.writeInt8(offset,
                                 *reinterpret_cast<bool *>(ref) ? 1 : 0);
        else if (typeId == asTYPEID_INT8 || typeId == asTYPEID_UINT8)
            controller.writeInt8(offset, *reinterpret_cast<qint8 *>(ref));
        else if (typeId == asTYPEID_INT16 || typeId == asTYPEID_UINT16)
            controller.writeInt16(offset, *reinterpret_cast<qint16 *>(ref));
        else if (typeId == asTYPEID_INT32 || typeId == asTYPEID_UINT32)
            controller.writeInt32(offset, *reinterpret_cast<qint32 *>(ref));
        else if (typeId == asTYPEID_INT64 || typeId == asTYPEID_UINT64)
            controller.writeInt64(offset, *reinterpret_cast<qint64 *>(ref));
        else if (typeId == asTYPEID_FLOAT)
            controller.writeFloat(offset, *reinterpret_cast<float *>(ref));
        else if (typeId == asTYPEID_DOUBLE)
            controller.writeDouble(offset, *reinterpret_cast<double *>(ref));
        else if ((typeId & asTYPEID_MASK_OBJECT) == 0)
            controller.writeInt32(offset, *reinterpret_cast<int *>(ref));
        else if (typeId & asTYPEID_SCRIPTOBJECT) {
            // Dereference handles, so we can see what it points to
            void *value = ref;
            if (typeId & asTYPEID_OBJHANDLE)
                value = *(void **)value;

            if (engine) {
                asITypeInfo *t = engine->GetTypeInfoById(typeId);
                for (int n = t->GetEnumValueCount(); n-- > 0;) {
                    int enumVal;
                    t->GetEnumValueByIndex(n, &enumVal);
                    if (enumVal == *(int *)value) {
                        controller.writeInt32(offset, enumVal);
                        break;
                    }
                }
            }
        } else {
            // Dereference handles, so we can see what it points to
            void *value = ref;

            if (typeId & asTYPEID_OBJHANDLE)
                value = *(void **)value;

            if (engine) {
                asITypeInfo *type = engine->GetTypeInfoById(typeId);
                if (value) {
                    // TODO support other type, now only string
                    if (type->GetTypeId() == (typeId & ~asTYPEID_OBJHANDLE)) {
                        controller.writeString(
                            offset, *reinterpret_cast<QString *>(value));
                    }
                }
            }
        }
        return true;
    } else {
        return false;
    }
}

bool WingAngelAPI::_HexReader_insert(qsizetype offset, void *ref, int typeId) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();

        if (typeId == asTYPEID_VOID)
            return false;
        else if (typeId == asTYPEID_BOOL)
            controller.insertInt8(offset,
                                  *reinterpret_cast<bool *>(ref) ? 1 : 0);
        else if (typeId == asTYPEID_INT8 || typeId == asTYPEID_UINT8)
            controller.insertInt8(offset, *reinterpret_cast<qint8 *>(ref));
        else if (typeId == asTYPEID_INT16 || typeId == asTYPEID_UINT16)
            controller.insertInt16(offset, *reinterpret_cast<qint16 *>(ref));
        else if (typeId == asTYPEID_INT32 || typeId == asTYPEID_UINT32)
            controller.insertInt32(offset, *reinterpret_cast<qint32 *>(ref));
        else if (typeId == asTYPEID_INT64 || typeId == asTYPEID_UINT64)
            controller.insertInt64(offset, *reinterpret_cast<qint64 *>(ref));
        else if (typeId == asTYPEID_FLOAT)
            controller.insertFloat(offset, *reinterpret_cast<float *>(ref));
        else if (typeId == asTYPEID_DOUBLE)
            controller.insertDouble(offset, *reinterpret_cast<double *>(ref));
        else if ((typeId & asTYPEID_MASK_OBJECT) == 0)
            controller.insertInt32(offset, *reinterpret_cast<int *>(ref));
        else if (typeId & asTYPEID_SCRIPTOBJECT) {
            // Dereference handles, so we can see what it points to
            void *value = ref;
            if (typeId & asTYPEID_OBJHANDLE)
                value = *(void **)value;

            if (engine) {
                asITypeInfo *t = engine->GetTypeInfoById(typeId);
                for (int n = t->GetEnumValueCount(); n-- > 0;) {
                    int enumVal;
                    t->GetEnumValueByIndex(n, &enumVal);
                    if (enumVal == *(int *)value) {
                        controller.insertInt32(offset, enumVal);
                        break;
                    }
                }
            }
        } else {
            // Dereference handles, so we can see what it points to
            void *value = ref;

            if (typeId & asTYPEID_OBJHANDLE)
                value = *(void **)value;

            if (engine) {
                asITypeInfo *type = engine->GetTypeInfoById(typeId);
                if (value) {
                    // TODO support other type, now only string
                    if (type->GetTypeId() == (typeId & ~asTYPEID_OBJHANDLE)) {
                        controller.insertString(
                            offset, *reinterpret_cast<QString *>(value));
                    }
                }
            }
        }
        return true;
    } else {
        return false;
    }
}

bool WingAngelAPI::_HexReader_append(qsizetype offset, void *ref, int typeId) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();

        if (typeId == asTYPEID_VOID)
            return false;
        else if (typeId == asTYPEID_BOOL)
            controller.appendInt8(*reinterpret_cast<bool *>(ref) ? 1 : 0);
        else if (typeId == asTYPEID_INT8 || typeId == asTYPEID_UINT8)
            controller.appendInt8(*reinterpret_cast<qint8 *>(ref));
        else if (typeId == asTYPEID_INT16 || typeId == asTYPEID_UINT16)
            controller.appendInt16(*reinterpret_cast<qint16 *>(ref));
        else if (typeId == asTYPEID_INT32 || typeId == asTYPEID_UINT32)
            controller.appendInt32(*reinterpret_cast<qint32 *>(ref));
        else if (typeId == asTYPEID_INT64 || typeId == asTYPEID_UINT64)
            controller.appendInt64(*reinterpret_cast<qint64 *>(ref));
        else if (typeId == asTYPEID_FLOAT)
            controller.appendFloat(*reinterpret_cast<float *>(ref));
        else if (typeId == asTYPEID_DOUBLE)
            controller.appendDouble(*reinterpret_cast<double *>(ref));
        else if ((typeId & asTYPEID_MASK_OBJECT) == 0)
            controller.appendInt32(*reinterpret_cast<int *>(ref));
        else if (typeId & asTYPEID_SCRIPTOBJECT) {
            // Dereference handles, so we can see what it points to
            void *value = ref;
            if (typeId & asTYPEID_OBJHANDLE)
                value = *(void **)value;

            if (engine) {
                asITypeInfo *t = engine->GetTypeInfoById(typeId);
                for (int n = t->GetEnumValueCount(); n-- > 0;) {
                    int enumVal;
                    t->GetEnumValueByIndex(n, &enumVal);
                    if (enumVal == *(int *)value) {
                        controller.appendInt32(enumVal);
                        break;
                    }
                }
            }
        } else {
            // Dereference handles, so we can see what it points to
            void *value = ref;

            if (typeId & asTYPEID_OBJHANDLE)
                value = *(void **)value;

            if (engine) {
                asITypeInfo *type = engine->GetTypeInfoById(typeId);
                if (value) {
                    // TODO support other type, now only string
                    if (type->GetTypeId() == (typeId & ~asTYPEID_OBJHANDLE)) {
                        controller.appendString(
                            *reinterpret_cast<QString *>(value));
                    }
                }
            }
        }
        return true;
    } else {
        return false;
    }
}

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

CScriptArray *WingAngelAPI::_HexReader_getSupportedEncodings() {
    return retarrayWrapperFunction(
        [this]() -> QStringList { return emit reader.getSupportedEncodings(); },
        "array<string>");
}

bool WingAngelAPI::_HexController_writeBytes(qsizetype offset,
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
        if (!ok) {
            return false;
        }
        return emit controller.writeBytes(offset, bab);
    } else {
        return false;
    }
}

bool WingAngelAPI::_HexController_insertBytes(qsizetype offset,
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
        if (!ok) {
            return false;
        }
        return emit controller.insertBytes(offset, bab);
    } else {
        return false;
    }
}

bool WingAngelAPI::_HexController_appendBytes(const CScriptArray &ba) {
    // If called from the script, there will always be an active
    // context, which can be used to obtain a pointer to the engine.
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();
        bool ok = false;
        auto byteID = engine->GetTypeIdByDecl("byte");
        Q_ASSERT(byteID);
        auto bab = cArray2ByteArray(ba, byteID, &ok);
        if (!ok) {
            return false;
        }
        return emit controller.appendBytes(bab);
    } else {
        return false;
    }
}

bool WingAngelAPI::_DataVisual_updateTextList(int stringID,
                                              const CScriptArray &data) {
    bool o = false;
    auto ret = cArray2QStringList(data, stringID, &o);
    if (o) {
        return visual.updateTextList(ret);
    } else {
        return false;
    }
}

bool WingAngelAPI::_DataVisual_updateTextTable(
    int stringID, const QString &json, const CScriptArray &headers,
    const CScriptArray &headerNames) {
    bool o = false;
    auto h = cArray2QStringList(headers, stringID, &o);
    if (o) {
        auto hn = cArray2QStringList(headerNames, stringID, &o);
        if (o) {
            return visual.updateTextTable(json, h, hn);
        } else {
            return false;
        }
    } else {
        return false;
    }
}
