#ifndef DEFINE_H
#define DEFINE_H

enum class CrashCode : int {
    AlreadyStart,
    LanguageFile,
    PluginSetting,
    GenericCallNotSupported
};

namespace AsUserDataType {
enum AsUserDataType {
    UserData_ContextDbgInfo,
    UserData_API,
    UserData_PluginFn,
    UserData_isDbg,
    UserData_ContextMode,
};
}

#endif // DEFINE_H
