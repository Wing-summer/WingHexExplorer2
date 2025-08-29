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

#include "codecompletionmodel.h"
#include "class/angellsp.h"

CodeCompletionModel::CodeCompletionModel(const QList<CodeInfoTip> &infos,
                                         QObject *parent)
    : QAbstractListModel(parent), _infos(infos) {}

int CodeCompletionModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return _infos.size();
}

QVariant CodeCompletionModel::data(const QModelIndex &index, int role) const {
    auto r = index.row();
    auto info = _infos.at(r);

    switch (role) {
    case Qt::SelfDataRole:
        return QVariant::fromValue(info);
    case Qt::UserRole:
        return info.name;
    case Qt::DisplayRole:
        return info.name;
    case Qt::ToolTipRole:
        if (info.comment.isEmpty() && !info.value.isNull()) {
            auto &ins = AngelLsp::instance();
            auto v = ins.requestResolve(info.value);
            info.comment = v["detail"].toString();
        }
        return info.comment; // TODO
    case Qt::DecorationRole:
        return CodeInfoTip::getDisplayIcon(info.type);
    }

    return {};
}
