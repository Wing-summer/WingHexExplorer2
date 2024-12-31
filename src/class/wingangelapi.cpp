/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#include "wingangelapi.h"

#include "AngelScript/sdk/angelscript/include/angelscript.h"
#include "class/angelscripthelper.h"
#include "class/logger.h"
#include "class/scriptmachine.h"
#include "class/wingfiledialog.h"
#include "class/winginputdialog.h"
#include "control/scriptingconsole.h"
#include "define.h"
#include "scriptaddon/scriptqdictionary.h"

#include <QJsonDocument>

#ifdef Q_OS_WIN
#undef MessageBox
#endif

WingAngelAPI::WingAngelAPI() {
    qsizetype signalCount = 0;
    const QMetaObject *objs[]{
        WingAngelAPI::metaObject(),    this->reader.metaObject(),
        this->controller.metaObject(), this->msgbox.metaObject(),
        this->inputbox.metaObject(),   this->filedlg.metaObject(),
        this->colordlg.metaObject(),   this->visual.metaObject()};
    for (auto &obj : objs) {
        for (auto i = obj->methodOffset(); i < obj->methodCount(); ++i) {
            if (obj->method(i).methodType() == QMetaMethod::Signal) {
                signalCount++;
            }
        }
    }

    _fnbuffer.reserve(signalCount);
}

WingAngelAPI::~WingAngelAPI() { _fnbuffer.clear(); }

int WingAngelAPI::sdkVersion() const { return WingHex::SDKVERSION; }

const QString WingAngelAPI::signature() const { return WingHex::WINGSUMMER; }

bool WingAngelAPI::init(const std::unique_ptr<QSettings> &set) {
    Q_UNUSED(set);
    return true;
}

void WingAngelAPI::unload(std::unique_ptr<QSettings> &set) {
    Q_UNUSED(set);
    this->disconnect();
}

const QString WingAngelAPI::pluginName() const {
    return tr("AngelScriptService");
}

const QString WingAngelAPI::pluginAuthor() const { return WingHex::WINGSUMMER; }

uint WingAngelAPI::pluginVersion() const { return WingHex::SDKVERSION; }

const QString WingAngelAPI::pluginComment() const {
    return tr("A internal plugin that provides AngelScript scripts with the "
              "ability to call the host API.");
}

void WingAngelAPI::registerScriptFns(const QString &ns,
                                     const QHash<QString, ScriptFnInfo> &rfns) {
    Q_ASSERT(!ns.isEmpty());
    if (rfns.isEmpty()) {
        return;
    }

    auto id = _sfns.size();
    for (auto p = rfns.constKeyValueBegin(); p != rfns.constKeyValueEnd();
         p++) {
        _rfns[ns][p->first] = id;
        id++;
        _sfns.append(p->second);
    }
}

void WingAngelAPI::installAPI(ScriptMachine *machine) {
    Q_ASSERT(machine);

    auto engine = machine->engine();
    auto stringTypeID = machine->typeInfo(ScriptMachine::tString)->GetTypeId();

    installExtAPI(engine);
    installLogAPI(engine);
    installMsgboxAPI(engine);
    installInputboxAPI(engine, stringTypeID);
    installFileDialogAPI(engine);
    installColorDialogAPI(engine);

    installHexBaseType(engine);
    installHexReaderAPI(engine);
    installHexControllerAPI(engine);
    installDataVisualAPI(engine, stringTypeID);

    installScriptFns(engine);
}

void WingAngelAPI::installLogAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("log");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    registerAPI<void(const QString &)>(
        engine, std::bind(&WingAngelAPI::info, this, std::placeholders::_1),
        "void info(const string &in message)");

    registerAPI<void(const QString &)>(
        engine, std::bind(&WingAngelAPI::trace, this, std::placeholders::_1),
        "void trace(const string &in message)");

    registerAPI<void(const QString &)>(
        engine, std::bind(&WingAngelAPI::debug, this, std::placeholders::_1),
        "void debug(const string &in message)");

    registerAPI<void(const QString &)>(
        engine, std::bind(&WingAngelAPI::warn, this, std::placeholders::_1),
        "void warn(const string &in message)");

    registerAPI<void(const QString &)>(
        engine, std::bind(&WingAngelAPI::error, this, std::placeholders::_1),
        "void error(const string &in message)");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installExtAPI(asIScriptEngine *engine) {
    // toast(message, iconPath)
    registerAPI<void(const QString &, const QString &)>(
        engine,
        std::bind(&WingAngelAPI::toast, this, std::placeholders::_2,
                  std::placeholders::_1),
        "void toast(const string &in message, const string &in icon =\"\")");
}

