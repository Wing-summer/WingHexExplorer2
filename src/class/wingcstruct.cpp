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

#include "wingcstruct.h"

#include "utilities.h"

WingCStruct::WingCStruct() : WingHex::IWingPlugin() {
    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::addStruct), this,
            std::placeholders::_1);
        info.ret = MetaType::Bool;

        info.params.append(
            qMakePair(MetaType::String, QStringLiteral("header")));

        _scriptInfo.insert(QStringLiteral("addStruct"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(QOverload<const QVariantList &>::of(
                                &WingCStruct::addStructFromFile),
                            this, std::placeholders::_1);
        info.ret = MetaType::Bool;

        info.params.append(
            qMakePair(MetaType::String, QStringLiteral("fileName")));

        _scriptInfo.insert(QStringLiteral("addStructFromFile"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::resetEnv), this,
            std::placeholders::_1);
        info.ret = MetaType::Void;

        _scriptInfo.insert(QStringLiteral("resetEnv"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::setStructPadding),
            this, std::placeholders::_1);
        info.ret = MetaType::Bool;

        info.params.append(qMakePair(MetaType::Int, QStringLiteral("padding")));

        _scriptInfo.insert(QStringLiteral("setStructPadding"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::structPadding),
            this, std::placeholders::_1);
        info.ret = MetaType::Int;

        _scriptInfo.insert(QStringLiteral("structPadding"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::structPadding),
            this, std::placeholders::_1);
        info.ret = MetaType::Int;

        _scriptInfo.insert(QStringLiteral("structPadding"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::structTypes),
            this, std::placeholders::_1);
        info.ret = MetaType(MetaType::String | MetaType::Array);

        _scriptInfo.insert(QStringLiteral("structTypes"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::sizeofStruct),
            this, std::placeholders::_1);
        info.ret = getqsizetypeMetaType();
        info.params.append(qMakePair(MetaType::String, QStringLiteral("type")));
        _scriptInfo.insert(QStringLiteral("sizeofStruct"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::existStruct),
            this, std::placeholders::_1);
        info.ret = MetaType::Bool;
        info.params.append(qMakePair(MetaType::String, QStringLiteral("type")));
        _scriptInfo.insert(QStringLiteral("existStruct"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::metadata), this,
            std::placeholders::_1);
        info.ret = MetaType::Bool;

        info.params.append(
            qMakePair(getqsizetypeMetaType(), QStringLiteral("offset")));
        info.params.append(qMakePair(MetaType::String, QStringLiteral("type")));
        info.params.append(qMakePair(MetaType::Color, QStringLiteral("fg")));
        info.params.append(qMakePair(MetaType::Color, QStringLiteral("bg")));
        info.params.append(
            qMakePair(MetaType::String, QStringLiteral("comment")));

        _scriptInfo.insert(QStringLiteral("metadata"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::foreground), this,
            std::placeholders::_1);
        info.ret = MetaType::Bool;

        info.params.append(
            qMakePair(getqsizetypeMetaType(), QStringLiteral("offset")));
        info.params.append(qMakePair(MetaType::String, QStringLiteral("type")));
        info.params.append(qMakePair(MetaType::Color, QStringLiteral("color")));

        _scriptInfo.insert(QStringLiteral("foreground"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::background), this,
            std::placeholders::_1);
        info.ret = MetaType::Bool;

        info.params.append(
            qMakePair(getqsizetypeMetaType(), QStringLiteral("offset")));
        info.params.append(qMakePair(MetaType::String, QStringLiteral("type")));
        info.params.append(qMakePair(MetaType::Color, QStringLiteral("color")));

        _scriptInfo.insert(QStringLiteral("background"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::comment), this,
            std::placeholders::_1);
        info.ret = MetaType::Bool;

        info.params.append(
            qMakePair(getqsizetypeMetaType(), QStringLiteral("offset")));
        info.params.append(qMakePair(MetaType::String, QStringLiteral("type")));
        info.params.append(
            qMakePair(MetaType::String, QStringLiteral("comment")));

        _scriptInfo.insert(QStringLiteral("comment"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn =
            std::bind(QOverload<const QVariantList &>::of(&WingCStruct::read),
                      this, std::placeholders::_1);
        info.ret = MetaType::Hash;

        info.params.append(
            qMakePair(getqsizetypeMetaType(), QStringLiteral("offset")));
        info.params.append(qMakePair(MetaType::String, QStringLiteral("type")));

        _scriptInfo.insert(QStringLiteral("read"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::readRaw), this,
            std::placeholders::_1);
        info.ret = MetaType(MetaType::Byte | MetaType::Array);

        info.params.append(
            qMakePair(getqsizetypeMetaType(), QStringLiteral("offset")));
        info.params.append(qMakePair(MetaType::String, QStringLiteral("type")));

        _scriptInfo.insert(QStringLiteral("readRaw"), info);
    }
}

WingCStruct::~WingCStruct() {}

int WingCStruct::sdkVersion() const { return WingHex::SDKVERSION; }

const QString WingCStruct::signature() const { return WingHex::WINGSUMMER; }

bool WingCStruct::init(const std::unique_ptr<QSettings> &set) {
    resetEnv();
    return true;
}

void WingCStruct::unload(std::unique_ptr<QSettings> &set) {
    // nothing
}

const QString WingCStruct::pluginName() const { return tr("WingCStruct"); }

const QString WingCStruct::pluginComment() const {
    return tr("Providing basic support for analyzing file structures");
}

QIcon WingCStruct::pluginIcon() const { return ICONRES("structure"); }

QString WingCStruct::retranslate(const QString &str) {
    return QApplication::tr(str.toLatin1());
}

WingCStruct::RegisteredEvents WingCStruct::registeredEvents() const {
    return RegisteredEvent::ScriptPragma;
}

QHash<WingHex::SettingPage *, bool>
WingCStruct::registeredSettingPages() const {
    return _setpgs;
}

QHash<QString, WingCStruct::ScriptFnInfo>
WingCStruct::registeredScriptFns() const {
    return _scriptInfo;
}

bool WingCStruct::eventOnScriptPragma(const QStringList &comments) {

    return true;
}

bool WingCStruct::eventScriptPragmaLineStep(const QString &codes) {

    return true;
}

void WingCStruct::eventScriptPragmaFinished() {}

bool WingCStruct::addStruct(const QString &header) { return true; }

bool WingCStruct::addStructFromFile(const QString &fileName) { return true; }

void WingCStruct::resetEnv() { _parser = CTypeParser(); }

bool WingCStruct::setStructPadding(int padding) {
    if (padding < 1 || padding > 8) {
        return false;
    }
    _parser.setPadAlignment(padding);
    return true;
}

int WingCStruct::structPadding() { return _parser.padAlignment(); }

QStringList WingCStruct::structTypes() { return _parser.structDefs().keys(); }

qsizetype WingCStruct::sizeofStruct(const QString &type) {
    auto types = _parser.types();
    if (types.contains(type)) {
        return types.value(type).second;
    }
    return -1;
}

bool WingCStruct::existStruct(const QString &type) {
    return _parser.structDefs().contains(type);
}

bool WingCStruct::metadata(qsizetype offset, const QString &type,
                           const QColor &fg, const QColor &bg,
                           const QString &comment) {
    if (!(fg.isValid() && bg.isValid() && !comment.isEmpty())) {
        return false;
    }

    auto len = sizeofStruct(type);
    if (len < 0) {
        return false;
    }

    if (!emit reader.isCurrentDocEditing()) {
        return false;
    }

    if (offset < 0) {
        return false;
    }

    auto doclen = emit reader.documentBytes();
    if (doclen < 0 || offset + len >= doclen) {
        return false;
    }

    auto handle = emit controller.openCurrent();
    if (handle < 0) {
        return false;
    }

    auto ret =
        emit controller.beginMarco(QStringLiteral("WingCStruct::metadata"));
    if (!ret) {
        return false;
    }

    auto struc = _parser.structDefs().value(type);
    for (auto &m : struc) {
        // TODO
    }

    emit controller.endMarco();
    emit controller.closeHandle(handle);
    return true;
}

bool WingCStruct::foreground(qsizetype offset, const QString &type,
                             const QColor &color) {
    return metadata(offset, type, color, QColor(), QString());
}

bool WingCStruct::background(qsizetype offset, const QString &type,
                             const QColor &color) {
    return metadata(offset, type, QColor(), color, QString());
}

bool WingCStruct::comment(qsizetype offset, const QString &type,
                          const QString &comment) {
    return metadata(offset, type, QColor(), QColor(), comment);
}

QVariantHash WingCStruct::read(qsizetype offset, const QString &type) {
    auto len = sizeofStruct(type);
    if (len < 0) {
        return {};
    }

    if (!emit reader.isCurrentDocEditing()) {
        return {};
    }

    if (offset < 0) {
        return {};
    }

    auto doclen = emit reader.documentBytes();
    if (doclen < 0 || offset + len >= doclen) {
        return {};
    }

    auto struc = _parser.structDefs().value(type);

    // ok, begin reading
    QVariantHash content;

    // first read all bytes
    auto raw = emit reader.readBytes(offset, len);

    // then slice and parse
    // TODO
    for (auto &m : struc) {
        auto t = _parser.getTokenType(m.data_type);
        if (t == kBasicDataType) {

        } else if (t == kStructName) {

        } else if (t == kUnionName) {
        }
    }
    return content;
}

QByteArray WingCStruct::readRaw(qsizetype offset, const QString &type) {
    auto len = sizeofStruct(type);
    if (len < 0) {
        return {};
    }

    if (!emit reader.isCurrentDocEditing()) {
        return {};
    }

    return {};
}

QByteArray WingCStruct::getRawData(const QString &type,
                                   const QVariantHash &content) {
    // TODO
    return {};
}

WingHex::IWingPlugin::MetaType WingCStruct::getqsizetypeMetaType() const {
    return sizeof(qsizetype) == sizeof(quint64) ? MetaType::Int64
                                                : MetaType::Int32;
}

QVariant WingCStruct::addStruct(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto header_v = params.at(0);
    if (!header_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto header = header_v.toString();
    return addStruct(header);
}

QVariant WingCStruct::addStructFromFile(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto fileName_v = params.at(0);
    if (!fileName_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }

    auto fileName = fileName_v.toString();
    return addStructFromFile(fileName);
}

QVariant WingCStruct::resetEnv(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    resetEnv();
    return {};
}

QVariant WingCStruct::setStructPadding(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto padding_v = params.at(0);
    if (!padding_v.canConvert<int>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto padding = padding_v.toInt();
    return setStructPadding(padding);
}

QVariant WingCStruct::structPadding(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    return structPadding();
}

QVariant WingCStruct::structTypes(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    return structTypes();
}

QVariant WingCStruct::sizeofStruct(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return sizeofStruct(type);
}

QVariant WingCStruct::existStruct(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return existStruct(type);
}

QVariant WingCStruct::metadata(const QVariantList &params) {
    if (params.size() != 5) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    auto offset_v = params.at(0);
    auto type_v = params.at(1);
    auto fg_v = params.at(2);
    auto bg_v = params.at(3);
    auto comment_v = params.at(4);

    if (!offset_v.canConvert<qsizetype>() || !type_v.canConvert<QString>() ||
        !fg_v.canConvert<QColor>() || !bg_v.canConvert<QColor>() ||
        !comment_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }

    auto offset = offset_v.value<qsizetype>();
    auto type = type_v.toString();
    auto fg = fg_v.value<QColor>();
    auto bg = bg_v.value<QColor>();
    auto comment = comment_v.toString();
    return metadata(offset, type, fg, bg, comment);
}

QVariant WingCStruct::foreground(const QVariantList &params) {
    if (params.size() != 3) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    auto offset_v = params.at(0);
    auto type_v = params.at(1);
    auto color_v = params.at(2);

    if (!offset_v.canConvert<qsizetype>() || !type_v.canConvert<QString>() ||
        !color_v.canConvert<QColor>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }

    auto offset = offset_v.value<qsizetype>();
    auto type = type_v.toString();
    auto color = color_v.value<QColor>();
    return foreground(offset, type, color);
}

QVariant WingCStruct::background(const QVariantList &params) {
    if (params.size() != 3) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    auto offset_v = params.at(0);
    auto type_v = params.at(1);
    auto color_v = params.at(2);

    if (!offset_v.canConvert<qsizetype>() || !type_v.canConvert<QString>() ||
        !color_v.canConvert<QColor>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }

    auto offset = offset_v.value<qsizetype>();
    auto type = type_v.toString();
    auto color = color_v.value<QColor>();
    return background(offset, type, color);
}

QVariant WingCStruct::comment(const QVariantList &params) {
    if (params.size() != 3) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    auto offset_v = params.at(0);
    auto type_v = params.at(1);
    auto comment_v = params.at(2);

    if (!offset_v.canConvert<qsizetype>() || !type_v.canConvert<QString>() ||
        !comment_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }

    auto offset = offset_v.value<qsizetype>();
    auto type = type_v.toString();
    auto comment = comment_v.toString();
    return this->comment(offset, type, comment);
}

QVariant WingCStruct::read(const QVariantList &params) {
    if (params.size() != 2) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    auto offset_v = params.at(0);
    auto type_v = params.at(1);
    if (!offset_v.canConvert<qsizetype>() || !type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }

    auto offset = offset_v.value<qsizetype>();
    auto type = type_v.toString();
    return read(offset, type);
}

QVariant WingCStruct::readRaw(const QVariantList &params) {
    if (params.size() != 2) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    auto offset_v = params.at(0);
    auto type_v = params.at(1);
    if (!offset_v.canConvert<qsizetype>() || !type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }

    auto offset = offset_v.value<qsizetype>();
    auto type = type_v.toString();
    return readRaw(offset, type);
}
