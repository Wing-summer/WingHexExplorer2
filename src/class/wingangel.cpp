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
#include "logger.h"
#include "scriptmachine.h"
#include "utilities.h"

WingAngel::WingAngel(WingAngelAPI *api, QStringList &marcos)
    : _api(api), _scriptMarcos(marcos) {
    Q_ASSERT(api);
}

bool WingAngel::setCurrentPluginSession(const QByteArray &ns) {
    if (ns != _plgsess) {
        // auto engine = ScriptMachine::instance().engine();
        // auto ret = engine->SetDefaultNamespace(ns.data());
        // if (ret != asSUCCESS) {
        //     return false;
        // }
        // _plgsess = ns;
    }
    return true;
}

WingHex::asRetCodes
WingAngel::registerGlobalFunction(uint retMetaType, const ScriptFn &fn,
                                  const QString &fnName,
                                  const QVector<QPair<uint, QString>> &params) {
    auto sig = getScriptFnSig(retMetaType, fn, fnName, params);
    if (sig.isEmpty()) {
        Logger::critical(QStringLiteral("[WingAngel::registerGlobalFunction] "
                                        "getScriptFnSig failed (") +
                         QString::fromUtf8(_plgsess) + QStringLiteral("::") +
                         fnName + QStringLiteral(")"));
        return WingHex::asRetCodes::asINVALID_ARG;
    }

    // auto engine = ScriptMachine::instance().engine();

    // auto ret = engine->RegisterGlobalFunction(
    //     sig.toUtf8(), asFUNCTION(WingAngelAPI::script_call),
    //     asECallConvTypes::asCALL_GENERIC);

    // auto minfo = QMetaEnum::fromType<WingHex::asRetCodes>();

    // if (ret < 0) {
    //     Logger::critical(
    //         QStringLiteral("[WingAngel::registerGlobalFunction] "
    //                        "RegisterGlobalFunction '%1' failed (%2)")
    //             .arg(sig, minfo.valueToKey(ret)));
    //     return returnValue(ret);
    // }

    // auto &sfns = _api->_sfns;
    // auto id = sfns.size();

    // auto f = engine->GetFunctionById(ret);
    // if (f) {
    //     f->SetUserData(_api, AsUserDataType::UserData_API);
    //     f->SetUserData(reinterpret_cast<void *>(id),
    //                    AsUserDataType::UserData_PluginFn);
    // } else {
    //     Logger::critical(QStringLiteral("[WingAngel::registerGlobalFunction]
    //     "
    //                                     "'%1' GetFunctionById failed")
    //                          .arg(sig));
    //     return WingHex::asRetCodes::asINVALID_ARG;
    // }

    // WingScriptInternal::ScriptFnInfo info;
    // info.ret = retMetaType;
    // info.fn = fn;
    // info.params = params;

    // sfns.append(info);

    return WingHex::asRetCodes::asSUCCESS;
}

WingHex::asRetCodes
WingAngel::registerGlobalFunction(const QString &decl,
                                  const WingHex::UNSAFE_SCFNPTR &fn) {
    if (decl.isEmpty()) {
        Logger::critical(QStringLiteral(
            "[WingAngel::registerGlobalFunction] Empty declaration"));
        return WingHex::asRetCodes::asINVALID_ARG;
    }

    // auto engine = ScriptMachine::instance().engine();

    // auto ret = engine->RegisterGlobalFunction(
    //     decl.toUtf8(), asFUNCTION(WingAngelAPI::script_unsafe_call),
    //     asECallConvTypes::asCALL_GENERIC);

    // auto minfo = QMetaEnum::fromType<WingHex::asRetCodes>();
    // if (ret < 0) {
    //     Logger::critical(
    //         QStringLiteral("[WingAngel::registerGlobalFunction] "
    //                        "RegisterGlobalFunction '%1' failed (%2)")
    //             .arg(decl, minfo.valueToKey(ret)));
    //     return returnValue(ret);
    // }

    // auto &sfns = _api->_usfns;
    // auto id = sfns.size();

    // auto f = engine->GetFunctionById(ret);
    // if (f) {
    //     f->SetUserData(_api, AsUserDataType::UserData_API);
    //     f->SetUserData(reinterpret_cast<void *>(id),
    //                    AsUserDataType::UserData_PluginFn);
    // } else {
    //     Logger::critical(QStringLiteral("[WingAngel::registerGlobalFunction]
    //     "
    //                                     "'%1' GetFunctionById failed")
    //                          .arg(decl));
    //     return WingHex::asRetCodes::asINVALID_ARG;
    // }

    // sfns.append(fn);

    return WingHex::asRetCodes::asSUCCESS;
}

