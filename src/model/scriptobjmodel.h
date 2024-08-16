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