void WingAngelAPI::installMsgboxAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("msgbox");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    registerAngelType<QMessageBox::StandardButtons>(engine, "buttons");
    registerAngelType<QMessageBox::Icon>(engine, "icon");
    auto msgbox = &this->msgbox;

    registerAPI<void(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::MessageBox::aboutQt, msgbox, nullptr,
                  std::placeholders::_1),
        "void aboutQt(const string &in title =\"\")");

    registerAPI<QMessageBox::StandardButton(const QString &, const QString &,
                                            QMessageBox::StandardButtons,
                                            QMessageBox::StandardButton)>(
        engine,
        std::bind(&WingHex::WingPlugin::MessageBox::information, msgbox,
                  nullptr, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "void information(const string &in title, const string &in text, "
        "msgbox::buttons buttons = msgbox::buttons::Ok, "
        "msgbox::buttons defaultButton = msgbox::buttons::NoButton)");

    registerAPI<QMessageBox::StandardButton(const QString &, const QString &,
                                            QMessageBox::StandardButtons,
                                            QMessageBox::StandardButton)>(
        engine,
        std::bind(&WingHex::WingPlugin::MessageBox::question, msgbox, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "void question(const string &in title, const string &in text, "
        "msgbox::buttons buttons = msgbox::buttons::Yes | "
        "msgbox::buttons::No, "
        "msgbox::buttons defaultButton = msgbox::buttons::NoButton)");

    registerAPI<QMessageBox::StandardButton(const QString &, const QString &,
                                            QMessageBox::StandardButtons,
                                            QMessageBox::StandardButton)>(
        engine,
        std::bind(&WingHex::WingPlugin::MessageBox::warning, msgbox, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "void warning(const string &in title, const string &in text, "
        "msgbox::buttons buttons = msgbox::buttons::Ok, "
        "msgbox::buttons defaultButton = msgbox::buttons::NoButton)");

    registerAPI<QMessageBox::StandardButton(const QString &, const QString &,
                                            QMessageBox::StandardButtons,
                                            QMessageBox::StandardButton)>(
        engine,
        std::bind(&WingHex::WingPlugin::MessageBox::critical, msgbox, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "void critical(const string &in title, const string &in text, "
        "msgbox::buttons buttons = msgbox::buttons::Ok, "
        "msgbox::buttons defaultButton = msgbox::buttons::NoButton)");

    registerAPI<QMessageBox::StandardButton(
        QMessageBox::Icon, const QString &, const QString &,
        QMessageBox::StandardButtons, QMessageBox::StandardButton)>(
        engine,
        std::bind(&WingHex::WingPlugin::MessageBox::msgbox, msgbox, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5),
        "void msgbox(msgbox::icon icon, const string &in title, "
        "const string &in text, "
        "msgbox::buttons buttons = msgbox::buttons::NoButton, "
        "msgbox::buttons defaultButton = msgbox::buttons::NoButton)");

    registerAPI<void(const QString &, const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::MessageBox::about, msgbox, nullptr,
                  std::placeholders::_1, std::placeholders::_2),
        "void about(const string &in title, const string &in text)");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installInputboxAPI(asIScriptEngine *engine, int stringID) {
    int r = engine->SetDefaultNamespace("inputbox");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

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
        "string getText(const string &in title, const string &in label, "
        "inputbox::EchoMode echo = inputbox::Normal, "
        "const string &in text = \"\", bool &out ok = false, "
        "inputbox::InputMethodHints inputMethodHints = inputbox::ImhNone)");

    registerAPI<QString(const QString &, const QString &, const QString &,
                        bool *, Qt::InputMethodHints inputMethodHints)>(
        engine,
        std::bind(&WingHex::WingPlugin::InputBox::getMultiLineText, inputbox,
                  nullptr, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5),
        "string getMultiLineText(const string &in title, "
        "const string &in label, "
        "const string &in text = \"\", bool &out ok = false, "
        "inputbox::InputMethodHints inputMethodHints = inputbox::ImhNone)");

    registerAPI<int(const QString &, const QString &, int, int, int, int,
                    bool *)>(
        engine,
        std::bind(&WingHex::WingPlugin::InputBox::getInt, inputbox, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5, std::placeholders::_6,
                  std::placeholders::_7),
        "int getInt(const string &in title, const string &in label, "
        "int &in value = 0, int &in minValue = -2147483647, "
        "int &in maxValue = 2147483647, "
        "int &in step = 1, bool &out ok = false)");

    registerAPI<int(const QString &, const QString &, double, double, double,
                    int, bool *, double)>(
        engine,
        std::bind(&WingHex::WingPlugin::InputBox::getDouble, inputbox, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5, std::placeholders::_6,
                  std::placeholders::_7, std::placeholders::_8),
        "double getDouble(const string &in title, const string &in label, "
        "double &in value = 0, double &in minValue = -2147483647, "
        "double &in maxValue = 2147483647, int &in decimals = 1, "
        "bool &out ok = false, double &in step = 1)");

    registerAPI<QString(const QString &, const QString &, const CScriptArray &,
                        int, bool, bool *, Qt::InputMethodHints)>(
        engine,
        std::bind(&WingAngelAPI::_InputBox_getItem, this, stringID,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5, std::placeholders::_6,
                  std::placeholders::_7),
        "string getItem(const string &in title, const string &in label, "
        "const array<string> &in items, int current = 0, "
        "bool editable = true, bool &out ok = false, "
        "inputbox::InputMethodHints inputMethodHints = inputbox::ImhNone)");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installFileDialogAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("filedlg");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    registerAngelType<QFileDialog::Options>(engine, "options");

    auto filedlg = &this->filedlg;

    registerAPI<QString(const QString &, const QString &,
                        QFileDialog::Options)>(
        engine,
        std::bind(&WingHex::WingPlugin::FileDialog::getExistingDirectory,
                  filedlg, nullptr, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3),
        "string getExistingDirectory(const string &in caption = \"\", "
        "const string &in dir = \"\", "
        "filedlg::options &in options = filedlg::options::ShowDirsOnly)");

    registerAPI<QString(const QString &, const QString &, const QString &,
                        QString *, QFileDialog::Options)>(
        engine,
        std::bind(&WingHex::WingPlugin::FileDialog::getOpenFileName, filedlg,
                  nullptr, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5),
        "string getOpenFileName(const string &in caption = \"\", "
        "const string &in dir = \"\", const string &in filter = \"\", "
        "string &out selectedFilter = \"\", filedlg::options &in options = 0)");

    registerAPI<QString(const QString &, const QString &, const QString &,
                        QString *, QFileDialog::Options)>(
        engine,
        std::bind(&WingHex::WingPlugin::FileDialog::getSaveFileName, filedlg,
                  nullptr, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5),
        "string getSaveFileName(const string &in caption = \"\", "
        "const string &in dir = \"\", const string &in filter = \"\", "
        "string &out selectedFilter = \"\", filedlg::options &in options = 0)");

    registerAPI<CScriptArray *(const QString &, const QString &,
                               const QString &, QString *,
                               QFileDialog::Options)>(
        engine,
        std::bind(&WingAngelAPI::_FileDialog_getOpenFileNames, this,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5),
        "array<string>@ getOpenFileNames(const string &in caption = \"\", "
        "const string &in dir = \"\", const string &in filter = \"\", "
        "string &out selectedFilter = \"\", filedlg::options &in options = 0)");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installColorDialogAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("colordlg");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    auto colordlg = &this->colordlg;

    registerAPI<QColor(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::ColorDialog::getColor, colordlg,
                  std::placeholders::_1, nullptr),
        "color getColor(const string &in caption)");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installHexBaseType(asIScriptEngine *engine) {
    registerAngelType<WingHex::ErrFile>(engine, "ErrFile");
    registerAngelType<WingHex::SelectionMode>(engine, "SelectionMode");

    int r = engine->RegisterTypedef("byte", "uint8");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // FindResult
    r = engine->RegisterObjectType("FindResult", sizeof(WingHex::FindResult),
                                   asOBJ_VALUE | asOBJ_POD |
                                       asGetTypeTraits<WingHex::FindResult>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectProperty("FindResult", QSIZETYPE_WRAP("offset"),
                                       asOFFSET(WingHex::FindResult, offset));
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectProperty("FindResult", QSIZETYPE_WRAP("line"),
                                       asOFFSET(WingHex::FindResult, line));
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectProperty("FindResult", QSIZETYPE_WRAP("col"),
                                       asOFFSET(WingHex::FindResult, col));
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // HexPosition
    r = engine->RegisterObjectType("HexPosition", sizeof(WingHex::HexPosition),
                                   asOBJ_VALUE | asOBJ_POD |
                                       asGetTypeTraits<WingHex::HexPosition>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectProperty("HexPosition", QSIZETYPE_WRAP("line"),
                                       asOFFSET(WingHex::HexPosition, line));
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectProperty("HexPosition", "int column",
                                       asOFFSET(WingHex::HexPosition, column));
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectProperty(
        "HexPosition", "uint8 lineWidth",
        asOFFSET(WingHex::HexPosition, lineWidth));
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectProperty(
        "HexPosition", "int nibbleindex",
        asOFFSET(WingHex::HexPosition, nibbleindex));
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("HexPosition", QSIZETYPE_WRAP("offset()"),
                                     asMETHOD(WingHex::HexPosition, offset),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "HexPosition", "int opSub(const HexPosition &in) const",
        asMETHODPR(WingHex::HexPosition, operator-,
                   (const WingHex::HexPosition &) const, qsizetype),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "HexPosition", "bool opEquals(const HexPosition &in) const",
        asMETHODPR(WingHex::HexPosition, operator==,
                   (const WingHex::HexPosition &) const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
}

void WingAngelAPI::installHexReaderAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("reader");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
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

    registerAPI<CScriptArray *(qsizetype)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_selectedBytes, this,
                  std::placeholders::_1),
        "array<byte>@ selectedBytes(" QSIZETYPE " index)");

    registerAPI<CScriptArray *()>(
        engine, std::bind(&WingAngelAPI::_HexReader_selectionBytes, this),
        "array<array<byte>>@ selectionBytes()");
    registerAPI<WingHex::HexPosition(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::selectionStart, reader,
                  std::placeholders::_1),
        "HexPosition selectionStart(" QSIZETYPE " index)");
    registerAPI<WingHex::HexPosition(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::selectionEnd, reader,
                  std::placeholders::_1),
        "HexPosition selectionEnd(" QSIZETYPE " index)");
    registerAPI<qsizetype(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::selectionLength, reader,
                  std::placeholders::_1),
        QSIZETYPE_WRAP("selectionLength(" QSIZETYPE " index)"));

    registerAPI<qsizetype()>(
        engine, std::bind(&WingHex::WingPlugin::Reader::selectionCount, reader),
        QSIZETYPE_WRAP("selectionCount()"));

    registerAPI<quintptr(void)>(
        engine, std::bind(&WingHex::WingPlugin::Reader::addressBase, reader),
        QPTR_WRAP("addressBase()"));

    registerAPI<CScriptArray *(qsizetype, qsizetype)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_readBytes, this,
                  std::placeholders::_1, std::placeholders::_2),
        "array<byte>@ readBytes(" QSIZETYPE " offset," QSIZETYPE " len)");

    registerAPI<qint8(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::readInt8, reader,
                  std::placeholders::_1),
        "int8 readInt8(" QSIZETYPE " offset)");

    registerAPI<qint16(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::readInt16, reader,
                  std::placeholders::_1),
        "int16 readInt16(" QSIZETYPE " offset)");

    registerAPI<qint32(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::readInt32, reader,
                  std::placeholders::_1),
        "int readInt32(" QSIZETYPE " offset)");

    registerAPI<qint64(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::readInt64, reader,
                  std::placeholders::_1),
        "int64 readInt64(" QSIZETYPE " offset)");

    registerAPI<float(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::readFloat, reader,
                  std::placeholders::_1),
        "float readFloat(" QSIZETYPE " offset)");

    registerAPI<double(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::readDouble, reader,
                  std::placeholders::_1),
        "double readDouble(" QSIZETYPE " offset)");

    registerAPI<QString(qsizetype, QString)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::readString, reader,
                  std::placeholders::_1, std::placeholders::_2),
        "string readInt64(" QSIZETYPE " offset, string &in encoding = \"\")");

    registerAPI<qsizetype(qsizetype, const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_searchForward, this,
                  std::placeholders::_1, std::placeholders::_2),
        QSIZETYPE_WRAP("searchForward(" QSIZETYPE
                       " begin, array<byte> &in ba)"));

    registerAPI<qsizetype(qsizetype, const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_searchBackward, this,
                  std::placeholders::_1, std::placeholders::_2),
        QSIZETYPE_WRAP("searchBackward(" QSIZETYPE
                       " begin, array<byte> &in ba)"));

    registerAPI<CScriptArray *(qsizetype, qsizetype, const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_findAllBytes, this,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "array<" QSIZETYPE ">@ findAllBytes(" QSIZETYPE " begin, " QSIZETYPE
        " end, array<byte> &in b)");

    registerAPI<bool(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::lineHasMetadata, reader,
                  std::placeholders::_1),
        "bool lineHasMetadata(" QSIZETYPE " line)");

    registerAPI<bool(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::lineHasBookMark, reader,
                  std::placeholders::_1),
        "bool lineHasBookMark(" QSIZETYPE " line)");

    registerAPI<CScriptArray *(qsizetype)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_getsBookmarkPos, this,
                  std::placeholders::_1),
        "array<" QSIZETYPE ">@ getsBookmarkPos(" QSIZETYPE " pos)");

    registerAPI<QString(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::bookMarkComment, reader,
                  std::placeholders::_1),
        "string bookMarkComment(" QSIZETYPE " pos)");

    registerAPI<bool(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::existBookMark, reader,
                  std::placeholders::_1),
        "bool existBookMark(" QSIZETYPE " pos)");

    registerAPI<CScriptArray *()>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_getSupportedEncodings, this),
        "array<string>@ getSupportedEncodings()");

    registerAPI<CScriptArray *()>(
        engine, std::bind(&WingAngelAPI::_HexReader_getStorageDrivers, this),
        "array<string>@ getStorageDrivers()");

    registerAPI<QString()>(
        engine,
        std::bind(&WingHex::WingPlugin::Reader::currentEncoding, reader),
        "string currentEncoding()");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installHexControllerAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("ctl");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    auto ctl = &this->controller;

    registerAPI<bool(int)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::switchDocument, ctl,
                  std::placeholders::_1),
        "bool switchDocument(int handle)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setLockedFile, ctl,
                  std::placeholders::_1),
        "bool setLockedFile(bool b)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setKeepSize, ctl,
                  std::placeholders::_1),
        "bool setKeepSize(bool b)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setStringVisible, ctl,
                  std::placeholders::_1),
        "bool setStringVisible(bool b)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setAddressVisible, ctl,
                  std::placeholders::_1),
        "bool setAddressVisible(bool b)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setHeaderVisible, ctl,
                  std::placeholders::_1),
        "bool setHeaderVisible(bool b)");

    registerAPI<bool(quintptr)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setAddressBase, ctl,
                  std::placeholders::_1),
        "bool setAddressBase(" QPTR " base)");

    registerAPI<bool(qsizetype, qint8)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::writeInt8, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeInt8(" QSIZETYPE " offset, int8 value)");

    registerAPI<bool(qsizetype, qint16)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::writeInt16, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeInt16(" QSIZETYPE " offset, int16 value)");

    registerAPI<bool(qsizetype, qint32)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::writeInt32, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeInt32(" QSIZETYPE " offset, int value)");

    registerAPI<bool(qsizetype, qint64)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::writeInt64, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeInt64(" QSIZETYPE " offset, int64 value)");

    registerAPI<bool(qsizetype, float)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::writeFloat, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeFloat(" QSIZETYPE " offset, float value)");

    registerAPI<bool(qsizetype, double)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::writeDouble, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeDouble(" QSIZETYPE " offset, double value)");

    registerAPI<bool(qsizetype, const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexController_writeBytes, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeBytes(" QSIZETYPE " offset, array<byte>&in data)");

    registerAPI<bool(qsizetype, const QString &, const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::writeString, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "bool writeString(" QSIZETYPE " offset, string &in value, "
        "string &in encoding = \"\")");

    registerAPI<bool(qsizetype, qint8)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::insertInt8, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertInt8(" QSIZETYPE " offset, int8 value)");

    registerAPI<bool(qsizetype, qint16)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::insertInt16, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertInt16(" QSIZETYPE " offset, int16 value)");

    registerAPI<bool(qsizetype, qint32)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::insertInt32, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertInt32(" QSIZETYPE " offset, int value)");

    registerAPI<bool(qsizetype, qint32)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::insertInt64, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertInt64(" QSIZETYPE " offset, int64 value)");

    registerAPI<bool(qsizetype, float)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::insertFloat, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertFloat(" QSIZETYPE " offset, float value)");

    registerAPI<bool(qsizetype, double)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::insertDouble, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertDouble(" QSIZETYPE " offset, double value)");

    registerAPI<bool(qsizetype, const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexController_insertBytes, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertBytes(" QSIZETYPE " offset, array<byte>&in data)");

    registerAPI<bool(qsizetype, const QString &, const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::insertString, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "bool insertString(" QSIZETYPE " offset, string &in value, "
        "string &in encoding = \"\")");

    registerAPI<bool(qint8)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::appendInt8, ctl,
                  std::placeholders::_1),
        "bool appendInt8(int8 value)");

    registerAPI<bool(qint16)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::appendInt16, ctl,
                  std::placeholders::_1),
        "bool appendInt16(int16 value)");

    registerAPI<bool(qint32)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::appendInt32, ctl,
                  std::placeholders::_1),
        "bool appendInt32(int value)");

    registerAPI<bool(qint32)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::appendInt64, ctl,
                  std::placeholders::_1),
        "bool appendInt64(int64 value)");

    registerAPI<bool(float)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::appendFloat, ctl,
                  std::placeholders::_1),
        "bool appendFloat(float value)");

    registerAPI<bool(double)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::appendDouble, ctl,
                  std::placeholders::_1),
        "bool appendDouble(double value)");

    registerAPI<bool(const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexController_appendBytes, this,
                  std::placeholders::_1),
        "bool appendBytes(array<byte>&in data)");

    registerAPI<bool(const QString &, const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::appendString, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool appendString(string &in value, string &in encoding = \"\")");

    registerAPI<bool(qsizetype, qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::remove, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool remove(" QSIZETYPE " offset, " QSIZETYPE " len)");

    registerAPI<bool()>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::removeAllBytes, ctl),
        "bool removeAllBytes()");

    registerAPI<bool(qsizetype, qsizetype, int, bool)>(
        engine,
        std::bind(QOverload<qsizetype, qsizetype, int, bool>::of(
                      &WingHex::WingPlugin::Controller::moveTo),
                  ctl, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "bool moveTo(" QSIZETYPE " line, " QSIZETYPE
        " column, int nibbleindex = -1, bool clearSelection = true)");

    registerAPI<bool(qsizetype, bool)>(
        engine,
        std::bind(QOverload<qsizetype, bool>::of(
                      &WingHex::WingPlugin::Controller::moveTo),
                  ctl, std::placeholders::_1, std::placeholders::_2),
        "bool moveTo(" QSIZETYPE " offset, bool clearSelection = true)");

    registerAPI<bool(qsizetype, qsizetype, WingHex::SelectionMode)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::select, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "bool select(" QSIZETYPE " offset, " QSIZETYPE
        " len, SelectionMode mode = SelectionMode::Add)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setInsertionMode, ctl,
                  std::placeholders::_1),
        "bool setInsertionMode(bool b)");

    registerAPI<bool(qsizetype, qsizetype, const QColor &, const QColor &,
                     const QString &)>(
        engine,
        std::bind(QOverload<qsizetype, WingHex::qusizetype, const QColor &,
                            const QColor &, const QString &>::
                      of(&WingHex::WingPlugin::Controller::metadata),
                  ctl, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5),
        "bool metadata(" QSIZETYPE " begin, " QUSIZETYPE
        " length, color &in fgcolor, color &in bgcolor, string &in comment)");

    registerAPI<bool(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::removeMetadata, ctl,
                  std::placeholders::_1),
        "bool removeMetadata(" QSIZETYPE " offset)");

    registerAPI<bool()>(
        engine, std::bind(&WingHex::WingPlugin::Controller::clearMetadata, ctl),
        "bool clearMetadata()");

    registerAPI<bool(qsizetype, WingHex::qusizetype, const QColor &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::foreground, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "bool foreground(" QSIZETYPE " begin, " QUSIZETYPE
        " length, color &in fgcolor)");

    registerAPI<bool(qsizetype, WingHex::qusizetype, const QColor &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::background, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "bool background(" QSIZETYPE " begin, " QUSIZETYPE
        " length, color &in bgcolor)");

    registerAPI<bool(qsizetype, WingHex::qusizetype, const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::comment, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "bool comment(" QSIZETYPE " begin, " QUSIZETYPE
        " length, string &in comment)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setMetaVisible, ctl,
                  std::placeholders::_1),
        "bool setMetaVisible(bool b)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setMetafgVisible, ctl,
                  std::placeholders::_1),
        "bool setMetafgVisible(bool b)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setMetabgVisible, ctl,
                  std::placeholders::_1),
        "bool setMetabgVisible(bool b)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setMetaCommentVisible, ctl,
                  std::placeholders::_1),
        "bool setMetaCommentVisible(bool b)");

    registerAPI<bool()>(
        engine, std::bind(&WingHex::WingPlugin::Controller::newFile, ctl),
        "bool newFile()");

    registerAPI<WingHex::ErrFile(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::openFile, ctl,
                  std::placeholders::_1),
        "ErrFile openFile(string &in filename)");

    registerAPI<WingHex::ErrFile(const QString &, qsizetype, qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::openRegionFile, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "ErrFile openRegionFile(string &in filename, " QSIZETYPE
        " start = 0, " QSIZETYPE " length = 1024)");

    registerAPI<WingHex::ErrFile(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::openDriver, ctl,
                  std::placeholders::_1),
        "ErrFile openDriver(string &in driver)");

    registerAPI<WingHex::ErrFile(int, bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::closeFile, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "ErrFile closeFile(int handle, bool force = false)");

    registerAPI<WingHex::ErrFile(int, bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::saveFile, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "ErrFile saveFile(int handle, bool ignoreMd5 = false)");

    registerAPI<WingHex::ErrFile(int, const QString &, bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::exportFile, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "ErrFile exportFile(int handle, string &in savename, "
        "bool ignoreMd5 = false)");

    registerAPI<WingHex::ErrFile(int, const QString &, bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::saveAsFile, ctl,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "ErrFile saveasFile(int handle, string &in savename, "
        "bool ignoreMd5 = false)");

    registerAPI<WingHex::ErrFile(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::closeCurrentFile, ctl,
                  std::placeholders::_1),
        "bool closeCurrentFile(bool force = false)");

    registerAPI<WingHex::ErrFile(bool)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::saveCurrentFile, ctl,
                  std::placeholders::_1),
        "bool saveCurrentFile(bool ignoreMd5 = false)");

    registerAPI<bool(qsizetype, const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::addBookMark, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool addBookMark(" QSIZETYPE " pos, string &in comment)");

    registerAPI<bool(qsizetype, const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::modBookMark, ctl,
                  std::placeholders::_1, std::placeholders::_2),
        "bool modBookMark(" QSIZETYPE " pos, string &in comment)");

    registerAPI<bool(qsizetype)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::removeBookMark, ctl,
                  std::placeholders::_1),
        "bool removeBookMark(" QSIZETYPE " pos)");

    registerAPI<bool()>(
        engine, std::bind(&WingHex::WingPlugin::Controller::clearBookMark, ctl),
        "bool clearBookMark()");

    registerAPI<WingHex::ErrFile(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::openWorkSpace, ctl,
                  std::placeholders::_1),
        "ErrFile openWorkSpace(string &in filename)");

    registerAPI<bool(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::setCurrentEncoding, ctl,
                  std::placeholders::_1),
        "bool setCurrentEncoding(string &in encoding)");

    registerAPI<void()>(
        engine,
        std::bind(&WingHex::WingPlugin::Controller::closeAllPluginFiles, ctl),
        "bool closeAllPluginFiles()");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installDataVisualAPI(asIScriptEngine *engine, int stringID) {
    int r = engine->SetDefaultNamespace("visual");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    auto datavis = &this->visual;

    registerAPI<bool(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::DataVisual::updateText, datavis,
                  std::placeholders::_1),
        "bool updateText(string &in data)");

    registerAPI<bool(const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_DataVisual_updateTextList, this, stringID,
                  std::placeholders::_1),
        "bool updateTextList(array<string> &in data)");

    registerAPI<bool(const QString &)>(
        engine,
        std::bind(&WingHex::WingPlugin::DataVisual::updateTextTree, datavis,
                  std::placeholders::_1,
                  WingHex::WingPlugin::DataVisual::ClickedCallBack(),
                  WingHex::WingPlugin::DataVisual::DoubleClickedCallBack()),
        "bool updateTextTree(string &in json)");

    registerAPI<bool(const QString &, const CScriptArray &,
                     const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_DataVisual_updateTextTable, this, stringID,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "bool updateTextTable(string &in json, array<string> &in headers, "
        "array<string> &in headerNames = array<string>())");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installScriptFns(asIScriptEngine *engine) {
    for (auto pfns = _rfns.constKeyValueBegin();
         pfns != _rfns.constKeyValueEnd(); pfns++) {
        auto ns = pfns->first;
        int r = engine->SetDefaultNamespace(ns.toUtf8());
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);

        auto &pfn = pfns->second;
        for (auto p = pfn.constKeyValueBegin(); p != pfn.constKeyValueEnd();
             p++) {
            auto sig = p->first;
            auto id = p->second;
            auto r = engine->RegisterGlobalFunction(
                sig.toUtf8(), asFUNCTION(script_call), asCALL_GENERIC);
            if (r >= 0) {
                // r is the AngelScript function ID
                auto fn = engine->GetFunctionById(r);
                fn->SetUserData(this, AsUserDataType::UserData_API);
                fn->SetUserData(reinterpret_cast<void *>(id),
                                AsUserDataType::UserData_PluginFn);
            } else {
                emit warn(tr("RegisterScriptFnInvalidSig:") + sig);
            }
        }

        engine->SetDefaultNamespace("");
    }
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
        if (type->GetTypeId() == typeId) {
            return reinterpret_cast<QString *>(data)->length() + 1;
        }
    }
    return -1;
}

