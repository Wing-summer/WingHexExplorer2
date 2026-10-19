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

#ifndef CHANGEDSTRINGLIST_H
#define CHANGEDSTRINGLIST_H

#include <QHash>
#include <QString>
#include <QStringList>

class ChangedStringList {
public:
    bool containChanges() const;
    bool containChanges(const QString &item) const;
    void clear();

    void pushAddItem(const QString &item);
    void pushRemoveItem(const QString &item);

    QStringList getContents() const;
    void setContents(const QStringList &newContents);

private:
    QStringList contents;
    QHash<QString, bool> mods;
};

#endif // CHANGEDSTRINGLIST_H
