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

#include "wingangel.h"
#include "define.h"
#include "wingangelapi.h"

#include "logger.h"
#include "scriptmachine.h"
#include "utilities.h"

inline asSFuncPtr asCSFuncPtr(const WingHex::asFuncPtr &ptr) {
    // some checks
    static_assert(std::is_standard_layout_v<asSFuncPtr>,
                  "asSFuncPtr must be standard layout");
    static_assert(std::is_standard_layout_v<WingHex::asFuncPtr>,
                  "WingHex::asFuncPtr must be standard layout");
    static_assert(sizeof(asSFuncPtr) == sizeof(WingHex::asFuncPtr),
                  "asSFuncPtr is not same as WingHex::asFuncPtr");
    static_assert(alignof(asSFuncPtr) == alignof(WingHex::asFuncPtr),
                  "asSFuncPtr is not same as WingHex::asFuncPtr");
    static_assert(offsetof(asSFuncPtr, ptr) ==
                      offsetof(WingHex::asFuncPtr, ptr),
                  "asSFuncPtr is not same as WingHex::asFuncPtr");
    static_assert(offsetof(asSFuncPtr, flag) ==
                      offsetof(WingHex::asFuncPtr, flag),
                  "asSFuncPtr is not same as WingHex::asFuncPtr");
    static_assert(sizeof(asSFuncPtr::ptr) == sizeof(WingHex::asFuncPtr::ptr),
                  "asSFuncPtr is not same as WingHex::asFuncPtr");
    static_assert(std::is_same_v<decltype(asSFuncPtr::flag),
                                 decltype(WingHex::asFuncPtr::flag)>,
                  "asSFuncPtr is not same as WingHex::asFuncPtr");
    static_assert(offsetof(asSFuncPtr, flag) ==
                      offsetof(WingHex::asFuncPtr, flag),
                  "asSFuncPtr is not same as WingHex::asFuncPtr");

    using StructM = decltype(std::declval<asSFuncPtr>().ptr.m);
    using StructF = decltype(std::declval<asSFuncPtr>().ptr.f);

    using wStructM = decltype(std::declval<WingHex::asFuncPtr>().ptr.m);
    using wStructF = decltype(std::declval<WingHex::asFuncPtr>().ptr.f);

    static_assert(offsetof(StructM, mthd) == offsetof(wStructM, mthd),
                  "asSFuncPtr is not same as WingHex::asFuncPtr");
    static_assert(offsetof(StructF, func) == offsetof(wStructF, func),
                  "asSFuncPtr is not same as WingHex::asFuncPtr");
    static_assert(offsetof(StructM, dummy) == offsetof(wStructM, dummy),
                  "asSFuncPtr is not same as WingHex::asFuncPtr");
    static_assert(offsetof(StructF, dummy) == offsetof(wStructF, dummy),
                  "asSFuncPtr is not same as WingHex::asFuncPtr");
    static_assert(sizeof(StructM::mthd) == sizeof(wStructM::mthd),
                  "asSFuncPtr is not same as WingHex::asFuncPtr");
    static_assert(sizeof(StructF::func) == sizeof(wStructF::func),
                  "asSFuncPtr is not same as WingHex::asFuncPtr");
    static_assert(sizeof(StructM::dummy) == sizeof(wStructM::dummy),
                  "asSFuncPtr is not same as WingHex::asFuncPtr");
    static_assert(sizeof(StructF::dummy) == sizeof(wStructF::dummy),
                  "asSFuncPtr is not same as WingHex::asFuncPtr");

    // just so simple
    asSFuncPtr ret;
    std::memcpy(&ret, &ptr, sizeof(ret));
    return ret;
}

