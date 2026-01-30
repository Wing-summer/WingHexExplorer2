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

#ifndef AS_SCRIPTFILESYSTEM_H
#define AS_SCRIPTFILESYSTEM_H

#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif

#include "AngelScript/sdk/add_on/scriptarray/scriptarray.h"
#include "scriptdatetime.h"

#include <QString>

BEGIN_AS_NAMESPACE

class CScriptFileSystem {
public:
    CScriptFileSystem();

    void AddRef() const;
    void Release() const;

    // Sets the current path that should be used in other calls when using
    // relative paths It can use relative paths too, so moving up a directory is
    // used by passing in ".."
    bool ChangeCurrentPath(const QString &path);
    QString GetCurrentPath() const;

    // Returns true if the path is a directory. Input can be either a full
    // path or a relative path. This method does not return the dirs '.' and
    // '..'
    bool IsDir(const QString &path) const;

    // Returns true if the path is a link. Input can be either a full path
    // or a relative path
    bool IsLink(const QString &path) const;

    // Returns the size of file. Input can be either a full path or a
    // relative path
    asINT64 GetSize(const QString &path) const;

    // Returns a list of the files in the current path
    CScriptArray *GetFiles() const;

    // Returns a list of the directories in the current path
    CScriptArray *GetDirs() const;

    // Creates a new directory.
    bool MakeDir(const QString &path);

    // Removes a directory. Will only remove the directory if it is empty.
    bool RemoveDir(const QString &path);

    // Deletes a file.
    bool DeleteFile(const QString &path);

    // Copies a file.
    bool CopyFile(const QString &source, const QString &target);

    // Moves or renames a file.
    bool MoveFile(const QString &source, const QString &target);

    // Gets the date and time of the file/dir creation
    CDateTime GetCreateDateTime(const QString &path) const;

    // Gets the date and time of the file/dir modification
    CDateTime GetModifyDateTime(const QString &path) const;

protected:
    QString getRealAbsPath(const QString &path);

protected:
    ~CScriptFileSystem();

    mutable int refCount;
    QString currentPath;
};

void RegisterScriptFileSystem(asIScriptEngine *engine);

END_AS_NAMESPACE

#endif
