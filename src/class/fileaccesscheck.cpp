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
#include <grp.h>
#include <pwd.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

FileAccessCheck::Status
FileAccessCheck::canStandardUserWriteFile(const QFileInfo &file) {
    if (!file.isFile()) {
        return Status::Unknown;
    }
    return canStandardUserWriteFile(file.absoluteFilePath());
}

#ifdef Q_OS_LINUX

static bool getActiveDesktopUid(uid_t &uid) {
    auto runtime = getenv("XDG_RUNTIME_DIR");

    if (!runtime) {
        return false;
    }

    const QLatin1String path(runtime);
    const auto st = QLatin1String("/run/user/");
    if (!path.startsWith(st)) {
        return false;
    }

    bool ok = false;
    uid = path.sliced(st.size()).toUInt(&ok);
    return ok;
}

#endif

FileAccessCheck::Status
FileAccessCheck::canStandardUserWriteFile(const QString &filePath) {
#ifdef Q_OS_WIN
    HANDLE hToken = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE,
                          &hToken)) {
        return Status::Unknown;
    }

    HANDLE hAccessToken = nullptr;
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID pAdminSid = nullptr;

    if (!AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                  &pAdminSid)) {
        CloseHandle(hToken);
        return Status::Unknown;
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
            return Status::Unknown;
        }
        if (!DuplicateToken(hRestrictedToken, SecurityImpersonation,
                            &hAccessToken)) {

            CloseHandle(hRestrictedToken);
            FreeSid(pAdminSid);
            CloseHandle(hToken);
            return Status::Unknown;
        }
        CloseHandle(hRestrictedToken);
    } else {
        if (!DuplicateToken(hToken, SecurityImpersonation, &hAccessToken)) {
            FreeSid(pAdminSid);
            CloseHandle(hToken);
            return Status::Unknown;
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
        return Status::Unknown;
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
    return (accessCheckResult && accessStatus) ? Status::OK : Status::Deny;
#else
    auto path = filePath.toUtf8();
    if (geteuid() != 0) {
        return access(path, W_OK) == 0 ? Status::OK : Status::Deny;
    }

    uid_t targetUid;
    if (!getActiveDesktopUid(targetUid)) {
        return Status::Unknown;
    }

    struct passwd *pw = getpwuid(targetUid);
    if (!pw) {
        return Status::Unknown;
    }
    gid_t targetGid = pw->pw_gid;

    pid_t pid = fork();
    if (pid == -1) {
        return Status::Unknown;
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
        return (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? Status::OK
                                                               : Status::Deny;
    }
#endif
}
