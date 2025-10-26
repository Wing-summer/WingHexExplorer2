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

#include "lspeditorinterface.h"
#include "appmanager.h"

void LspEditorInterace::syncUpdate() {
    sendDocChange();
    auto app = AppManager::instance();
    auto curTime = app->currentMSecsSinceEpoch();
    while (isContentLspUpdated()) {
        // wait for a moment
        // timeout for 100ms
        auto nowTime = app->currentMSecsSinceEpoch();
        if (nowTime - curTime > 100) {
            sendDocChange();
            curTime = nowTime;
        }
        app->processEvents();
    }
}