void WingAngelAPI::qvariantCastOp(
    asIScriptEngine *engine, const QVariant &var,
    const std::function<void(void *, QMetaType::Type)> &fn) {
    static asQWORD buffer;
    buffer = 0;

    auto dicop = [](asIScriptEngine *engine, CScriptDictionary *dic,
                    const QString &key, void *addr, QMetaType::Type type) {
        auto b = isTempBuffered(type);
        if (b) {
            switch (type) {
            case QMetaType::Type::Bool:
                dic->Set(key, addr, asTYPEID_BOOL);
                break;
            case QMetaType::Short:
                dic->Set(key, addr, asTYPEID_INT16);
                break;
            case QMetaType::UShort:
                dic->Set(key, addr, asTYPEID_UINT16);
                break;
            case QMetaType::Int:
            case QMetaType::Long:
                dic->Set(key, addr, asTYPEID_INT32);
                break;
            case QMetaType::UInt:
            case QMetaType::ULong:
                dic->Set(key, addr, asTYPEID_UINT32);
                break;
            case QMetaType::LongLong:
                dic->Set(key, addr, asTYPEID_INT64);
                break;
            case QMetaType::ULongLong:
                dic->Set(key, addr, asTYPEID_UINT64);
                break;
            case QMetaType::Float:
                dic->Set(key, qvariantCastGetValue<float>(addr));
                break;
            case QMetaType::Double:
                dic->Set(key, qvariantCastGetValue<double>(addr));
                break;
            case QMetaType::UChar:
                dic->Set(key, addr, asTYPEID_UINT8);
                break;
            case QMetaType::SChar:
            case QMetaType::Char:
                dic->Set(key, addr, asTYPEID_INT8);
                break;
            default:
                // should not go here
                Q_ASSERT(false);
                break;
            }
        } else {
            auto id = qvariantCastASID(engine, type);
            Q_ASSERT(id >= 0);
            dic->Set(key, addr, id);
        }
    };
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto type =
        var.isNull() ? QMetaType::Type::Void : QMetaType::Type(var.userType());
#else
    auto type =
        var.isNull() ? QMetaType::Type::Void : QMetaType::Type(var.typeId());
#endif
    switch (type) {
    case QMetaType::Type::Bool:
        assignTmpBuffer(buffer, var.toBool());
        fn(&buffer, type);
        break;
    case QMetaType::Short:
    case QMetaType::UShort:
        assignTmpBuffer(buffer, var.value<ushort>());
        fn(&buffer, type);
        break;
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::UInt:
    case QMetaType::ULong:
        assignTmpBuffer(buffer, var.toUInt());
        fn(&buffer, type);
        break;
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
        assignTmpBuffer(buffer, var.toULongLong());
        fn(&buffer, type);
        break;
    case QMetaType::QChar:
        fn(new QChar(var.toChar()), type);
        break;
    case QMetaType::Float:
        assignTmpBuffer(buffer, var.toULongLong());
        fn(&buffer, type);
        break;
    case QMetaType::Double:
        assignTmpBuffer(buffer, var.toDouble());
        fn(&buffer, type);
        break;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    case QMetaType::Char16:
        fn(new QChar(var.value<char16_t>()), type);
        break;
    case QMetaType::Char32:
        fn(new QChar(var.value<char32_t>()), type);
        break;
#endif
    case QMetaType::UChar:
    case QMetaType::SChar:
    case QMetaType::Char:
        assignTmpBuffer(buffer, var.value<uchar>());
        break;
    case QMetaType::QString:
        fn(new QString(var.toString()), type);
        break;
    case QMetaType::QByteArray: {
        auto value = var.toByteArray();
        auto info = engine->GetTypeInfoByDecl("array<byte>");
        Q_ASSERT(info);
        auto len = value.length();
        auto arr = CScriptArray::Create(info, len);
        arr->AddRef();
        for (int i = 0; i < len; ++i) {
            auto addr = arr->At(i);
            *reinterpret_cast<char *>(addr) = value.at(i);
        }
        fn(arr, type);
        arr->Release();
    } break;
    case QMetaType::QVariantMap: {
        auto values = var.value<QVariantMap>();
        auto dic = CScriptDictionary::Create(engine);
        dic->AddRef();
        for (auto p = values.constKeyValueBegin();
             p != values.constKeyValueEnd(); ++p) {
            auto op = std::bind(dicop, engine, dic, p->first,
                                std::placeholders::_1, std::placeholders::_2);
            qvariantCastOp(engine, p->second, op);
        }
        fn(dic, type);
        dic->Release();
    } break;
    case QMetaType::QVariantHash: {
        auto values = var.value<QVariantHash>();
        auto dic = CScriptDictionary::Create(engine);
        dic->AddRef();
        for (auto p = values.constKeyValueBegin();
             p != values.constKeyValueEnd(); ++p) {
            auto op = std::bind(dicop, engine, dic, p->first,
                                std::placeholders::_1, std::placeholders::_2);
            qvariantCastOp(engine, p->second, op);
        }
        fn(dic, type);
        dic->Release();
    } break;
    case QMetaType::QStringList: {
        auto values = var.toStringList();
        auto info = engine->GetTypeInfoByDecl("string");
        Q_ASSERT(info);
        auto len = values.length();
        auto arr = CScriptArray::Create(info, len);
        arr->AddRef();
        for (int i = 0; i < len; ++i) {
            auto addr = arr->At(i);
            *reinterpret_cast<QString *>(addr) = values.at(i);
        }
        fn(arr, type);
        arr->Release();
    } break;
    case QMetaType::QColor:
        fn(new QColor(var.value<QColor>()), type);
        break;
    case QMetaType::Void:
        break;
    default:
        Logger::critical(tr("NotSupportedQMetaType:") + QMetaType(type).name());
        break;
    }
}

