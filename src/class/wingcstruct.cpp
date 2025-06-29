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

#include "WingPlugin/iwingangel.h"
#include "scriptaddon/scriptqdictionary.h"
#include "utilities.h"
#include "wingangelapi.h"

WingCStruct::WingCStruct() : WingHex::IWingPlugin() {}

WingCStruct::~WingCStruct() {}

bool WingCStruct::init(const std::unique_ptr<QSettings> &set) {
    Q_UNUSED(set);
    resetEnv();
    return true;
}

void WingCStruct::unload(std::unique_ptr<QSettings> &set) {
    // nothing
    Q_UNUSED(set);
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
    RegisteredEvents evs;
    evs.setFlag(RegisteredEvent::ScriptPragma);
    evs.setFlag(RegisteredEvent::ScriptPragmaInit);
    return evs;
}

QList<WingHex::SettingPage *> WingCStruct::registeredSettingPages() const {
    return _setpgs;
}

bool WingCStruct::eventOnScriptPragma(const QString &script,
                                      const QStringList &comments) {
    // #pragma WingCStruct Arch [32 | 64]
    // #pragma WingCStruct Env reset
    // #pragma WingCStruct Pak [1-8]
    // #pragma WingCStruct Inc [fileName]
    // #pragma WingCStruct Long [ LLP64 | LP64 ]
    if (comments.size() != 2) {
        return false;
    }

    auto cmd = comments.at(0);
    auto param = comments.at(1);
    if (cmd == QStringLiteral("Arch")) {
        if (param == QStringLiteral("32")) {
            _parser.setPointerMode(PointerMode::X86);
            return true;
        } else if (param == QStringLiteral("64")) {
            _parser.setPointerMode(PointerMode::X64);
            return true;
        }
    } else if (cmd == QStringLiteral("Env")) {
        if (param == QStringLiteral("reset")) {
            resetEnv();
            return true;
        }
    } else if (cmd == QStringLiteral("Pak")) {
        if (param.length() == 1) {
            auto num = param.at(0).toLatin1();
            if (num >= '0' && num <= '9') {
                _parser.setPadAlignment(num - '0');
                return true;
            }
        }
    } else if (cmd == QStringLiteral("Inc")) {
        if (param.startsWith("\"")) {
            if (param.endsWith("\"")) {
                param = param.mid(1, param.length() - 2);
            } else {
                return false;
            }
        } else if (param.startsWith('\'')) {
            if (param.endsWith("\'")) {
                param = param.mid(1, param.length() - 2);
            } else {
                return false;
            }
        }
        QFileInfo finc(param);
        if (finc.isAbsolute()) {
            return addStructFromFile(param);
        } else {
            QFileInfo finfo(script);
            return addStructFromFile(finfo.absoluteDir().filePath(param));
        }
    } else if (cmd == QStringLiteral("Long")) {
        if (param == QStringLiteral("LLP64")) {
            _parser.setLongmode(LongMode::LLP64);
            return true;
        } else if (param == QStringLiteral("LP64")) {
            _parser.setLongmode(LongMode::LP64);
            return true;
        }
    }

    return false;
}

void WingCStruct::eventOnScriptPragmaInit() { resetEnv(); }

