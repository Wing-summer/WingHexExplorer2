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

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "Qt-Advanced-Docking-System/src/DockWidget.h"
#include "qcodeedit.h"

class ScriptEditor : public ads::CDockWidget {
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr);
    virtual ~ScriptEditor();

    QString fileName() const;

    QEditor *editor() const;

    bool formatCode();

signals:
    void onToggleMark(int lineIndex);

public slots:
    bool openFile(const QString &filename);

    bool save(const QString &path = QString());
    bool reload();

private:
    void processTitle();

private:
    QCodeEdit *m_editor = nullptr;
};

#endif // SCRIPTEDITOR_H