QVariant WingAngelAPI::qvariantGet(asIScriptEngine *engine, const void *raw,
                                   int typeID) {
    switch (typeID) {
    case asTYPEID_BOOL:
        return *reinterpret_cast<const bool *>(raw);
    case asTYPEID_INT8:
        return *reinterpret_cast<const qint8 *>(raw);
    case asTYPEID_INT16:
        return *reinterpret_cast<const qint16 *>(raw);
    case asTYPEID_INT32:
        return *reinterpret_cast<const qint32 *>(raw);
    case asTYPEID_INT64:
        return *reinterpret_cast<const qint64 *>(raw);
    case asTYPEID_UINT8:
        return *reinterpret_cast<const quint8 *>(raw);
    case asTYPEID_UINT16:
        return *reinterpret_cast<const quint16 *>(raw);
    case asTYPEID_UINT32:
        return *reinterpret_cast<const quint32 *>(raw);
    case asTYPEID_UINT64:
        return *reinterpret_cast<const quint64 *>(raw);
    case asTYPEID_FLOAT:
        return *reinterpret_cast<const float *>(raw);
    case asTYPEID_DOUBLE:
        return *reinterpret_cast<const double *>(raw);
    default: {
        typeID &= ~asTYPEID_OBJHANDLE;
        auto id = engine->GetTypeIdByDecl("char");
        if (id == typeID) {
            return *reinterpret_cast<const QChar *>(raw);
        }
        id = engine->GetTypeIdByDecl("string");
        if (id == typeID) {
            return *reinterpret_cast<const QString *>(raw);
        }
        id = engine->GetTypeIdByDecl("array<byte>");
        if (id == typeID) {
            auto values = reinterpret_cast<const CScriptArray *>(raw);
            auto len = values->GetSize();
            QByteArray arr;
            arr.resize(len);
            for (auto i = 0u; i < len; ++i) {
                arr[i] = *reinterpret_cast<const char *>(values->At(i));
            }
            return arr;
        }
        id = engine->GetTypeIdByDecl("dictionary");
        if (id == typeID) {
            auto values = reinterpret_cast<const CScriptDictionary *>(raw);

            QVariantHash map;
            for (auto p = values->begin(); p != values->end(); ++p) {
                auto id = p.GetTypeId();
                auto value = qvariantGet(engine, p.GetAddressOfValue(), id);
                map.insert(p.GetKey(), value);
            }
            return map;
        }

        id = engine->GetTypeIdByDecl("array<string>");
        if (id == typeID) {
            auto values = reinterpret_cast<const CScriptArray *>(raw);
            auto len = values->GetSize();
            QStringList arr;
            for (auto i = 0u; i < len; ++i) {
                arr.append(*reinterpret_cast<const QString *>(values->At(i)));
            }
            return arr;
        }

        id = engine->GetTypeIdByDecl("color");
        if (id == typeID) {
            return *reinterpret_cast<const QColor *>(raw);
        }
    } break;
    }
    return {};
}

