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

#include "scriptaddon/scriptany.h"

#include "WingPlugin/iwingangel.h"
#include "angelscripthelper.h"
#include "define.h"
#include "utilities.h"
#include "wingangelapi.h"

class ScriptStructParser : public StructParser {
public:
    ScriptStructParser(WingCStruct *api) : StructParser(api), m_ref(1) {}
    ~ScriptStructParser() = default;

    void AddRef() { ++m_ref; }
    void Release() {
        if (--m_ref == 0) {
            this->~ScriptStructParser();
            asFreeMem(const_cast<ScriptStructParser *>(this));
        }
    }

    bool parse(const QString &fileName) {
        auto ctx = asGetActiveContext();
        if (ctx == nullptr) {
            return false;
        }

        QString path;
        auto ptr = static_cast<const QString *>(
            ctx->GetUserData(AsUserDataType::UserData_Section_StringPtr));
        if (ptr) {
            QFileInfo sinfo(*ptr);
            QFileInfo finfo(sinfo.absoluteDir(), fileName);
            path = finfo.absoluteFilePath();
        } else {
            path = fileName;
        }

        return StructParser::parse(path);
    }

public:
    CScriptArray *structTypeDefs() {
        return stringListWrapper(StructParser::structTypeDefs());
    }
    CScriptArray *unionTypeDefs() {
        return stringListWrapper(StructParser::unionTypeDefs());
    }
    CScriptArray *typedefTypeDefs() {
        return stringListWrapper(StructParser::typedefTypeDefs());
    }
    CScriptArray *enumTypeDefs() {
        return stringListWrapper(StructParser::enumTypeDefs());
    }
    CScriptArray *constVarDefs() {
        return stringListWrapper(StructParser::constVarDefs());
    }

    CScriptArray *enumValueNames(const QString &name) {
        return stringListWrapper(StructParser::enumValueNames(name));
    }

    CScriptArray *getMissingDependencise(const QString &name) {
        return stringListWrapper(StructParser::getMissingDependencise(name));
    }

    CScriptArray *getParsedErrors() const {
        return stringListWrapper(StructParser::getParsedErrors());
    }
    CScriptArray *getParsedWarns() const {
        return stringListWrapper(StructParser::getParsedWarns());
    }

    CScriptArray *structOrUnionMemberNames(const QString &type) const {
        return stringListWrapper(StructParser::structOrUnionMemberNames(type));
    }

    CScriptArray *structOrUnionMemberDataTypes(const QString &type) const {
        return stringListWrapper(
            StructParser::structOrUnionMemberDataTypes(type));
    }

    CScriptArray *structOrUnionMemberDecls(const QString &type) const {
        return stringListWrapper(StructParser::structOrUnionMemberDecls(type));
    }

    CScriptArray *
    structOrUnionMemberDeclWithoutNames(const QString &type) const {
        return stringListWrapper(
            StructParser::structOrUnionMemberDeclWithoutNames(type));
    }

    CScriptDictionary *read(qsizetype offset, const QString &type) {
        return convert2AsDictionary(__read(offset, type, true));
    }

    CScriptDictionary *readMembers(qsizetype offset, const QString &type,
                                   CScriptArray *members) {
        return convert2AsDictionary(__readMembers(
            offset, type,
            [members](uint index) -> QString {
                return *reinterpret_cast<const QString *>(members->At(index));
            },
            members->GetSize(), true));
    }

