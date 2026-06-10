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

#include "fileaccesscheck.h"

#ifdef Q_OS_WIN
#include <aclapi.h>
#include <windows.h>
#endif

#ifdef Q_OS_LINUX
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusVariant>

#include <grp.h>
#include <pwd.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

bool FileAccessCheck::canStandardUserWriteFile(const QFileInfo &file) {
    if (!file.isFile()) {
        return false;
    }
    return canStandardUserWriteFile(file.absoluteFilePath());
}

#ifdef Q_OS_LINUX

#define Q_DECLARE_AND_REGISTER_METATYPE(TYPE)                                  \
    Q_DECLARE_METATYPE(TYPE)                                                   \
    struct TYPE##_metatype_registrar {                                         \
        class inner {                                                          \
            inner() { qRegisterMetaType<TYPE>(); }                             \
            inner(const inner &) = delete;                                     \
            void operator=(const inner &) = delete;                            \
                                                                               \
        public:                                                                \
            static inner &once() {                                             \
                static inner instance;                                         \
                return instance;                                               \
            }                                                                  \
        };                                                                     \
        TYPE##_metatype_registrar() { inner::once(); }                         \
    };                                                                         \
    Q_GLOBAL_STATIC(TYPE##_metatype_registrar, _##TYPE##_metatype_registrar);

struct Login1SessionRef {
    QString id;
    QDBusObjectPath path;
};

struct Login1UserRef {
    uint uid = 0;
    QDBusObjectPath path;
};

Q_DECLARE_AND_REGISTER_METATYPE(Login1SessionRef)
Q_DECLARE_AND_REGISTER_METATYPE(Login1UserRef)

static QDBusArgument &operator<<(QDBusArgument &arg,
                                 const Login1SessionRef &v) {
    arg.beginStructure();
    arg << v.id << v.path;
    arg.endStructure();
    return arg;
}
static const QDBusArgument &operator>>(const QDBusArgument &arg,
                                       Login1SessionRef &v) {
    arg.beginStructure();
    arg >> v.id >> v.path;
    arg.endStructure();
    return arg;
}
static QDBusArgument &operator<<(QDBusArgument &arg, const Login1UserRef &v) {
    arg.beginStructure();
    arg << v.uid << v.path;
    arg.endStructure();
    return arg;
}
static const QDBusArgument &operator>>(const QDBusArgument &arg,
                                       Login1UserRef &v) {
    arg.beginStructure();
    arg >> v.uid >> v.path;
    arg.endStructure();
    return arg;
}

template <typename T>
static bool variantToDbusStruct(const QVariant &value, T &out) {
    if (!value.canConvert<QDBusArgument>()) {
        return false;
    }
    const QDBusArgument arg = qvariant_cast<QDBusArgument>(value);
    arg >> out;
    return true;
}

static bool readProperty(const QString &service, const QString &path,
                         const QString &interface, const QString &property,
                         QVariant &value) {
    QDBusInterface props(service, path,
                         QStringLiteral("org.freedesktop.DBus.Properties"),
                         QDBusConnection::systemBus());
    if (!props.isValid()) {
        return false;
    }
    QDBusMessage reply = props.call(QStringLiteral("Get"), interface, property);
    if (reply.type() == QDBusMessage::ErrorMessage ||
        reply.arguments().isEmpty()) {
        return false;
    }
    auto args = reply.arguments();
    if (args.isEmpty()) {
        return false;
    }
    const QDBusVariant dbusVariant = qvariant_cast<QDBusVariant>(args.first());
    value = dbusVariant.variant();
    return true;
}

static bool getActiveDesktopUid(uid_t &uid) {
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected()) {
        return false;
    }
    QDBusInterface manager(QStringLiteral("org.freedesktop.login1"),
                           QStringLiteral("/org/freedesktop/login1"),
                           QStringLiteral("org.freedesktop.login1.Manager"),
                           bus);
    if (!manager.isValid()) {
        return false;
    }
    QDBusReply<QDBusObjectPath> seatReply =
        manager.call(QStringLiteral("GetSeat"), QStringLiteral("seat0"));
    if (!seatReply.isValid()) {
        return false;
    }
    QString seatPath = seatReply.value().path();
    if (seatPath.isEmpty() || seatPath == QLatin1String("/")) {
        return false;
    }
    QVariant activeSessionVar;
    if (!readProperty(QLatin1String("org.freedesktop.login1"), seatPath,
                      QLatin1String("org.freedesktop.login1.Seat"),
                      QLatin1String("ActiveSession"), activeSessionVar)) {
        return false;
    }
    Login1SessionRef activeSession;
    if (!variantToDbusStruct(activeSessionVar, activeSession)) {
        return false;
    }
    if (activeSession.path.path().isEmpty() ||
        activeSession.path.path() == QLatin1String("/")) {
        return false;
    }
    QVariant userVar;
    if (!readProperty(QLatin1String("org.freedesktop.login1"),
                      activeSession.path.path(),
                      QLatin1String("org.freedesktop.login1.Session"),
                      QLatin1String("User"), userVar)) {
        return false;
    }
    Login1UserRef user;
    if (!variantToDbusStruct(userVar, user)) {
        return false;
    }
    uid = uid_t(user.uid);
    return true;
}