int WingAngelAPI::qvariantCastASID(asIScriptEngine *engine,
                                   const QMetaType::Type &id) {
    switch (id) {
    case QMetaType::Type::Bool:
        return asTYPEID_BOOL;
    case QMetaType::Short:
        return asTYPEID_INT16;
    case QMetaType::UShort:
        return asTYPEID_UINT16;
    case QMetaType::Int:
    case QMetaType::Long:
        return asTYPEID_INT32;
    case QMetaType::UInt:
    case QMetaType::ULong:
        return asTYPEID_UINT32;
    case QMetaType::LongLong:
        return asTYPEID_INT64;
    case QMetaType::ULongLong:
        return asTYPEID_UINT64;
    case QMetaType::Float:
        return asTYPEID_FLOAT;
    case QMetaType::Double:
        return asTYPEID_DOUBLE;
    case QMetaType::QChar:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    case QMetaType::Char16:
    case QMetaType::Char32:
#endif
        return engine->GetTypeIdByDecl("char");
    case QMetaType::UChar:
        return asTYPEID_UINT8;
    case QMetaType::SChar:
    case QMetaType::Char:
        return asTYPEID_INT8;
    case QMetaType::QString:
        return engine->GetTypeIdByDecl("string");
    case QMetaType::QByteArray:
        return engine->GetTypeIdByDecl("array<byte>");
    case QMetaType::QVariantMap:
    case QMetaType::QVariantHash:
        return engine->GetTypeIdByDecl("dictionary");
    case QMetaType::QStringList:
        return engine->GetTypeIdByDecl("array<string>");
    case QMetaType::QColor:
        return engine->GetTypeIdByDecl("color");
    case QMetaType::Void:
        return asTYPEID_VOID;
    default:
        return -1;
    }
}

