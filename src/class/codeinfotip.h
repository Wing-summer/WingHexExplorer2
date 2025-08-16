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

#ifndef CODEINFOTIP_H
#define CODEINFOTIP_H

#include <QIcon>
#include <QMap>
#include <QObject>
#include <QString>

class CodeInfoTip {
public:
    enum class Type {
        Unknown,
        KeyWord,
        Group,
        Class,
        Function,
        ClsFunction,
        Enum,
        Variable,
        Property = Variable,
        Enumerater,
        TypeDef
    };

    enum CacheIndex {
        ICON_ENUM,
        ICON_ENUMERATOR,
        ICON_CLASS,
        ICON_KEYWORD,
        ICON_TYPEDEF,
        ICON_NAMESPACE,
        ICON_FUNCTION = ICON_NAMESPACE + 2,
        ICON_VARIABLE = ICON_FUNCTION + 5
    };

    enum CodeInfoVisibility {
        VISIBILITY_DEFAULT = -1,
        VISIBILITY_PUBLIC,
        VISIBILITY_PROTECTED,
        VISIBILITY_PRIVATE
    };

    static QIcon getDisplayIcon(Type type,
                                CodeInfoVisibility vis = VISIBILITY_DEFAULT);

    static QIcon icon(int cacheIndex);

    enum ArgsIndex {
        Comment,
        Reserved,
        // for function
        RetType = Reserved,
        Args,
        SuffixQualifier,
    };

public:
    QString getTooltip() const;
    QString getDisplayStr() const;

public:
    QString name;
    Type type = Type::Unknown;
    QString nameSpace;
    CodeInfoVisibility visualpo = CodeInfoVisibility::VISIBILITY_DEFAULT;

    QMap<ArgsIndex, QString> addinfo; // additonal info
    QList<CodeInfoTip> children;
};

Q_DECLARE_METATYPE(CodeInfoTip);

#endif // CODEINFOTIP_H