// some checks
static_assert(asEBehaviours::asBEHAVE_CONSTRUCT ==
                  int(WingHex::IWingAngel::asBEHAVE_CONSTRUCT),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_LIST_CONSTRUCT ==
                  int(WingHex::IWingAngel::asBEHAVE_LIST_CONSTRUCT),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_DESTRUCT ==
                  int(WingHex::IWingAngel::asBEHAVE_DESTRUCT),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_FACTORY ==
                  int(WingHex::IWingAngel::asBEHAVE_FACTORY),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_LIST_FACTORY ==
                  int(WingHex::IWingAngel::asBEHAVE_LIST_FACTORY),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_ADDREF ==
                  int(WingHex::IWingAngel::asBEHAVE_ADDREF),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_RELEASE ==
                  int(WingHex::IWingAngel::asBEHAVE_RELEASE),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_GET_WEAKREF_FLAG ==
                  int(WingHex::IWingAngel::asBEHAVE_GET_WEAKREF_FLAG),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_TEMPLATE_CALLBACK ==
                  int(WingHex::IWingAngel::asBEHAVE_TEMPLATE_CALLBACK),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_FIRST_GC ==
                  int(WingHex::IWingAngel::asBEHAVE_FIRST_GC),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_GETREFCOUNT ==
                  int(WingHex::IWingAngel::asBEHAVE_GETREFCOUNT),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_SETGCFLAG ==
                  int(WingHex::IWingAngel::asBEHAVE_SETGCFLAG),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_GETGCFLAG ==
                  int(WingHex::IWingAngel::asBEHAVE_GETGCFLAG),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_ENUMREFS ==
                  int(WingHex::IWingAngel::asBEHAVE_ENUMREFS),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_RELEASEREFS ==
                  int(WingHex::IWingAngel::asBEHAVE_RELEASEREFS),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_LAST_GC ==
                  int(WingHex::IWingAngel::asBEHAVE_LAST_GC),
              "asEBehaviours check failed");
static_assert(asEBehaviours::asBEHAVE_RELEASEREFS ==
                  int(WingHex::IWingAngel::asBEHAVE_RELEASEREFS),
              "asEBehaviours check failed");

static_assert(asERetCodes::asSUCCESS == int(WingHex::asRetCodes::asSUCCESS),
              "asERetCodes check failed");
static_assert(asERetCodes::asERROR == int(WingHex::asRetCodes::asERROR),
              "asERetCodes check failed");
static_assert(asERetCodes::asCONTEXT_ACTIVE ==
                  int(WingHex::asRetCodes::asCONTEXT_ACTIVE),
              "asERetCodes check failed");
static_assert(asERetCodes::asCONTEXT_NOT_FINISHED ==
                  int(WingHex::asRetCodes::asCONTEXT_NOT_FINISHED),
              "asERetCodes check failed");
static_assert(asERetCodes::asCONTEXT_NOT_PREPARED ==
                  int(WingHex::asRetCodes::asCONTEXT_NOT_PREPARED),
              "asERetCodes check failed");
static_assert(asERetCodes::asINVALID_ARG ==
                  int(WingHex::asRetCodes::asINVALID_ARG),
              "asERetCodes check failed");
static_assert(asERetCodes::asNO_FUNCTION ==
                  int(WingHex::asRetCodes::asNO_FUNCTION),
              "asERetCodes check failed");
static_assert(asERetCodes::asNOT_SUPPORTED ==
                  int(WingHex::asRetCodes::asNOT_SUPPORTED),
              "asERetCodes check failed");
static_assert(asERetCodes::asINVALID_NAME ==
                  int(WingHex::asRetCodes::asINVALID_NAME),
              "asERetCodes check failed");
static_assert(asERetCodes::asNAME_TAKEN ==
                  int(WingHex::asRetCodes::asNAME_TAKEN),
              "asERetCodes check failed");
static_assert(asERetCodes::asINVALID_DECLARATION ==
                  int(WingHex::asRetCodes::asINVALID_DECLARATION),
              "asERetCodes check failed");
static_assert(asERetCodes::asINVALID_OBJECT ==
                  int(WingHex::asRetCodes::asINVALID_OBJECT),
              "asERetCodes check failed");
static_assert(asERetCodes::asINVALID_TYPE ==
                  int(WingHex::asRetCodes::asINVALID_TYPE),
              "asERetCodes check failed");