void WingCStruct::onRegisterScriptObj(WingHex::IWingAngel *o) {
    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(QOverload<const QVariantList &>::of(&WingCStruct::addStruct),
                  this, std::placeholders::_1),
        QStringLiteral("addStruct"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("header"))});

    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(QOverload<const QVariantList &>::of(
                      &WingCStruct::addStructFromFile),
                  this, std::placeholders::_1),
        QStringLiteral("addStructFromFile"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("fileName"))});

    o->registerGlobalFunction(
        WingHex::Meta_Void,
        std::bind(QOverload<const QVariantList &>::of(&WingCStruct::resetEnv),
                  this, std::placeholders::_1),
        QStringLiteral("resetEnv"));

    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::setStructPadding),
            this, std::placeholders::_1),
        QStringLiteral("setStructPadding"),
        {qMakePair(WingHex::Meta_Int, QStringLiteral("padding"))});

    o->registerGlobalFunction(WingHex::Meta_Int,
                              std::bind(QOverload<const QVariantList &>::of(
                                            &WingCStruct::structPadding),
                                        this, std::placeholders::_1),
                              QStringLiteral("structPadding"));

    o->registerGlobalFunction(WingHex::Meta_String | WingHex::Meta_Array,
                              std::bind(QOverload<const QVariantList &>::of(
                                            &WingCStruct::structTypes),
                                        this, std::placeholders::_1),
                              QStringLiteral("structTypes"));

    o->registerGlobalFunction(
        getqsizetypeMetaType(),
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::sizeofStruct),
            this, std::placeholders::_1),
        QStringLiteral("sizeofStruct"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});

    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::existStruct),
            this, std::placeholders::_1),
        QStringLiteral("existStruct"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});

    o->registerGlobalFunction(WingHex::Meta_String | WingHex::Meta_Array,
                              std::bind(QOverload<const QVariantList &>::of(
                                            &WingCStruct::constDefines),
                                        this, std::placeholders::_1),
                              QStringLiteral("constDefines"));

    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::existDefineValue),
            this, std::placeholders::_1),
        QStringLiteral("existDefineValue"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});

    o->registerGlobalFunction(
        WingHex::Meta_Int,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::defineValue),
            this, std::placeholders::_1),
        QStringLiteral("defineValue"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});

    // nested dictionary is not supported, so unsafe registering will help
    o->registerGlobalFunction(
        QStringLiteral("dictionary@ read(") + getqsizeTypeAsString() +
            (" offset, const string &in type)"),
        std::bind(QOverload<const QList<void *> &>::of(&WingCStruct::read),
                  this, std::placeholders::_1));

    o->registerGlobalFunction(
        WingHex::Meta_Byte | WingHex::Meta_Array,
        std::bind(QOverload<const QVariantList &>::of(&WingCStruct::readRaw),
                  this, std::placeholders::_1),
        QStringLiteral("readRaw"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});
}

bool WingCStruct::addStruct(const QString &header) {
    return _parser.parseSource(header);
}

bool WingCStruct::addStructFromFile(const QString &fileName) {
    return _parser.parseFile(fileName);
}

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

QStringList WingCStruct::constDefines() { return _parser.constDefs().keys(); }

bool WingCStruct::existDefineValue(const QString &type) {
    return _parser.constDefs().contains(type);
}

int WingCStruct::defineValue(const QString &type) {
    return _parser.constDefs().value(type, 0);
}

QVariantHash WingCStruct::read(qsizetype offset, const QString &type) {
    auto len = sizeofStruct(type);
    if (len < 0) {
        return {};
    }

    if (!isCurrentDocEditing()) {
        return {};
    }

    if (offset < 0) {
        return {};
    }

    auto doclen = documentBytes();
    if (doclen < 0 || offset + len > doclen) {
        return {};
    }

    // first read all bytes
    auto raw = readBytes(offset, len);

    // then slice and parse
    const auto *pdata = raw.data();
    const auto *pend = pdata + raw.length();

    return readStruct(pdata, pend, type);
}

QByteArray WingCStruct::readRaw(qsizetype offset, const QString &type) {
    auto len = sizeofStruct(type);
    if (len < 0) {
        return {};
    }

    if (!isCurrentDocEditing()) {
        return {};
    }

    return readBytes(offset, len);
}

QString WingCStruct::getqsizeTypeAsString() const {
    return sizeof(qsizetype) == sizeof(quint64) ? QStringLiteral("int64")
                                                : QStringLiteral("int32");
}

QVariant WingCStruct::getData(const char *ptr, const char *end,
                              QMetaType::Type type, qsizetype size) {
    if (ptr + size > end) {
        return {};
    }
    switch (type) {
    case QMetaType::Bool:
        return (*ptr) ? true : false;
    case QMetaType::Int:
        return *reinterpret_cast<const int *>(ptr);
    case QMetaType::UInt:
        return *reinterpret_cast<const unsigned int *>(ptr);
    case QMetaType::LongLong:
        return *reinterpret_cast<const long long *>(ptr);
    case QMetaType::ULongLong:
        return *reinterpret_cast<const unsigned long long *>(ptr);
    case QMetaType::Double:
        return *reinterpret_cast<const double *>(ptr);
    case QMetaType::Long:
        return QVariant::fromValue(*reinterpret_cast<const long *>(ptr));
    case QMetaType::Short:
        return *reinterpret_cast<const short *>(ptr);
    case QMetaType::Char:
        return *ptr;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    case QMetaType::Char16:
        return *reinterpret_cast<const char16_t *>(ptr);
    case QMetaType::Char32:
        return *reinterpret_cast<const char32_t *>(ptr);
#else
    case QMetaType::QChar: {
        if (size == sizeof(quint16)) {
            auto ch = *reinterpret_cast<const quint16 *>(ptr);
            return QChar(ch);
        } else if (size == sizeof(quint32)) {
            auto ch = *reinterpret_cast<const quint32 *>(ptr);
            return QChar(ch);
        }
        return {};
    }
#endif
    case QMetaType::ULong:
        return QVariant::fromValue(
            *reinterpret_cast<const unsigned long *>(ptr));
    case QMetaType::UShort:
        return *reinterpret_cast<const unsigned short *>(ptr);
    case QMetaType::UChar:
        return *reinterpret_cast<const unsigned char *>(ptr);
    case QMetaType::Float:
        return *reinterpret_cast<const float *>(ptr);
    case QMetaType::SChar:
        return *reinterpret_cast<const signed char *>(ptr);
    default:
        return {};
    }
}

