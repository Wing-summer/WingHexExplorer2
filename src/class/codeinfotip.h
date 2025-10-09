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

#ifndef CODEINFOTIP_H
#define CODEINFOTIP_H

#include "lsp.h"

#include <QIcon>
#include <QJsonValue>
#include <QMap>
#include <QObject>
#include <QString>

class CodeInfoTip {
public:
    static QIcon getDisplayIcon(LSP::CompletionItemKind type);

public:
    QString name;
    LSP::CompletionItemKind type = LSP::CompletionItemKind::Missing;

    QJsonValue value;
    QList<CodeInfoTip> children;

public:
    QString comment() const;
    QString completion() const;

    void setComment(const QString &comment);
    bool isSnippet() const;

private:
    mutable QString _comment;
    mutable QString _completion;
    mutable bool _isSnippet = false;

public:
    void resolve() const;
};

Q_DECLARE_METATYPE(CodeInfoTip);

#endif // CODEINFOTIP_H