static_assert(asERetCodes::asALREADY_REGISTERED ==
                  int(WingHex::asRetCodes::asALREADY_REGISTERED),
              "asERetCodes check failed");
static_assert(asERetCodes::asMULTIPLE_FUNCTIONS ==
                  int(WingHex::asRetCodes::asMULTIPLE_FUNCTIONS),
              "asERetCodes check failed");
static_assert(asERetCodes::asNO_MODULE == int(WingHex::asRetCodes::asNO_MODULE),
              "asERetCodes check failed");
static_assert(asERetCodes::asNO_GLOBAL_VAR ==
                  int(WingHex::asRetCodes::asNO_GLOBAL_VAR),
              "asERetCodes check failed");
static_assert(asERetCodes::asINVALID_CONFIGURATION ==
                  int(WingHex::asRetCodes::asINVALID_CONFIGURATION),
              "asERetCodes check failed");
static_assert(asERetCodes::asINVALID_INTERFACE ==
                  int(WingHex::asRetCodes::asINVALID_INTERFACE),
              "asERetCodes check failed");
static_assert(asERetCodes::asCANT_BIND_ALL_FUNCTIONS ==
                  int(WingHex::asRetCodes::asCANT_BIND_ALL_FUNCTIONS),
              "asERetCodes check failed");
static_assert(
    asERetCodes::asLOWER_ARRAY_DIMENSION_NOT_REGISTERED ==
        int(WingHex::asRetCodes::asLOWER_ARRAY_DIMENSION_NOT_REGISTERED),
    "asERetCodes check failed");
static_assert(asERetCodes::asWRONG_CONFIG_GROUP ==
                  int(WingHex::asRetCodes::asWRONG_CONFIG_GROUP),
              "asERetCodes check failed");
static_assert(asERetCodes::asCONFIG_GROUP_IS_IN_USE ==
                  int(WingHex::asRetCodes::asCONFIG_GROUP_IS_IN_USE),
              "asERetCodes check failed");
static_assert(asERetCodes::asILLEGAL_BEHAVIOUR_FOR_TYPE ==
                  int(WingHex::asRetCodes::asILLEGAL_BEHAVIOUR_FOR_TYPE),
              "asERetCodes check failed");
static_assert(asERetCodes::asWRONG_CALLING_CONV ==
                  int(WingHex::asRetCodes::asWRONG_CALLING_CONV),
              "asERetCodes check failed");
static_assert(asERetCodes::asBUILD_IN_PROGRESS ==
                  int(WingHex::asRetCodes::asBUILD_IN_PROGRESS),
              "asERetCodes check failed");
static_assert(asERetCodes::asINIT_GLOBAL_VARS_FAILED ==
                  int(WingHex::asRetCodes::asINIT_GLOBAL_VARS_FAILED),
              "asERetCodes check failed");
static_assert(asERetCodes::asOUT_OF_MEMORY ==
                  int(WingHex::asRetCodes::asOUT_OF_MEMORY),
              "asERetCodes check failed");
static_assert(asERetCodes::asMODULE_IS_IN_USE ==
                  int(WingHex::asRetCodes::asMODULE_IS_IN_USE),
              "asERetCodes check failed");

WingAngel::WingAngel(WingAngelAPI *api, QStringList &marcos)
    : _api(api), _scriptMarcos(marcos) {
    Q_ASSERT(api);
}

bool WingAngel::setCurrentPluginSession(const QByteArray &ns) {
    if (ns != _plgsess) {
        auto engine = ScriptMachine::instance().engine();
        auto ret = engine->SetDefaultNamespace(ns.data());
        if (ret != asSUCCESS) {
            return false;
        }
        _plgsess = ns;
    }
    return true;
}

