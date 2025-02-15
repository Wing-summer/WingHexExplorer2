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

#ifndef SKINMANAGER_H
#define SKINMANAGER_H

#include <QObject>

class SkinManager : public QObject {
    Q_OBJECT
public:
    enum class Theme {
        Dark,
        Light,
    };
    Q_ENUM(Theme)

public:
    virtual ~SkinManager() override;

    static SkinManager &instance();

    void setTheme(Theme theme);
    Theme currentTheme() const;

private:
    Q_DECL_UNUSED void translate();

private:
    Theme m_theme;

    explicit SkinManager(QObject *parent = nullptr);

    Q_DISABLE_COPY_MOVE(SkinManager)
};

#endif // SKINMANAGER_H