    CScriptAny *readMember(qsizetype offset, const QString &type,
                           const QString &member) {
        auto ctx = asGetActiveContext();
        if (ctx == nullptr) {
            return {};
        }

        auto ret = __readMember(offset, type, member, false);
        auto engine = ctx->GetEngine();
        auto any = new CScriptAny(engine);

        auto meta = ret.isNull() ? QMetaType::Type::Void
                                 : QMetaType::Type(ret.typeId());

        switch (meta) {
        case QMetaType::Bool: {
            auto v = ret.toBool();
            any->Store(&v, asTYPEID_BOOL);
            break;
        }
        case QMetaType::UChar: {
            auto v = ret.value<uchar>();
            any->Store(&v, asTYPEID_UINT8);
            break;
        }
        case QMetaType::SChar: {
            auto v = ret.value<uchar>();
            any->Store(&v, asTYPEID_INT8);
            break;
        }
        case QMetaType::Short: {
            auto v = ret.value<short>();
            static_assert(sizeof(short) == 2, "sizeof(short) != 2");
            any->Store(&v, asTYPEID_INT16);
            break;
        }
        case QMetaType::UShort: {
            auto v = ret.value<ushort>();
            static_assert(sizeof(short) == 2, "sizeof(short) != 2");
            any->Store(&v, asTYPEID_UINT16);
            break;
        }
        case QMetaType::Int:
        case QMetaType::Long: {
            auto v = ret.toInt();
            any->Store(&v, asTYPEID_INT32);
        }
        case QMetaType::LongLong: {
            auto v = ret.toLongLong();
            any->Store(&v, asTYPEID_INT64);
            break;
        }
        case QMetaType::UInt:
        case QMetaType::ULong: {
            auto v = ret.toUInt();
            any->Store(&v, asTYPEID_UINT32);
            break;
        }
        case QMetaType::ULongLong: {
            auto v = ret.toULongLong();
            any->Store(&v, asTYPEID_UINT64);
            break;
        }
        case QMetaType::Double: {
            auto v = ret.toDouble();
            any->Store(&v, asTYPEID_DOUBLE);
            break;
        }
        case QMetaType::Float: {
            auto v = ret.toFloat();
            any->Store(&v, asTYPEID_FLOAT);
            break;
        }
        case QMetaType::Char: {
            auto v = ret.value<char>();
            QChar ch(v);
            auto type = static_cast<asITypeInfo *>(
                engine->GetUserData(AsUserDataType::UserData_CharTypeInfo));
            auto id = type->GetTypeId();
            any->Store(&ch, id);
            break;
        }
        case QMetaType::Char16: {
            auto v = ret.value<char16_t>();
            QChar ch(v);
            auto type = static_cast<asITypeInfo *>(
                engine->GetUserData(AsUserDataType::UserData_CharTypeInfo));
            auto id = type->GetTypeId();
            any->Store(&ch, id);
            break;
        }
        case QMetaType::Char32: {
            auto v = ret.value<char32_t>();
            QChar ch(v);
            auto type = static_cast<asITypeInfo *>(
                engine->GetUserData(AsUserDataType::UserData_CharTypeInfo));
            auto id = type->GetTypeId();
            any->Store(&ch, id);
            break;
        }
        case QMetaType::QVariantList: {
            // note: empty list is not allowed!
            // If empty, it will be ignored
            auto v = ret.toList();
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
                QString arrType =
                    QStringLiteral("array<") + idStr + QStringLiteral(">");
                auto arrTypeID = engine->GetTypeIdByDecl(arrType.toUtf8());
                if (arrTypeID < 0) {
                    // ignore
                    break;
                }

                any->Store(convert2AsArray(v, type, arrTypeID), arrTypeID);
            }
            break;
        }
        case QMetaType::QVariantHash: {
            auto type = static_cast<asITypeInfo *>(engine->GetUserData(
                AsUserDataType::UserData_DictionaryTypeInfo));
            auto id = type->GetTypeId();
            any->Store(convert2AsDictionary(ret.toHash()), id);
            break;
        }
        default:
            if (QMetaType::fromType<CEnumValue>().id() == int(meta)) {
                static auto type =
                    engine->GetTypeInfoByName("WingCStruct::EnumValue");
                Q_ASSERT(type);
                if (type) {
                    auto id = type->GetTypeId();
                    Q_ASSERT(id >= 0);
                    auto v = ret.value<CEnumValue>();
                    any->Store(&v, id);
                }
            } else if (QMetaType::fromType<CINT_TYPE>().id() == int(meta)) {
                static auto type =
                    engine->GetTypeInfoByName("WingCStruct::IntType");
                Q_ASSERT(type);
                if (type) {
                    auto id = type->GetTypeId();
                    Q_ASSERT(id >= 0);
                    auto v = ret.value<CEnumValue>();
                    any->Store(&v, id);
                }
            }
            break;
        }
        return any;
    }

    CScriptArray *readRaw(qsizetype offset, const QString &type) {
        return byteArrayWrapper(__readRaw(offset, type));
    }