WingHex::asRetCodes
WingAngel::registerGlobalFunction(uint retMetaType, const ScriptFn &fn,
                                  const QString &fnName,
                                  const QVector<QPair<uint, QString>> &params) {
    auto sig = getScriptFnSig(retMetaType, fn, fnName, params);
    if (sig.isEmpty()) {
        Logger::critical(tr("RegisterScriptFnUnSupportedTypes:") + _plgsess +
                         QStringLiteral("::") + fnName);
        return WingHex::asRetCodes::asINVALID_ARG;
    }

    auto engine = ScriptMachine::instance().engine();

    auto ret = engine->RegisterGlobalFunction(
        sig.toUtf8(), asFUNCTION(WingAngelAPI::script_call),
        asECallConvTypes::asCALL_GENERIC);

    if (ret < 0) {
        // TODO
        return returnValue(ret);
    }

    auto &sfns = _api->_sfns;
    auto id = sfns.size();

    auto f = engine->GetFunctionById(ret);
    if (f) {
        f->SetUserData(_api, AsUserDataType::UserData_API);
        f->SetUserData(reinterpret_cast<void *>(id),
                       AsUserDataType::UserData_PluginFn);
    } else {
        return WingHex::asRetCodes::asINVALID_ARG;
    }

    WingScriptInternal::ScriptFnInfo info;
    info.ret = retMetaType;
    info.fn = fn;
    info.params = params;

    sfns.append(info);

    return WingHex::asRetCodes::asSUCCESS;
}

WingHex::asRetCodes
WingAngel::registerGlobalFunction(const QString &decl,
                                  const WingHex::UNSAFE_SCFNPTR &fn) {
    if (decl.isEmpty()) {
        Logger::critical(tr("RegisterScriptFnUnSupportedTypes:") + _plgsess +
                         QStringLiteral("::") + decl);
        return WingHex::asRetCodes::asINVALID_ARG;
    }

    auto engine = ScriptMachine::instance().engine();

    auto ret = engine->RegisterGlobalFunction(
        decl.toUtf8(), asFUNCTION(WingAngelAPI::script_unsafe_call),
        asECallConvTypes::asCALL_GENERIC);

    if (ret < 0) {
        // TODO
        return returnValue(ret);
    }

    auto &sfns = _api->_usfns;
    auto id = sfns.size();

    auto f = engine->GetFunctionById(ret);
    if (f) {
        f->SetUserData(_api, AsUserDataType::UserData_API);
        f->SetUserData(reinterpret_cast<void *>(id),
                       AsUserDataType::UserData_PluginFn);
    } else {
        return WingHex::asRetCodes::asINVALID_ARG;
    }

    sfns.append(fn);

    return WingHex::asRetCodes::asSUCCESS;
}

void WingAngel::registerScriptMarco(const QString &marco) {
    if (Utilities::isValidIdentifier(marco)) {
        static auto sep = QStringLiteral("_");
        _scriptMarcos.append(sep + _plgsess + sep + marco + sep);
    } else {

        // TODO
    }
}

void WingAngel::registerScriptMarcos(const QStringList &marcos) {
    for (auto &m : marcos) {
        registerScriptMarco(m);
    }
}

WingHex::asRetCodes
WingAngel::registerGlobalFunction(const char *declaration,
                                  const WingHex::asFuncPtr &funcPointer,
                                  asCallConvTypes callConv, void *auxiliary) {
    auto engine = ScriptMachine::instance().engine();
    auto ret = engine->RegisterGlobalFunction(
        declaration, asCSFuncPtr(funcPointer), asDWORD(callConv), auxiliary);
    return returnValue(ret);
}

WingHex::asRetCodes
WingAngel::registerInterfaceMethod(const char *intf, const char *declaration) {
    auto engine = ScriptMachine::instance().engine();
    auto ret = engine->RegisterInterfaceMethod(intf, declaration);
    return returnValue(ret);
}

WingHex::asRetCodes WingAngel::registerInterface(const char *name) {
    auto engine = ScriptMachine::instance().engine();
    auto ret = engine->RegisterInterface(name);
    return returnValue(ret);
}