bool WingAngelAPI::isTempBuffered(QMetaType::Type type) {
    switch (type) {
    case QMetaType::Bool:
    case QMetaType::Short:
    case QMetaType::UShort:
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::Float:
    case QMetaType::Double:
    case QMetaType::UChar:
    case QMetaType::SChar:
    case QMetaType::Char:
        return true;
    default:
        return false;
    }
}

void WingAngelAPI::script_call(asIScriptGeneric *gen) {
    auto fn = gen->GetFunction();

    auto p = reinterpret_cast<WingAngelAPI *>(
        fn->GetUserData(AsUserDataType::UserData_API));
    auto id = reinterpret_cast<qsizetype>(
        fn->GetUserData(AsUserDataType::UserData_PluginFn));
    auto engine = fn->GetEngine();

    Q_ASSERT(p);
    Q_ASSERT(id >= 0 && id < p->_sfns.size());
    if (id < 0 || id >= p->_sfns.size()) {
        return;
    }

    QVariantList params;
    auto total = gen->GetArgCount();
    for (int i = 0; i < total; ++i) {
        auto raw = gen->GetAddressOfArg(i);
        auto id = gen->GetArgTypeId(i);
        auto obj = qvariantGet(engine, raw, id);
        params.append(obj);
    }

    auto ret = p->_sfns.at(id).fn(params);
    auto op = [](asIScriptGeneric *gen, void *addr, QMetaType::Type type) {
        auto b = isTempBuffered(type);
        if (b) {
            switch (type) {
            case QMetaType::Type::Bool:
                gen->SetReturnByte(qvariantCastGetValue<bool>(addr));
                break;
            case QMetaType::Short:
            case QMetaType::UShort:
                gen->SetReturnWord(qvariantCastGetValue<ushort>(addr));
                break;
            case QMetaType::Int:
            case QMetaType::Long:
            case QMetaType::UInt:
            case QMetaType::ULong:
                gen->SetReturnDWord(qvariantCastGetValue<asDWORD>(addr));
                break;
            case QMetaType::LongLong:
            case QMetaType::ULongLong:
                gen->SetReturnQWord(
                    qvariantCastGetValue<unsigned long long>(addr));
                break;
            case QMetaType::Float:
                gen->SetReturnFloat(qvariantCastGetValue<float>(addr));
                break;
            case QMetaType::Double:
                gen->SetReturnDouble(qvariantCastGetValue<double>(addr));
                break;
            case QMetaType::UChar:
            case QMetaType::SChar:
            case QMetaType::Char:
                gen->SetReturnByte(qvariantCastGetValue<uchar>(addr));
                break;
            default:
                // should not go here
                Q_ASSERT(false);
                break;
            }
        } else {
            gen->SetReturnObject(addr);
        }
    };

    qvariantCastOp(
        engine, ret,
        std::bind(op, gen, std::placeholders::_1, std::placeholders::_2));
}

