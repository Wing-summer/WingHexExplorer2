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

#ifndef DEFINE_H
#define DEFINE_H

enum class CrashCode : int {
    AlreadyStart,
    LanguageFile,
    PluginSetting,
    ScriptInitFailed,
    GenericCallNotSupported,
    OutofMemory
};

namespace AsUserDataType {
enum AsUserDataType {
    UserData_API,
    UserData_PluginFn,
    UserData_isDbg,
    UserData_ContextMode,
    UserData_Timer,
    UserData_TimeOut
};
}

#endif // DEFINE_H