QVariantHash WingCStruct::readStruct(const char *&ptr, const char *end,
                                     const QString &type) {
    auto struc = _parser.structDefs().value(type);
    QVariantHash content;

    // then slice and parse
    for (auto &m : struc) {
        auto t = _parser.getTokenType(m.data_type);
        if (t == kBasicDataType) {
            auto t = _parser.type(m.data_type);
            if (m.array_size) {
                QVariantList l;
                for (qsizetype i = 0; i < m.array_size; ++i) {
                    auto data = getData(ptr, end, t.first, t.second);
                    if (data.isNull()) {
                        return content;
                    }
                    ptr += t.second;
                    l.append(data);
                }
                content.insert(m.var_name, l);
            } else {
                auto data = getData(ptr, end, t.first, t.second);
                if (data.isNull()) {
                    return content;
                }
                ptr += t.second;
                content.insert(m.var_name, data);
            }
        } else if (t == kStructName) {
            content.insert(m.var_name, readStruct(ptr, end, m.data_type));
        } else if (t == kUnionName) {
            // union, but i dont really know how to diplay it...
            // QByteArray will a good container for it
            auto size = sizeofStruct(m.data_type);
            if (ptr + size < end) {
                content.insert(m.var_name, QByteArray(ptr, size));
                ptr += size;
            } else {
                return content;
            }
        }
    }

    return content;
}

bool WingCStruct::isValidCStructMetaType(QMetaType::Type type) {
    switch (type) {
    case QMetaType::Bool:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::Double:
    case QMetaType::Long:
    case QMetaType::Short:
    case QMetaType::Char:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    case QMetaType::Char16:
    case QMetaType::Char32:
#else
    case QMetaType::QChar:
#endif
    case QMetaType::ULong:
    case QMetaType::UShort:
    case QMetaType::UChar:
    case QMetaType::Float:
    case QMetaType::SChar:
        return true;
    default:
        return false;
    }
}

CScriptDictionary *WingCStruct::convert2AsDictionary(const QVariantHash &hash) {
    auto ctx = asGetActiveContext();
    if (ctx == nullptr) {
        return nullptr;
    }

    auto engine = ctx->GetEngine();
    Q_ASSERT(engine);

    auto dic = CScriptDictionary::Create(engine);
    for (auto p = hash.constKeyValueBegin(); p != hash.constKeyValueEnd();
         ++p) {
        auto var = p->second;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        auto type = var.isNull() ? QMetaType::Type::Void
                                 : QMetaType::Type(var.userType());
#else
        auto type = var.isNull() ? QMetaType::Type::Void
                                 : QMetaType::Type(var.typeId());
#endif
        switch (type) {
        case QMetaType::Bool:
        case QMetaType::UChar:
        case QMetaType::Int:
        case QMetaType::Long:
        case QMetaType::LongLong:
        case QMetaType::SChar:
        case QMetaType::Short: {
            auto v = var.toLongLong();
            dic->Set(p->first, asINT64(v));
            break;
        }
        case QMetaType::UInt:
        case QMetaType::ULong:
        case QMetaType::UShort:
        case QMetaType::ULongLong: {
            auto v = var.toULongLong();
            dic->Set(p->first, asQWORD(v));
            break;
        }
        case QMetaType::Double:
        case QMetaType::Float:
            dic->Set(p->first, var.toDouble());
            break;
        case QMetaType::Char: {
            auto v = var.value<char>();
            auto ch = new QChar(v);
            auto id = engine->GetTypeIdByDecl("char");
            dic->Set(p->first, ch, id);
            break;
        }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        case QMetaType::Char16: {
            auto v = var.value<char16_t>();
            auto ch = new QChar(v);
            auto id = engine->GetTypeIdByDecl("char");
            dic->Set(p->first, ch, id);
            break;
        }
        case QMetaType::Char32: {
            auto v = var.value<char32_t>();
            auto ch = new QChar(v);
            auto id = engine->GetTypeIdByDecl("char");
            dic->Set(p->first, ch, id);
            break;
        }
#endif
        case QMetaType::QVariantList: {
            // note: empty list is not allowed!
            // If empty, it will be ignored
            auto v = var.toList();
            if (!v.isEmpty()) {
                // reguard the first element type is the specilization
                auto var = v.first();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                auto type = var.isNull() ? QMetaType::Type::Void
                                         : QMetaType::Type(var.userType());
#else
                auto type = var.isNull() ? QMetaType::Type::Void
                                         : QMetaType::Type(var.typeId());
#endif
                if (type == QMetaType::Type::Void) {
                    // ignore
                    break;
                }
                if (!isValidCStructMetaType(type)) {
                    // ignore
                    break;
                }

                auto idStr = WingAngelAPI::qvariantCastASString(type);
                if (idStr.isEmpty()) {
                    // ignore
                    break;
                }

                auto arrType =
                    QStringLiteral("array<") + idStr + QStringLiteral(">");
                auto arrTypeID = engine->GetTypeIdByDecl(arrType.toUtf8());
                if (arrTypeID < 0) {
                    // ignore
                    break;
                }

                dic->Set(p->first, convert2AsArray(v, type, arrTypeID),
                         arrTypeID);
            }
            break;
        }
        case QMetaType::QVariantHash: {
            auto id = engine->GetTypeIdByDecl("dictionary");
            dic->Set(p->first, convert2AsDictionary(var.toHash()), id);
            break;
        }
        default:
            // ignore
            break;
        }
    }
    return dic;
}

