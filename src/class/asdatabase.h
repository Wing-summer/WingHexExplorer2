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

#ifndef _QAS_PARSER_H_
#define _QAS_PARSER_H_

#include "angelscript.h"
#include "codeinfotip.h"

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QScopedPointer>

class ASDataBase {
public:
    explicit ASDataBase(asIScriptEngine *engine);
    virtual ~ASDataBase();

private:
    struct FnInfo {
        QByteArray retType;
        QByteArray fnName;
        QByteArray params;
        bool isConst = false;
    };

    struct EnumInfo {
        QByteArray name;
        QList<QPair<QByteArray, int>> enums;
    };

    struct PropertyInfo {
        QByteArray name;
        QByteArray type;
        bool isProtected = false;
        bool isPrivate = false;
        bool isRef = false;
    };

    struct ClassInfo {
        QByteArray name;
        QList<FnInfo> methods;
        QList<PropertyInfo> properties;
    };

public:
    struct HeaderType {
        QString name;
        CodeInfoTip::Type type = CodeInfoTip::Type::Unknown;

        bool operator==(const HeaderType &other) const {
            return name == other.name && type == other.type;
        }
    };

private:
    QByteArray getFnParamDeclString(asIScriptFunction *fn,
                                    bool includeNamespace,
                                    bool includeParamNames);

    QByteArray getFnRealName(asIScriptFunction *fn);

    QByteArray getFnRetTypeString(asIScriptFunction *fn, bool includeNamespace);

public:
    const QHash<HeaderType, QList<CodeInfoTip>> &headerNodes() const;

    const QList<CodeInfoTip> &keywordNodes() const;

private:
    void addGlobalFunctionCompletion(asIScriptEngine *engine);
    void addEnumCompletion(asIScriptEngine *engine);
    void addClassCompletion(asIScriptEngine *engine);

private:
    QHash<HeaderType, QList<CodeInfoTip>> _headerNodes;
    QList<CodeInfoTip> _keywordNode;
};

#endif // !_QCPP_PARSER_H_
