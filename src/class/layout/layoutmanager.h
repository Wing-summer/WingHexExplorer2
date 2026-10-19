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

#ifndef LAYOUTMANAGER_H
#define LAYOUTMANAGER_H

#include <QDir>
#include <QHash>
#include <QSettings>
#include <QString>

class LayoutManager {
public:
    static LayoutManager &instance();

public:
    const QHash<QString, QByteArray> &layouts() const;

    QStringList layoutNames() const;

    void addLayout(const QString &name, const QByteArray &data);

    bool contains(const QString &name);

    QByteArray layout(const QString &v) const;

    qsizetype layoutCount() const;

    static QDir layoutDir();

private:
    LayoutManager();

    void process(const QDir &dir);

private:
    QHash<QString, QByteArray> _layouts;
    QStringList _caches;
    Q_DISABLE_COPY_MOVE(LayoutManager)
};

#endif // LAYOUTMANAGER_H