CScriptArray *WingCStruct::convert2AsArray(const QVariantList &array,
                                           QMetaType::Type type, int id) {
    Q_ASSERT(!array.isEmpty());
    auto ctx = asGetActiveContext();
    if (ctx == nullptr) {
        return nullptr;
    }

    auto engine = ctx->GetEngine();
    Q_ASSERT(engine);

    auto arr = CScriptArray::Create(engine->GetTypeInfoById(id), array.size());

    static asQWORD buffer;
    buffer = 0;

    for (asUINT i = 0; i < arr->GetSize(); ++i) {
        auto var = array.at(i);
        switch (type) {
        case QMetaType::Type::Bool:
            WingAngelAPI::assignTmpBuffer(buffer, var.toBool());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::Short:
        case QMetaType::UShort:
            WingAngelAPI::assignTmpBuffer(buffer, var.value<ushort>());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::Int:
        case QMetaType::Long:
        case QMetaType::UInt:
        case QMetaType::ULong:
            WingAngelAPI::assignTmpBuffer(buffer, var.toUInt());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
            WingAngelAPI::assignTmpBuffer(buffer, var.toULongLong());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::Float:
            WingAngelAPI::assignTmpBuffer(buffer, var.toULongLong());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::Double:
            WingAngelAPI::assignTmpBuffer(buffer, var.toDouble());
            arr->SetValue(i, &buffer);
            break;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        case QMetaType::Char16:
            arr->SetValue(i, new QChar(var.value<char16_t>()));
            break;
        case QMetaType::Char32:
            arr->SetValue(i, new QChar(var.value<char32_t>()));
            break;
#endif
        case QMetaType::UChar:
        case QMetaType::SChar:
        case QMetaType::Char:
            WingAngelAPI::assignTmpBuffer(buffer, var.value<uchar>());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::QVariantHash:
            // struct
            arr->SetValue(i, convert2AsDictionary(var.toHash()));
        default:
            // ignore, but should not go there
            // nested array is not allowed
            break;
        }
    }

    return arr;
}

WingHex::MetaType WingCStruct::getqsizetypeMetaType() const {
    return sizeof(qsizetype) == sizeof(quint64) ? WingHex::MetaType::Meta_Int64
                                                : WingHex::MetaType::Meta_Int32;
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

QVariant WingCStruct::constDefines(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    return constDefines();
}

QVariant WingCStruct::existDefineValue(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return existDefineValue(type);
}

QVariant WingCStruct::defineValue(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return defineValue(type);
}

WingHex::UNSAFE_RET WingCStruct::read(const QList<void *> &params) {
    if (params.size() != 2) {
        return generateScriptCallError(-1, tr("InvalidParamsCount"));
    }

    auto offset = *reinterpret_cast<qsizetype *>(params.at(0));
    auto type = **reinterpret_cast<QString **>(params.at(1));

    auto ret = read(offset, type);
    return static_cast<void *>(convert2AsDictionary(ret));
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
