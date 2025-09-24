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
    _parser = new CTypeParser([](const MsgInfo &info) {
        // TODO
    });
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
    return packupEvent(RegisteredEvent::ScriptPragma,
                       RegisteredEvent::ScriptPragmaInit);
}

QList<WingHex::SettingPage *> WingCStruct::registeredSettingPages() const {
    return _setpgs;
}

std::optional<WingHex::PragmaResult>
WingCStruct::eventOnScriptPragma(const QString &script,
                                 const QStringList &comments) {
    // #pragma WingCStruct Arch [32 | 64]
    // #pragma WingCStruct Env reset
    // #pragma WingCStruct Pak [1-8]
    // #pragma WingCStruct Inc [fileName]
    // #pragma WingCStruct Long [ LLP64 | LP64 ]
    if (comments.size() != 2) {
        WingHex::PragmaResult r;
        r.error.append(QStringLiteral("Excessive count of parameters"));
        return r;
    }

    auto cmd = comments.at(0);
    auto param = comments.at(1);
    if (cmd == QStringLiteral("Arch")) {
        if (param == QStringLiteral("32")) {
            _parser->setPointerMode(PointerMode::X86);
            return {};
        } else if (param == QStringLiteral("64")) {
            _parser->setPointerMode(PointerMode::X64);
            return {};
        } else {
            WingHex::PragmaResult r;
            r.error.append(
                QStringLiteral("Unsupported '%1' with 'Arch'").arg(param));
            return r;
        }
    } else if (cmd == QStringLiteral("Env")) {
        if (param == QStringLiteral("reset")) {
            reset();
            return {};
        } else {
            WingHex::PragmaResult r;
            r.error.append(
                QStringLiteral("Unsupported '%1' with 'Env'").arg(param));
            return r;
        }
    } else if (cmd == QStringLiteral("Pak")) {
        bool ok;
        auto num = param.toInt(&ok, 0);
        if (_parser->setPadAlignment(num)) {
            return {};
        } else {
            WingHex::PragmaResult r;
            r.error.append(
                QStringLiteral("Unsupported padding '%1' with 'Pak'").arg(num));
            return r;
        }
    } else if (cmd == QStringLiteral("Inc")) {
        if ((param.startsWith("\"") && param.endsWith("\"")) ||
            (param.startsWith('\'') && param.endsWith("\'"))) {
            param = param.mid(1, param.length() - 2);
        } else {
            WingHex::PragmaResult r;
            r.error.append(QStringLiteral("Invalid syntax with 'Inc'"));
            return r;
        }

        QFileInfo finc(param);
        if (finc.isAbsolute()) {
            if (parse(param)) {
                return {};
            } else {
                WingHex::PragmaResult r;
                r.error.append(
                    QStringLiteral("Error parsing '%1' with 'Inc'").arg(param));
                return r;
            }
        } else {
            QFileInfo finfo(script);
            auto path = finfo.absoluteDir().filePath(param);
            if (parse(path)) {
                return {};
            } else {
                WingHex::PragmaResult r;
                r.error.append(
                    QStringLiteral("Error parsing '%1' with 'Inc'").arg(path));
                return r;
            }
        }
    } else if (cmd == QStringLiteral("Long")) {
        if (param == QStringLiteral("LLP64")) {
            _parser->setLongmode(LongMode::LLP64);
            return {};
        } else if (param == QStringLiteral("LP64")) {
            _parser->setLongmode(LongMode::LP64);
            return {};
        } else {
            WingHex::PragmaResult r;
            r.error.append(
                QStringLiteral("Unsupported '%1' with 'Long'").arg(param));
            return r;
        }
    } else {
        WingHex::PragmaResult r;
        r.error.append(QStringLiteral("Unsupported pragma command: %1")
                           .arg(comments.join(' ')));
        return r;
    }
}

void WingCStruct::eventOnScriptPragmaInit() { reset(); }