bool WingAngelAPI::execScriptCode(const QString &code) {
    if (code.isEmpty()) {
        return true;
    }

    if (_console) {
        QTemporaryFile f;
        if (f.open()) {
            f.write(code.toUtf8());
            f.close();
        }

        _console->setMode(ScriptingConsole::Output);
        _console->write(QStringLiteral("(") %
                        property("__LAST_CALLER__").toString() %
                        QStringLiteral(") "));
        _console->machine()->executeScript(f.fileName());
        _console->appendCommandPrompt();
        _console->setMode(ScriptingConsole::Input);
    }

    return false;
}

bool WingAngelAPI::execScript(const QString &fileName) {
    _console->setMode(ScriptingConsole::Output);
    _console->write(QStringLiteral("(") %
                    property("__LAST_CALLER__").toString() %
                    QStringLiteral(") "));
    auto ret = _console->machine()->executeScript(fileName);
    _console->appendCommandPrompt();
    _console->setMode(ScriptingConsole::Input);
    return ret;
}

bool WingAngelAPI::execCode(const QString &code) {
    _console->setMode(ScriptingConsole::Output);
    _console->write(QStringLiteral("(") %
                    property("__LAST_CALLER__").toString() %
                    QStringLiteral(") "));
    auto ret = _console->machine()->executeCode(code);
    _console->appendCommandPrompt();
    _console->setMode(ScriptingConsole::Input);
    return ret;
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

CScriptArray *WingAngelAPI::_HexReader_selectedBytes(qsizetype index) {
    return byteArrayWrapperFunction([this, index]() -> QByteArray {
        return emit reader.selectedBytes(index);
    });
}

CScriptArray *WingAngelAPI::_HexReader_selectionBytes() {
    // context, which can be used to obtain a pointer to the
    // engine.
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();

        auto ret = emit reader.selectionBytes();

        // The script array needs to know its type to properly handle the
        // elements. Note that the object type should be cached to avoid
        // performance issues if the function is called frequently.
        asITypeInfo *arrt = engine->GetTypeInfoByDecl("array<array<byte>>");
        asITypeInfo *t = engine->GetTypeInfoByDecl("array<byte>");
        Q_ASSERT(arrt && t);
        auto array = CScriptArray::Create(arrt, ret.size());
        for (qsizetype i = 0; i < ret.size(); ++i) {
            auto buffer = ret.at(i);
            auto conarr = CScriptArray::Create(t, ret.size());
            for (qsizetype i = 0; i < ret.size(); ++i) {
                auto v = ret.at(buffer.at(i));
                conarr->SetValue(i, &v);
            }
            array->SetValue(i, conarr);
        }
        return array;
    } else {
        return nullptr;
    }
}

