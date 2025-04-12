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
    QByteArray getFnParamDeclString(asIScriptFunction *fn,
                                    bool includeNamespace,
                                    bool includeParamNames);

    QByteArray getFnRealName(asIScriptFunction *fn);

    QByteArray getFnRetTypeString(asIScriptFunction *fn, bool includeNamespace);

public:
    const QHash<QString, QList<CodeInfoTip>> &headerNodes() const;
    const QHash<QString, QList<CodeInfoTip>> &enumsNodes() const;

    const QList<CodeInfoTip> &keywordNodes() const;

private:
    void addGlobalFunctionCompletion(asIScriptEngine *engine);
    void addEnumCompletion(asIScriptEngine *engine);
    void addClassCompletion(asIScriptEngine *engine);

    QString getSuffixQualifier(asIScriptFunction *fn);

private:
    // <namespace, content>
    QHash<QString, QList<CodeInfoTip>> _headerNodes;
    QHash<QString, QList<CodeInfoTip>> _enumsNodes;
    QList<CodeInfoTip> _keywordNode;
};

#endif // !_QCPP_PARSER_H_