void WingCStruct::onRegisterScriptObj(WingHex::IWingAngel *o) {
    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::parseFromSource),
            this, std::placeholders::_1),
        QStringLiteral("parseFromSource"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("header"))});

    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(QOverload<const QVariantList &>::of(&WingCStruct::parse),
                  this, std::placeholders::_1),
        QStringLiteral("parse"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("fileName"))});

    o->registerGlobalFunction(
        WingHex::Meta_Void,
        std::bind(QOverload<const QVariantList &>::of(&WingCStruct::reset),
                  this, std::placeholders::_1),
        QStringLiteral("reset"));

    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::setPadAlignment),
            this, std::placeholders::_1),
        QStringLiteral("setPadAlignment"),
        {qMakePair(WingHex::Meta_Int, QStringLiteral("padding"))});

    o->registerGlobalFunction(WingHex::Meta_Int,
                              std::bind(QOverload<const QVariantList &>::of(
                                            &WingCStruct::padAlignment),
                                        this, std::placeholders::_1),
                              QStringLiteral("padAlignment"));

    o->registerGlobalFunction(WingHex::Meta_String | WingHex::Meta_Array,
                              std::bind(QOverload<const QVariantList &>::of(
                                            &WingCStruct::structTypeDefs),
                                        this, std::placeholders::_1),
                              QStringLiteral("structTypes"));

    o->registerGlobalFunction(
        getqsizetypeMetaType(),
        std::bind(QOverload<const QVariantList &>::of(&WingCStruct::sizeOf),
                  this, std::placeholders::_1),
        QStringLiteral("sizeofStruct"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});

    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::containsStruct),
            this, std::placeholders::_1),
        QStringLiteral("containsStruct"),
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

bool WingCStruct::parseFromSource(const QString &header) {
    return _parser->parseFromSource(header.toUtf8());
}

bool WingCStruct::parse(const QString &fileName) {
    return _parser->parse(fileName);
}

void WingCStruct::reset() { _parser->clear(); }

bool WingCStruct::setPadAlignment(int padding) {
    return _parser->setPadAlignment(padding);
}

int WingCStruct::padAlignment() { return _parser->padAlignment(); }

QStringList WingCStruct::structTypeDefs() { return _parser->structTypeDefs(); }

QStringList WingCStruct::unionTypeDefs() { return _parser->unionTypeDefs(); }

QStringList WingCStruct::typedefTypeDefs() {
    return _parser->typedefTypeDefs();
}

QStringList WingCStruct::enumTypeDefs() { return _parser->enumTypeDefs(); }

QStringList WingCStruct::constVarDefs() { return _parser->constVarDefs(); }

quint64 WingCStruct::sizeOf(const QString &type) {
    auto r = _parser->getTypeSize(type);
    if (r) {
        return r.value();
    }
    return 0;
}

bool WingCStruct::containsType(const QString &name) {
    return _parser->containsType(name);
}

bool WingCStruct::isBasicType(const QString &name) {
    return _parser->isBasicType(name);
}

bool WingCStruct::isUnsignedBasicType(const QString &name) {
    return _parser->isUnsignedBasicType(name);
}

bool WingCStruct::containsEnum(const QString &name) {
    return _parser->containsEnum(name);
}

bool WingCStruct::containsStruct(const QString &name) {
    return _parser->containsStruct(name);
}

bool WingCStruct::containsUnion(const QString &name) {
    return _parser->containsUnion(name);
}

bool WingCStruct::containsTypeDef(const QString &name) {
    return _parser->containsTypeDef(name);
}

bool WingCStruct::containsConstVar(const QString &name) {
    return _parser->containsConstVar(name);
}

bool WingCStruct::isCompletedType(const QString &name) {
    return _parser->isCompletedType(name);
}

QStringList WingCStruct::enumValueNames(const QString &name) {
    return _parser->enumMembers(name);
}

qint64 WingCStruct::constVarValueInt(const QString &name, bool *ok) {
    auto v = _parser->constVarValue(name);
    if (std::holds_alternative<qint64>(v)) {
        if (ok) {
            *ok = true;
        }
        return std::get<qint64>(v);
    } else {
        if (ok) {
            *ok = false;
        }
        return 0;
    }
}

