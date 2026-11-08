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

#include "changedstringlist.h"

bool ChangedStringList::containChanges() const { return !m_mods.isEmpty(); }

bool ChangedStringList::containChanges(const QString &item) const {
    return m_contents.contains(item);
}

void ChangedStringList::clear() {
    m_contents.clear();
    m_mods.clear();
}

void ChangedStringList::pushAddItem(const QString &item) {
    if (m_mods.contains(item)) {
        auto v = m_mods.value(item);
        if (v) {
            return;
        } else {
            m_mods.remove(item);
        }
    } else {
        m_mods.insert(item, true);
    }
    m_contents.append(item);
}

void ChangedStringList::pushRemoveItem(const QString &item) {
    if (m_mods.contains(item)) {
        auto v = m_mods.value(item);
        if (v) {
            m_mods.remove(item);
        } else {
            return;
        }
    } else {
        m_mods.insert(item, false);
    }
    m_contents.removeOne(item);
}

QStringList ChangedStringList::getContents() const { return m_contents; }

void ChangedStringList::setContents(const QStringList &newContents) {
    m_contents = newContents;
    m_mods.clear();
}