WingHex::asRetCodes WingAngel::registerObjectBehaviour(
    const char *obj, asBehaviours behaviour, const char *declaration,
    const WingHex::asFuncPtr &funcPointer, asCallConvTypes callConv,
    void *auxiliary, int compositeOffset, bool isCompositeIndirect) {
    auto engine = ScriptMachine::instance().engine();
    auto ret = engine->RegisterObjectBehaviour(
        obj, asEBehaviours(behaviour), declaration, asCSFuncPtr(funcPointer),
        asECallConvTypes(callConv), auxiliary, compositeOffset,
        isCompositeIndirect);
    return returnValue(ret);
}

WingHex::asRetCodes
WingAngel::registerObjectMethod(const char *obj, const char *declaration,
                                const WingHex::asFuncPtr &funcPointer,
                                asCallConvTypes callConv, void *auxiliary,
                                int compositeOffset, bool isCompositeIndirect) {
    auto engine = ScriptMachine::instance().engine();
    auto ret = engine->RegisterObjectMethod(
        obj, declaration, asCSFuncPtr(funcPointer), asECallConvTypes(callConv),
        auxiliary, compositeOffset, isCompositeIndirect);
    return returnValue(ret);
}

WingHex::asRetCodes
WingAngel::registerObjectProperty(const char *obj, const char *declaration,
                                  int byteOffset, int compositeOffset,
                                  bool isCompositeIndirect) {
    auto engine = ScriptMachine::instance().engine();
    auto ret = engine->RegisterObjectProperty(
        obj, declaration, byteOffset, compositeOffset, isCompositeIndirect);
    return returnValue(ret);
}

WingHex::asRetCodes WingAngel::registerObjectType(const char *obj, int byteSize,
                                                  quint64 flags) {
    auto engine = ScriptMachine::instance().engine();
    auto ret = engine->RegisterObjectType(obj, byteSize, flags);
    return returnValue(ret);
}

WingHex::asRetCodes WingAngel::registerGlobalProperty(const char *declaration,
                                                      void *pointer) {
    auto engine = ScriptMachine::instance().engine();
    auto ret = engine->RegisterGlobalProperty(declaration, pointer);
    return returnValue(ret);
}

WingHex::asRetCodes WingAngel::registerTypedef(const char *type,
                                               const char *decl) {
    auto engine = ScriptMachine::instance().engine();
    auto ret = engine->RegisterTypedef(type, decl);
    return returnValue(ret);
}

WingHex::asRetCodes WingAngel::registerFuncdef(const char *decl) {
    auto engine = ScriptMachine::instance().engine();
    auto ret = engine->RegisterFuncdef(decl);
    return returnValue(ret);
}

WingHex::asRetCodes WingAngel::registerEnumValue(const char *type,
                                                 const char *name, int value) {
    auto engine = ScriptMachine::instance().engine();
    auto ret = engine->RegisterEnumValue(type, name, value);
    return returnValue(ret);
}

WingHex::asRetCodes WingAngel::registerEnum(const char *type) {
    auto engine = ScriptMachine::instance().engine();
    auto ret = engine->RegisterEnum(type);
    return returnValue(ret);
}

WingHex::asRetCodes WingAngel::restoreDefaultNamespace() {
    auto engine = ScriptMachine::instance().engine();
    auto ret = engine->SetDefaultNamespace(_plgsess);
    return returnValue(ret);
}

WingHex::asRetCodes WingAngel::setDefaultNamespace(const char *nameSpace) {
    auto engine = ScriptMachine::instance().engine();
    if (nameSpace) {
        if (qstrlen(nameSpace) > 1024) {
            return WingHex::asRetCodes::asINVALID_NAME;
        }
        auto ns = _plgsess.append(QByteArrayLiteral("::")).append(nameSpace);
        auto ret = engine->SetDefaultNamespace(ns.data());
        return returnValue(ret);
    } else {
        return restoreDefaultNamespace();
    }
}

