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

#include "angelscriptconsolevisitor.h"

#include <QRegularExpression>

AngelScriptConsoleVisitor::AngelScriptConsoleVisitor(
    antlr4::CommonTokenStream &codes)
    : _codes(codes) {}

std::any AngelScriptConsoleVisitor::visitScript(
    AngelscriptConsoleParser::ScriptContext *context) {
    if (context == nullptr) {
        return defaultResult();
    }

    _declCode.clear();

    for (auto &item : context->scriptItem()) {
        if (item->declSpecifierSeq()) {
            _declCode.append(getSrcCode(item));
        }
    }

    return defaultResult();
}

QByteArray
AngelScriptConsoleVisitor::getSrcCode(antlr4::ParserRuleContext *content) {
    Q_ASSERT(content);
    return QByteArray::fromStdString(
        _codes.getText(content->getSourceInterval()));
}

QByteArray AngelScriptConsoleVisitor::getSrcCode(antlr4::Token *start,
                                                 antlr4::Token *end) {
    if (start && end) {
        return QByteArray::fromStdString(_codes.getText(start, end));
    }
    return {};
}

QByteArrayList AngelScriptConsoleVisitor::declCode() const { return _declCode; }
