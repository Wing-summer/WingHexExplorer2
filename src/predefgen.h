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
#include <QString>
#include <QStringView>
#include <QTextStream>

#include <cassert>
#include <string>

#include "angelscript.h"

namespace {
static QString toQString(const char *s) {
    return s ? QString::fromUtf8(s) : QString();
}

void printEnumList(const asIScriptEngine *engine, QTextStream &out) {
    for (int i = 0; i < engine->GetEnumCount(); ++i) {
        const auto e = engine->GetEnumByIndex(i);
        if (!e)
            continue;

        const QString ns = toQString(e->GetNamespace());
        if (!ns.isEmpty())
            out << QStringLiteral("namespace %1 {\n").arg(ns);

        out << QStringLiteral("enum %1 {\n").arg(toQString(e->GetName()));
        for (int j = 0; j < e->GetEnumValueCount(); ++j) {
            asINT64 value = 0;
            const char *name =
                e->GetEnumValueByIndex(static_cast<asUINT>(j), &value);
            const QString qname =
                name ? QString::fromUtf8(name) : QStringLiteral("<unnamed>");

            out << '\t' << qname << " = " << QString::number(value);
            if (j < e->GetEnumValueCount() - 1)
                out << ',';
            out << '\n';
        }
        out << "}\n";
        if (!ns.isEmpty())
            out << "}\n";
    }
}

void printClassTypeList(const asIScriptEngine *engine, QTextStream &out) {
    for (int i = 0; i < engine->GetObjectTypeCount(); ++i) {
        const auto t = engine->GetObjectTypeByIndex(i);
        if (!t)
            continue;

        const QString ns = toQString(t->GetNamespace());
        if (!ns.isEmpty())
            out << QStringLiteral("namespace %1 {\n").arg(ns);

        out << QStringLiteral("class %1").arg(toQString(t->GetName()));

        if (t->GetSubTypeCount() > 0) {
            out << '<';
            for (int sub = 0; sub < t->GetSubTypeCount(); ++sub) {
                if (sub > 0)
                    out << ", ";
                const auto st = t->GetSubType(sub);
                out << toQString(st->GetName());
            }
            out << '>';
        }

        out << "{\n";

        for (int j = 0; j < t->GetBehaviourCount(); ++j) {
            asEBehaviours behaviours;
            const auto f = t->GetBehaviourByIndex(j, &behaviours);
            if (!f)
                continue;

            if (behaviours == asBEHAVE_CONSTRUCT ||
                behaviours == asBEHAVE_DESTRUCT) {
                // Store declaration in std::string to accept either const char*
                // or std::string return types
                std::string decl = f->GetDeclaration(false, true, true);
                out << '\t' << QString::fromStdString(decl) << ";\n";
            }
        }

        for (int j = 0; j < t->GetMethodCount(); ++j) {
            const auto m = t->GetMethodByIndex(j);
            if (!m)
                continue;
            std::string decl = m->GetDeclaration(false, true, true);
            out << '\t' << QString::fromStdString(decl) << ";\n";
        }

        for (int j = 0; j < t->GetPropertyCount(); ++j) {
            std::string propDecl = t->GetPropertyDeclaration(j, true);
            out << '\t' << QString::fromStdString(propDecl) << ";\n";
        }

        for (int j = 0; j < t->GetChildFuncdefCount(); ++j) {
            const auto fd = t->GetChildFuncdef(j);
            if (!fd)
                continue;
            const auto sig = fd->GetFuncdefSignature();
            if (!sig)
                continue;
            std::string fdDecl = sig->GetDeclaration(false);
            out << '\t' << "funcdef " << QString::fromStdString(fdDecl)
                << ";\n";
        }

        out << "}\n";
        if (!ns.isEmpty())
            out << "}\n";
    }
}

void printGlobalFunctionList(const asIScriptEngine *engine, QTextStream &out) {
    for (int i = 0; i < engine->GetGlobalFunctionCount(); ++i) {
        const auto f = engine->GetGlobalFunctionByIndex(i);
        if (!f)
            continue;

        const QString ns = toQString(f->GetNamespace());
        if (!ns.isEmpty())
            out << QStringLiteral("namespace %1 { ").arg(ns);

        std::string decl = f->GetDeclaration(false, false, true);
        out << QString::fromStdString(decl) << ';';

        if (!ns.isEmpty())
            out << " }";
        out << '\n';
    }
}

void printGlobalPropertyList(const asIScriptEngine *engine, QTextStream &out) {
    for (int i = 0; i < engine->GetGlobalPropertyCount(); ++i) {
        const char *name = nullptr;
        const char *ns0 = nullptr;
        int typeId = 0;
        engine->GetGlobalPropertyByIndex(i, &name, &ns0, &typeId, nullptr,
                                         nullptr, nullptr, nullptr);

        // GetTypeDeclaration commonly returns a std::string or const char*;
        // store into std::string first
        std::string t = engine->GetTypeDeclaration(typeId, true);
        if (t.empty())
            continue;

        const QString ns = toQString(ns0);
        if (!ns.isEmpty())
            out << QStringLiteral("namespace %1 { ").arg(ns);

        out << QString::fromStdString(t) << ' ' << toQString(name) << ';';

        if (!ns.isEmpty())
            out << " }";
        out << '\n';
    }
}

void printGlobalTypedef(const asIScriptEngine *engine, QTextStream &out) {
    for (int i = 0; i < engine->GetTypedefCount(); ++i) {
        const auto type = engine->GetTypedefByIndex(i);
        if (!type)
            continue;

        const QString ns = toQString(type->GetNamespace());
        if (!ns.isEmpty())
            out << QStringLiteral("namespace %1 {\n").arg(ns);

        auto underlying =
            engine->GetTypeDeclaration(type->GetUnderlyingTypeId());
        out << QStringLiteral("typedef %1 %2;\n")
                   .arg(QString::fromStdString(underlying),
                        toQString(type->GetName()));

        if (!ns.isEmpty())
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

        const QString ns = toQString(fn->GetNamespace());
        if (!ns.isEmpty())
            out << QStringLiteral("namespace %1 {\n").arg(ns);

        out << QStringLiteral("funcdef %1;\n")
                   .arg(toQString(fn->GetDeclaration()));

        if (!ns.isEmpty())
            out << "}\n";
    }
}

void printMarcos(QTextStream &out) {
    // marco, type
    static QHash<QString, QString> marcos;
    static QString INT_TYPE = QStringLiteral("int");
    static QString STRING_TYPE = QStringLiteral("string");
    if (marcos.isEmpty()) {
        marcos.insert(QStringLiteral("__LINE__"), INT_TYPE);
        marcos.insert(QStringLiteral("__SECTION__"), STRING_TYPE);
        marcos.insert(QStringLiteral("__SECTION_BASE__"), STRING_TYPE);
        marcos.insert(QStringLiteral("__WING_VERSION__"), STRING_TYPE);
        marcos.insert(QStringLiteral("__WING_VERSION_MAJOR__"), INT_TYPE);
        marcos.insert(QStringLiteral("__WING_VERSION_MINOR__"), INT_TYPE);
        marcos.insert(QStringLiteral("__WING_VERSION_PATCH__"), INT_TYPE);
        marcos.insert(QStringLiteral("__ANGELSCRIPT_VERSION__"), STRING_TYPE);
        marcos.insert(QStringLiteral("__ANGELSCRIPT_VERSION_MAJOR__"),
                      INT_TYPE);
        marcos.insert(QStringLiteral("__ANGELSCRIPT_VERSION_MINOR__"),
                      INT_TYPE);
        marcos.insert(QStringLiteral("__ANGELSCRIPT_VERSION_PATCH__"),
                      INT_TYPE);
        marcos.insert(QStringLiteral("__WINGHEX_APPNAME__"), STRING_TYPE);
        marcos.insert(QStringLiteral("__WINGHEX_AUTHOR__"), STRING_TYPE);
        marcos.insert(QStringLiteral("__QT_VERSION__"), STRING_TYPE);
        marcos.insert(QStringLiteral("__QT_VERSION_MAJOR__"), INT_TYPE);
        marcos.insert(QStringLiteral("__QT_VERSION_MINOR__"), INT_TYPE);
        marcos.insert(QStringLiteral("__QT_VERSION_PATCH__"), INT_TYPE);
        marcos.insert(QStringLiteral("__LANG__"), STRING_TYPE);
        marcos.insert(QStringLiteral("__THEME__"), STRING_TYPE);
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
