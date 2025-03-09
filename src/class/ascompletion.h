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

#ifndef _AS_COMPLETION_H_
#define _AS_COMPLETION_H_

#include "WingCodeEdit/wingcompleter.h"
#include "class/qasparser.h"

class AsCompletion : public WingCompleter {
    Q_OBJECT
public:
    explicit AsCompletion(asIScriptEngine *engine, WingCodeEdit *p);

    virtual ~AsCompletion();

signals:
    void onFunctionTip(AsCompletion *cp, const QString &content);

private:
    void applyEmptyNsNode(QList<QCodeNode *> &nodes);

    // void parse(const QDocumentCursor &c);

    QList<QCodeNode *> lookupNamespace(const QByteArrayList &ns);

private:
    QAsParser parser;
    asIScriptEngine *_engine;

    QList<QCodeNode *> _emptyNsNodes;
};

#endif // _CPP_COMPLETION_H_
