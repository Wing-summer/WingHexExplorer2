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

#include "class/settings/thememanager.h"

// The interaction tests exercise real completion edits and tooltip widgets.
// Theme loading is independent and would require the full application settings.
ThemeManager::ThemeManager() : QObject(nullptr) {}

ThemeManager::~ThemeManager() = default;

ThemeManager &ThemeManager::instance() {
    static ThemeManager manager;
    return manager;
}

QIcon ThemeManager::resourceIcon(const QString &) const { return {}; }