void WingAngel::registerScriptMarco(const QString &marco) {
    if (Utilities::isValidIdentifier(marco)) {
        static auto sep = QStringLiteral("_");
        _scriptMarcos.append(sep + QString::fromUtf8(_plgsess) + sep + marco +
                             sep);
    } else {
        Logger::critical(QStringLiteral(
            "[WingAngel::registerScriptMarco] isValidIdentifier failed"));
    }
}

void WingAngel::registerScriptMarcos(const QStringList &marcos) {
    for (const auto &m : marcos) {
        registerScriptMarco(m);
    }
}

WingHex::asRetCodes
WingAngel::registerGlobalFunction(const char *declaration,
                                  const WingHex::asFuncPtr &funcPointer,
                                  asCallConvTypes callConv, void *auxiliary) {
    // auto engine = ScriptMachine::instance().engine();
    // auto ret = engine->RegisterGlobalFunction(
    //     declaration, asCSFuncPtr(funcPointer), asDWORD(callConv), auxiliary);
    // return returnValue(ret);
    return WingHex::asRetCodes::asERROR;
}

QHash<std::string_view, WingHex::IWingAngel::Evaluator>
WingAngel::customEvals() const {
    return _customEvals;
}

WingHex::asRetCodes
WingAngel::registerInterfaceMethod(const char *intf, const char *declaration) {
    // auto engine = ScriptMachine::instance().engine();
    // auto ret = engine->RegisterInterfaceMethod(intf, declaration);
    // return returnValue(ret);
    return WingHex::asRetCodes::asERROR;
}

WingHex::asRetCodes WingAngel::registerInterface(const char *name) {
    // auto engine = ScriptMachine::instance().engine();
    // auto ret = engine->RegisterInterface(name);
    // return returnValue(ret);
    return WingHex::asRetCodes::asERROR;
}

WingHex::asRetCodes WingAngel::registerObjectBehaviour(
    const char *obj, asBehaviours behaviour, const char *declaration,
    const WingHex::asFuncPtr &funcPointer, asCallConvTypes callConv,
    void *auxiliary, int compositeOffset, bool isCompositeIndirect) {
    // auto engine = ScriptMachine::instance().engine();
    // auto ret = engine->RegisterObjectBehaviour(
    //     obj, asEBehaviours(behaviour), declaration, asCSFuncPtr(funcPointer),
    //     asECallConvTypes(callConv), auxiliary, compositeOffset,
    //     isCompositeIndirect);
    // return returnValue(ret);
    return WingHex::asRetCodes::asERROR;
}

WingHex::asRetCodes WingAngel::registerObjectEvaluator(const char *obj,
                                                       const Evaluator &ev) {

    return WingHex::asRetCodes::asINVALID_TYPE;
}

WingHex::asRetCodes
WingAngel::registerObjectMethod(const char *obj, const char *declaration,
                                const WingHex::asFuncPtr &funcPointer,
                                asCallConvTypes callConv, void *auxiliary,
                                int compositeOffset, bool isCompositeIndirect) {
    // auto engine = ScriptMachine::instance().engine();
    // auto ret = engine->RegisterObjectMethod(
    //     obj, declaration, asCSFuncPtr(funcPointer),
    //     asECallConvTypes(callConv), auxiliary, compositeOffset,
    //     isCompositeIndirect);
    // return returnValue(ret);
    return WingHex::asRetCodes::asERROR;
}