private:
    int m_ref;
};

WING_DECLARE_STATIC_API;

static void ScriptStructParser_ConstVarValue(WingHex::asIWingGeneric *gen) {
    auto g = createParamContext(gen);
    auto self = static_cast<ScriptStructParser *>(g->object());
    auto param = static_cast<QString *>(g->argObject(0));
    *(CINT_TYPE *)g->addressOfReturnLocation() = self->constVarValue(*param);
};

static WingCStruct *self = nullptr;

static void ScriptStructParser_Factory(WingHex::asIWingGeneric *gen) {
    auto g = createParamContext(gen);
    auto mem = static_cast<ScriptStructParser *>(
        asAllocMem(sizeof(ScriptStructParser)));

    *(ScriptStructParser **)g->addressOfReturnLocation() =
        new (mem) ScriptStructParser(self);
};

static void ScriptStructParser_String_Factory(WingHex::asIWingGeneric *gen) {
    auto g = createParamContext(gen);
    auto param = static_cast<QString *>(g->argObject(0));
    auto mem = static_cast<ScriptStructParser *>(
        asAllocMem(sizeof(ScriptStructParser)));

    *(ScriptStructParser **)g->addressOfReturnLocation() =
        new (mem) ScriptStructParser(self);

    mem->parse(*param);
};

WingCStruct::WingCStruct() : WingHex::IWingPlugin() {
    self = this;
    WING_INIT_STATIC_API;
}

WingCStruct::~WingCStruct() {
    qDeleteAll(_parsers);
    _parsers.clear();
}

