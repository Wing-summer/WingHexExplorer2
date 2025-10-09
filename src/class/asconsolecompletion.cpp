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

#include "asconsolecompletion.h"
#include "control/scriptingconsole.h"

#include <QJsonArray>

AsConsoleCompletion::AsConsoleCompletion(ScriptingConsole *p)
    : AsCompletion(p), _console(p) {}

AsConsoleCompletion::AsConsoleCompletion(ConsoleCodeEdit *p)
    : AsCompletion(p), _console(p) {}

QList<CodeInfoTip> AsConsoleCompletion::parseMarcos() {
    static QList<CodeInfoTip> marcos;
    if (marcos.isEmpty()) {
        static QStringList m{
            "ls",
            "del",
            "cls",
        };
        for (auto &i : m) {
            CodeInfoTip tip;
            tip.name = i;
            tip.type = LSP::CompletionItemKind::Keyword;
            marcos.append(tip);
        }
    }
    return marcos;
}

LspEditorInterace *AsConsoleCompletion::getEditor() const { return _console; }
