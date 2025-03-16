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

#ifndef CODECOMPLETIONMODEL_H
#define CODECOMPLETIONMODEL_H

#include "class/codeinfotip.h"

#include <QAbstractListModel>

namespace Qt {
constexpr int SelfDataRole = Qt::UserRole + 1;
}

class CodeCompletionModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit CodeCompletionModel(const QList<CodeInfoTip> &infos,
                                 QObject *parent = nullptr);

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;

private:
    QList<CodeInfoTip> _infos;
};

#endif // CODECOMPLETIONMODEL_H
