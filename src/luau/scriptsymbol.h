/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#ifndef SCRIPTSYMBOL_H
#define SCRIPTSYMBOL_H

#include <QList>
#include <QString>

namespace WingScriptBind {
// An immutable snapshot of native registrations; no VM or frontend pointers.
struct ScriptSymbol {
    enum class Kind { Module, Class, Function, Property, Constant };
    QString id;
    QString parent;
    QString label;
    Kind kind = Kind::Module;
    QString detail;
};
using ScriptSymbolCatalog = QList<ScriptSymbol>;
} // namespace WingScriptBind

#endif // SCRIPTSYMBOL_H
