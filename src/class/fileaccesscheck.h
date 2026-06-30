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

#ifndef FILEACCESSCHECK_H
#define FILEACCESSCHECK_H

#include <QFileInfo>

class FileAccessCheck {
public:
    enum class Status { OK, Deny, Unknown };

public:
    static Status canStandardUserWriteFile(const QFileInfo &file);
    static Status canStandardUserWriteFile(const QString &filePath);
};

#endif // FILEACCESSCHECK_H
