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

#ifndef SCRIPTINFOBROSWER_H
#define SCRIPTINFOBROSWER_H

#include <QTextBrowser>

#include "class/scriptmanager.h"

class ScriptInfoBroswer : public QTextBrowser {
    Q_OBJECT
public:
    explicit ScriptInfoBroswer(QWidget *parent = nullptr);

public:
    void updateData(const ScriptManager::ScriptDirMeta &meta);

    // QTextEdit interface
public:
    virtual QVariant loadResource(int type, const QUrl &name) override;

private:
    QIcon _icon;
    bool _isSys = true;
};

#endif // SCRIPTINFOBROSWER_H