bool WingCStruct::init(const std::unique_ptr<QSettings> &set) {
    Q_UNUSED(set);
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

QList<WingHex::SettingPage *> WingCStruct::registeredSettingPages() const {
    return _setpgs;
}

void WingCStruct::onRegisterScriptObj(WingHex::IWingAngel *o) {
    using asCallConvTypes = WingHex::IWingAngel::asCallConvTypes;

    o->registerObjectType("IntType", sizeof(CINT_TYPE),
                          WingHex::IWingAngel::asOBJ_VALUE |
                              WingHex::asGetTypeTraits<CINT_TYPE>());
    o->registerObjectBehaviour(
        "IntType", WingHex::IWingAngel::asBEHAVE_CONSTRUCT, "void f()",
        asWINGFUNCTIONPR([](void *memory) { new (memory) CINT_TYPE(); },
                         (void *), void),
        asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectBehaviour(
        "IntType", WingHex::IWingAngel::asBEHAVE_CONSTRUCT, "void f(int64 v)",
        asWINGFUNCTIONPR(
            [](void *memory, qint64 v) { new (memory) CINT_TYPE(v); },
            (void *, qint64), void),
        asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectBehaviour("IntType",
                               WingHex::IWingAngel::asBEHAVE_CONSTRUCT,
                               "void f(const IntType &in)",
                               asWINGFUNCTIONPR(
                                   [](CINT_TYPE *self, const CINT_TYPE &other) {
                                       new (self) CINT_TYPE(other);
                                   },
                                   (CINT_TYPE *, const CINT_TYPE &), void),
                               asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectBehaviour(
        "IntType", WingHex::IWingAngel::asBEHAVE_CONSTRUCT, "void f(uint64 v)",
        asWINGFUNCTIONPR(
            [](void *memory, quint64 v) { new (memory) CINT_TYPE(v); },
            (void *, quint64), void),
        asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectBehaviour(
        "IntType", WingHex::IWingAngel::asBEHAVE_DESTRUCT, "void f()",
        asWINGFUNCTIONPR([](CINT_TYPE *memory) { memory->~CINT_TYPE(); },
                         (CINT_TYPE *), void),
        asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectMethod("IntType", "bool isInt() const",
                            asWINGMETHOD(CINT_TYPE, isInt),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("IntType", "bool isUInt() const",
                            asWINGMETHOD(CINT_TYPE, isUInt),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("IntType", "int64 toInt() const",
                            asWINGMETHOD(CINT_TYPE, toInt),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("IntType", "uint64 toUInt() const",
                            asWINGMETHOD(CINT_TYPE, toUInt),
                            asCallConvTypes::asCALL_THISCALL);

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

    o->registerObjectType("EnumValue", sizeof(CEnumValue),
                          WingHex::IWingAngel::asOBJ_VALUE |
                              WingHex::asGetTypeTraits<CEnumValue>());
    o->registerObjectBehaviour(
        "EnumValue", WingHex::IWingAngel::asBEHAVE_CONSTRUCT,
        "void f(const IntType &in, const string &in)",
        asWINGFUNCTIONPR(
            [](void *memory, const CINT_TYPE &k, const QString &v) {
                new (memory) CEnumValue(k, v);
            },
            (void *, const CINT_TYPE &, const QString &), void),
        asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectBehaviour(
        "EnumValue", WingHex::IWingAngel::asBEHAVE_CONSTRUCT,
        "void f(const EnumValue &in)",
        asWINGFUNCTIONPR(
            [](CEnumValue *self, const CEnumValue &other) {
                new (self) CEnumValue(other);
            },
            (CEnumValue *, const CEnumValue &), void),
        asCallConvTypes::asCALL_CDECL_OBJFIRST);
    o->registerObjectBehaviour(
        "EnumValue", WingHex::IWingAngel::asBEHAVE_DESTRUCT, "void f()",
        asWINGFUNCTIONPR([](CEnumValue *memory) { memory->~CEnumValue(); },
                         (CEnumValue *), void),
        asCallConvTypes::asCALL_CDECL_OBJFIRST);
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

                r.value = QLatin1String("(%1, \"%2\")").arg(intv, v->second);
                r.expandable = true;
                return r;
            } break;
            case WingHex::IWingAngel::EvalMode::Expand: {
                QHash<QString, WingHex::IWingAngel::ExpandResult> r;

                WingHex::IWingAngel::ExpandResult value;
                value.type = QLatin1String("WingCStruct::IntType");
                value.buffer = &v->first;
                r.insert(QStringLiteral("value"), value);

                WingHex::IWingAngel::ExpandResult name;
                name.type = QLatin1String("string");
                name.buffer = &v->second;
                r.insert(QLatin1String("name"), name);

                return r;
            } break;
            }

            return {};
        });

    o->registerObjectType("parser", 0, WingHex::IWingAngel::asOBJ_REF);
    o->registerObjectBehaviour("parser", WingHex::IWingAngel::asBEHAVE_ADDREF,
                               "void f()",
                               asWINGMETHOD(ScriptStructParser, AddRef),
                               asCallConvTypes::asCALL_THISCALL);
    o->registerObjectBehaviour("parser", WingHex::IWingAngel::asBEHAVE_RELEASE,
                               "void f()",
                               asWINGMETHOD(ScriptStructParser, Release),
                               asCallConvTypes::asCALL_THISCALL);
    o->registerObjectBehaviour("parser", WingHex::IWingAngel::asBEHAVE_FACTORY,
                               "parser@ f()",
                               asWINGFUNCTION(ScriptStructParser_Factory),
                               asCallConvTypes::asCALL_GENERIC);
    o->registerObjectBehaviour(
        "parser", WingHex::IWingAngel::asBEHAVE_FACTORY,
        "parser@ f(const string&in)",
        asWINGFUNCTION(ScriptStructParser_String_Factory),
        asCallConvTypes::asCALL_GENERIC);
    o->registerObjectMethod("parser",
                            "bool parseFromSource(const string&in header)",
                            asWINGMETHOD(ScriptStructParser, parseFromSource),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "bool parse(const string&in fileName)",
                            asWINGMETHOD(ScriptStructParser, parse),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "void setIsLittleEndian(bool)",
                            asWINGMETHOD(ScriptStructParser, setIsLittleEndian),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "bool isLittleEndian() const",
                            asWINGMETHOD(ScriptStructParser, isLittleEndian),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "void reset()",
                            asWINGMETHOD(ScriptStructParser, reset),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "bool setPadAlignment(int)",
                            asWINGMETHOD(ScriptStructParser, setPadAlignment),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "int padAlignment() const",
                            asWINGMETHOD(ScriptStructParser, padAlignment),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "string[]@ structTypeDefs() const",
                            asWINGMETHOD(ScriptStructParser, structTypeDefs),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "string[]@ unionTypeDefs() const",
                            asWINGMETHOD(ScriptStructParser, unionTypeDefs),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "string[]@ typedefTypeDefs() const",
                            asWINGMETHOD(ScriptStructParser, typedefTypeDefs),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "string[]@ enumTypeDefs() const",
                            asWINGMETHOD(ScriptStructParser, enumTypeDefs),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "string[]@ constVarDefs() const",
                            asWINGMETHOD(ScriptStructParser, constVarDefs),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser",
                            "int64 sizeOf(const string&in type) const",
                            asWINGMETHOD(ScriptStructParser, sizeOf),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser",
                            "bool containsType(const string&in type) const",
                            asWINGMETHOD(ScriptStructParser, containsType),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser",
                            "bool isBasicType(const string&in type) const",
                            asWINGMETHOD(ScriptStructParser, isBasicType),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "parser", "bool isUnsignedBasicType(const string&in type) const",
        asWINGMETHOD(ScriptStructParser, isUnsignedBasicType),
        asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser",
                            "bool containsEnum(const string&in type) const",
                            asWINGMETHOD(ScriptStructParser, containsEnum),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser",
                            "bool containsStruct(const string&in type) const",
                            asWINGMETHOD(ScriptStructParser, containsStruct),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser",
                            "bool containsUnion(const string&in type) const",
                            asWINGMETHOD(ScriptStructParser, containsUnion),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser",
                            "bool containsTypeDef(const string&in type) const",
                            asWINGMETHOD(ScriptStructParser, containsTypeDef),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser",
                            "bool containsConstVar(const string&in type) const",
                            asWINGMETHOD(ScriptStructParser, containsConstVar),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser",
                            "bool isCompletedType(const string&in type) const",
                            asWINGMETHOD(ScriptStructParser, isCompletedType),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "parser", "string[]@ enumValueNames(const string&in type) const",
        asWINGMETHOD(ScriptStructParser, enumValueNames),
        asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "parser", "bool isCompletedStruct(const string&in type) const",
        asWINGMETHOD(ScriptStructParser, isCompletedStruct),
        asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser",
                            "bool isCompletedUnion(const string&in type) const",
                            asWINGMETHOD(ScriptStructParser, isCompletedUnion),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "parser",
        "string[]@ getMissingDependencise(const string&in type) const",
        asWINGMETHOD(ScriptStructParser, getMissingDependencise),
        asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser",
                            "int64 constVarValueInt(const string &in type, "
                            "bool &out ok = void) const",
                            asWINGMETHOD(ScriptStructParser, constVarValueInt),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser",
                            "uint64 constVarValueUInt(const string &in type, "
                            "bool &out ok = void) const",
                            asWINGMETHOD(ScriptStructParser, constVarValueUInt),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "parser",
        "WingCStruct::IntType constVarValue(const string &in type) const",
        asWINGFUNCTION(ScriptStructParser_ConstVarValue),
        asCallConvTypes::asCALL_GENERIC);

    static_assert(sizeof(qsizetype) == sizeof(quint64));

    o->registerObjectMethod(
        "parser", "dictionary@ read(int64 offset, const string &in type) const",
        asWINGMETHOD(ScriptStructParser, read),
        asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser",
                            "any@ readMember(int64 offset, const string &in "
                            "type, const string &in member) const",
                            asWINGMETHOD(ScriptStructParser, readMember),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "parser",
        "dictionary@ readMembers(int64 offset, const string &in type, "
        "const string[] &in members) const",
        asWINGMETHOD(ScriptStructParser, readMembers),
        asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "parser", "byte[]@ readRaw(int64 offset, const string &in type) const",
        asWINGMETHOD(ScriptStructParser, readRaw),
        asCallConvTypes::asCALL_THISCALL);

    o->registerObjectMethod("parser", "string dumpAllTypes() const",
                            asWINGMETHOD(ScriptStructParser, dumpAllTypes),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "string dumpTypeDefines() const",
                            asWINGMETHOD(ScriptStructParser, dumpTypeDefines),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "string dumpConstants() const",
                            asWINGMETHOD(ScriptStructParser, dumpConstants),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "string dumpStructs()",
                            asWINGMETHOD(ScriptStructParser, dumpStructs),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "string dumpUnions() const",
                            asWINGMETHOD(ScriptStructParser, dumpUnions),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "string dumpEnums() const",
                            asWINGMETHOD(ScriptStructParser, dumpEnums),
                            asCallConvTypes::asCALL_THISCALL);

    o->registerObjectMethod("parser", "string[]@ getParsedErrors() const",
                            asWINGMETHOD(ScriptStructParser, getParsedErrors),
                            asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod("parser", "string[]@ getParsedWarns() const",
                            asWINGMETHOD(ScriptStructParser, getParsedWarns),
                            asCallConvTypes::asCALL_THISCALL);

    o->registerObjectMethod(
        "parser",
        "string[]@ structOrUnionMemberNames(const string&in type) const",
        asWINGMETHOD(ScriptStructParser, structOrUnionMemberNames),
        asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "parser",
        "string[]@ structOrUnionMemberDataTypes(const string&in type) const",
        asWINGMETHOD(ScriptStructParser, structOrUnionMemberDataTypes),
        asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "parser",
        "string[]@ structOrUnionMemberDecls(const string&in type) const",
        asWINGMETHOD(ScriptStructParser, structOrUnionMemberDecls),
        asCallConvTypes::asCALL_THISCALL);
    o->registerObjectMethod(
        "parser",
        "string[]@ structOrUnionMemberDeclWithoutNames(const string&in type) "
        "const",
        asWINGMETHOD(ScriptStructParser, structOrUnionMemberDeclWithoutNames),
        asCallConvTypes::asCALL_THISCALL);

    o->registerObjectMethod(
        "parser", "string resolveTypeName(const string&in type) const",
        asWINGMETHOD(ScriptStructParser, resolveTypeName),
        asCallConvTypes::asCALL_THISCALL);
}

bool WingCStruct::createParser(const WingHex::SenderInfo &sender) {
    if (_parsers.contains(sender.puid)) {
        return false;
    }
    _parsers.insert(sender.puid, new StructParser(this));
    return true;
}

void WingCStruct::destroyParser(const WingHex::SenderInfo &sender) {
    _parsers.remove(sender.puid);
}

bool WingCStruct::parseFromSource(const WingHex::SenderInfo &sender,
                                  const QString &header) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    return parser->parseFromSource(header);
}

bool WingCStruct::parse(const WingHex::SenderInfo &sender,
                        const QString &fileName) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    return parser->parse(fileName);
}

bool WingCStruct::reset(const WingHex::SenderInfo &sender) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    parser->reset();
    return true;
}

bool WingCStruct::isLittleEndian(const WingHex::SenderInfo &sender) const {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    return parser->isLittleEndian();
}

bool WingCStruct::setIsLittleEndian(const WingHex::SenderInfo &sender,
                                    bool newIslittle) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    parser->setIsLittleEndian(newIslittle);
    return true;
}

bool WingCStruct::setPadAlignment(const WingHex::SenderInfo &sender,
                                  int padding) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    return parser->setPadAlignment(padding);
}

int WingCStruct::padAlignment(const WingHex::SenderInfo &sender) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return 0;
    }
    return parser->padAlignment();
}

