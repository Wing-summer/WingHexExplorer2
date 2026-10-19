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

#ifndef EDITORINFO_H
#define EDITORINFO_H

#include <QIcon>
#include <QString>

class EditorInfo {
public:
    virtual QIcon editorIcon() const = 0;
    virtual QString infoFileName() const = 0;
    virtual QString infoTooltip() const = 0;
};

class EditorsCtl {
public:
    virtual bool save(EditorInfo *info) = 0;
    virtual void discard(EditorInfo *info) = 0;
};

#endif // EDITORINFO_H
