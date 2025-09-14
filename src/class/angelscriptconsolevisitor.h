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

#ifndef ANGELSCRIPTCONSOLEVISITOR_H
#define ANGELSCRIPTCONSOLEVISITOR_H

#include "grammar/ASConsole/AngelscriptConsoleParserBaseVisitor.h"

#include <QByteArray>
#include <QByteArrayList>

class AngelScriptConsoleVisitor : public AngelscriptConsoleParserBaseVisitor {
public:
    AngelScriptConsoleVisitor(antlr4::CommonTokenStream &codes);

public:
    std::any visitScript(AngelscriptConsoleParser::ScriptContext *context);

    QByteArrayList declCode() const;

    QByteArray execCode() const;

private:
    QByteArray getSrcCode(antlr4::ParserRuleContext *content);
    QByteArray getSrcCode(antlr4::Token *start, antlr4::Token *end);

private:
    QByteArrayList _declCode;
    QByteArray _execCode;

    antlr4::CommonTokenStream &_codes;
};

#endif // ANGELSCRIPTCONSOLEVISITOR_H