QStringList WingCStruct::structTypeDefs(const WingHex::SenderInfo &sender) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }
    return parser->structTypeDefs();
}

QStringList WingCStruct::unionTypeDefs(const WingHex::SenderInfo &sender) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }
    return parser->unionTypeDefs();
}

QStringList WingCStruct::typedefTypeDefs(const WingHex::SenderInfo &sender) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }
    return parser->typedefTypeDefs();
}

QStringList WingCStruct::enumTypeDefs(const WingHex::SenderInfo &sender) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }
    return parser->enumTypeDefs();
}

QStringList WingCStruct::constVarDefs(const WingHex::SenderInfo &sender) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }
    return parser->constVarDefs();
}

qint64 WingCStruct::sizeOf(const WingHex::SenderInfo &sender,
                           const QString &type) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return 0;
    }
    return parser->sizeOf(type);
}

bool WingCStruct::containsType(const WingHex::SenderInfo &sender,
                               const QString &name) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    return parser->containsType(name);
}

bool WingCStruct::isBasicType(const WingHex::SenderInfo &sender,
                              const QString &name) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    return parser->isBasicType(name);
}

bool WingCStruct::isUnsignedBasicType(const WingHex::SenderInfo &sender,
                                      const QString &name) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    return parser->isUnsignedBasicType(name);
}

