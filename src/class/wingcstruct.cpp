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
        auto msg = QStringLiteral(": [%1, %2] ")
                       .arg(info.line)
                       .arg(info.charPositionInLine) +
                   info.info;
        switch (info.type) {
        case MsgType::Error:
            _errors.append(msg);
            break;
        case MsgType::Warn:
            _warns.append(msg);
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
                WingHex::PragmaResult r;
                for (auto &msg : _warns) {
                    r.warn.append(msg);
                }
                return r;
            } else {
                WingHex::PragmaResult r;
                for (auto &msg : _errors) {
                    r.error.append(msg);
                }
                for (auto &msg : _warns) {
                    r.warn.append(msg);
                }
                return r;
            }
        } else {
            QFileInfo finfo(script);
            auto path = finfo.absoluteDir().filePath(param);
            if (parse(path)) {
                WingHex::PragmaResult r;
                for (auto &msg : _warns) {
                    r.warn.append(msg);
                }
                return r;
            } else {
                WingHex::PragmaResult r;
                for (auto &msg : _errors) {
                    r.error.append(msg);
                }
                for (auto &msg : _warns) {
                    r.warn.append(msg);
                }
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

    o->registerObjectType("IntType", sizeof(CINT_TYPE),
                          asOBJ_VALUE | WingHex::asGetTypeTraits<CINT_TYPE>());
    o->registerObjectBehaviour(
        "IntType", WingHex::IWingAngel::asBehaviours::asBEHAVE_CONSTRUCT,
        "void f(int64 v)",
        asWINGFUNCTIONPR(
            [](void *memory, qint64 v) { new (memory) CINT_TYPE(v); },
            (void *, qint64), void),
        WingHex::IWingAngel::asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectBehaviour(
        "IntType", WingHex::IWingAngel::asBehaviours::asBEHAVE_CONSTRUCT,
        "void f(const IntType &in)",
        asWINGFUNCTIONPR(
            [](CINT_TYPE *self, const CINT_TYPE &other) {
                new (self) CINT_TYPE(other);
            },
            (CINT_TYPE *, const CINT_TYPE &), void),
        WingHex::IWingAngel::asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectBehaviour(
        "IntType", WingHex::IWingAngel::asBehaviours::asBEHAVE_CONSTRUCT,
        "void f(uint64 v)",
        asWINGFUNCTIONPR(
            [](void *memory, quint64 v) { new (memory) CINT_TYPE(v); },
            (void *, quint64), void),
        WingHex::IWingAngel::asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectBehaviour(
        "IntType", WingHex::IWingAngel::asBehaviours::asBEHAVE_DESTRUCT,
        "void f()",
        asWINGFUNCTIONPR([](CINT_TYPE *memory) { memory->~CINT_TYPE(); },
                         (CINT_TYPE *), void),
        WingHex::IWingAngel::asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectMethod(
        "IntType", "bool isInt() const", asWINGMETHOD(CINT_TYPE, isInt),
        WingHex::IWingAngel::asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "IntType", "bool isUInt() const", asWINGMETHOD(CINT_TYPE, isUInt),
        WingHex::IWingAngel::asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "IntType", "int64 toInt() const", asWINGMETHOD(CINT_TYPE, toInt),
        WingHex::IWingAngel::asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "IntType", "uint64 toUInt() const", asWINGMETHOD(CINT_TYPE, toUInt),
        WingHex::IWingAngel::asCallConvTypes::asCALL_THISCALL);

    o->registerObjectEvaluator(
        "IntType",
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
            "WingCStruct::IntType constVarValue(const string &in type)"),
        std::bind(
            QOverload<const QList<void *> &>::of(&WingCStruct::constVarValue),
            this, std::placeholders::_1));

    o->registerObjectType("EnumValue", sizeof(CEnumValue),
                          asOBJ_VALUE | WingHex::asGetTypeTraits<CEnumValue>());
    o->registerObjectBehaviour(
        "EnumValue", WingHex::IWingAngel::asBehaviours::asBEHAVE_CONSTRUCT,
        "void f(const IntType &in, const string &in)",
        asWINGFUNCTIONPR(
            [](void *memory, const CINT_TYPE &k, const QString &v) {
                new (memory) CEnumValue(k, v);
            },
            (void *, const CINT_TYPE &, const QString &), void),
        WingHex::IWingAngel::asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectBehaviour(
        "EnumValue", WingHex::IWingAngel::asBehaviours::asBEHAVE_CONSTRUCT,
        "void f(const EnumValue &in)",
        asWINGFUNCTIONPR(
            [](CEnumValue *self, const CEnumValue &other) {
                new (self) CEnumValue(other);
            },
            (CEnumValue *, const CEnumValue &), void),
        WingHex::IWingAngel::asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectBehaviour(
        "EnumValue", WingHex::IWingAngel::asBehaviours::asBEHAVE_DESTRUCT,
        "void f()",
        asWINGFUNCTIONPR([](CEnumValue *memory) { memory->~CEnumValue(); },
                         (CEnumValue *), void),
        WingHex::IWingAngel::asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectProperty("EnumValue", "IntType value",
                              Q_OFFSETOF(CEnumValue, first));
    o->registerObjectProperty("EnumValue", "string name",
                              Q_OFFSETOF(CEnumValue, second));
    o->registerObjectEvaluator(
        "EnumValue",
        [](void *result, WingHex::IWingAngel::EvalMode mode)
            -> WingHex::IWingAngel::EvaluateResult {
            auto v = static_cast<CEnumValue *>(result);
            switch (mode) {
            case WingHex::IWingAngel::EvalMode::Eval: {
                WingHex::IWingAngel::EvalResult r;
                QString intv;
                if (v->first.isInt()) {
                    intv = QString::number(v->first.toInt());
                } else {
                    intv = QString::number(v->first.toUInt());
                }

                r.value = QStringLiteral("(%1, \"%2\")").arg(intv, v->second);
                r.expandable = true;
                return r;
            } break;
            case WingHex::IWingAngel::EvalMode::Expand: {
                QHash<QString, WingHex::IWingAngel::ExpandResult> r;

                WingHex::IWingAngel::ExpandResult value;
                value.type = "WingCStruct::IntType";
                value.buffer = &v->first;
                r.insert(QStringLiteral("value"), value);

                WingHex::IWingAngel::ExpandResult name;
                name.type = "string";
                name.buffer = &v->second;
                r.insert(QStringLiteral("name"), name);

                return r;
            } break;
            }

            return {};
        });

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

    o->registerGlobalFunction(
        "string dumpAllTypes()", asWINGMETHOD(WingCStruct, dumpAllTypes),
        WingHex::IWingAngel::asCallConvTypes::asCALL_THISCALL_ASGLOBAL, this);
    o->registerGlobalFunction(
        "string dumpTypeDefines()", asWINGMETHOD(WingCStruct, dumpTypeDefines),
        WingHex::IWingAngel::asCallConvTypes::asCALL_THISCALL_ASGLOBAL, this);
    o->registerGlobalFunction(
        "string dumpConstants()", asWINGMETHOD(WingCStruct, dumpConstants),
        WingHex::IWingAngel::asCallConvTypes::asCALL_THISCALL_ASGLOBAL, this);
    o->registerGlobalFunction(
        "string dumpStructs()", asWINGMETHOD(WingCStruct, dumpStructs),
        WingHex::IWingAngel::asCallConvTypes::asCALL_THISCALL_ASGLOBAL, this);
    o->registerGlobalFunction(
        "string dumpUnions()", asWINGMETHOD(WingCStruct, dumpUnions),
        WingHex::IWingAngel::asCallConvTypes::asCALL_THISCALL_ASGLOBAL, this);
    o->registerGlobalFunction(
        "string dumpEnums()", asWINGMETHOD(WingCStruct, dumpEnums),
        WingHex::IWingAngel::asCallConvTypes::asCALL_THISCALL_ASGLOBAL, this);

    o->registerGlobalFunction(WingHex::Meta_String | WingHex::Meta_Array,
                              std::bind(QOverload<const QVariantList &>::of(
                                            &WingCStruct::getParsedErrors),
                                        this, std::placeholders::_1),
                              QStringLiteral("getParsedErrors"));
    o->registerGlobalFunction(WingHex::Meta_String | WingHex::Meta_Array,
                              std::bind(QOverload<const QVariantList &>::of(
                                            &WingCStruct::getParsedWarns),
                                        this, std::placeholders::_1),
                              QStringLiteral("getParsedWarns"));
}

bool WingCStruct::parseFromSource(const QString &header) {
    _errors.clear();
    _warns.clear();
    return _parser->parseFromSource(header.toUtf8());
}

bool WingCStruct::parse(const QString &fileName) {
    _errors.clear();
    _warns.clear();
    return _parser->parse(fileName);
}

void WingCStruct::reset() {
    _errors.clear();
    _warns.clear();
    _parser->clear();
}

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
    return __read(offset, type, true);
}

QVariantHash WingCStruct::__read(qsizetype offset, const QString &type,
                                 bool efmtInVariantList) {
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

    return readStruct(pdata, pend, rtype, efmtInVariantList);
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

QString WingCStruct::dumpAllTypes() {
    QString ret;
    QTextStream buffer(&ret);
    _parser->dumpAllTypes(buffer);
    return ret;
}

QString WingCStruct::dumpTypeDefines() {
    QString ret;
    QTextStream buffer(&ret);
    _parser->dumpTypeDefines(buffer);
    return ret;
}

QString WingCStruct::dumpConstants() {
    QString ret;
    QTextStream buffer(&ret);
    _parser->dumpConstants(buffer);
    return ret;
}

QString WingCStruct::dumpStructs() {
    QString ret;
    QTextStream buffer(&ret);
    _parser->dumpStructs(buffer);
    return ret;
}

QString WingCStruct::dumpUnions() {
    QString ret;
    QTextStream buffer(&ret);
    _parser->dumpUnions(buffer);
    return ret;
}

QString WingCStruct::dumpEnums() {
    QString ret;
    QTextStream buffer(&ret);
    _parser->dumpEnums(buffer);
    return ret;
}

QStringList WingCStruct::getParsedErrors() { return _errors; }

QStringList WingCStruct::getParsedWarns() { return _warns; }

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
        return QVariant::fromValue(
            getShiftAndMasked<bool>(buffer, shift, mask));
    case QMetaType::Int:
        return QVariant::fromValue(getShiftAndMasked<int>(buffer, shift, mask));
    case QMetaType::UInt:
        return QVariant::fromValue(
            getShiftAndMasked<unsigned int>(buffer, shift, mask));
    case QMetaType::LongLong:
        return QVariant::fromValue(
            getShiftAndMasked<long long>(buffer, shift, mask));
    case QMetaType::ULongLong:
        return QVariant::fromValue(
            getShiftAndMasked<unsigned long long>(buffer, shift, mask));
    case QMetaType::Double:
        return QVariant::fromValue(
            getShiftAndMasked<double>(buffer, shift, mask));
    case QMetaType::Long:
        return QVariant::fromValue(
            getShiftAndMasked<long>(buffer, shift, mask));
    case QMetaType::Short:
        return QVariant::fromValue(
            getShiftAndMasked<short>(buffer, shift, mask));
    case QMetaType::Char:
        return QVariant::fromValue(
            getShiftAndMasked<char>(buffer, shift, mask));
    case QMetaType::Char16:
        return QVariant::fromValue(
            getShiftAndMasked<char16_t>(buffer, shift, mask));
    case QMetaType::Char32:
        return QVariant::fromValue(
            getShiftAndMasked<char32_t>(buffer, shift, mask));
    case QMetaType::ULong:
        return QVariant::fromValue(
            getShiftAndMasked<unsigned long>(buffer, shift, mask));
    case QMetaType::UShort:
        return QVariant::fromValue(
            getShiftAndMasked<unsigned short>(buffer, shift, mask));
    case QMetaType::UChar:
        return QVariant::fromValue(
            getShiftAndMasked<unsigned char>(buffer, shift, mask));
    case QMetaType::Float:
        return QVariant::fromValue(
            getShiftAndMasked<float>(buffer, shift, mask));
    case QMetaType::SChar:
        return QVariant::fromValue(
            getShiftAndMasked<signed char>(buffer, shift, mask));
    case QMetaType::VoidStar:
        return QVariant::fromValue(getShiftAndMaskedPtr(buffer));
    default:
        return {};
    }
}

QVariantHash WingCStruct::readStruct(const char *ptr, const char *end,
                                     const QString &type,
                                     bool efmtInVariantList) {
    if (!_parser->isCompletedStruct(type)) {
        return {};
    }
    auto struc = _parser->structMembers(type);
    QVariantHash content;
    // then slice and parse
    for (auto &m : struc) {
        content.insert(m.var_name,
                       readContent(ptr + m.offset, end, m, efmtInVariantList));
    }
    return content;
}

QVariant WingCStruct::readContent(const char *ptr, const char *end,
                                  const VariableDeclaration &m,
                                  bool efmtInVariantList) {

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
            if (m.element_count > 1) {
                // array with bit field is not allowed
                auto size = _parser->getTypeSize(dt);
                Q_ASSERT(size);
                auto size_v = size.value();

                QVariantList l;
                auto begin = ptr;
                for (qsizetype i = 0; i < m.element_count; ++i) {
                    auto data = getData(begin, end, meta, m.op.shift, m.op.mask,
                                        size_v);
                    if (data.isNull()) {
                        return {};
                    }
                    l.append(data);
                    begin += size_v;
                }
                return l;
            } else {
                qsizetype size;
                if (m.bit_size) {
                    auto div = std::div(m.bit_size, 8);
                    size = div.quot;
                    if (div.rem) {
                        size++;
                    }
                } else {
                    size = m.var_size;
                }
                auto data =
                    getData(ptr, end, meta, m.op.shift, m.op.mask, size);
                if (data.isNull()) {
                    return {};
                }

                if (!m.fmt_type.isEmpty()) {
                    // format will take effects if basic type
                    // is not double or float
                    auto typeID = data.typeId();

                    if (typeID == QMetaType::Float ||
                        typeID == QMetaType::Double ||
                        typeID == QMetaType::Float16) {
                        // warn and ignore format
                        logWarn(QStringLiteral(
                            "Invalid format type on float number"));
                    }

                    QString v;

                    if (efmtInVariantList) {
                        // [0] = int_value, [1] = decoded_enum
                        QVariantList rlist;
                        bool ok;
                        auto var = data.toLongLong(&ok);
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

                        if (!ok) {
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
                        }

                        rlist.append(data);
                        if (v.isEmpty()) {
                            rlist.append(QStringLiteral("<?>"));
                        } else {
                            rlist.append(m.fmt_type + QStringLiteral("::") + v);
                        }
                        data = rlist;
                    } else {
                        CEnumValue rlist;
                        bool ok;
                        auto var = data.toLongLong(&ok);
                        auto evs = _parser->enumMembers(m.fmt_type);

                        for (auto &e : evs) {
                            auto ev = _parser->constVarValue(e);
                            if (std::holds_alternative<qint64>(ev)) {
                                auto rv = std::get<qint64>(ev);
                                if (rv == var) {
                                    rlist.first = rv;
                                    v = e;
                                    break;
                                }
                            } else {
                                auto rv = std::get<quint64>(ev);
                                if (rv == var) {
                                    rlist.first = rv;
                                    v = e;
                                    break;
                                }
                            }
                        }
                        if (!ok) {
                            auto var = data.toULongLong();
                            auto evs = _parser->enumMembers(m.fmt_type);

                            for (auto &e : evs) {
                                auto ev = _parser->constVarValue(e);
                                if (std::holds_alternative<qint64>(ev)) {
                                    auto rv = std::get<qint64>(ev);
                                    if (rv == var) {
                                        rlist.first = rv;
                                        v = e;
                                        break;
                                    }
                                } else {
                                    auto rv = std::get<quint64>(ev);
                                    if (rv == var) {
                                        rlist.first = rv;
                                        v = e;
                                        break;
                                    }
                                }
                            }
                        }

                        if (v.isEmpty()) {
                            rlist.second = QStringLiteral("<?>");
                        } else {
                            rlist.second =
                                m.fmt_type + QStringLiteral("::") + v;
                        }

                        data = QVariant::fromValue(rlist);
                    }
                }
                return data;
            }
        } break;
        case CTypeParser::CType::Enum:
            // should not go there
            Q_ASSERT(false);
            break;
        case CTypeParser::CType::Struct:
            return readStruct(ptr, end, dt, efmtInVariantList);
        case CTypeParser::CType::Union: {
            auto size = _parser->getTypeSize(dt);
            Q_ASSERT(size);
            auto size_v = size.value();
            auto ptrend = ptr + size_v;
            if (ptrend <= end) {
                auto ms = _parser->unionMembers(dt);
                QVariantHash ret;
                for (auto &m : ms) {
                    auto r = readContent(ptr, ptrend, m, efmtInVariantList);
                    ret.insert(m.var_name, r);
                }
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
    case QMetaType::Char16:
    case QMetaType::Char32:
    case QMetaType::ULong:
    case QMetaType::UShort:
    case QMetaType::UChar:
    case QMetaType::Float:
    case QMetaType::SChar:
        return true;
    default:
        if (QMetaType::fromType<CEnumValue>().id() == int(type)) {
            return true;
        }
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
        auto type = var.isNull() ? QMetaType::Type::Void
                                 : QMetaType::Type(var.typeId());
        switch (type) {
        case QMetaType::Bool: {
            auto v = var.toBool();
            dic->Set(p->first, &v, asTYPEID_BOOL);
            break;
        }
        case QMetaType::UChar: {
            auto v = var.value<uchar>();
            dic->Set(p->first, &v, asTYPEID_UINT8);
            break;
        }
        case QMetaType::SChar: {
            auto v = var.value<uchar>();
            dic->Set(p->first, &v, asTYPEID_INT8);
            break;
        }
        case QMetaType::Short: {
            auto v = var.value<short>();
            static_assert(sizeof(short) == 2, "sizeof(short) != 2");
            dic->Set(p->first, &v, asTYPEID_INT16);
            break;
        }
        case QMetaType::UShort: {
            auto v = var.value<ushort>();
            static_assert(sizeof(short) == 2, "sizeof(short) != 2");
            dic->Set(p->first, &v, asTYPEID_UINT16);
            break;
        }
        case QMetaType::Int:
        case QMetaType::Long: {
            auto v = var.toInt();
            dic->Set(p->first, &v, asTYPEID_INT32);
        }
        case QMetaType::LongLong: {
            auto v = var.toLongLong();
            dic->Set(p->first, &v, asTYPEID_INT64);
            break;
        }
        case QMetaType::UInt:
        case QMetaType::ULong: {
            auto v = var.toUInt();
            dic->Set(p->first, &v, asTYPEID_UINT32);
            break;
        }
        case QMetaType::ULongLong: {
            auto v = var.toULongLong();
            dic->Set(p->first, asQWORD(v));
            break;
        }
        case QMetaType::Double:
            dic->Set(p->first, var.toDouble());
            break;
        case QMetaType::Float: {
            auto v = var.toFloat();
            dic->Set(p->first, &v, asTYPEID_FLOAT);
            break;
        }
        case QMetaType::Char: {
            auto v = var.value<char>();
            QChar ch(v);
            auto id = engine->GetTypeIdByDecl("char");
            dic->Set(p->first, &ch, id);
            break;
        }
        case QMetaType::Char16: {
            auto v = var.value<char16_t>();
            QChar ch(v);
            auto id = engine->GetTypeIdByDecl("char");
            dic->Set(p->first, &ch, id);
            break;
        }
        case QMetaType::Char32: {
            auto v = var.value<char32_t>();
            QChar ch(v);
            auto id = engine->GetTypeIdByDecl("char");
            dic->Set(p->first, &ch, id);
            break;
        }
        case QMetaType::QVariantList: {
            // note: empty list is not allowed!
            // If empty, it will be ignored
            auto v = var.toList();
            if (!v.isEmpty()) {
                // reguard the first element type is the specilization
                auto var = v.first();
                auto type = var.isNull() ? QMetaType::Type::Void
                                         : QMetaType::Type(var.typeId());
                if (type == QMetaType::Type::Void) {
                    // ignore
                    break;
                }
                if (!isValidCStructMetaType(type)) {
                    // ignore
                    break;
                }

                QString idStr;

                if (QMetaType::fromType<CEnumValue>().id() == int(type)) {
                    idStr = QStringLiteral("WingCStruct::EnumValue");
                } else if (QMetaType::fromType<CINT_TYPE>().id() == int(type)) {
                    idStr = QStringLiteral("WingCStruct::IntType");
                } else {
                    idStr = WingAngelAPI::qvariantCastASString(type);
                }

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
            if (QMetaType::fromType<CEnumValue>().id() == int(type)) {
                auto id = engine->GetTypeIdByDecl("WingCStruct::EnumValue");
                Q_ASSERT(id >= 0);
                auto v = var.value<CEnumValue>();
                dic->Set(p->first, &v, id);
            } else if (QMetaType::fromType<CINT_TYPE>().id() == int(type)) {
                auto id = engine->GetTypeIdByDecl("WingCStruct::IntType");
                Q_ASSERT(id >= 0);
                auto v = var.value<CEnumValue>();
                dic->Set(p->first, &v, id);
            }
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
            WingAngelAPI::assignTmpBuffer(buffer, var.value<short>());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::UShort:
            WingAngelAPI::assignTmpBuffer(buffer, var.value<ushort>());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::Int:
            WingAngelAPI::assignTmpBuffer(buffer, var.toInt());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::UInt:
            WingAngelAPI::assignTmpBuffer(buffer, var.toUInt());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::Long:
        case QMetaType::LongLong:
            WingAngelAPI::assignTmpBuffer(buffer, var.toLongLong());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::ULong:
        case QMetaType::ULongLong:
            WingAngelAPI::assignTmpBuffer(buffer, var.toULongLong());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::Float:
            WingAngelAPI::assignTmpBuffer(buffer, var.toFloat());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::Double:
            WingAngelAPI::assignTmpBuffer(buffer, var.toDouble());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::Char16: {
            QChar ch(var.value<char16_t>());
            arr->SetValue(i, &ch);
        } break;
        case QMetaType::Char32: {
            QChar ch(var.value<char32_t>());
            arr->SetValue(i, &ch);
        } break;
        case QMetaType::Char: {
            QChar ch(var.toChar());
            arr->SetValue(i, &ch);
        } break;
        case QMetaType::UChar:
        case QMetaType::SChar:
            WingAngelAPI::assignTmpBuffer(buffer, var.value<uchar>());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::QVariantHash:
            // struct
            arr->SetValue(i, convert2AsDictionary(var.toHash()));
        default:
            if (QMetaType::fromType<CEnumValue>().id() == int(type)) {
                auto v = var.value<CEnumValue>();
                arr->SetValue(i, &v);
            }
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

    auto ret = __read(offset, type, false);
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

QVariant WingCStruct::getParsedErrors(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    return getParsedErrors();
}

QVariant WingCStruct::getParsedWarns(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    return getParsedWarns();
}

bool WingCStruct::isLittleEndian() const { return m_islittle; }

void WingCStruct::setIsLittleEndian(bool newIslittle) {
    m_islittle = newIslittle;
}