#endif

bool FileAccessCheck::canStandardUserWriteFile(const QString &filePath) {
#ifdef Q_OS_WIN
    HANDLE hToken = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE,
                          &hToken)) {
        return false;
    }

    HANDLE hAccessToken = nullptr;
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID pAdminSid = nullptr;

    if (!AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                  &pAdminSid)) {
        CloseHandle(hToken);
        return false;
    }

    CheckTokenMembership(nullptr, pAdminSid, &isAdmin);
    if (isAdmin) {
        SID_AND_ATTRIBUTES sidToDisable = {pAdminSid, 0};
        HANDLE hRestrictedToken = nullptr;
        if (!CreateRestrictedToken(hToken, DISABLE_MAX_PRIVILEGE, 1,
                                   &sidToDisable, 0, nullptr, 0, nullptr,
                                   &hRestrictedToken)) {

            FreeSid(pAdminSid);
            CloseHandle(hToken);
            return false;
        }
        if (!DuplicateToken(hRestrictedToken, SecurityImpersonation,
                            &hAccessToken)) {

            CloseHandle(hRestrictedToken);
            FreeSid(pAdminSid);
            CloseHandle(hToken);
            return false;
        }
        CloseHandle(hRestrictedToken);
    } else {
        if (!DuplicateToken(hToken, SecurityImpersonation, &hAccessToken)) {
            FreeSid(pAdminSid);
            CloseHandle(hToken);
            return false;
        }
    }

    FreeSid(pAdminSid);
    CloseHandle(hToken);

    PSECURITY_DESCRIPTOR pSD = nullptr;
    DWORD err = GetNamedSecurityInfoW(
        reinterpret_cast<LPCWSTR>(filePath.utf16()), SE_FILE_OBJECT,
        OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
            DACL_SECURITY_INFORMATION,
        nullptr, nullptr, nullptr, nullptr, &pSD);

    if (err != ERROR_SUCCESS) {
        CloseHandle(hAccessToken);
        return false;
    }

    DWORD desiredAccess = FILE_GENERIC_WRITE;
    GENERIC_MAPPING mapping = {FILE_GENERIC_READ, FILE_GENERIC_WRITE,
                               FILE_GENERIC_EXECUTE, FILE_ALL_ACCESS};
    MapGenericMask(&desiredAccess, &mapping);

    PRIVILEGE_SET privilegeSet{};
    DWORD privilegeSetLength = sizeof(privilegeSet);
    DWORD grantedAccess = 0;
    BOOL accessStatus = FALSE;
    BOOL accessCheckResult =
        AccessCheck(pSD, hAccessToken, desiredAccess, &mapping, &privilegeSet,
                    &privilegeSetLength, &grantedAccess, &accessStatus);

    LocalFree(pSD);
    CloseHandle(hAccessToken);
    return accessCheckResult && accessStatus;
#else
    auto path = filePath.toUtf8();
    if (geteuid() != 0) {
        return access(path, W_OK) == 0;
    }

    uid_t targetUid;
    if (!getActiveDesktopUid(targetUid)) {
        return false;
    }

    struct passwd *pw = getpwuid(targetUid);
    if (!pw) {
        return false;
    }
    gid_t targetGid = pw->pw_gid;

    pid_t pid = fork();
    if (pid == -1) {
        return false;
    }

    if (pid == 0) {
        if (setgroups(0, nullptr) == -1) {
            _exit(1);
        }
        if (setgid(targetGid) == -1) {
            _exit(1);
        }
        if (setuid(targetUid) == -1) {
            _exit(1);
        }
        _exit(access(path, W_OK) == 0 ? 0 : 2);
    } else {
        int status;
        waitpid(pid, &status, 0);
        return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
    }
#endif
}
