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

#include "asdatabase.h"

#include "AngelScript/sdk/angelscript/source/as_objecttype.h"
#include "AngelScript/sdk/angelscript/source/as_scriptengine.h"
#include "AngelScript/sdk/angelscript/source/as_scriptfunction.h"

#include <QDebug>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using qhash_result_t = uint;

// copying from QT6 source code for supporting QT5's qHashMulti
namespace QtPrivate {
template <typename T>
inline constexpr bool QNothrowHashableHelper_v =
    noexcept(qHash(std::declval<const T &>()));

template <typename T, typename Enable = void>
struct QNothrowHashable : std::false_type {};

template <typename T>
struct QNothrowHashable<T, std::enable_if_t<QNothrowHashableHelper_v<T>>>
    : std::true_type {};

template <typename T>
constexpr inline bool QNothrowHashable_v = QNothrowHashable<T>::value;

} // namespace QtPrivate

template <typename... T>
constexpr qhash_result_t
qHashMulti(qhash_result_t seed, const T &...args) noexcept(
    std::conjunction_v<QtPrivate::QNothrowHashable<T>...>) {
    QtPrivate::QHashCombine hash;
    return ((seed = hash(seed, args)), ...), seed;
}
#else
using qhash_result_t = size_t;
#endif

inline qhash_result_t qHash(const ASDataBase::HeaderType &c,
                            qhash_result_t seed) noexcept {
    return qHashMulti(seed, c.name, int(c.type));
}

ASDataBase::ASDataBase(asIScriptEngine *engine) {
    addGlobalFunctionCompletion(engine);
    addClassCompletion(engine);
    addEnumCompletion(engine);

    // generate keyword completion
    QStringList kws{
        "const",     "in",        "inout",    "out",    "auto",     "public",
        "protected", "private",   "void",     "int8",   "int16",    "int",
        "int64",     "uint8",     "uint16",   "uint",   "uint64",   "float",
        "double",    "bool",      "enum",     "string", "array",    "any",
        "for",       "while",     "do",       "if",     "else",     "switch",
        "break",     "continue",  "try",      "catch",  "throw",    "abstract",
        "delete",    "cast",      "class",    "final",  "property", "external",
        "function",  "interface", "shared",   "this",   "explicit", "override",
        "namespace", "get",       "set",      "super",  "mixin",    "false",
        "true",      "null",      "typename", "return", "typedef",  "funcdef",
        "from",      "import",    "not",      "xor",    "or",       "is"};
    for (auto &k : kws) {
        CodeInfoTip t;
        t.type = CodeInfoTip::Type::KeyWord;
        t.name = k;
        _keywordNode.append(t);
    }
}

ASDataBase::~ASDataBase() {}

QByteArray ASDataBase::getFnParamDeclString(asIScriptFunction *fn,
                                            bool includeNamespace,
                                            bool includeParamNames) {
    auto fun = dynamic_cast<asCScriptFunction *>(fn);
    if (fun == nullptr) {
        return {};
    }

    asCString str;

    auto &parameterTypes = fun->parameterTypes;
    auto &parameterNames = fun->parameterNames;
    auto &nameSpace = fun->nameSpace;
    auto &inOutFlags = fun->inOutFlags;
    auto &defaultArgs = fun->defaultArgs;

    if (parameterTypes.GetLength() > 0) {
        asUINT n;
        for (n = 0; n < parameterTypes.GetLength() - 1; n++) {
            str += parameterTypes[n].Format(nameSpace, includeNamespace);
            if (parameterTypes[n].IsReference() && inOutFlags.GetLength() > n) {
                if (inOutFlags[n] == asTM_INREF)
                    str += "in";
                else if (inOutFlags[n] == asTM_OUTREF)
                    str += "out";
                else if (inOutFlags[n] == asTM_INOUTREF)
                    str += "inout";
            }

            if (includeParamNames && n < parameterNames.GetLength() &&
                parameterNames[n].GetLength() != 0) {
                str += " ";
                str += parameterNames[n];
            }

            if (defaultArgs.GetLength() > n && defaultArgs[n]) {
                asCString tmp;
                tmp.Format(" = %s", defaultArgs[n]->AddressOf());
                str += tmp;
            }

            str += ", ";
        }

        // Add the last parameter
        str += parameterTypes[n].Format(nameSpace, includeNamespace);
        if (parameterTypes[n].IsReference() && inOutFlags.GetLength() > n) {
            if (inOutFlags[n] == asTM_INREF)
                str += "in";
            else if (inOutFlags[n] == asTM_OUTREF)
                str += "out";
            else if (inOutFlags[n] == asTM_INOUTREF)
                str += "inout";
        }

        if (includeParamNames && n < parameterNames.GetLength() &&
            parameterNames[n].GetLength() != 0) {
            str += " ";
            str += parameterNames[n];
        }

        if (defaultArgs.GetLength() > n && defaultArgs[n]) {
            asCString tmp;
            tmp.Format(" = %s", defaultArgs[n]->AddressOf());
            str += tmp;
        }
    }

    return QByteArray(str.AddressOf(), QByteArray::size_type(str.GetLength()));
}

