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