WingHex::asRetCodes
WingAngel::registerObjectProperty(const char *obj, const char *declaration,
                                  int byteOffset, int compositeOffset,
                                  bool isCompositeIndirect) {
    // auto engine = ScriptMachine::instance().engine();
    // auto ret = engine->RegisterObjectProperty(
    //     obj, declaration, byteOffset, compositeOffset, isCompositeIndirect);
    // return returnValue(ret);
    return WingHex::asRetCodes::asERROR;
}

WingHex::asRetCodes WingAngel::registerObjectType(const char *obj, int byteSize,
                                                  quint64 flags) {
    // auto engine = ScriptMachine::instance().engine();
    // auto ret = engine->RegisterObjectType(obj, byteSize, flags);
    // return returnValue(ret);
    return WingHex::asRetCodes::asERROR;
}

WingHex::asRetCodes WingAngel::registerGlobalProperty(const char *declaration,
                                                      void *pointer) {
    // auto engine = ScriptMachine::instance().engine();
    // auto ret = engine->RegisterGlobalProperty(declaration, pointer);
    // return returnValue(ret);
    return WingHex::asRetCodes::asERROR;
}

WingHex::asRetCodes WingAngel::registerTypedef(const char *type,
                                               const char *decl) {
    // auto engine = ScriptMachine::instance().engine();
    // auto ret = engine->RegisterTypedef(type, decl);
    // return returnValue(ret);
    return WingHex::asRetCodes::asERROR;
}

WingHex::asRetCodes WingAngel::registerFuncdef(const char *decl) {
    // auto engine = ScriptMachine::instance().engine();
    // auto ret = engine->RegisterFuncdef(decl);
    // return returnValue(ret);
    return WingHex::asRetCodes::asERROR;
}

WingHex::asRetCodes WingAngel::registerEnumValue(const char *type,
                                                 const char *name, int value) {
    // auto engine = ScriptMachine::instance().engine();
    // auto ret = engine->RegisterEnumValue(type, name, value);
    // return returnValue(ret);
    return WingHex::asRetCodes::asERROR;
}

WingHex::asRetCodes WingAngel::registerEnum(const char *type) {
    // auto engine = ScriptMachine::instance().engine();
    // auto ret = engine->RegisterEnum(type);
    // return returnValue(ret);
    return WingHex::asRetCodes::asERROR;
}

WingHex::asRetCodes WingAngel::restoreDefaultNamespace() {
    // auto engine = ScriptMachine::instance().engine();
    // auto ret = engine->SetDefaultNamespace(_plgsess);
    // return returnValue(ret);
    return WingHex::asRetCodes::asERROR;
}

WingHex::asRetCodes WingAngel::setDefaultNamespace(const char *nameSpace) {
    // auto engine = ScriptMachine::instance().engine();
    // if (nameSpace) {
    //     if (qstrlen(nameSpace) > 1024) {
    //         return WingHex::asRetCodes::asINVALID_NAME;
    //     }
    //     auto ns = _plgsess.append(QByteArrayLiteral("::")).append(nameSpace);
    //     auto ret = engine->SetDefaultNamespace(ns.data());
    //     return returnValue(ret);
    // } else {
    //     return restoreDefaultNamespace();
    // }
    return WingHex::asRetCodes::asERROR;
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
    for (const auto &param : params) {
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
        complexType = true;
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
        isContainer = true;
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
                if (isArray || isList) {
                    if (type != WingHex::Meta_Byte &&
                        type != WingHex::Meta_String) {
                        return {};
                    }
                }

                retype.append(QStringLiteral("@"));
            }
        }
    }

    return retype.trimmed();
}

WingHex::asRetCodes WingAngel::returnValue(int ret) {
    Q_ASSERT(ret >= 0);
    if (ret < 0) {
        return WingHex::asRetCodes(ret);
    } else {
        return WingHex::asRetCodes::asSUCCESS;
    }
}

QList<int> WingAngel::excludeEvalIDs() const { return _excludeEvalIDs; }

void WingAngel::setExcludeEvalIDs(const QList<int> &newExcludeEvals) {
    _excludeEvalIDs = newExcludeEvals;
}