QByteArray ASDataBase::getFnRealName(asIScriptFunction *fn) {
    auto fun = dynamic_cast<asCScriptFunction *>(fn);
    if (fun == nullptr) {
        return {};
    }

    asCString str;
    asCString name = fun->GetName();

    if (name.GetLength() == 0)
        str += "_unnamed_function_";
    else if (name.SubString(0, 4) == "$beh" && name.GetLength() == 5) {
        if (name[4] == '0' + asBEHAVE_CONSTRUCT)
            str += fun->objectType->name;
        else if (name[4] == '0' + asBEHAVE_FACTORY)
            str += fun->returnType.GetTypeInfo()->name;
        else if (name[4] == '0' + asBEHAVE_DESTRUCT)
            str += "~" + fun->objectType->name;
        else
            str += name;
    } else {
        str = name;
    }

    return QByteArray(str.AddressOf(), QByteArray::size_type(str.GetLength()));
}

QByteArray ASDataBase::getFnRetTypeString(asIScriptFunction *fn,
                                          bool includeNamespace) {
    auto fun = dynamic_cast<asCScriptFunction *>(fn);
    if (fun == nullptr) {
        return {};
    }

    auto &returnType = fun->returnType;
    auto &objectType = fun->objectType;
    auto &name = fun->name;
    auto &nameSpace = fun->nameSpace;

    if (!(returnType.GetTokenType() == ttVoid && objectType &&
          (name == objectType->name ||
           (name.GetLength() > 0 && name[0] == '~') || name == "$beh0" ||
           name == "$beh2"))) {
        auto str = returnType.Format(nameSpace, includeNamespace);
        return QByteArray(str.AddressOf(),
                          QByteArray::size_type(str.GetLength()));
    }

    return {};
}

const QHash<ASDataBase::HeaderType, QList<CodeInfoTip>> &
ASDataBase::headerNodes() const {
    return _headerNodes;
}

void ASDataBase::addGlobalFunctionCompletion(asIScriptEngine *engine) {
    Q_ASSERT(engine);

    HeaderType nst;
    nst.type = CodeInfoTip::Type::Group;
    for (asUINT i = 0; i < engine->GetGlobalFunctionCount(); ++i) {
        auto fn = engine->GetGlobalFunctionByIndex(i);
        CodeInfoTip fnInfo;
        auto ns = fn->GetNamespace();
        fnInfo.nameSpace = ns;
        fnInfo.name = fn->GetName();
        fnInfo.type = CodeInfoTip::Type::Function;
        fnInfo.args.insert(CodeInfoTip::RetType, getFnRetTypeString(fn, true));
        fnInfo.args.insert(CodeInfoTip::Args,
                           getFnParamDeclString(fn, false, true));
        fnInfo.args.insert(CodeInfoTip::SuffixQualifier,
                           fn->IsReadOnly() ? QStringLiteral("const")
                                            : QString());
        nst.name = ns;
        _headerNodes[nst].append(fnInfo);
    }
}

void ASDataBase::addEnumCompletion(asIScriptEngine *engine) {
    Q_ASSERT(engine);

    QHash<QByteArray, QList<EnumInfo>> _maps;

    for (asUINT i = 0; i < engine->GetEnumCount(); ++i) {
        auto etype = engine->GetEnumByIndex(i);
        etype->AddRef();

        EnumInfo einfo;
        auto ns = etype->GetNamespace();
        einfo.name = etype->GetName();

        for (asUINT i = 0; i < etype->GetEnumValueCount(); ++i) {
            int v;
            auto e = etype->GetEnumValueByIndex(i, &v);
            einfo.enums.append(qMakePair(QByteArray(e), v));
        }

        etype->Release();

        _maps[ns] << einfo;
    }
}

