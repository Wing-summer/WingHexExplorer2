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
#include "class/pluginsystem.h"
#include "class/scriptmachine.h"
#include "class/wingfiledialog.h"
#include "class/winginputdialog.h"
#include "control/scriptingconsole.h"
#include "define.h"
#include "scriptaddon/scriptqdictionary.h"

#include <QApplication>
#include <QJsonDocument>
#include <QMessageBox>
#include <QTemporaryFile>

#ifdef Q_OS_WIN
#undef MessageBox
#endif

WingAngelAPI::WingAngelAPI() {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qRegisterMetaType<QVector<QVariant>>();
#endif

    _fnbuffer.reserve(PluginSystem::instance().pluginAPICount());
}

WingAngelAPI::~WingAngelAPI() { _fnbuffer.clear(); }

int WingAngelAPI::sdkVersion() const { return WingHex::SDKVERSION; }

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

const QString WingAngelAPI::pluginComment() const {
    return tr("A internal plugin that provides AngelScript scripts with the "
              "ability to call the host API.");
}

QString WingAngelAPI::retranslate(const QString &str) {
    return QApplication::tr(str.toLatin1());
}

QStringList WingAngelAPI::registerScriptMarcos() const {
    return {"EXEC_BASE", "AS_ARRAY_EXT", "AS_DICTIONARY_EXT"};
}

WingHex::IWingPlugin::RegisteredEvents WingAngelAPI::registeredEvents() const {
    RegisteredEvents evs;
    evs.setFlag(RegisteredEvent::PluginFileOpened);
    evs.setFlag(RegisteredEvent::PluginFileClosed);
    return evs;
}

void WingAngelAPI::eventPluginFile(PluginFileEvent e, FileType type,
                                   const QString &newfileName, int handle,
                                   const QString &oldfileName) {
    Q_UNUSED(type);
    Q_UNUSED(newfileName);
    Q_UNUSED(oldfileName);
    if (e == PluginFileEvent::PluginOpened) {
        _handles.append(handle);
    } else if (e == PluginFileEvent::PluginClosed) {
        _handles.removeOne(handle);
    }
}

void WingAngelAPI::registerUnSafeScriptFns(
    const QString &ns, const QHash<QString, UNSAFE_SCFNPTR> &rfns) {
    Q_ASSERT(!ns.isEmpty());
    if (rfns.isEmpty()) {
        return;
    }

    auto id = _usfns.size();
    for (auto p = rfns.constKeyValueBegin(); p != rfns.constKeyValueEnd();
         p++) {
        _urfns[ns][p->first] = id;
        id++;
        _usfns.append(p->second);
    }
}

void WingAngelAPI::registerScriptEnums(
    const QString &ns, const QHash<QString, QList<QPair<QString, int>>> &objs) {
    Q_ASSERT(!ns.isEmpty());
    if (objs.isEmpty()) {
        return;
    }

    _objs.insert(ns, objs);
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

    installBasicTypes(engine);
    installExtAPI(engine);
    installLogAPI(engine);
    installMsgboxAPI(engine);
    installInputboxAPI(engine);
    installFileDialogAPI(engine);
    installColorDialogAPI(engine);

    installHexReaderAPI(engine);
    installHexControllerAPI(engine);
    installDataVisualAPI(engine);

    installScriptEnums(engine);
    installScriptFns(engine);
    installScriptUnSafeFns(engine);
}

