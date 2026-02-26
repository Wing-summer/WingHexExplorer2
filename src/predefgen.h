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

#ifndef PREDEFGEN_H
#define PREDEFGEN_H

#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QTextStream>

#include "angelscript.h"

namespace {

void printEnumList(const asIScriptEngine *engine, QTextStream &out) {
    for (int i = 0; i < engine->GetEnumCount(); ++i) {
        const auto e = engine->GetEnumByIndex(i);
        if (!e)
            continue;

        auto ns = e->GetNamespace();
        bool hasNs = qstrlen(ns) > 0;
        if (hasNs)
            out << "namespace " << ns << " {\n";

        out << "enum " << e->GetName() << " {\n";
        for (int j = 0; j < e->GetEnumValueCount(); ++j) {
            asINT64 value = 0;
            const char *name =
                e->GetEnumValueByIndex(static_cast<asUINT>(j), &value);

            out << '\t' << (name ? name : "<unnamed>") << " = " << value;
            if (j < e->GetEnumValueCount() - 1)
                out << ',';
            out << '\n';
        }
        out << "}\n";
        if (hasNs)
            out << "}\n";
    }
}

void printClassTypeList(const asIScriptEngine *engine, QTextStream &out) {
    for (int i = 0; i < engine->GetObjectTypeCount(); ++i) {
        const auto t = engine->GetObjectTypeByIndex(i);
        if (!t)
            continue;

        auto ns = t->GetNamespace();

        bool hasNs = qstrlen(ns) > 0;
        if (hasNs)
            out << "namespace " << ns << " {\n";

        out << "class " << t->GetName();

        if (t->GetSubTypeCount() > 0) {
            out << '<';
            for (int sub = 0; sub < t->GetSubTypeCount(); ++sub) {
                if (sub > 0)
                    out << ", ";
                const auto st = t->GetSubType(sub);
                out << st->GetName();
            }
            out << '>';
        }

        out << "{\n";

        for (int j = 0; j < t->GetFactoryCount(); ++j) {
            asEBehaviours behaviours;
            const auto f = t->GetFactoryByIndex(j);
            if (!f)
                continue;
            out << '\t' << f->GetDeclaration(false, true, true) << ";\n";
        }

        for (int j = 0; j < t->GetBehaviourCount(); ++j) {
            asEBehaviours behaviours;
            const auto f = t->GetBehaviourByIndex(j, &behaviours);
            if (!f)
                continue;

            if (behaviours == asBEHAVE_CONSTRUCT ||
                behaviours == asBEHAVE_DESTRUCT) {
                out << '\t' << f->GetDeclaration(false, true, true) << ";\n";
            }
        }

        for (int j = 0; j < t->GetMethodCount(); ++j) {
            const auto m = t->GetMethodByIndex(j);
            if (!m)
                continue;
            out << '\t' << m->GetDeclaration(false, true, true) << ";\n";
        }

        for (int j = 0; j < t->GetPropertyCount(); ++j) {
            out << '\t' << t->GetPropertyDeclaration(j, true) << ";\n";
        }

        for (int j = 0; j < t->GetChildFuncdefCount(); ++j) {
            const auto fd = t->GetChildFuncdef(j);
            if (!fd)
                continue;
            const auto sig = fd->GetFuncdefSignature();
            if (!sig)
                continue;
            out << '\t' << "funcdef " << sig->GetDeclaration(false) << ";\n";
        }

        out << "}\n";
        if (hasNs)
            out << "}\n";
    }
}

void printGlobalFunctionList(const asIScriptEngine *engine, QTextStream &out) {
    for (int i = 0; i < engine->GetGlobalFunctionCount(); ++i) {
        const auto f = engine->GetGlobalFunctionByIndex(i);
        if (!f)
            continue;

        auto ns = f->GetNamespace();
        bool hasNs = qstrlen(ns) > 0;
        if (hasNs)
            out << "namespace " << ns << " { ";

        out << f->GetDeclaration(false, false, true) << ';';

        if (hasNs)
            out << " }";
        out << '\n';
    }
}

void printGlobalPropertyList(const asIScriptEngine *engine, QTextStream &out) {
    for (int i = 0; i < engine->GetGlobalPropertyCount(); ++i) {
        const char *name = nullptr;
        const char *ns = nullptr;
        int typeId = 0;
        engine->GetGlobalPropertyByIndex(i, &name, &ns, &typeId, nullptr,
                                         nullptr, nullptr, nullptr);

        auto t = engine->GetTypeDeclaration(typeId, true);
        if (qstrlen(t) == 0)
            continue;

        bool hasNs = qstrlen(ns) > 0;
        if (hasNs)
            out << "namespace " << ns << " { ";

        out << t << ' ' << name << ';';

        if (hasNs)
            out << " }";
        out << '\n';
    }
}

void printGlobalTypedef(const asIScriptEngine *engine, QTextStream &out) {
    for (int i = 0; i < engine->GetTypedefCount(); ++i) {
        const auto type = engine->GetTypedefByIndex(i);
        if (!type)
            continue;

        auto ns = type->GetNamespace();
        bool hasNs = qstrlen(ns) > 0;
        if (hasNs)
            out << "namespace " << ns << " {\n";

        auto underlying =
            engine->GetTypeDeclaration(type->GetUnderlyingTypeId());
        out << "typedef " << underlying << ' ' << type->GetName() << ";\n";

        if (hasNs)
            out << "}\n";
    }
}

void printFuncdef(const asIScriptEngine *engine, QTextStream &out) {
    for (int i = 0; i < engine->GetFuncdefCount(); ++i) {
        const auto type = engine->GetFuncdefByIndex(i);
        if (!type)
            continue;

        const auto fn = type->GetFuncdefSignature();
        if (type->GetParentType()) { // check whether is in global scope
            continue;
        }

        auto ns = fn->GetNamespace();
        bool hasNs = qstrlen(ns) > 0;
        if (hasNs)
            out << "namespace " << ns << " {\n";

        out << "funcdef " << fn->GetDeclaration() << ";\n";

        if (hasNs)
            out << "}\n";
    }
}

void printMarcos(QTextStream &out) {
    // marco, type
    static QHash<QLatin1String, QString> marcos;
    static QLatin1String INT_TYPE = QLatin1String("int");
    static QLatin1String STRING_TYPE = QLatin1String("string");
    if (marcos.isEmpty()) {
        marcos.insert(QLatin1String("__LINE__"), INT_TYPE);
        marcos.insert(QLatin1String("__SECTION__"), STRING_TYPE);
        marcos.insert(QLatin1String("__SECTION_BASE__"), STRING_TYPE);
        marcos.insert(QLatin1String("__WING_VERSION__"), STRING_TYPE);
        marcos.insert(QLatin1String("__WING_VERSION_MAJOR__"), INT_TYPE);
        marcos.insert(QLatin1String("__WING_VERSION_MINOR__"), INT_TYPE);
        marcos.insert(QLatin1String("__WING_VERSION_PATCH__"), INT_TYPE);
        marcos.insert(QLatin1String("__ANGELSCRIPT_VERSION__"), STRING_TYPE);
        marcos.insert(QLatin1String("__ANGELSCRIPT_VERSION_MAJOR__"), INT_TYPE);
        marcos.insert(QLatin1String("__ANGELSCRIPT_VERSION_MINOR__"), INT_TYPE);
        marcos.insert(QLatin1String("__ANGELSCRIPT_VERSION_PATCH__"), INT_TYPE);
        marcos.insert(QLatin1String("__WINGHEX_APPNAME__"), STRING_TYPE);
        marcos.insert(QLatin1String("__WINGHEX_AUTHOR__"), STRING_TYPE);
        marcos.insert(QLatin1String("__QT_VERSION__"), STRING_TYPE);
        marcos.insert(QLatin1String("__QT_VERSION_MAJOR__"), INT_TYPE);
        marcos.insert(QLatin1String("__QT_VERSION_MINOR__"), INT_TYPE);
        marcos.insert(QLatin1String("__QT_VERSION_PATCH__"), INT_TYPE);
        marcos.insert(QLatin1String("__LANG__"), STRING_TYPE);
        marcos.insert(QLatin1String("__THEME__"), STRING_TYPE);
    }
    for (auto &&item : marcos.asKeyValueRange()) {
        out << item.second << ' ' << item.first << ';' << Qt::endl;
    }
}

} // namespace

static inline void generateScriptPredefined(const asIScriptEngine *engine,
                                            const QString &path) {
    Q_ASSERT(path.endsWith(QStringLiteral("as.predefined")));

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate |
                   QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << path;
        return;
    }

    QTextStream out(&file);

    printEnumList(engine, out);
    printClassTypeList(engine, out);
    printGlobalFunctionList(engine, out);
    printGlobalPropertyList(engine, out);
    printGlobalTypedef(engine, out);
    printFuncdef(engine, out);
    printMarcos(out);

    file.close();
}

#endif // PREDEFGEN_H
