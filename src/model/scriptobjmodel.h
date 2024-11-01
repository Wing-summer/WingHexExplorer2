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

#ifndef SCRIPTOBJMODEL_H
#define SCRIPTOBJMODEL_H

#include "angelscript.h"
#include "class/asdebugger.h"
#include <QAbstractTableModel>

class ScriptObjModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit ScriptObjModel(asIScriptEngine *engine, asDebugger *debugger,
                            QObject *parent = nullptr);

    void updateData();

private:
    struct Data {
        QString name;
        QString type;
        QString value;
    };

    QList<Data> _datas;

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

private:
    asIScriptEngine *_engine = nullptr;
    asDebugger *_debugger = nullptr;
    asIScriptModule *_mod = nullptr;
};

#endif // SCRIPTOBJMODEL_H