void ASDataBase::addClassCompletion(asIScriptEngine *engine) {
    auto eng = dynamic_cast<asCScriptEngine *>(engine);
    Q_ASSERT(eng);

    QHash<QByteArray, QList<ClassInfo>> _maps;

    for (asUINT i = 0; i < engine->GetObjectTypeCount(); ++i) {
        auto obj = eng->registeredObjTypes[i];
        obj->AddRef();

        ClassInfo cls;
        cls.name = obj->GetName();
        auto ns = obj->GetNamespace();

        for (asUINT i = 0; i < obj->GetBehaviourCount(); ++i) {
            asEBehaviours bv;
            auto b = obj->GetBehaviourByIndex(i, &bv);

            switch (bv) {
            case asBEHAVE_CONSTRUCT: {
                // only these are supported
                b->AddRef();
                FnInfo fn;
                fn.fnName = getFnRealName(b);
                fn.params = getFnParamDeclString(b, false, true);
                fn.isConst = b->IsReadOnly();
                cls.methods << fn;
                b->Release();
            }
            default:
                continue;
            }
        }

        for (asUINT i = 0; i < obj->GetMethodCount(); ++i) {
            auto m = obj->GetMethodByIndex(i, true);

            m->AddRef();
            FnInfo fn;
            fn.retType = getFnRetTypeString(m, true);
            fn.fnName = getFnRealName(m);
            fn.params = getFnParamDeclString(m, false, true);
            fn.isConst = m->IsReadOnly();
            cls.methods << fn;
            m->Release();
        }

        for (asUINT i = 0; i < obj->GetPropertyCount(); ++i) {
            auto p = obj->properties[i];

            PropertyInfo pi;
            pi.name = QByteArray(p->name.AddressOf(),
                                 QByteArray::size_type(p->name.GetLength()));
            auto tn = p->type.Format(obj->nameSpace);
            pi.type = QByteArray(tn.AddressOf(),
                                 QByteArray::size_type(tn.GetLength()));
            pi.isPrivate = pi.isPrivate;
            pi.isProtected = pi.isProtected;
            pi.isRef = pi.isRef;

            cls.properties << pi;
        }

        obj->Release();

        _maps[ns] << cls;
    }

    // auto applyClsNode = [](const QList<ClassInfo> &clsinfos,
    //                        bool isComplete) -> QList<QCodeNode *> {
    //     QList<QCodeNode *> ret;

    //     for (auto &cls : clsinfos) {
    //         QCodeNode *clsnode = new QCodeNode;
    //         clsnode->setNodeType(QCodeNode::Class);
    //         clsnode->setRole(QCodeNode::Name, cls.name);

    //         for (auto &m : cls.methods) {
    //             if (isComplete) {
    //                 if (m.fnName == cls.name) {
    //                     continue;
    //                 }
    //                 if (m.fnName.startsWith('~')) {
    //                     continue;
    //                 }
    //                 if (m.fnName.startsWith("op")) {
    //                     continue;
    //                 }
    //             }

    //             auto node = newFnCodeNode(m);
    //             node->attach(clsnode);
    //         }

    //         for (auto &p : cls.properties) {
    //             auto node = new QCodeNode;
    //             node->setNodeType(QCodeNode::Variable);
    //             node->setRole(QCodeNode::Name, p.name);
    //             node->setRole(QCodeNode::Type, p.type);

    //             QByteArray visibility;
    //             if (p.isPrivate) {
    //                 visibility.setNum(QCodeNode::VISIBILITY_PRIVATE);
    //             } else if (p.isProtected) {
    //                 visibility.setNum(QCodeNode::VISIBILITY_PROTECTED);
    //             } else {
    //                 visibility.setNum(QCodeNode::VISIBILITY_PUBLIC);
    //             }

    //             node->setRole(QCodeNode::Visibility, visibility);
    //             node->setParent(clsnode);
    //             clsnode->children().append(node);
    //         }
    //         ret.append(clsnode);
    //     }

    //     return ret;
    // };

    // for (auto p = _maps.keyValueBegin(); p != _maps.keyValueEnd(); p++) {
    //     auto node = getNewHeadNodePointer(p->first);
    //     if (p->first.isEmpty()) {
    //         node->setNodeType(QCodeNode::Group);
    //     } else {
    //         node->setNodeType(QCodeNode::Namespace);
    //     }
    //     auto nodes = applyClsNode(p->second, false);
    //     for (auto &n : nodes) {
    //         n->attach(node);
    //     }

    //     _classNodes.append(applyClsNode(p->second, true));
    // }
}

const QList<CodeInfoTip> &ASDataBase::keywordNodes() const {
    return _keywordNode;
}