QString WingAngel::getScriptFnSig(uint retMetaType, const ScriptFn &fn,
                                  const QString &fnName,
                                  const QVector<QPair<uint, QString>> &params) {
    if (fnName.isEmpty()) {
        return {};
    }

    QString sig;

    auto ret = type2AngelScriptString(retMetaType, false);
    if (ret.isEmpty()) {
        return {};
    }

    sig += ret + QStringLiteral(" ") + fnName + QStringLiteral("(");

    QStringList _params;
    for (auto &param : params) {
        auto ret = type2AngelScriptString(param.first, true);
        if (ret.isEmpty()) {
            return {};
        }

        _params << ret + QStringLiteral(" ") + param.second;
    }

    return sig + _params.join(',') + QStringLiteral(")");
}

QString WingAngel::type2AngelScriptString(uint type, bool isArg,
                                          bool noModifier) {
    auto isArray = !!(type & WingHex::Meta_Array);
    auto isList = !!(type & WingHex::Meta_List);
    auto isContainer = isArray || isList;
    if (isContainer) {
        if (isArray && isList) {
            return {};
        }
    }

    QString retype;
    bool complexType = false;

    type = WingHex::MetaType(type & WingHex::MetaTypeMask);
    switch (type) {
    case WingHex::Meta_Void:
        retype = QStringLiteral("void");
        break;
    case WingHex::Meta_Bool:
        retype = QStringLiteral("bool");
        break;
    case WingHex::Meta_Int:
        retype = QStringLiteral("int");
        break;
    case WingHex::Meta_UInt:
        retype = QStringLiteral("uint");
        break;
    case WingHex::Meta_Int8:
        retype = QStringLiteral("int8");
        break;
    case WingHex::Meta_UInt8:
        retype = QStringLiteral("uint8");
        break;
    case WingHex::Meta_Int16:
        retype = QStringLiteral("int16");
        break;
    case WingHex::Meta_UInt16:
        retype = QStringLiteral("uint16");
        break;
    case WingHex::Meta_Int64:
        retype = QStringLiteral("int64");
        break;
    case WingHex::Meta_UInt64:
        retype = QStringLiteral("uint64");
        break;
    case WingHex::Meta_Float:
        retype = QStringLiteral("float");
        break;
    case WingHex::Meta_Double:
        retype = QStringLiteral("double");
        break;
    case WingHex::Meta_String:
        retype = QStringLiteral("string");
        break;
    case WingHex::Meta_Char:
        retype = QStringLiteral("char");
        break;
    case WingHex::Meta_Byte:
        retype = QStringLiteral("byte");
        break;
    case WingHex::Meta_Color:
        retype = QStringLiteral("color");
        complexType = true;
        break;
    case WingHex::Meta_Map:
    case WingHex::Meta_Hash:
        retype = QStringLiteral("dictionary");
        complexType = true;
        break;
    default:
        return {};
    }

    if (isArray || isList) {
        retype.append(QStringLiteral("[]"));
    }

    if (isArg) {
        if (!noModifier && (isContainer || complexType)) {
            retype.append(QStringLiteral(" &in"))
                .prepend(QStringLiteral("const "));
        }
    } else {
        if (!noModifier) {
            // if it's a return type, only array<byte> and array<string> are
            // supported in AngelScript
            // ( array<byte> -> QByteArray , array<string> -> QStringList ),
            // other array types are not suported. PRs are welcomed !!!
            // IT'S TOO COMPLEX TO SUPPORT QVARIANTLIST !!!

            // You can use unsafe registering to support
            // extensive scripting system.
            // It will be faster and flexible but not easy to implement.
            if (isContainer) {
                if (type != WingHex::Meta_Byte &&
                    type != WingHex::Meta_String) {
                    return {};
                }

                retype.append(QStringLiteral("@"));
            }

            if (complexType) {
                retype.append(QStringLiteral("@"));
            }
        }
    }

    return retype.trimmed();
}

WingHex::asRetCodes WingAngel::returnValue(int ret) {
    if (ret < 0) {
        return WingHex::asRetCodes(ret);
    } else {
        return WingHex::asRetCodes::asSUCCESS;
    }
}
