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
    _parser = new CTypeParser([this](const MsgInfo &info) {
        auto msg = QStringLiteral("(%1, %2) ")
                       .arg(info.line)
                       .arg(info.charPositionInLine) +
                   info.info;
        switch (info.type) {
        case MsgType::Error:
            logError(msg);
            break;
        case MsgType::Warn:
            logWarn(msg);
            break;
        }
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
        std::bind(QOverload<const QVariantList &>::of(
                      &WingCStruct::setIsLittleEndian),
                  this, std::placeholders::_1),
        QStringLiteral("setIsLittleEndian"),
        {qMakePair(WingHex::Meta_Bool, QStringLiteral("value"))});

    o->registerGlobalFunction(WingHex::Meta_Bool,
                              std::bind(QOverload<const QVariantList &>::of(
                                            &WingCStruct::isLittleEndian),
                                        this, std::placeholders::_1),
                              QStringLiteral("isLittleEndian"));

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
    o->registerGlobalFunction(WingHex::Meta_String | WingHex::Meta_Array,
                              std::bind(QOverload<const QVariantList &>::of(
                                            &WingCStruct::unionTypeDefs),
                                        this, std::placeholders::_1),
                              QStringLiteral("unionTypeDefs"));
    o->registerGlobalFunction(WingHex::Meta_String | WingHex::Meta_Array,
                              std::bind(QOverload<const QVariantList &>::of(
                                            &WingCStruct::typedefTypeDefs),
                                        this, std::placeholders::_1),
                              QStringLiteral("typedefTypeDefs"));
    o->registerGlobalFunction(WingHex::Meta_String | WingHex::Meta_Array,
                              std::bind(QOverload<const QVariantList &>::of(
                                            &WingCStruct::enumTypeDefs),
                                        this, std::placeholders::_1),
                              QStringLiteral("enumTypeDefs"));
    o->registerGlobalFunction(WingHex::Meta_String | WingHex::Meta_Array,
                              std::bind(QOverload<const QVariantList &>::of(
                                            &WingCStruct::constVarDefs),
                                        this, std::placeholders::_1),
                              QStringLiteral("constVarDefs"));

    o->registerGlobalFunction(
        WingHex::Meta_UInt64,
        std::bind(QOverload<const QVariantList &>::of(&WingCStruct::sizeOf),
                  this, std::placeholders::_1),
        QStringLiteral("sizeOf"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});

    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::containsType),
            this, std::placeholders::_1),
        QStringLiteral("containsType"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});
    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::isBasicType),
            this, std::placeholders::_1),
        QStringLiteral("isBasicType"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});
    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(QOverload<const QVariantList &>::of(
                      &WingCStruct::isUnsignedBasicType),
                  this, std::placeholders::_1),
        QStringLiteral("isUnsignedBasicType"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});

    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::containsEnum),
            this, std::placeholders::_1),
        QStringLiteral("containsEnum"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});
    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::containsStruct),
            this, std::placeholders::_1),
        QStringLiteral("containsStruct"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});
    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::containsUnion),
            this, std::placeholders::_1),
        QStringLiteral("containsUnion"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});
    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::containsTypeDef),
            this, std::placeholders::_1),
        QStringLiteral("containsTypeDef"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});
    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::containsConstVar),
            this, std::placeholders::_1),
        QStringLiteral("containsConstVar"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});
    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::isCompletedType),
            this, std::placeholders::_1),
        QStringLiteral("isCompletedType"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});

    o->registerGlobalFunction(WingHex::Meta_String | WingHex::Meta_Array,
                              std::bind(QOverload<const QVariantList &>::of(
                                            &WingCStruct::enumValueNames),
                                        this, std::placeholders::_1),
                              QStringLiteral("enumValueNames"));

    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(QOverload<const QVariantList &>::of(
                      &WingCStruct::isCompletedStruct),
                  this, std::placeholders::_1),
        QStringLiteral("isCompletedStruct"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});

    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(
            QOverload<const QVariantList &>::of(&WingCStruct::isCompletedUnion),
            this, std::placeholders::_1),
        QStringLiteral("isCompletedUnion"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});

    o->registerGlobalFunction(
        WingHex::Meta_Bool,
        std::bind(QOverload<const QVariantList &>::of(
                      &WingCStruct::getMissingDependencise),
                  this, std::placeholders::_1),
        QStringLiteral("getMissingDependencise"),
        {qMakePair(WingHex::Meta_String, QStringLiteral("type"))});

    o->registerGlobalFunction(
        QStringLiteral("int64 constVarValueInt(const string &in type, bool "
                       "&out ok = void)"),
        std::bind(QOverload<const QList<void *> &>::of(
                      &WingCStruct::constVarValueInt),
                  this, std::placeholders::_1));
    o->registerGlobalFunction(
        QStringLiteral("uint64 constVarValueUInt(const string &in type, bool "
                       "&out ok = void)"),
        std::bind(QOverload<const QList<void *> &>::of(
                      &WingCStruct::constVarValueUInt),
                  this, std::placeholders::_1));

    o->registerObjectType("int_type", sizeof(CINT_TYPE),
                          asOBJ_VALUE | WingHex::asGetTypeTraits<CINT_TYPE>());
    o->registerObjectBehaviour(
        "int_type", WingHex::IWingAngel::asBehaviours::asBEHAVE_CONSTRUCT,
        "void f(int64 v)",
        asWINGFUNCTIONPR(
            [](void *memory, qint64 v) { new (memory) CINT_TYPE(v); },
            (void *, qint64), void),
        WingHex::IWingAngel::asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectBehaviour(
        "int_type", WingHex::IWingAngel::asBehaviours::asBEHAVE_CONSTRUCT,
        "void f(const int_type &in)",
        asWINGFUNCTIONPR(
            [](CINT_TYPE *self, const CINT_TYPE &other) {
                new (self) CINT_TYPE(other);
            },
            (CINT_TYPE *, const CINT_TYPE &), void),
        WingHex::IWingAngel::asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectBehaviour(
        "int_type", WingHex::IWingAngel::asBehaviours::asBEHAVE_CONSTRUCT,
        "void f(uint64 v)",
        asWINGFUNCTIONPR(
            [](void *memory, quint64 v) { new (memory) CINT_TYPE(v); },
            (void *, quint64), void),
        WingHex::IWingAngel::asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectBehaviour(
        "int_type", WingHex::IWingAngel::asBehaviours::asBEHAVE_DESTRUCT,
        "void f()",
        asWINGFUNCTIONPR([](CINT_TYPE *memory) { memory->~CINT_TYPE(); },
                         (CINT_TYPE *), void),
        WingHex::IWingAngel::asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectMethod(
        "int_type", "bool isInt() const", asWINGMETHOD(CINT_TYPE, isInt),
        WingHex::IWingAngel::asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "int_type", "bool isUInt() const", asWINGMETHOD(CINT_TYPE, isUInt),
        WingHex::IWingAngel::asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "int_type", "int64 toInt() const", asWINGMETHOD(CINT_TYPE, toInt),
        WingHex::IWingAngel::asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "int_type", "uint64 toUInt() const", asWINGMETHOD(CINT_TYPE, toUInt),
        WingHex::IWingAngel::asCallConvTypes::asCALL_THISCALL);

    o->registerObjectEvaluator(
        "int_type",
        [](void *result, WingHex::IWingAngel::EvalMode mode)
            -> WingHex::IWingAngel::EvaluateResult {
            auto v = static_cast<CINT_TYPE *>(result);
            WingHex::IWingAngel::EvalResult r;
            if (v->isInt()) {
                r.value = QString::number(v->toInt());
            } else {
                r.value = QString::number(v->toUInt());
            }
            r.expandable = false;
            return r;
        });
    o->registerGlobalFunction(
        QStringLiteral(
            "WingCStruct::int_type constVarValue(const string &in type)"),
        std::bind(
            QOverload<const QList<void *> &>::of(&WingCStruct::constVarValue),
            this, std::placeholders::_1));

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
    return _parser->resolveType(name) == CTypeParser::CType::BasicType;
}

bool WingCStruct::isUnsignedBasicType(const QString &name) {
    auto rname = _parser->resolveTypeName(name);
    return _parser->isUnsignedBasicType(rname);
}

bool WingCStruct::containsEnum(const QString &name) {
    return _parser->resolveType(name) == CTypeParser::CType::Enum;
}

bool WingCStruct::containsStruct(const QString &name) {
    return _parser->resolveType(name) == CTypeParser::CType::Struct;
}

bool WingCStruct::containsUnion(const QString &name) {
    return _parser->resolveType(name) == CTypeParser::CType::Union;
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
    auto rname = _parser->resolveTypeName(name);
    return _parser->enumMembers(rname);
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
    auto rname = _parser->resolveTypeName(name);
    return _parser->isCompletedStruct(rname);
}

bool WingCStruct::isCompletedUnion(const QString &name) {
    auto rname = _parser->resolveTypeName(name);
    return _parser->isCompletedUnion(rname);
}

QStringList WingCStruct::getMissingDependencise(const QString &name) {
    auto rname = _parser->resolveTypeName(name);
    return _parser->getMissingDependencise(rname);
}

QVariantHash WingCStruct::read(qsizetype offset, const QString &type) {
    auto rtype = _parser->resolveTypeName(type);
    auto len = sizeOf(rtype);
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

    return readStruct(pdata, pend, rtype);
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
                              QMetaType::Type type, size_t shift, size_t mask,
                              qsizetype size) {
    if (ptr + size > end) {
        return {};
    }

    QByteArray buffer(ptr, size);
    switch (type) {
    case QMetaType::Bool:
        return getShiftAndMasked<bool>(buffer, shift, mask);
    case QMetaType::Int:
        return getShiftAndMasked<int>(buffer, shift, mask);
    case QMetaType::UInt:
        return getShiftAndMasked<unsigned int>(buffer, shift, mask);
    case QMetaType::LongLong:
        return getShiftAndMasked<long long>(buffer, shift, mask);
    case QMetaType::ULongLong:
        return getShiftAndMasked<unsigned long long>(buffer, shift, mask);
    case QMetaType::Double:
        return getShiftAndMasked<double>(buffer, shift, mask);
    case QMetaType::Long:
        return QVariant::fromValue(
            getShiftAndMasked<long>(buffer, shift, mask));
    case QMetaType::Short:
        return getShiftAndMasked<short>(buffer, shift, mask);
    case QMetaType::Char:
        return getShiftAndMasked<char>(buffer, shift, mask);
    case QMetaType::Char16:
        return getShiftAndMasked<char16_t>(buffer, shift, mask);
    case QMetaType::Char32:
        return getShiftAndMasked<char32_t>(buffer, shift, mask);
    case QMetaType::ULong:
        return QVariant::fromValue(
            getShiftAndMasked<unsigned long>(buffer, shift, mask));
    case QMetaType::UShort:
        return getShiftAndMasked<unsigned short>(buffer, shift, mask);
    case QMetaType::UChar:
        return getShiftAndMasked<unsigned char>(buffer, shift, mask);
    case QMetaType::Float:
        return getShiftAndMasked<float>(buffer, shift, mask);
    case QMetaType::SChar:
        return getShiftAndMasked<signed char>(buffer, shift, mask);
    case QMetaType::VoidStar:
        return getShiftAndMaskedPtr(buffer);
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
    QVariantHash ret;

    QVariantHash content;
    // then slice and parse
    for (auto &m : struc) {
        content.insert(m.var_name, readContent(ptr, end, m));
    }
    return content;
}

QVariant WingCStruct::readContent(const char *&ptr, const char *end,
                                  const VariableDeclaration &m) {

    bool retry = false;
    auto dt = m.data_type;
    do {
        auto t = _parser->type(dt);
        switch (t) {
        case CTypeParser::CType::Unknown:
        case CTypeParser::CType::ConstVar:
            Q_ASSERT(false);
            break;
        case CTypeParser::CType::BasicType: {
            auto meta = _parser->metaType(dt);
            auto size = _parser->getTypeSize(dt);
            Q_ASSERT(size);
            auto size_v = size.value();
            if (m.element_count > 1) {
                QVariantList l;
                for (qsizetype i = 0; i < m.element_count; ++i) {
                    auto data =
                        getData(ptr, end, meta, m.op.shift, m.op.mask, size_v);
                    if (data.isNull()) {
                        return {};
                    }
                    ptr += size_v;
                    l.append(data);
                }
                return l;
            } else {
                auto data =
                    getData(ptr, end, meta, m.op.shift, m.op.mask, size_v);
                if (data.isNull()) {
                    return {};
                }

                if (!m.fmt_type.isEmpty()) {
                    // format will take effects if basic type
                    // is not double or float
                    auto typeID = data.typeId();

                    // if ok, we create a array
                    // [0] = int_value, [1] = decoded_enum
                    QVariantList rlist;

                    if (typeID == QMetaType::Float ||
                        typeID == QMetaType::Double ||
                        typeID == QMetaType::Float16) {
                        // warn and ignore format
                        logWarn(QStringLiteral(
                            "Invalid format type on float number"));
                    }

                    QString v;
                    if (typeID == QMetaType::ULongLong) {
                        auto var = data.toULongLong();
                        auto evs = _parser->enumMembers(m.fmt_type);

                        for (auto &e : evs) {
                            auto ev = _parser->constVarValue(e);
                            if (std::holds_alternative<qint64>(ev)) {
                                auto rv = std::get<qint64>(ev);
                                if (rv == var) {
                                    v = e;
                                    break;
                                }
                            } else {
                                auto rv = std::get<quint64>(ev);
                                if (rv == var) {
                                    v = e;
                                    break;
                                }
                            }
                        }
                    } else {
                        auto var = data.toLongLong();
                        auto evs = _parser->enumMembers(m.fmt_type);

                        for (auto &e : evs) {
                            auto ev = _parser->constVarValue(e);
                            if (std::holds_alternative<qint64>(ev)) {
                                auto rv = std::get<qint64>(ev);
                                if (rv == var) {
                                    v = e;
                                    break;
                                }
                            } else {
                                auto rv = std::get<quint64>(ev);
                                if (rv == var) {
                                    v = e;
                                    break;
                                }
                            }
                        }
                    }

                    rlist.append(data);
                    if (v.isEmpty()) {
                        rlist.append(QStringLiteral("<?>"));
                    } else {
                        rlist.append(v);
                    }
                    data = rlist;
                }

                ptr += size_v;
                return data;
            }
        } break;
        case CTypeParser::CType::Enum:
            // should not go there
            Q_ASSERT(false);
            break;
        case CTypeParser::CType::Struct:
            return readStruct(ptr, end, dt);
        case CTypeParser::CType::Union: {
            auto size = _parser->getTypeSize(dt);
            Q_ASSERT(size);
            auto size_v = size.value();
            auto ptrend = ptr + size_v;
            if (ptrend <= end) {
                auto ms = _parser->unionMembers(dt);
                QVariantHash ret;
                for (auto &m : ms) {
                    auto rptr = ptr;
                    auto r = readContent(rptr, ptrend, m);
                    ret.insert(m.var_name, r);
                }
                ptr += size_v;
                return ret;
            } else {
                return {};
            }
        } break;
        case CTypeParser::CType::TypeDef: {
            dt = _parser->resolveTypeName(dt);
            retry = true;
        } break;
        case CTypeParser::CType::Pointer: {
            if (_parser->pointerMode() == PointerMode::X86) {
                auto data = getData(ptr, end, QMetaType::VoidStar, m.op.shift,
                                    m.op.mask, sizeof(quint32));
                return data;
            } else {
                auto data = getData(ptr, end, QMetaType::VoidStar, m.op.shift,
                                    m.op.mask, sizeof(quint64));
                return data;
            }
        } break;
        }
    } while (retry);

    return {};
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

QVariant WingCStruct::isLittleEndian(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    return isLittleEndian();
}

QVariant WingCStruct::setIsLittleEndian(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto islittle_v = params.at(1);
    if (!islittle_v.canConvert<bool>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }

    auto islittle = islittle_v.toBool();
    setIsLittleEndian(islittle);
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

QVariant WingCStruct::unionTypeDefs(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    return unionTypeDefs();
}

QVariant WingCStruct::typedefTypeDefs(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    return typedefTypeDefs();
}

QVariant WingCStruct::enumTypeDefs(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    return enumTypeDefs();
}

QVariant WingCStruct::constVarDefs(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    return constVarDefs();
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

QVariant WingCStruct::containsType(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return containsType(type);
}

QVariant WingCStruct::isBasicType(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return isBasicType(type);
}

QVariant WingCStruct::isUnsignedBasicType(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return isUnsignedBasicType(type);
}

QVariant WingCStruct::containsEnum(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return containsEnum(type);
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

QVariant WingCStruct::containsUnion(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return containsUnion(type);
}

QVariant WingCStruct::containsTypeDef(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return containsTypeDef(type);
}

QVariant WingCStruct::containsConstVar(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return containsConstVar(type);
}

QVariant WingCStruct::isCompletedType(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return isCompletedType(type);
}

QVariant WingCStruct::enumValueNames(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return enumValueNames(type);
}

QVariant WingCStruct::isCompletedStruct(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return isCompletedStruct(type);
}

QVariant WingCStruct::isCompletedUnion(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return isCompletedUnion(type);
}

QVariant WingCStruct::getMissingDependencise(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type_v = params.at(0);
    if (!type_v.canConvert<QString>()) {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    auto type = type_v.toString();
    return getMissingDependencise(type);
}

WingHex::UNSAFE_RET WingCStruct::constVarValueInt(const QList<void *> &params) {
    if (params.size() != 2) {
        return generateScriptCallError(-1, tr("InvalidParamsCount"));
    }

    auto type = WingHex::resolveUnsafeParamAsIn<QString>(params.at(0));
    auto ok = WingHex::resolveUnsafeParamAsOut<bool>(params.at(1));
    return WingHex::makeUnsafeRet(constVarValueInt(type, ok));
}

WingHex::UNSAFE_RET
WingCStruct::constVarValueUInt(const QList<void *> &params) {
    if (params.size() != 2) {
        return generateScriptCallError(-1, tr("InvalidParamsCount"));
    }

    auto type = WingHex::resolveUnsafeParamAsIn<QString>(params.at(0));
    auto ok = WingHex::resolveUnsafeParamAsOut<bool>(params.at(1));
    return constVarValueUInt(type, ok);
}

WingHex::UNSAFE_RET WingCStruct::constVarValue(const QList<void *> &params) {
    if (params.size() != 1) {
        return generateScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto type = WingHex::resolveUnsafeParamAsIn<QString>(params.at(0));
    auto v = _parser->constVarValue(type);
    return WingHex::makeValueObject<CINT_TYPE>(v);
}

WingHex::UNSAFE_RET WingCStruct::read(const QList<void *> &params) {
    if (params.size() != 2) {
        return generateScriptCallError(-1, tr("InvalidParamsCount"));
    }

    auto offset = WingHex::resolveUnsafeParamAs<qsizetype>(params.at(0));
    auto type = WingHex::resolveUnsafeParamAsIn<QString>(params.at(1));

    auto ret = read(offset, type);
    return WingHex::makeRefObjPtr(convert2AsDictionary(ret));
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

bool WingCStruct::isLittleEndian() const { return m_islittle; }

void WingCStruct::setIsLittleEndian(bool newIslittle) {
    m_islittle = newIslittle;
}