void WingAngelAPI::installBasicTypes(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("msgbox");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    registerAngelType<QMessageBox::StandardButtons>(engine, "buttons");
    registerAngelType<QMessageBox::Icon>(engine, "icon");

    r = engine->SetDefaultNamespace("inputbox");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    registerAngelType<QLineEdit::EchoMode>(engine, "EchoMode");
    registerAngelType<Qt::InputMethodHints>(engine, "InputMethodHints");

    r = engine->SetDefaultNamespace("filedlg");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    registerAngelType<QFileDialog::Options>(engine, "options");

    engine->SetDefaultNamespace("");

    // QModelIndex
    r = engine->RegisterObjectType("ModelIndex", sizeof(QModelIndex),
                                   asOBJ_VALUE | asOBJ_POD |
                                       asGetTypeTraits<QModelIndex>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("ModelIndex", "ModelIndex parent() const",
                                     asMETHOD(QModelIndex, parent),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("ModelIndex", "int row() const",
                                     asMETHOD(QModelIndex, row),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("ModelIndex", "int column() const",
                                     asMETHOD(QModelIndex, column),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // extension function for data, we dont want to register QVariant to it
    r = engine->RegisterObjectMethod("ModelIndex", "string dataString() const",
                                     asFUNCTION(QModelIndex_dataString),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("ModelIndex", "char dataChar() const",
                                     asFUNCTION(QModelIndex_dataChar),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("ModelIndex", "uint dataUInt() const",
                                     asFUNCTION(QModelIndex_dataUInt),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("ModelIndex", "int dataInt() const",
                                     asFUNCTION(QModelIndex_dataInt),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("ModelIndex", "int64 dataLongLong() const",
                                     asFUNCTION(QModelIndex_dataLongLong),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "ModelIndex", "uint64 dataULongLong() const",
        asFUNCTION(QModelIndex_dataULongLong), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("ModelIndex", "float dataFloat() const",
                                     asFUNCTION(QModelIndex_dataFloat),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("ModelIndex", "double dataDouble() const",
                                     asFUNCTION(QModelIndex_dataDouble),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterFuncdef(
        "void ClickCallBack(const ModelIndex &in index)");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    installHexBaseType(engine);
}

void WingAngelAPI::installLogAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("log");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    registerAPI<void(const QString &)>(
        engine, std::bind(&WingAngelAPI::logInfo, this, std::placeholders::_1),
        "void info(const string &in message)");

    registerAPI<void(const QString &)>(
        engine, std::bind(&WingAngelAPI::logTrace, this, std::placeholders::_1),
        "void trace(const string &in message)");

    registerAPI<void(const QString &)>(
        engine, std::bind(&WingAngelAPI::logDebug, this, std::placeholders::_1),
        "void debug(const string &in message)");

    registerAPI<void(const QString &)>(
        engine, std::bind(&WingAngelAPI::logWarn, this, std::placeholders::_1),
        "void warn(const string &in message)");

    registerAPI<void(const QString &)>(
        engine, std::bind(&WingAngelAPI::logError, this, std::placeholders::_1),
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

    registerAPI<void(const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::msgAboutQt, this, nullptr,
                  std::placeholders::_1),
        "void aboutQt(const string &in title =\"\")");

    registerAPI<QMessageBox::StandardButton(const QString &, const QString &,
                                            QMessageBox::StandardButtons,
                                            QMessageBox::StandardButton)>(
        engine,
        std::bind(&WingHex::IWingPlugin::msgInformation, this, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "void information(const string &in title, const string &in text, "
        "msgbox::buttons buttons = msgbox::buttons::Ok, "
        "msgbox::buttons defaultButton = msgbox::buttons::NoButton)");

    registerAPI<QMessageBox::StandardButton(const QString &, const QString &,
                                            QMessageBox::StandardButtons,
                                            QMessageBox::StandardButton)>(
        engine,
        std::bind(&WingHex::IWingPlugin::msgQuestion, this, nullptr,
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
        std::bind(&WingHex::IWingPlugin::msgWarning, this, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "void warning(const string &in title, const string &in text, "
        "msgbox::buttons buttons = msgbox::buttons::Ok, "
        "msgbox::buttons defaultButton = msgbox::buttons::NoButton)");

    registerAPI<QMessageBox::StandardButton(const QString &, const QString &,
                                            QMessageBox::StandardButtons,
                                            QMessageBox::StandardButton)>(
        engine,
        std::bind(&WingHex::IWingPlugin::critical, this, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "void critical(const string &in title, const string &in text, "
        "msgbox::buttons buttons = msgbox::buttons::Ok, "
        "msgbox::buttons defaultButton = msgbox::buttons::NoButton)");

    registerAPI<QMessageBox::StandardButton(
        QMessageBox::Icon, const QString &, const QString &,
        QMessageBox::StandardButtons, QMessageBox::StandardButton)>(
        engine,
        std::bind(&WingHex::IWingPlugin::msgbox, this, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5),
        "void msgbox(msgbox::icon icon, const string &in title, "
        "const string &in text, "
        "msgbox::buttons buttons = msgbox::buttons::NoButton, "
        "msgbox::buttons defaultButton = msgbox::buttons::NoButton)");

    registerAPI<void(const QString &, const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::msgAbout, this, nullptr,
                  std::placeholders::_1, std::placeholders::_2),
        "void about(const string &in title, const string &in text)");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installInputboxAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("inputbox");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    registerAPI<QString(const QString &, const QString &, QLineEdit::EchoMode,
                        const QString &, bool *,
                        Qt::InputMethodHints inputMethodHints)>(
        engine,
        std::bind(&WingHex::IWingPlugin::dlgGetText, this, nullptr,
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
        std::bind(&WingHex::IWingPlugin::dlgGetMultiLineText, this, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5),
        "string getMultiLineText(const string &in title, "
        "const string &in label, "
        "const string &in text = \"\", bool &out ok = false, "
        "inputbox::InputMethodHints inputMethodHints = inputbox::ImhNone)");

    registerAPI<int(const QString &, const QString &, int, int, int, int,
                    bool *)>(
        engine,
        std::bind(&WingHex::IWingPlugin::dlgGetInt, this, nullptr,
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
        std::bind(&WingHex::IWingPlugin::dlgGetDouble, this, nullptr,
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
        std::bind(&WingAngelAPI::_InputBox_getItem, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3,
                  std::placeholders::_4, std::placeholders::_5,
                  std::placeholders::_6, std::placeholders::_7),
        "string getItem(const string &in title, const string &in label, "
        "const string[] &in items, int current = 0, "
        "bool editable = true, bool &out ok = false, "
        "inputbox::InputMethodHints inputMethodHints = inputbox::ImhNone)");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installFileDialogAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("filedlg");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    registerAPI<QString(const QString &, const QString &,
                        QFileDialog::Options)>(
        engine,
        std::bind(&WingHex::IWingPlugin::dlgGetExistingDirectory, this, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "string getExistingDirectory(const string &in caption = \"\", "
        "const string &in dir = \"\", "
        "filedlg::options &in options = filedlg::options::ShowDirsOnly)");

    registerAPI<QString(const QString &, const QString &, const QString &,
                        QString *, QFileDialog::Options)>(
        engine,
        std::bind(&WingHex::IWingPlugin::dlgGetOpenFileName, this, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5),
        "string getOpenFileName(const string &in caption = \"\", "
        "const string &in dir = \"\", const string &in filter = \"\", "
        "string &out selectedFilter = \"\", filedlg::options &in options = 0)");

    registerAPI<QString(const QString &, const QString &, const QString &,
                        QString *, QFileDialog::Options)>(
        engine,
        std::bind(&WingHex::IWingPlugin::dlgGetSaveFileName, this, nullptr,
                  std::placeholders::_1, std::placeholders::_2,
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
        "string[]@ getOpenFileNames(const string &in caption = \"\", "
        "const string &in dir = \"\", const string &in filter = \"\", "
        "string &out selectedFilter = \"\", filedlg::options &in options = 0)");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installColorDialogAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("colordlg");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    registerAPI<QColor(const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::dlgGetColor, this,
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

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::IWingPlugin::isCurrentDocEditing, this),
        "bool isCurrentDocEditing()");

    registerAPI<QString(void)>(
        engine, std::bind(&WingHex::IWingPlugin::currentDocFilename, this),
        "string currentDocFilename()");

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::IWingPlugin::isInsertionMode, this),
        "bool isInsertionMode()");

    registerAPI<bool(void)>(engine,
                            std::bind(&WingHex::IWingPlugin::isReadOnly, this),
                            "bool isReadOnly()");

    registerAPI<bool(void)>(engine,
                            std::bind(&WingHex::IWingPlugin::isKeepSize, this),
                            "bool isKeepSize()");

    registerAPI<bool(void)>(engine,
                            std::bind(&WingHex::IWingPlugin::isLocked, this),
                            "bool isLocked()");

    registerAPI<WingHex::HexPosition(void)>(
        engine, std::bind(&WingHex::IWingPlugin::currentPos, this),
        "HexPosition currentPos()");

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::IWingPlugin::stringVisible, this),
        "bool stringVisible()");

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::IWingPlugin::addressVisible, this),
        "bool addressVisible()");

    registerAPI<bool(void)>(
        engine, std::bind(&WingHex::IWingPlugin::headerVisible, this),
        "bool headerVisible()");

    registerAPI<bool(void)>(engine,
                            std::bind(&WingHex::IWingPlugin::isModified, this),
                            "bool isModified()");

    registerAPI<qsizetype(void)>(
        engine, std::bind(&WingHex::IWingPlugin::documentLines, this),
        QSIZETYPE_WRAP("documentLines()"));

    registerAPI<qsizetype(void)>(
        engine, std::bind(&WingHex::IWingPlugin::documentBytes, this),
        QSIZETYPE_WRAP("documentBytes()"));

    registerAPI<qsizetype(void)>(
        engine, std::bind(&WingHex::IWingPlugin::currentRow, this),
        QSIZETYPE_WRAP("currentRow()"));

    registerAPI<qsizetype(void)>(
        engine, std::bind(&WingHex::IWingPlugin::currentColumn, this),
        QSIZETYPE_WRAP("currentColumn()"));

    registerAPI<qsizetype(void)>(
        engine, std::bind(&WingHex::IWingPlugin::currentOffset, this),
        QSIZETYPE_WRAP("currentOffset()"));

    registerAPI<qsizetype(void)>(
        engine, std::bind(&WingHex::IWingPlugin::selectedLength, this),
        QSIZETYPE_WRAP("selectedLength()"));

    registerAPI<CScriptArray *(qsizetype)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_selectedBytes, this,
                  std::placeholders::_1),
        "byte[]@ selectedBytes(" QSIZETYPE " index)");

    registerAPI<CScriptArray *()>(
        engine, std::bind(&WingAngelAPI::_HexReader_selectionBytes, this),
        "byte[][]@ selectionBytes()");
    registerAPI<WingHex::HexPosition(qsizetype)>(
        engine,
        std::bind(&WingHex::IWingPlugin::selectionStart, this,
                  std::placeholders::_1),
        "HexPosition selectionStart(" QSIZETYPE " index)");
    registerAPI<WingHex::HexPosition(qsizetype)>(
        engine,
        std::bind(&WingHex::IWingPlugin::selectionEnd, this,
                  std::placeholders::_1),
        "HexPosition selectionEnd(" QSIZETYPE " index)");
    registerAPI<qsizetype(qsizetype)>(
        engine,
        std::bind(&WingHex::IWingPlugin::selectionLength, this,
                  std::placeholders::_1),
        QSIZETYPE_WRAP("selectionLength(" QSIZETYPE " index)"));

    registerAPI<qsizetype()>(
        engine, std::bind(&WingHex::IWingPlugin::selectionCount, this),
        QSIZETYPE_WRAP("selectionCount()"));

    registerAPI<quintptr(void)>(
        engine, std::bind(&WingHex::IWingPlugin::addressBase, this),
        QPTR_WRAP("addressBase()"));

    registerAPI<CScriptArray *(qsizetype, qsizetype)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_readBytes, this,
                  std::placeholders::_1, std::placeholders::_2),
        "byte[]@ readBytes(" QSIZETYPE " offset," QSIZETYPE " len)");

    registerAPI<qint8(qsizetype)>(
        engine,
        std::bind(&WingHex::IWingPlugin::readInt8, this, std::placeholders::_1),
        "int8 readInt8(" QSIZETYPE " offset)");

    registerAPI<quint8(qsizetype)>(engine,
                                   std::bind(&WingHex::IWingPlugin::readUInt8,
                                             this, std::placeholders::_1),
                                   "uint8 readUInt8(" QSIZETYPE " offset)");

    registerAPI<qint16(qsizetype)>(engine,
                                   std::bind(&WingHex::IWingPlugin::readInt16,
                                             this, std::placeholders::_1),
                                   "int16 readInt16(" QSIZETYPE " offset)");

    registerAPI<quint16(qsizetype)>(engine,
                                    std::bind(&WingHex::IWingPlugin::readUInt16,
                                              this, std::placeholders::_1),
                                    "uint16 readUInt16(" QSIZETYPE " offset)");

    registerAPI<qint32(qsizetype)>(engine,
                                   std::bind(&WingHex::IWingPlugin::readInt32,
                                             this, std::placeholders::_1),
                                   "int readInt32(" QSIZETYPE " offset)");

    registerAPI<quint32(qsizetype)>(engine,
                                    std::bind(&WingHex::IWingPlugin::readUInt32,
                                              this, std::placeholders::_1),
                                    "uint readUInt32(" QSIZETYPE " offset)");

    registerAPI<qint64(qsizetype)>(engine,
                                   std::bind(&WingHex::IWingPlugin::readInt64,
                                             this, std::placeholders::_1),
                                   "int64 readInt64(" QSIZETYPE " offset)");

    registerAPI<quint64(qsizetype)>(engine,
                                    std::bind(&WingHex::IWingPlugin::readUInt64,
                                              this, std::placeholders::_1),
                                    "uint64 readUInt64(" QSIZETYPE " offset)");

    registerAPI<float(qsizetype)>(engine,
                                  std::bind(&WingHex::IWingPlugin::readFloat,
                                            this, std::placeholders::_1),
                                  "float readFloat(" QSIZETYPE " offset)");

    registerAPI<double(qsizetype)>(engine,
                                   std::bind(&WingHex::IWingPlugin::readDouble,
                                             this, std::placeholders::_1),
                                   "double readDouble(" QSIZETYPE " offset)");

    registerAPI<QString(qsizetype, QString)>(
        engine,
        std::bind(&WingHex::IWingPlugin::readString, this,
                  std::placeholders::_1, std::placeholders::_2),
        "string readInt64(" QSIZETYPE " offset, string &in encoding = \"\")");

    registerAPI<qsizetype(qsizetype, const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_findNext, this,
                  std::placeholders::_1, std::placeholders::_2),
        QSIZETYPE_WRAP("findNext(" QSIZETYPE " begin, byte[] &in ba)"));

    registerAPI<qsizetype(qsizetype, const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexReader_findPrevious, this,
                  std::placeholders::_1, std::placeholders::_2),
        QSIZETYPE_WRAP("findPrevious(" QSIZETYPE " begin, byte[] &in ba)"));

    registerAPI<QString(qsizetype)>(
        engine,
        std::bind(&WingHex::IWingPlugin::bookMarkComment, this,
                  std::placeholders::_1),
        "string bookMarkComment(" QSIZETYPE " pos)");

    registerAPI<bool(qsizetype)>(engine,
                                 std::bind(&WingHex::IWingPlugin::existBookMark,
                                           this, std::placeholders::_1),
                                 "bool existBookMark(" QSIZETYPE " pos)");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installHexControllerAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("ctl");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    registerAPI<bool(int)>(engine,
                           std::bind(&WingHex::IWingPlugin::switchDocument,
                                     this, std::placeholders::_1),
                           "bool switchDocument(int handle)");

    registerAPI<bool(bool)>(engine,
                            std::bind(&WingHex::IWingPlugin::setLockedFile,
                                      this, std::placeholders::_1),
                            "bool setLockedFile(bool b)");

    registerAPI<bool(bool)>(engine,
                            std::bind(&WingHex::IWingPlugin::setKeepSize, this,
                                      std::placeholders::_1),
                            "bool setKeepSize(bool b)");

    registerAPI<bool(bool)>(engine,
                            std::bind(&WingHex::IWingPlugin::setStringVisible,
                                      this, std::placeholders::_1),
                            "bool setStringVisible(bool b)");

    registerAPI<bool(bool)>(engine,
                            std::bind(&WingHex::IWingPlugin::setAddressVisible,
                                      this, std::placeholders::_1),
                            "bool setAddressVisible(bool b)");

    registerAPI<bool(bool)>(engine,
                            std::bind(&WingHex::IWingPlugin::setHeaderVisible,
                                      this, std::placeholders::_1),
                            "bool setHeaderVisible(bool b)");

    registerAPI<bool(quintptr)>(engine,
                                std::bind(&WingHex::IWingPlugin::setAddressBase,
                                          this, std::placeholders::_1),
                                "bool setAddressBase(" QPTR " base)");

    registerAPI<bool(const QString)>(
        engine,
        std::bind(&WingHex::IWingPlugin::beginMarco, this,
                  std::placeholders::_1),
        "bool beginMarco(string &in name = \"\")");

    registerAPI<bool()>(engine,
                        std::bind(&WingHex::IWingPlugin::endMarco, this),
                        "bool endMarco()");

    registerAPI<bool(qsizetype, qint8)>(
        engine,
        std::bind(&WingHex::IWingPlugin::writeInt8, this, std::placeholders::_1,
                  std::placeholders::_2),
        "bool writeInt8(" QSIZETYPE " offset, int8 value)");

    registerAPI<bool(qsizetype, quint8)>(
        engine,
        std::bind(&WingHex::IWingPlugin::writeUInt8, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeUInt8(" QSIZETYPE " offset, uint8 value)");

    registerAPI<bool(qsizetype, qint16)>(
        engine,
        std::bind(&WingHex::IWingPlugin::writeInt16, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeInt16(" QSIZETYPE " offset, int16 value)");

    registerAPI<bool(qsizetype, quint16)>(
        engine,
        std::bind(&WingHex::IWingPlugin::writeUInt16, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeUInt16(" QSIZETYPE " offset, uint16 value)");

    registerAPI<bool(qsizetype, qint32)>(
        engine,
        std::bind(&WingHex::IWingPlugin::writeInt32, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeInt32(" QSIZETYPE " offset, int value)");

    registerAPI<bool(qsizetype, quint32)>(
        engine,
        std::bind(&WingHex::IWingPlugin::writeUInt32, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeUInt32(" QSIZETYPE " offset, uint value)");

    registerAPI<bool(qsizetype, qint64)>(
        engine,
        std::bind(&WingHex::IWingPlugin::writeInt64, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeInt64(" QSIZETYPE " offset, int64 value)");

    registerAPI<bool(qsizetype, quint64)>(
        engine,
        std::bind(&WingHex::IWingPlugin::writeUInt64, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeUInt64(" QSIZETYPE " offset, uint64 value)");

    registerAPI<bool(qsizetype, float)>(
        engine,
        std::bind(&WingHex::IWingPlugin::writeFloat, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeFloat(" QSIZETYPE " offset, float value)");

    registerAPI<bool(qsizetype, double)>(
        engine,
        std::bind(&WingHex::IWingPlugin::writeDouble, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeDouble(" QSIZETYPE " offset, double value)");

    registerAPI<bool(qsizetype, const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexController_writeBytes, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool writeBytes(" QSIZETYPE " offset, byte[] &in data)");

    registerAPI<bool(qsizetype, const QString &, const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::writeString, this,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "bool writeString(" QSIZETYPE " offset, string &in value, "
        "string &in encoding = \"\")");

    registerAPI<bool(qsizetype, qint8)>(
        engine,
        std::bind(&WingHex::IWingPlugin::insertInt8, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertInt8(" QSIZETYPE " offset, int8 value)");

    registerAPI<bool(qsizetype, quint8)>(
        engine,
        std::bind(&WingHex::IWingPlugin::insertUInt8, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertUInt8(" QSIZETYPE " offset, uint8 value)");

    registerAPI<bool(qsizetype, qint16)>(
        engine,
        std::bind(&WingHex::IWingPlugin::insertInt16, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertInt16(" QSIZETYPE " offset, int16 value)");

    registerAPI<bool(qsizetype, quint16)>(
        engine,
        std::bind(&WingHex::IWingPlugin::insertUInt16, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertUInt16(" QSIZETYPE " offset, uint16 value)");

    registerAPI<bool(qsizetype, qint32)>(
        engine,
        std::bind(&WingHex::IWingPlugin::insertInt32, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertInt32(" QSIZETYPE " offset, int value)");

    registerAPI<bool(qsizetype, quint32)>(
        engine,
        std::bind(&WingHex::IWingPlugin::insertUInt32, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertUInt32(" QSIZETYPE " offset, uint value)");

    registerAPI<bool(qsizetype, qint64)>(
        engine,
        std::bind(&WingHex::IWingPlugin::insertInt64, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertInt64(" QSIZETYPE " offset, int64 value)");

    registerAPI<bool(qsizetype, quint64)>(
        engine,
        std::bind(&WingHex::IWingPlugin::insertUInt64, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertUInt64(" QSIZETYPE " offset, uint64 value)");

    registerAPI<bool(qsizetype, float)>(
        engine,
        std::bind(&WingHex::IWingPlugin::insertFloat, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertFloat(" QSIZETYPE " offset, float value)");

    registerAPI<bool(qsizetype, double)>(
        engine,
        std::bind(&WingHex::IWingPlugin::insertDouble, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertDouble(" QSIZETYPE " offset, double value)");

    registerAPI<bool(qsizetype, const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexController_insertBytes, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool insertBytes(" QSIZETYPE " offset, byte[] &in data)");

    registerAPI<bool(qsizetype, const QString &, const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::insertString, this,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "bool insertString(" QSIZETYPE " offset, string &in value, "
        "string &in encoding = \"\")");

    registerAPI<bool(qint8)>(engine,
                             std::bind(&WingHex::IWingPlugin::appendInt8, this,
                                       std::placeholders::_1),
                             "bool appendInt8(int8 value)");

    registerAPI<bool(quint8)>(engine,
                              std::bind(&WingHex::IWingPlugin::appendUInt8,
                                        this, std::placeholders::_1),
                              "bool appendUInt8(uint8 value)");

    registerAPI<bool(qint16)>(engine,
                              std::bind(&WingHex::IWingPlugin::appendInt16,
                                        this, std::placeholders::_1),
                              "bool appendInt16(int16 value)");

    registerAPI<bool(quint16)>(engine,
                               std::bind(&WingHex::IWingPlugin::appendUInt16,
                                         this, std::placeholders::_1),
                               "bool appendUInt16(uint16 value)");

    registerAPI<bool(qint32)>(engine,
                              std::bind(&WingHex::IWingPlugin::appendInt32,
                                        this, std::placeholders::_1),
                              "bool appendInt32(int value)");

    registerAPI<bool(quint32)>(engine,
                               std::bind(&WingHex::IWingPlugin::appendUInt32,
                                         this, std::placeholders::_1),
                               "bool appendUInt32(uint value)");

    registerAPI<bool(qint64)>(engine,
                              std::bind(&WingHex::IWingPlugin::appendInt64,
                                        this, std::placeholders::_1),
                              "bool appendInt64(int64 value)");

    registerAPI<bool(quint64)>(engine,
                               std::bind(&WingHex::IWingPlugin::appendUInt64,
                                         this, std::placeholders::_1),
                               "bool appendUInt64(uint64 value)");

    registerAPI<bool(float)>(engine,
                             std::bind(&WingHex::IWingPlugin::appendFloat, this,
                                       std::placeholders::_1),
                             "bool appendFloat(float value)");

    registerAPI<bool(double)>(engine,
                              std::bind(&WingHex::IWingPlugin::appendDouble,
                                        this, std::placeholders::_1),
                              "bool appendDouble(double value)");

    registerAPI<bool(const CScriptArray &)>(
        engine,
        std::bind(&WingAngelAPI::_HexController_appendBytes, this,
                  std::placeholders::_1),
        "bool appendBytes(byte[] &in data)");

    registerAPI<bool(const QString &, const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::appendString, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool appendString(string &in value, string &in encoding = \"\")");

    registerAPI<bool(qsizetype, qsizetype)>(
        engine,
        std::bind(&WingHex::IWingPlugin::removeBytes, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool removeBytes(" QSIZETYPE " offset, " QSIZETYPE " len)");

    registerAPI<bool(qsizetype, qsizetype, int, bool)>(
        engine,
        std::bind(QOverload<qsizetype, qsizetype, int, bool>::of(
                      &WingHex::IWingPlugin::moveTo),
                  this, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "bool moveTo(" QSIZETYPE " line, " QSIZETYPE
        " column, int nibbleindex = -1, bool clearSelection = true)");

    registerAPI<bool(qsizetype, bool)>(
        engine,
        std::bind(QOverload<qsizetype, bool>::of(&WingHex::IWingPlugin::moveTo),
                  this, std::placeholders::_1, std::placeholders::_2),
        "bool moveTo(" QSIZETYPE " offset, bool clearSelection = true)");

    registerAPI<bool(qsizetype, qsizetype, WingHex::SelectionMode)>(
        engine,
        std::bind(&WingHex::IWingPlugin::select, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3),
        "bool select(" QSIZETYPE " offset, " QSIZETYPE
        " len, SelectionMode mode = SelectionMode::Add)");

    registerAPI<bool(bool)>(engine,
                            std::bind(&WingHex::IWingPlugin::setInsertionMode,
                                      this, std::placeholders::_1),
                            "bool setInsertionMode(bool b)");

    registerAPI<bool(qsizetype, qsizetype, const QColor &, const QColor &,
                     const QString &)>(
        engine,
        std::bind(
            QOverload<qsizetype, qsizetype, const QColor &, const QColor &,
                      const QString &>::of(&WingHex::IWingPlugin::metadata),
            this, std::placeholders::_1, std::placeholders::_2,
            std::placeholders::_3, std::placeholders::_4,
            std::placeholders::_5),
        "bool metadata(" QSIZETYPE " begin, " QSIZETYPE
        " length, color &in fgcolor, color &in bgcolor, string &in comment)");

    registerAPI<bool(qsizetype)>(
        engine,
        std::bind(&WingHex::IWingPlugin::removeMetadata, this,
                  std::placeholders::_1),
        "bool removeMetadata(" QSIZETYPE " offset)");

    registerAPI<bool()>(engine,
                        std::bind(&WingHex::IWingPlugin::clearMetadata, this),
                        "bool clearMetadata()");

    registerAPI<bool(qsizetype, qsizetype, const QColor &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::foreground, this,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "bool foreground(" QSIZETYPE " begin, " QSIZETYPE
        " length, color &in fgcolor)");

    registerAPI<bool(qsizetype, qsizetype, const QColor &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::background, this,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3),
        "bool background(" QSIZETYPE " begin, " QSIZETYPE
        " length, color &in bgcolor)");

    registerAPI<bool(qsizetype, qsizetype, const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::comment, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3),
        "bool comment(" QSIZETYPE " begin, " QSIZETYPE
        " length, string &in comment)");

    registerAPI<bool(bool)>(engine,
                            std::bind(&WingHex::IWingPlugin::setMetaVisible,
                                      this, std::placeholders::_1),
                            "bool setMetaVisible(bool b)");

    registerAPI<bool(bool)>(engine,
                            std::bind(&WingHex::IWingPlugin::setMetafgVisible,
                                      this, std::placeholders::_1),
                            "bool setMetafgVisible(bool b)");

    registerAPI<bool(bool)>(engine,
                            std::bind(&WingHex::IWingPlugin::setMetabgVisible,
                                      this, std::placeholders::_1),
                            "bool setMetabgVisible(bool b)");

    registerAPI<bool(bool)>(
        engine,
        std::bind(&WingHex::IWingPlugin::setMetaCommentVisible, this,
                  std::placeholders::_1),
        "bool setMetaCommentVisible(bool b)");

    registerAPI<WingHex::ErrFile()>(
        engine, std::bind(&WingHex::IWingPlugin::newFile, this),
        "ErrFile newFile()");

    registerAPI<WingHex::ErrFile(const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::openFile, this, std::placeholders::_1),
        "ErrFile openFile(string &in filename)");

    registerAPI<WingHex::ErrFile(const QString &, const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::openExtFile, this,
                  std::placeholders::_1, std::placeholders::_2),
        "ErrFile openExtFile(string &in ext, string &in file)");

    registerAPI<WingHex::ErrFile(int, bool)>(
        engine,
        std::bind(&WingHex::IWingPlugin::closeFile, this, std::placeholders::_1,
                  std::placeholders::_2),
        "ErrFile closeFile(int handle, bool force = false)");

    registerAPI<WingHex::ErrFile(int)>(
        engine,
        std::bind(&WingHex::IWingPlugin::closeHandle, this,
                  std::placeholders::_1),
        "ErrFile closeHandle(int handle)");

    registerAPI<WingHex::ErrFile(int)>(
        engine,
        std::bind(&WingHex::IWingPlugin::saveFile, this, std::placeholders::_1),
        "ErrFile saveFile(int handle)");

    registerAPI<WingHex::ErrFile(int, const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::exportFile, this,
                  std::placeholders::_1, std::placeholders::_2),
        "ErrFile exportFile(int handle, string &in savename)");

    registerAPI<WingHex::ErrFile(int, const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::saveAsFile, this,
                  std::placeholders::_1, std::placeholders::_2),
        "ErrFile saveAsFile(int handle, string &in savename)");

    registerAPI<WingHex::ErrFile()>(
        engine, std::bind(&WingHex::IWingPlugin::openCurrent, this),
        "ErrFile openCurrent()");

    registerAPI<WingHex::ErrFile(bool)>(
        engine,
        std::bind(&WingHex::IWingPlugin::closeCurrent, this,
                  std::placeholders::_1),
        "ErrFile closeCurrent(bool force = false)");

    registerAPI<WingHex::ErrFile()>(
        engine, std::bind(&WingHex::IWingPlugin::saveCurrent, this),
        "ErrFile saveCurrent()");

    registerAPI<WingHex::ErrFile(const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::saveAsCurrent, this,
                  std::placeholders::_1),
        "ErrFile saveAsCurrent(string &in savename)");

    registerAPI<WingHex::ErrFile(const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::exportCurrent, this,
                  std::placeholders::_1),
        "ErrFile exportCurrent(string &in savename)");

    registerAPI<bool(qsizetype, const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::addBookMark, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool addBookMark(" QSIZETYPE " pos, string &in comment)");

    registerAPI<bool(qsizetype, const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::modBookMark, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool modBookMark(" QSIZETYPE " pos, string &in comment)");

    registerAPI<bool(qsizetype)>(
        engine,
        std::bind(&WingHex::IWingPlugin::removeBookMark, this,
                  std::placeholders::_1),
        "bool removeBookMark(" QSIZETYPE " pos)");

    registerAPI<bool()>(engine,
                        std::bind(&WingHex::IWingPlugin::clearBookMark, this),
                        "bool clearBookMark()");

    registerAPI<WingHex::ErrFile(const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::openWorkSpace, this,
                  std::placeholders::_1),
        "ErrFile openWorkSpace(string &in filename)");

    registerAPI<void()>(engine,
                        std::bind(&WingHex::IWingPlugin::closeAllFiles, this),
                        "bool closeAll()");

    engine->SetDefaultNamespace("");
}

void WingAngelAPI::installDataVisualAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("visual");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    registerAPI<bool(const QString &, const QString &)>(
        engine,
        std::bind(&WingHex::IWingPlugin::dataVisualText, this,
                  std::placeholders::_1, std::placeholders::_2),
        "bool updateText(string &in data, string &in title=\"\")");

    registerAPI<bool(const CScriptArray &, const QString &, asIScriptFunction *,
                     asIScriptFunction *)>(
        engine,
        std::bind(&WingAngelAPI::_DataVisual_updateTextList, this,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "bool updateTextList(string[] &in data, string &in title=\"\", "
        "ClickCallBack @clickfn = null, ClickCallBack @dblclick = null)");

    registerAPI<bool(const QString &, const QString &, asIScriptFunction *,
                     asIScriptFunction *)>(
        engine,
        std::bind(&WingAngelAPI::_DataVisual_updateTextTree, this,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4),
        "bool updateTextTree(string &in json, string &in title=\"\", "
        "ClickCallBack @clickfn = null, ClickCallBack @dblclick = null)");

    registerAPI<bool(const QString &, const CScriptArray &,
                     const CScriptArray &, const QString &, asIScriptFunction *,
                     asIScriptFunction *)>(
        engine,
        std::bind(&WingAngelAPI::_DataVisual_updateTextTable, this,
                  std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5, std::placeholders::_6),
        "bool updateTextTable(string &in json, string[] &in headers, "
        "string[] &in headerNames = {}, string &in title=\"\", "
        "ClickCallBack @clickfn = null, ClickCallBack @dblclick = null)");

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
            Q_ASSERT(r >= 0);
            Q_UNUSED(r);

            // r is the AngelScript function ID
            auto fn = engine->GetFunctionById(r);
            fn->SetUserData(this, AsUserDataType::UserData_API);
            fn->SetUserData(reinterpret_cast<void *>(id),
                            AsUserDataType::UserData_PluginFn);
        }
        engine->SetDefaultNamespace("");
    }
}

void WingAngelAPI::installScriptUnSafeFns(asIScriptEngine *engine) {
    for (auto pfns = _urfns.constKeyValueBegin();
         pfns != _urfns.constKeyValueEnd(); pfns++) {
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
                sig.toUtf8(), asFUNCTION(script_unsafe_call), asCALL_GENERIC);
            Q_ASSERT(r >= 0);
            Q_UNUSED(r);

            // r is the AngelScript function ID
            auto fn = engine->GetFunctionById(r);
            fn->SetUserData(this, AsUserDataType::UserData_API);
            fn->SetUserData(reinterpret_cast<void *>(id),
                            AsUserDataType::UserData_PluginFn);
        }
        engine->SetDefaultNamespace("");
    }
}

void WingAngelAPI::installScriptEnums(asIScriptEngine *engine) {
    for (auto pobjs = _objs.constKeyValueBegin();
         pobjs != _objs.constKeyValueEnd(); ++pobjs) {
        auto ns = pobjs->first;
        int r = engine->SetDefaultNamespace(ns.toUtf8());
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);

        auto &pobj = pobjs->second;
        for (auto p = pobj.constKeyValueBegin(); p != pobj.constKeyValueEnd();
             p++) {
            auto en = p->first.toUtf8();
            r = engine->RegisterEnum(en.data());
            Q_ASSERT(r >= 0);
            Q_UNUSED(r);

            for (auto &e : p->second) {
                auto ev = e.first.toUtf8();
                r = engine->RegisterEnumValue(en.data(), ev.data(), e.second);
                Q_ASSERT(r >= 0);
                Q_UNUSED(r);
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
                // ignore
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
                                   int typeID, bool flag) {
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
        bool isHandle = !!(typeID & asTYPEID_OBJHANDLE);
        typeID &= ~asTYPEID_OBJHANDLE;

        auto id = engine->GetTypeIdByDecl("char");
        if (id == typeID) {
            return **getDereferencePointer<QChar *>(raw, isHandle);
        }

        id = engine->GetTypeIdByDecl("color");
        if (id == typeID) {
            return **getDereferencePointer<QColor *>(raw, isHandle);
        }

        id = engine->GetTypeIdByDecl("string");
        if (id == typeID) {
            return *getDereferencePointer<QString>(raw, isHandle);
        }

        id = engine->GetTypeIdByDecl("array<byte>");
        if (id == typeID) {
            auto values = *getDereferencePointer<CScriptArray *>(raw, isHandle);
            auto len = values->GetSize();
            QByteArray arr;
            arr.resize(len);
            for (auto i = 0u; i < len; ++i) {
                arr[i] = *reinterpret_cast<const char *>(values->At(i));
            }
            return arr;
        }

        // ok, checking template type
        // no nested supported
        auto info = engine->GetTypeInfoById(typeID);

        {
            auto tname = info->GetName();
            if (qstrcmp(tname, "array") == 0) {
                auto values =
                    *getDereferencePointer<CScriptArray *>(raw, isHandle);
                auto len = values->GetSize();

                // QVector or QList ?
                // though are the same on QT6
                if (flag) {
                    QVariantList list;
                    for (asUINT i = 0; i < len; ++i) {
                        auto v = values->At(i);
                        auto item = qvariantGet(
                            engine, v, values->GetElementTypeId(), false);
                        list.append(item);
                    }
                    return list;
                } else {
                    QVector<QVariant> vector;
                    for (asUINT i = 0; i < len; ++i) {
                        auto v = values->At(i);
                        auto item = qvariantGet(
                            engine, v, values->GetElementTypeId(), false);
                        vector.append(item);
                    }
                    return QVariant::fromValue(vector);
                }
            }

            if (qstrcmp(tname, "dictionary") == 0) {
                auto values =
                    *getDereferencePointer<CScriptDictionary *>(raw, isHandle);

                // QMap or QHash ?
                if (flag) {
                    QVariantHash hash;
                    for (auto &it : *values) {
                        // Determine the name of the key
                        auto key = it.GetKey();

                        // Get the type and address of the value
                        int cid = it.GetTypeId();
                        const void *v = it.GetAddressOfValue();
                        auto item = qvariantGet(engine, v, cid, false);
                        hash.insert(key, item);
                    }
                    return hash;
                } else {
                    QVariantMap map;
                    for (auto &it : *values) {
                        // Determine the name of the key
                        auto key = it.GetKey();

                        // Get the type and address of the value
                        int cid = it.GetTypeId();
                        const void *v = it.GetAddressOfValue();
                        auto item = qvariantGet(engine, v, cid, false);
                        map.insert(key, item);
                    }
                    return map;
                }
            }
        }

    } break;
    }
    return {};
}

bool WingAngelAPI::getQVariantGetFlag(const ScriptFnInfo &info, int index) {
    auto &params = info.params;
    Q_ASSERT(index >= 0 && index < params.size());

    auto minfo = params.at(index).first;
    return !!(minfo & MetaType::List) || !!(minfo & MetaType::Hash);
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

QString WingAngelAPI::qvariantCastASString(const QMetaType::Type &id) {
    switch (id) {
    case QMetaType::Type::Bool:
        return QStringLiteral("bool");
    case QMetaType::Short:
        return QStringLiteral("int16");
    case QMetaType::UShort:
        return QStringLiteral("uint16");
    case QMetaType::Int:
    case QMetaType::Long:
        return QStringLiteral("int32");
    case QMetaType::UInt:
    case QMetaType::ULong:
        return QStringLiteral("uint32");
    case QMetaType::LongLong:
        return QStringLiteral("int64");
    case QMetaType::ULongLong:
        return QStringLiteral("uint64");
    case QMetaType::Float:
        return QStringLiteral("float");
    case QMetaType::Double:
        return QStringLiteral("double");
    case QMetaType::QChar:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    case QMetaType::Char16:
    case QMetaType::Char32:
#endif
        return QStringLiteral("char");
    case QMetaType::UChar:
        return QStringLiteral("uint8");
    case QMetaType::SChar:
    case QMetaType::Char:
        return QStringLiteral("int8");
    case QMetaType::QString:
        return QStringLiteral("string");
    case QMetaType::QByteArray:
        return QStringLiteral("array<byte>");
    case QMetaType::QVariantMap:
    case QMetaType::QVariantHash:
        return QStringLiteral("dictionary");
    case QMetaType::QStringList:
        return QStringLiteral("array<string>");
    case QMetaType::QColor:
        return QStringLiteral("color");
    case QMetaType::Void:
        return QStringLiteral("void");
    default:
        return {};
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

    auto &fns = p->_sfns.at(id);

    QVariantList params;
    auto total = gen->GetArgCount();
    for (decltype(total) i = 0; i < total; ++i) {
        auto raw = gen->GetAddressOfArg(i);
        auto id = gen->GetArgTypeId(i);

        auto obj = qvariantGet(engine, raw, id, getQVariantGetFlag(fns, i));
        params.append(obj);
    }

    auto ret = fns.fn(params);
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

    // Checking return type
    auto ctx = asGetActiveContext();
    if (ret.canConvert<WingHex::ScriptCallError>()) {
        auto err = ret.value<WingHex::ScriptCallError>();

        auto errmsg = tr("Get Exception While ScriptCall: (%1) %2")
                          .arg(err.errorCode)
                          .arg(err.errmsg);
        ctx->SetException(errmsg.toUtf8());
        return;
    }

    auto rettype = fns.ret;
    auto r = PluginSystem::type2AngelScriptString(rettype, false, true);
    if (r == QStringLiteral("int")) {
        r = QStringLiteral("int32");
    } else if (r == QStringLiteral("uint")) {
        r = QStringLiteral("uint32");
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto type =
        ret.isNull() ? QMetaType::Type::Void : QMetaType::Type(ret.userType());
#else
    auto type =
        ret.isNull() ? QMetaType::Type::Void : QMetaType::Type(ret.typeId());
#endif

    auto rr = qvariantCastASString(type);

    if (r != rr) {
        auto msg = tr("InvalidRetType: need ") + r;
        ctx->SetException(msg.toUtf8());
        return;
    }

    qvariantCastOp(
        engine, ret,
        std::bind(op, gen, std::placeholders::_1, std::placeholders::_2));
}

void WingAngelAPI::script_unsafe_call(asIScriptGeneric *gen) {
    auto fn = gen->GetFunction();

    auto p = reinterpret_cast<WingAngelAPI *>(
        fn->GetUserData(AsUserDataType::UserData_API));
    auto id = reinterpret_cast<qsizetype>(
        fn->GetUserData(AsUserDataType::UserData_PluginFn));

    Q_ASSERT(p);
    Q_ASSERT(id >= 0 && id < p->_usfns.size());
    if (id < 0 || id >= p->_usfns.size()) {
        return;
    }

    // haha, so much simpler than script_call. Pay yourself and
    // have a good time!!!

    auto &fns = p->_usfns.at(id);

    QList<void *> params;
    auto total = gen->GetArgCount();
    WingHex::IWingPlugin::UNSAFE_RET ret;
    for (decltype(total) i = 0; i < total; ++i) {
        auto raw = gen->GetAddressOfArg(i);
        params.append(raw);
    }

    ret = fns(params);

    if (std::holds_alternative<bool>(ret)) {
        gen->SetReturnByte(std::get<bool>(ret));
    } else if (std::holds_alternative<quint8>(ret)) {
        gen->SetReturnByte(std::get<quint8>(ret));
    } else if (std::holds_alternative<quint16>(ret)) {
        gen->SetReturnWord(std::get<quint16>(ret));
    } else if (std::holds_alternative<quint32>(ret)) {
        gen->SetReturnDWord(std::get<quint32>(ret));
    } else if (std::holds_alternative<quint64>(ret)) {
        gen->SetReturnQWord(std::get<quint64>(ret));
    } else if (std::holds_alternative<float>(ret)) {
        gen->SetReturnFloat(std::get<float>(ret));
    } else if (std::holds_alternative<double>(ret)) {
        gen->SetReturnFloat(std::get<double>(ret));
    } else if (std::holds_alternative<void *>(ret)) {
        gen->SetReturnAddress(std::get<void *>(ret));
    } else if (std::holds_alternative<WingHex::ScriptCallError>(ret)) {
        auto ctx = asGetActiveContext();
        auto err = std::get<WingHex::ScriptCallError>(ret);
        auto errmsg = tr("Get Exception While ScriptCall: (%1) %2")
                          .arg(err.errorCode)
                          .arg(err.errmsg);
        ctx->SetException(errmsg.toUtf8());
    }
}

QString WingAngelAPI::QModelIndex_dataString(const QModelIndex &idx) {
    return idx.data().toString();
}

QString WingAngelAPI::QModelIndex_dataChar(const QModelIndex &idx) {
    return idx.data().toChar();
}

uint WingAngelAPI::QModelIndex_dataUInt(const QModelIndex &idx) {
    return idx.data().toUInt();
}

int WingAngelAPI::QModelIndex_dataInt(const QModelIndex &idx) {
    return idx.data().toInt();
}

qlonglong WingAngelAPI::QModelIndex_dataLongLong(const QModelIndex &idx) {
    return idx.data().toLongLong();
}

qlonglong WingAngelAPI::QModelIndex_dataULongLong(const QModelIndex &idx) {
    return idx.data().toULongLong();
}

float WingAngelAPI::QModelIndex_dataFloat(const QModelIndex &idx) {
    return idx.data().toFloat();
}

double WingAngelAPI::QModelIndex_dataDouble(const QModelIndex &idx) {
    return idx.data().toDouble();
}

bool WingAngelAPI::execScriptCode(const WingHex::SenderInfo &sender,
                                  const QString &code) {
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
        _console->stdOut(getSenderHeader(sender));
        auto handles = _handles;
        _console->machine()->executeScript(f.fileName());
        cleanUpHandles(handles);
        _console->appendCommandPrompt();
        _console->setMode(ScriptingConsole::Input);
    }

    return false;
}

bool WingAngelAPI::execScript(const WingHex::SenderInfo &sender,
                              const QString &fileName) {
    _console->setMode(ScriptingConsole::Output);
    _console->stdOut(getSenderHeader(sender));
    auto handles = _handles;
    auto ret = _console->machine()->executeScript(fileName);
    cleanUpHandles(handles);
    _console->appendCommandPrompt();
    _console->setMode(ScriptingConsole::Input);
    return ret;
}

bool WingAngelAPI::execCode(const WingHex::SenderInfo &sender,
                            const QString &code) {
    _console->setMode(ScriptingConsole::Output);
    _console->stdOut(getSenderHeader(sender));
    auto ret = _console->machine()->executeCode(code);
    _console->appendCommandPrompt();
    _console->setMode(ScriptingConsole::Input);
    return ret;
}

QVector<void *> WingAngelAPI::retriveAsCArray(const WingHex::SenderInfo &sender,
                                              void *array) {
    Q_UNUSED(sender);
    if (array == nullptr) {
        return {};
    }

    QVector<void *> ret;
    auto arr = reinterpret_cast<CScriptArray *>(array);
    auto len = arr->GetSize();
    for (asUINT i = 0; i < len; ++i) {
        ret.append(arr->At(i));
    }
    return ret;
}

QHash<QString, QPair<QString, const void *>>
WingAngelAPI::retriveAsDictionary(const WingHex::SenderInfo &sender,
                                  void *dic) {
    Q_UNUSED(sender);
    if (dic == nullptr) {
        return {};
    }

    QHash<QString, QPair<QString, const void *>> ret;
    auto dictionary = reinterpret_cast<CScriptDictionary *>(dic);
    auto engine = dictionary->GetEngine();

    for (auto &it : *dictionary) {
        auto info = engine->GetTypeInfoById(it.GetTypeId());
        QString type;
        if (info) {
            type =
                info->GetNamespace() + QStringLiteral("::") + info->GetName();
        }
        ret.insert(it.GetKey(), qMakePair(type, it.GetAddressOfValue()));
    }
    return ret;
}

void *WingAngelAPI::vector2AsArray(const WingHex::SenderInfo &sender,
                                   MetaType type,
                                   const QVector<void *> &content) {
    Q_UNUSED(sender);
    auto typeStr = PluginSystem::type2AngelScriptString(
        MetaType(type | MetaType::Array), false, true);
    if (typeStr.isEmpty()) {
        return nullptr;
    }

    auto engine = _console->machine()->engine();
    auto info = engine->GetTypeInfoByDecl(typeStr.toUtf8());
    if (info) {
        auto len = content.length();
        auto arr = CScriptArray::Create(info, len);
        for (decltype(len) i = 0; i < len; ++i) {
            arr->SetValue(i, content.at(i));
        }
        return arr;
    }
    return nullptr;
}

void *WingAngelAPI::list2AsArray(const WingHex::SenderInfo &sender,
                                 MetaType type, const QList<void *> &content) {
    Q_UNUSED(sender);
    auto typeStr = PluginSystem::type2AngelScriptString(
        MetaType(type | MetaType::Array), false, true);
    if (typeStr.isEmpty()) {
        return nullptr;
    }

    auto engine = _console->machine()->engine();
    auto info = engine->GetTypeInfoByDecl(typeStr.toUtf8());
    if (info) {
        auto len = content.length();
        auto arr = CScriptArray::Create(info, len);
        for (decltype(len) i = 0; i < len; ++i) {
            arr->SetValue(i, content.at(i));
        }
        return arr;
    }
    return nullptr;
}

void WingAngelAPI::deleteAsArray(const WingHex::SenderInfo &sender,
                                 void *array) {
    Q_UNUSED(sender);
    if (array) {
        reinterpret_cast<CScriptArray *>(array)->Release();
    }
}

void *WingAngelAPI::newAsDictionary(
    const WingHex::SenderInfo &sender,
    const QHash<QString, QPair<MetaType, void *>> &content) {
    Q_UNUSED(sender);
    auto engine = _console->machine()->engine();
    auto dic = CScriptDictionary::Create(engine);

    for (auto p = content.constKeyValueBegin(); p != content.constKeyValueEnd();
         ++p) {
        auto key = p->first;
        auto typeStr =
            PluginSystem::type2AngelScriptString(p->second.first, false, true);
        auto id = engine->GetTypeIdByDecl(typeStr.toUtf8());
        if (id < 0) {
            continue;
        }
        dic->Set(key, p->second.second, id);
    }

    return dic;
}

void WingAngelAPI::deleteAsDictionary(const WingHex::SenderInfo &sender,
                                      void *dic) {
    Q_UNUSED(sender);
    if (dic) {
        reinterpret_cast<CScriptDictionary *>(dic)->Release();
    }
}

QString WingAngelAPI::getSenderHeader(const WingHex::SenderInfo &sender) {
    return QStringLiteral("(") + sender.puid + QStringLiteral("::") +
           sender.plgcls + QStringLiteral(") ");
}

void WingAngelAPI::cleanUpHandles(const QVector<int> &handles) {
    for (auto &h : _handles) {
        if (!handles.contains(h)) {
            closeHandle(h);
        }
    }
    _handles = handles;
}

QString WingAngelAPI::_InputBox_getItem(const QString &title,
                                        const QString &label,
                                        const CScriptArray &items, int current,
                                        bool editable, bool *ok,
                                        Qt::InputMethodHints inputMethodHints) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        auto engine = ctx->GetEngine();
        Q_ASSERT(engine);
        auto stringID = engine->GetTypeIdByDecl("string");
        Q_ASSERT(stringID >= 0);

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
    return {};
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
    return byteArrayWrapperFunction(
        [this, index]() -> QByteArray { return selectedBytes(index); });
}

CScriptArray *WingAngelAPI::_HexReader_selectionBytes() {
    // context, which can be used to obtain a pointer to the
    // engine.
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        asIScriptEngine *engine = ctx->GetEngine();

        auto ret = selectionBytes();

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
    return byteArrayWrapperFunction(
        [this, offset, len]() -> QByteArray { return readBytes(offset, len); });
}

qsizetype WingAngelAPI::_HexReader_findNext(qsizetype begin,
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
        return findNext(begin, bab);
    } else {
        return qsizetype(-1);
    }
}

qsizetype WingAngelAPI::_HexReader_findPrevious(qsizetype begin,
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
            return findPrevious(begin, bab);
        } else {
            return qsizetype(-1);
        }
    } else {
        return qsizetype(-1);
    }
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
        return writeBytes(offset, bab);
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
        return insertBytes(offset, bab);
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
        return appendBytes(bab);
    }
    return false;
}

bool WingAngelAPI::_DataVisual_updateTextList(const CScriptArray &data,
                                              const QString &title,
                                              asIScriptFunction *click,
                                              asIScriptFunction *dblclick) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        auto engine = ctx->GetEngine();
        Q_ASSERT(engine);
        auto stringID = engine->GetTypeIdByDecl("string");
        Q_ASSERT(stringID >= 0);

        // we dont call visual.updateTextList
        bool o = false;
        auto ret = cArray2QStringList(data, stringID, &o);
        if (o) {
            ClickCallBack c(engine, click);
            ClickCallBack dblc(engine, dblclick);

            return PluginSystem::instance().updateTextList_API(ret, title, c,
                                                               dblc);
        }
    }
    return false;
}

bool WingAngelAPI::_DataVisual_updateTextTree(const QString &json,
                                              const QString &title,
                                              asIScriptFunction *click,
                                              asIScriptFunction *dblclick) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        auto engine = ctx->GetEngine();
        Q_ASSERT(engine);

        // we dont call visual.updateTextTree
        ClickCallBack c(engine, click);
        ClickCallBack dblc(engine, dblclick);

        return PluginSystem::instance().updateTextTree_API(json, title, c,
                                                           dblc);
    }
    return false;
}

bool WingAngelAPI::_DataVisual_updateTextTable(const QString &json,
                                               const CScriptArray &headers,
                                               const CScriptArray &headerNames,
                                               const QString &title,
                                               asIScriptFunction *click,
                                               asIScriptFunction *dblclick) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        auto engine = ctx->GetEngine();
        Q_ASSERT(engine);
        auto stringID = engine->GetTypeIdByDecl("string");
        Q_ASSERT(stringID >= 0);

        // we dont call visual.updateTextTable
        bool o = false;
        auto h = cArray2QStringList(headers, stringID, &o);
        if (o) {
            auto hn = cArray2QStringList(headerNames, stringID, &o);
            if (o) {
                ClickCallBack c(engine, click);
                ClickCallBack dblc(engine, dblclick);

                return PluginSystem::instance().updateTextTable_API(
                    json, h, hn, title, c, dblc);
            }
        }
    }
    return false;
}

void WingAngelAPI::setBindingConsole(ScriptingConsole *console) {
    _console = console;
}