bool WingCStruct::containsEnum(const WingHex::SenderInfo &sender,
                               const QString &name) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    return parser->containsEnum(name);
}

bool WingCStruct::containsStruct(const WingHex::SenderInfo &sender,
                                 const QString &name) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    return parser->containsStruct(name);
}

bool WingCStruct::containsUnion(const WingHex::SenderInfo &sender,
                                const QString &name) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    return parser->containsUnion(name);
}

bool WingCStruct::containsTypeDef(const WingHex::SenderInfo &sender,
                                  const QString &name) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    return parser->containsTypeDef(name);
}

bool WingCStruct::containsConstVar(const WingHex::SenderInfo &sender,
                                   const QString &name) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    return parser->containsConstVar(name);
}

bool WingCStruct::isCompletedType(const WingHex::SenderInfo &sender,
                                  const QString &name) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    return parser->isCompletedType(name);
}

QStringList WingCStruct::enumValueNames(const WingHex::SenderInfo &sender,
                                        const QString &name) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }
    return parser->enumValueNames(name);
}

qint64 WingCStruct::constVarValueInt(const WingHex::SenderInfo &sender,
                                     const QString &name, bool *ok) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return 0;
    }
    return parser->constVarValueInt(name, ok);
}

quint64 WingCStruct::constVarValueUInt(const WingHex::SenderInfo &sender,
                                       const QString &name, bool *ok) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return 0;
    }
    return parser->constVarValueUInt(name, ok);
}

