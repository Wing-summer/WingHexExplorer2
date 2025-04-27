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

#include "class/qascodeparser.h"
#include "control/scriptingconsole.h"

AsConsoleCompletion::AsConsoleCompletion(ScriptingConsole *p)
    : AsCompletion(p), _console(p) {}

QList<CodeInfoTip> AsConsoleCompletion::parseDocument() {
    auto editor = _console;
    if (editor == nullptr) {
        return {};
    }

    auto code = editor->currentCodes();
    auto engine = ScriptMachine::instance().engine();

    QAsCodeParser parser(engine);
    auto seg = parser.parse(code.toUtf8());
    if (std::find_if(seg.begin(), seg.end(),
                     [](const QAsCodeParser::CodeSegment &seg) {
                         return seg.isValid();
                     }) == seg.end()) {
        // wrap it to let code-completion work
        code.prepend("void main(){").append('}');
    }

    // first preprocess the code
    AsPreprocesser prepc(engine);

    // including is not supported in console
    auto r = prepc.loadSectionFromMemory(QStringLiteral("ASConCOMPLETION"),
                                         code.toUtf8());
    if (r <= 0) {
        return {};
    }

    auto data = prepc.scriptData();
    if (data.size() == 1) {
        auto d = data[0];
        return parseScriptData(d.script.length() - 1, d.script);
    } else {
        return {};
    }
}
