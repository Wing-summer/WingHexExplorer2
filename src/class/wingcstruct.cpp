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

WingCStruct::WingCStruct()
    : WingHex::IWingPlugin(),
      _colortable({Qt::red, Qt::green, Qt::yellow, Qt::cyan}) {
    qRegisterMetaType<QVector<QColor>>();
}

WingCStruct::~WingCStruct() {}

int WingCStruct::sdkVersion() const { return WingHex::SDKVERSION; }

const QString WingCStruct::signature() const { return WingHex::WINGSUMMER; }

bool WingCStruct::init(const std::unique_ptr<QSettings> &set) {
    // load color table

    _curColor = _colortable.cbegin();
    resetEnv();

    return true;
}

void WingCStruct::unload(std::unique_ptr<QSettings> &set) {
    // save color table
}

const QString WingCStruct::pluginName() const { return tr("WingCStruct"); }

const QString WingCStruct::pluginComment() const {
    return tr("Providing basic support for analyzing file structures");
}

QIcon WingCStruct::pluginIcon() const { return ICONRES("structure"); }

WingCStruct::RegisteredEvents WingCStruct::registeredEvents() const {
    return RegisteredEvent::ScriptPragma;
}

QHash<WingHex::SettingPage *, bool>
WingCStruct::registeredSettingPages() const {
    return _setpgs;
}

QHash<QString, WingCStruct::ScriptFnInfo>
WingCStruct::registeredScriptFns() const {
    return _scriptfns;
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

bool WingCStruct::setColorTable(const QVector<QColor> &table) {
    // only main thread can modified it
    if (QThread::currentThread() != qApp->thread()) {
        return false;
    }

    if (table.isEmpty()) {
        return false;
    }

    auto r = std::find_if_not(table.begin(), table.end(), [](const QColor &c) {
        return c.isValid() && c.alpha() == 255;
    });

    if (r != table.end()) {
        return false;
    }

    _colortable = table;

    return true;
}

QVector<QColor> WingCStruct::colorTable() { return _colortable; }

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
    return _parser.typeSizes().value(type, -1);
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

    for (auto &m : struc) {
        auto t = _parser.getTokenType(m.data_type);
        if (t == kBasicDataType) {

        } else if (t == kStructName) {

        } else if (t == kUnionName) {
        }
    }
    return content;
}

bool WingCStruct::write(qsizetype offset, const QString &type,
                        const QVariantHash &content) {
    auto len = sizeofStruct(type);
    if (len < 0) {
        return false;
    }

    if (!emit reader.isCurrentDocEditing()) {
        return false;
    }

    return emit controller.writeBytes(offset, getRawData(type, content));
}

bool WingCStruct::insert(qsizetype offset, const QString &type,
                         const QVariantHash &content) {
    auto len = sizeofStruct(type);
    if (len < 0) {
        return false;
    }

    if (!emit reader.isCurrentDocEditing()) {
        return false;
    }

    return emit controller.insertBytes(offset, getRawData(type, content));
}

bool WingCStruct::append(const QString &type, const QVariantHash &content) {
    auto len = sizeofStruct(type);
    if (len < 0) {
        return false;
    }

    if (!emit reader.isCurrentDocEditing()) {
        return false;
    }

    return emit controller.appendBytes(getRawData(type, content));
}

QByteArray WingCStruct::getRawData(const QString &type,
                                   const QVariantHash &content) {
    // TODO
    return {};
}

QColor WingCStruct::getNextColor() {
    auto color = *_curColor;
    _curColor++;
    if (_curColor == _colortable.end()) {
        _curColor = _colortable.begin();
    }
    return color;
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

QVariant WingCStruct::setColorTable(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto table_v = params.at(0);
    if (!table_v.canConvert<QVector<QColor>>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }

    auto table = table_v.value<QVector<QColor>>();
    return setColorTable(table);
}

QVariant WingCStruct::colorTable(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    return QVariant::fromValue(colorTable());
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

QVariant WingCStruct::write(const QVariantList &params) {
    if (params.size() != 3) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    auto offset_v = params.at(0);
    auto type_v = params.at(1);
    auto content_v = params.at(2);

    if (!offset_v.canConvert<qsizetype>() || !type_v.canConvert<QString>() ||
        !content_v.canConvert<QVariantHash>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }

    auto offset = offset_v.value<qsizetype>();
    auto type = type_v.toString();
    auto content = content_v.toHash();
    return write(offset, type, content);
}

QVariant WingCStruct::insert(const QVariantList &params) {
    if (params.size() != 3) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    auto offset_v = params.at(0);
    auto type_v = params.at(1);
    auto content_v = params.at(2);

    if (!offset_v.canConvert<qsizetype>() || !type_v.canConvert<QString>() ||
        !content_v.canConvert<QVariantHash>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }

    auto offset = offset_v.value<qsizetype>();
    auto type = type_v.toString();
    auto content = content_v.toHash();
    return insert(offset, type, content);
}

QVariant WingCStruct::append(const QVariantList &params) {
    if (params.size() != 2) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    auto type_v = params.at(0);
    auto content_v = params.at(1);

    if (!type_v.canConvert<QString>() ||
        !content_v.canConvert<QVariantHash>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }

    auto type = type_v.toString();
    auto content = content_v.toHash();
    return append(type, content);
}