bool WingCStruct::isCompletedStruct(const WingHex::SenderInfo &sender,
                                    const QString &name) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    return parser->isCompletedStruct(name);
}

bool WingCStruct::isCompletedUnion(const WingHex::SenderInfo &sender,
                                   const QString &name) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return false;
    }
    return parser->isCompletedUnion(name);
}

QStringList
WingCStruct::getMissingDependencise(const WingHex::SenderInfo &sender,
                                    const QString &name) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }
    return parser->getMissingDependencise(name);
}

QVariantHash WingCStruct::read(const WingHex::SenderInfo &sender,
                               qsizetype offset, const QString &type) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    return parser->__read(offset, type, true);
}

QVariant WingCStruct::readMember(const WingHex::SenderInfo &sender,
                                 qsizetype offset, const QString &type,
                                 const QString &member) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }
    return parser->__readMember(offset, type, member, true);
}

QVariantHash WingCStruct::readMembers(const WingHex::SenderInfo &sender,
                                      qsizetype offset, const QString &type,
                                      const QStringList &members) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    return parser->__readMembers(
        offset, type,
        [members](uint index) -> QString { return members.at(index); },
        members.size(), true);
}

QByteArray WingCStruct::readRaw(const WingHex::SenderInfo &sender,
                                qsizetype offset, const QString &type) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    if (!isCurrentDocEditing()) {
        return {};
    }

    if (offset < 0) {
        return {};
    }

    auto rtype = parser->resolveTypeName(type);
    auto len = parser->sizeOf(rtype);
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

    return readBytes(offset, len);
}

