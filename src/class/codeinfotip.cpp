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

#include "codeinfotip.h"

#include <QHash>

using IconHash = QHash<int, QIcon>;
Q_GLOBAL_STATIC(IconHash, q_icon_cache)

inline static QIcon getIcon(const QString &name) {
    return QIcon(QStringLiteral(":/completion/images/completion/") + name +
                 QStringLiteral(".png"));
}

QIcon CodeInfoTip::getDisplayIcon(Type type, CodeInfoVisibility vis) {
    switch (type) {
    default:
        return {};
    case Type::KeyWord:
        return icon(ICON_KEYWORD);
    case Type::Group:
        return icon(ICON_NAMESPACE);
    case Type::Class:
    case Type::TypeDef:
        return icon(ICON_CLASS);
    case Type::ClsFunction:
    case Type::Function:
        return icon(ICON_FUNCTION + vis);
    case Type::Enum:
        return icon(ICON_ENUM);
    case Type::Variable:
        return icon(ICON_VARIABLE + vis);
    case Type::Enumerater:
        return icon(ICON_ENUMERATOR);
    }
}

QIcon CodeInfoTip::icon(int cacheIndex) {
    static bool setup = false;

    if (!setup) {
        // q_icon_cache[ICON_UNION] = ;

        (*q_icon_cache)[CodeInfoTip::ICON_ENUM] =
            getIcon(QStringLiteral("CVenum"));
        (*q_icon_cache)[CodeInfoTip::ICON_ENUMERATOR] =
            getIcon(QStringLiteral("CVenumerator"));

        (*q_icon_cache)[CodeInfoTip::ICON_CLASS] =
            getIcon(QStringLiteral("CVclass"));

        // q_icon_cache[ICON_STRUCT] = ;
        (*q_icon_cache)[CodeInfoTip::ICON_KEYWORD] =
            getIcon(QStringLiteral("CVKeyword"));

        (*q_icon_cache)[CodeInfoTip::ICON_TYPEDEF] =
            getIcon(QStringLiteral("CVtypedef"));

        (*q_icon_cache)[CodeInfoTip::ICON_NAMESPACE] =
            getIcon(QStringLiteral("CVnamespace"));

        (*q_icon_cache)[CodeInfoTip::ICON_FUNCTION +
                        CodeInfoTip::VISIBILITY_DEFAULT] =
            getIcon(QStringLiteral("CVglobal_meth"));

        (*q_icon_cache)[CodeInfoTip::ICON_FUNCTION +
                        CodeInfoTip::VISIBILITY_PUBLIC] =
            getIcon(QStringLiteral("CVpublic_meth"));

        (*q_icon_cache)[CodeInfoTip::ICON_FUNCTION +
                        CodeInfoTip::VISIBILITY_PROTECTED] =
            getIcon(QStringLiteral("CVprotected_meth"));

        (*q_icon_cache)[CodeInfoTip::ICON_FUNCTION +
                        CodeInfoTip::VISIBILITY_PRIVATE] =
            getIcon(QStringLiteral("CVprivate_meth"));

        (*q_icon_cache)[CodeInfoTip::ICON_VARIABLE +
                        CodeInfoTip::VISIBILITY_DEFAULT] =
            getIcon(QStringLiteral("CVglobal_var"));

        (*q_icon_cache)[CodeInfoTip::ICON_VARIABLE +
                        CodeInfoTip::VISIBILITY_PUBLIC] =
            getIcon(QStringLiteral("CVpublic_var"));

        (*q_icon_cache)[CodeInfoTip::ICON_VARIABLE +
                        CodeInfoTip::VISIBILITY_PROTECTED] =
            getIcon(QStringLiteral("CVprotected_var"));

        (*q_icon_cache)[CodeInfoTip::ICON_VARIABLE +
                        CodeInfoTip::VISIBILITY_PRIVATE] =
            getIcon(QStringLiteral("CVprivate_var"));

        setup = true;
    }

    return q_icon_cache->value(cacheIndex);
}

QString CodeInfoTip::getTooltip() const {
    QString tip = addinfo.value(Comment);
    if (!tip.isEmpty()) {
        return tip;
    }

    tip = name;
    if (!nameSpace.isEmpty()) {
        tip.prepend(QStringLiteral("::")).prepend(nameSpace);
    }

    if (type == Type::Function || type == Type::ClsFunction) {
        tip.prepend(' ')
            .prepend(addinfo.value(RetType))
            .append('(')
            .append(addinfo.value(Args))
            .append(')');

        auto cq = addinfo.value(SuffixQualifier);
        if (!cq.isEmpty()) {
            tip.append(' ').append(cq);
        }
    }

    return tip;
}

QString CodeInfoTip::getDisplayStr() const {
    QString tip = addinfo.value(Comment);
    if (!tip.isEmpty()) {
        return tip;
    }

    tip = name;

    if (type == Type::ClsFunction && !nameSpace.isEmpty()) {
        tip.prepend(QStringLiteral("::")).prepend(nameSpace);
    }

    if (type == Type::Function || type == Type::ClsFunction) {
        tip.append('(').append(addinfo.value(Args)).append(')');
    }

    return tip;
}