CScriptArray *WingAngelAPI::_HexReader_readBytes(qsizetype offset,
                                                 qsizetype len) {
    return byteArrayWrapperFunction([this, offset, len]() -> QByteArray {
        return emit reader.readBytes(offset, len);
    });
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

CScriptArray *WingAngelAPI::_HexReader_getsBookmarkPos(qsizetype line) {
    return retarrayWrapperFunction(
        [this, line]() -> QList<qsizetype> {
            return emit reader.getsBookmarkPos(line);
        },
        "array<" QSIZETYPE ">");
}

CScriptArray *WingAngelAPI::_HexReader_getSupportedEncodings() {
    return retarrayWrapperFunction(
        [this]() -> QStringList { return emit reader.getSupportedEncodings(); },
        "array<string>");
}

CScriptArray *WingAngelAPI::_HexReader_getStorageDrivers() {
    return retarrayWrapperFunction(
        [this]() -> QStringList { return emit reader.getStorageDrivers(); },
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
        return emit visual.updateTextList(ret);
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
            return emit visual.updateTextTable(json, h, hn);
        } else {
            return false;
        }
    } else {
        return false;
    }
}

ScriptingConsole *WingAngelAPI::bindingConsole() const { return _console; }

void WingAngelAPI::setBindingConsole(ScriptingConsole *console) {
    _console = console;
}
