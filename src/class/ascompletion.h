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

#include "class/qasparser.h"
#include "qcodecompletionengine.h"

class QCodeCompletionWidget;

class AsCompletion : public QCodeCompletionEngine {
    Q_OBJECT
public:
    explicit AsCompletion(asIScriptEngine *engine, QObject *p = nullptr);

    virtual ~AsCompletion();

    virtual QCodeCompletionEngine *clone() override;

    virtual QString language() const override;

    virtual QStringList extensions() const override;

    virtual void setEditor(QEditor *e) override;

    QCodeCompletionWidget *codeCompletionWidget() const;

signals:
    void onFunctionTip(AsCompletion *cp, const QString &content);

protected:
    virtual void complete(const QDocumentCursor &c,
                          const QString &trigger) override;

private:
    void applyEmptyNsNode(QList<QCodeNode *> &nodes);

    void parse(const QDocumentCursor &c);

    QList<QCodeNode *> lookupNamespace(const QByteArrayList &ns);

private:
    QAsParser parser;
    asIScriptEngine *_engine;
    QCodeCompletionWidget *pPopup;

    QList<QCodeNode *> _emptyNsNodes;
};

#endif // _CPP_COMPLETION_H_
