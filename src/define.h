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

#include <QtAssert>
#include <qtpreprocessorsupport.h>

enum class CrashCode : int {
    AlreadyStart,
    LanguageFile,
    PluginSetting,
    ScriptInitFailed,
    GenericCallNotSupported,
    OutofMemory
};

namespace AsUserDataType {
enum AsUserDataType : unsigned int {
    // internal user data
    UserData_API,
    UserData_PluginFn,
    UserData_StringTypeInfo,
    UserData_DateTimeTypeInfo,
    UserData_JsonValueTypeInfo,
    UserData_ByteArrayTypeInfo,
    UserData_StringListTypeInfo,
    UserData_CharArrayTypeInfo,
    UserData_CharTypeInfo,
    UserData_DictionaryTypeInfo,
    UserData_DictionaryValueTypeInfo,
    UserData_AnyTypeInfo,
    UserData_ArrayTypeInfo,

    // copyable context attribute
    UserData_CopyAttr_Begin = 100,
    UserData_isDbg = UserData_CopyAttr_Begin,
    UserData_ContextMode,
    UserData_Timer,
    UserData_TimeOut,
    UserData_Section_StringPtr,
    UserData_CopyAttr_End,

    // The AngelScript add-ons have reserved the numbers 1000
    // through 1999 for this purpose.
    UserData_Revserved_Begin = 1000,
    // const asPWORD CONTEXT_MGR = 1002;
    UserData_ContextMgr = 1002,
    UserData_Revserved_End = 1999
};
}

inline void ASSERT(bool b) { Q_ASSERT(b); }

#endif // DEFINE_H
