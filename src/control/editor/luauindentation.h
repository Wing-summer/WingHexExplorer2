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

#ifndef LUAUINDENTATION_H
#define LUAUINDENTATION_H

#include "WingCodeEdit/wingcodeedit.h"

namespace LuauIndentation {

// Source must contain only the current script up to the insertion point.
QString forNewLine(const QString &source, const WingCodeEdit &editor);

// Source includes the entire current line, excluding console prefixes.
std::optional<QString> forCurrentLine(const QString &source,
                                      const WingCodeEdit &editor);

std::optional<QString>
forEditor(const WingCodeEdit::IndentationContext &context);

} // namespace LuauIndentation

#endif // LUAUINDENTATION_H
