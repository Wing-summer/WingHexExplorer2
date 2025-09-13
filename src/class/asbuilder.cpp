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

#include "asbuilder.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>

asBuilder::asBuilder(asIScriptEngine *engine) : AsPreprocesser(engine) {}

int asBuilder::build(asIScriptModule *module) {
    Q_ASSERT(module);
    if (module == nullptr) {
        return -1;
    }

    module->ResetGlobalVars();

    for (auto &mod : scriptData()) {
        auto script = mod.result.script.toUtf8();
        module->AddScriptSection(mod.section.toUtf8(), script.data(),
                                 script.size());
    }

    int r = module->Build();
    if (r < 0)
        return r;

    module->SetDefaultNamespace("");

    return 0;
}
