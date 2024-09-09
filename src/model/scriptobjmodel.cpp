#include "scriptobjmodel.h"

ScriptObjModel::ScriptObjModel(asIScriptEngine *engine, asDebugger *debugger,
                               QObject *parent)
    : QAbstractTableModel(parent), _engine(engine), _debugger(debugger) {
    Q_ASSERT(engine && debugger);
    _mod = engine->GetModule("Console", asGM_CREATE_IF_NOT_EXISTS);
    Q_ASSERT(_mod);
}

void ScriptObjModel::updateData() {
    this->beginResetModel();
    _datas.reserve(_engine->GetGlobalPropertyCount() +
                   _mod->GetGlobalVarCount());
    _datas.clear();

    for (asUINT n = 0; n < _engine->GetGlobalPropertyCount(); n++) {
        const char *name;
        int typeId;
        bool isConst;
        void *v;

        _engine->GetGlobalPropertyByIndex(n, &name, nullptr, &typeId, &isConst,
                                          nullptr, &v);

        Data d;
        d.name = name;
        d.type = (isConst ? QStringLiteral(" const ") : QStringLiteral(" ")) +
                 _engine->GetTypeDeclaration(typeId);
        d.value = _debugger->toString(v, typeId, 3, _engine);

        _datas.append(d);
    }

    for (asUINT n = 0; n < _mod->GetGlobalVarCount(); n++) {
        const char *name;
        int typeId;
        bool isConst;
        void *v;

        _mod->GetGlobalVar(n, &name, nullptr, &typeId, &isConst);
        v = _mod->GetAddressOfGlobalVar(n);

        Data d;
        d.name = name;
        d.type = (isConst ? QStringLiteral(" const ") : QStringLiteral(" ")) +
                 _engine->GetTypeDeclaration(typeId);
        d.value = _debugger->toString(v, typeId, 3, _engine);
        _datas.append(d);
    }
    this->endResetModel();
}

int ScriptObjModel::rowCount(const QModelIndex &parent) const {
    return _datas.size();
}

int ScriptObjModel::columnCount(const QModelIndex &parent) const { return 3; }

QVariant ScriptObjModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole: {
        auto d = _datas.at(index.row());
        switch (index.column()) {
        case 0:
            return d.name;
        case 1:
            return d.type;
        case 2:
            return d.value;
        default:
            return QVariant();
        }
    }
    case Qt::TextAlignmentRole:
        return int(Qt::AlignCenter);
    }
    return QVariant();
}

QVariant ScriptObjModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return tr("Name");
            case 1:
                return tr("Type");
            case 2:
                return tr("Value");
            }
        } else {
            return section + 1;
        }
    }
    return QVariant();
}