QString WingCStruct::dumpAllTypes(const WingHex::SenderInfo &sender) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    return parser->dumpAllTypes();
}

QString WingCStruct::dumpTypeDefines(const WingHex::SenderInfo &sender) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    return parser->dumpTypeDefines();
}

QString WingCStruct::dumpConstants(const WingHex::SenderInfo &sender) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    return parser->dumpConstants();
}

QString WingCStruct::dumpStructs(const WingHex::SenderInfo &sender) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    return parser->dumpStructs();
}

QString WingCStruct::dumpUnions(const WingHex::SenderInfo &sender) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    return parser->dumpUnions();
}

QString WingCStruct::dumpEnums(const WingHex::SenderInfo &sender) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    return parser->dumpEnums();
}

QStringList WingCStruct::getParsedErrors(const WingHex::SenderInfo &sender) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    return parser->getParsedErrors();
}

QStringList WingCStruct::getParsedWarns(const WingHex::SenderInfo &sender) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    return parser->getParsedWarns();
}

QStringList
WingCStruct::structOrUnionMemberNames(const WingHex::SenderInfo &sender,
                                      const QString &type) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    return parser->structOrUnionMemberNames(type);
}

QStringList
WingCStruct::structOrUnionMemberDataTypes(const WingHex::SenderInfo &sender,
                                          const QString &type) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    return parser->structOrUnionMemberDataTypes(type);
}

QStringList
WingCStruct::structOrUnionMemberDecls(const WingHex::SenderInfo &sender,
                                      const QString &type) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    return parser->structOrUnionMemberDecls(type);
}

QStringList WingCStruct::structOrUnionMemberDeclWithoutNames(
    const WingHex::SenderInfo &sender, const QString &type) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    return parser->structOrUnionMemberDeclWithoutNames(type);
}

QString WingCStruct::resolveTypeName(const WingHex::SenderInfo &sender,
                                     const QString &type) {
    auto parser = _parsers.value(sender.puid, nullptr);
    if (!parser) {
        return {};
    }

    return parser->resolveTypeName(type);
}

WingHex::MetaType WingCStruct::getqsizetypeMetaType() const {
    return sizeof(qsizetype) == sizeof(quint64) ? WingHex::MetaType::Meta_Int64
                                                : WingHex::MetaType::Meta_Int32;
}

QString WingCStruct::getqsizeTypeAsString() const {
    return sizeof(qsizetype) == sizeof(quint64) ? QStringLiteral("int64")
                                                : QStringLiteral("int32");
}
