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

const QHash<QString, QList<CodeInfoTip>> &ASDataBase::headerNodes() const {
    return _headerNodes;
}

void ASDataBase::addGlobalFunctionCompletion(asIScriptEngine *engine) {
    Q_ASSERT(engine);

    for (asUINT i = 0; i < engine->GetGlobalFunctionCount(); ++i) {
        auto fn = engine->GetGlobalFunctionByIndex(i);
        CodeInfoTip fnInfo;
        auto ns = fn->GetNamespace();
        fnInfo.nameSpace = ns;
        fnInfo.name = fn->GetName();
        fnInfo.type = CodeInfoTip::Type::Function;
        fnInfo.addinfo.insert(CodeInfoTip::RetType,
                              getFnRetTypeString(fn, true));
        fnInfo.addinfo.insert(CodeInfoTip::Args,
                              getFnParamDeclString(fn, false, true));
        fnInfo.addinfo.insert(CodeInfoTip::SuffixQualifier,
                              getSuffixQualifier(fn));
        _headerNodes[ns].append(fnInfo);
    }
}

void ASDataBase::addEnumCompletion(asIScriptEngine *engine) {
    Q_ASSERT(engine);

    for (asUINT i = 0; i < engine->GetEnumCount(); ++i) {
        auto etype = engine->GetEnumByIndex(i);
        etype->AddRef();

        CodeInfoTip einfo;
        auto ns = etype->GetNamespace();
        einfo.nameSpace = ns;
        einfo.name = etype->GetName();
        einfo.type = CodeInfoTip::Type::Enum;

        for (asUINT i = 0; i < etype->GetEnumValueCount(); ++i) {
            int v;
            auto e = etype->GetEnumValueByIndex(i, &v);

            CodeInfoTip en;
            en.type = CodeInfoTip::Type::Enumerater;
            en.name = QString::fromLatin1(e);
            en.addinfo.insert(CodeInfoTip::Comment, en.name +
                                                        QStringLiteral(" = ") +
                                                        QString::number(v));
            einfo.children.append(en);
        }

        etype->Release();
        _headerNodes[ns].append(einfo);
    }
}

void ASDataBase::addClassCompletion(asIScriptEngine *engine) {
    auto eng = dynamic_cast<asCScriptEngine *>(engine);
    Q_ASSERT(eng);

    for (asUINT i = 0; i < engine->GetObjectTypeCount(); ++i) {
        auto obj = eng->registeredObjTypes[i];
        obj->AddRef();

        CodeInfoTip cls;
        cls.type = CodeInfoTip::Type::Class;
        cls.name = obj->GetName();
        auto ns = obj->GetNamespace();

        for (asUINT i = 0; i < obj->GetBehaviourCount(); ++i) {
            asEBehaviours bv;
            auto b = obj->GetBehaviourByIndex(i, &bv);

            switch (bv) {
            case asBEHAVE_CONSTRUCT: {
                // only these are supported
                b->AddRef();
                CodeInfoTip fn;
                fn.type = CodeInfoTip::Type::ClsFunction;
                fn.nameSpace = cls.name;
                fn.name = getFnRealName(b);
                fn.addinfo.insert(CodeInfoTip::Args,
                                  getFnParamDeclString(b, false, true));
                fn.addinfo.insert(CodeInfoTip::SuffixQualifier,
                                  getSuffixQualifier(b));
                cls.children.append(fn);
                b->Release();
            }
            default:
                continue;
            }
        }

        for (asUINT i = 0; i < obj->GetMethodCount(); ++i) {
            auto m = obj->GetMethodByIndex(i, true);

            m->AddRef();
            CodeInfoTip fn;
            fn.type = CodeInfoTip::Type::ClsFunction;
            fn.nameSpace = cls.name;
            fn.addinfo.insert(CodeInfoTip::RetType,
                              getFnRetTypeString(m, true));
            fn.name = getFnRealName(m);
            fn.addinfo.insert(CodeInfoTip::Args,
                              getFnParamDeclString(m, false, true));
            fn.addinfo.insert(CodeInfoTip::SuffixQualifier,
                              getSuffixQualifier(m));
            cls.children.append(fn);
            m->Release();
        }

        for (asUINT i = 0; i < obj->GetPropertyCount(); ++i) {
            auto p = obj->properties[i];

            CodeInfoTip pi;
            pi.type = CodeInfoTip::Type::Property;
            pi.nameSpace = cls.name;
            pi.name =
                QString::fromLatin1(p->name.AddressOf(),
                                    QByteArray::size_type(p->name.GetLength()));
            auto tn = p->type.Format(obj->nameSpace);
            auto type = QString::fromLatin1(
                tn.AddressOf(), QByteArray::size_type(tn.GetLength()));

            QString prefix;
            if (p->isPrivate) {
                prefix = QStringLiteral("(private) ");
            } else if (p->isProtected) {
                prefix = QStringLiteral("(protected) ");
            } else {
                prefix = QStringLiteral("(public) ");
            }

            pi.addinfo.insert(CodeInfoTip::Comment,
                              prefix + type + QChar(' ') + pi.name);
            cls.children.append(pi);
        }

        obj->Release();

        _headerNodes[ns].append(cls);
    }
}

QString ASDataBase::getSuffixQualifier(asIScriptFunction *fn) {
    if (fn) {
        return fn->IsReadOnly() ? QStringLiteral("const") : QString();
    }
    return {};
}

const QList<CodeInfoTip> &ASDataBase::keywordNodes() const {
    return _keywordNode;
}
