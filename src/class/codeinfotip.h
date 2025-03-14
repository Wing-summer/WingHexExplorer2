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

#include <QString>

class CodeInfoTip {
public:
    enum class Type { Class, Function, Enum, Variable };

public:
    QString name;
    Type type;
    QString nameSpace;

    QString args; // only function use
};

#endif // CODEINFOTIP_H