quint64 WingCStruct::constVarValueUInt(const QString &name, bool *ok) {
    auto v = _parser->constVarValue(name);
    if (std::holds_alternative<quint64>(v)) {
        if (ok) {
            *ok = true;
        }
        return std::get<quint64>(v);
    } else {
        if (ok) {
            *ok = false;
        }
        return 0;
    }
}

bool WingCStruct::isCompletedStruct(const QString &name) {
    return _parser->isCompletedStruct(name);
}

bool WingCStruct::isCompletedUnion(const QString &name) {
    return _parser->isCompletedUnion(name);
}

QStringList WingCStruct::getMissingDependencise(const QString &name) {
    return _parser->getMissingDependencise(name);
}

QVariantHash WingCStruct::read(qsizetype offset, const QString &type) {
    auto len = sizeOf(type);
    if (len <= 0) {
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
    auto len = sizeOf(type);
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
    if (!_parser->isCompletedStruct(type)) {
        return {};
    }
    auto struc = _parser->structMembers(type);

    QVariantHash content;

    // then slice and parse
    for (auto &m : struc) {
        bool retry = false;
        auto t = _parser->type(m.data_type);
        do {
            retry = false;
            switch (t) {
            case CTypeParser::CType::Unknown:
            case CTypeParser::CType::ConstVar:
                Q_ASSERT(false);
                break;
            case CTypeParser::CType::BasicType: {
                auto meta = _parser->metaType(m.data_type);
                auto size = _parser->getTypeSize(m.data_type);
                Q_ASSERT(size);
                auto size_v = size.value();
                if (m.element_count) {
                    QVariantList l;
                    for (qsizetype i = 0; i < m.element_count; ++i) {
                        auto data = getData(ptr, end, meta, size_v);
                        if (data.isNull()) {
                            return content;
                        }
                        ptr += size_v;
                        l.append(data);
                    }
                    content.insert(m.var_name, l);
                } else {
                    // TODO
                    // auto mask = m.op.mask;
                    // auto shift = m.op.shift;

                    auto data = getData(ptr, end, meta, size_v);
                    if (data.isNull()) {
                        return content;
                    }
                    ptr += size_v;
                    content.insert(m.var_name, data);
                }
            } break;
            case CTypeParser::CType::Enum:
                // should not go there
                break;
            case CTypeParser::CType::Struct:
                content.insert(m.var_name, readStruct(ptr, end, m.data_type));
                break;
            case CTypeParser::CType::Union: {
                // union, but i dont really know how to diplay it...
                // QByteArray will a good container for it
                // TODO
                auto size = _parser->getTypeSize(m.data_type);
                Q_ASSERT(size);
                auto size_v = size.value();
                if (ptr + size_v < end) {
                    content.insert(m.var_name, QByteArray(ptr, size_v));
                    ptr += size_v;
                } else {
                    return content;
                }
            } break;
            case CTypeParser::CType::TypeDef: {
                // TODO
                retry = true;
            } break;
            case CTypeParser::CType::Pointer:
                break;
            }
        } while (retry);
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

QVariant WingCStruct::parseFromSource(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto header_v = params.at(0);
    if (!header_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto header = header_v.toString();
    return parseFromSource(header);
}

QVariant WingCStruct::parse(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto fileName_v = params.at(0);
    if (!fileName_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }

    auto fileName = fileName_v.toString();
    return parse(fileName);
}

QVariant WingCStruct::reset(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    reset();
    return {};
}

QVariant WingCStruct::setPadAlignment(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto padding_v = params.at(0);
    if (!padding_v.canConvert<int>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto padding = padding_v.toInt();
    return setPadAlignment(padding);
}

QVariant WingCStruct::padAlignment(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    return padAlignment();
}

QVariant WingCStruct::structTypeDefs(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    return structTypeDefs();
}

QVariant WingCStruct::sizeOf(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return sizeOf(type);
}

QVariant WingCStruct::containsStruct(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return containsStruct(type);
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
