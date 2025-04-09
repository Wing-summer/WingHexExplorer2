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
#include "class/asdatabase.h"

class AsCompletion : public WingCompleter {
    Q_OBJECT
public:
    explicit AsCompletion(WingCodeEdit *p);

    virtual ~AsCompletion();

    // WingCompleter interface
public:
    virtual QString wordSeperators() const override;

    bool parseDocument() const;
    void setParseDocument(bool newParseDocument);

    void clearFunctionTip();

protected:
    virtual void processTrigger(const QString &trigger,
                                const QString &content) override;

    virtual QList<CodeInfoTip> parseDocument();

signals:
    void onFunctionTip(const QString &content);

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void applyEmptyNsNode(QList<CodeInfoTip> &nodes);
    void applyClassNodes(QList<CodeInfoTip> &nodes);

private:
    ASDataBase parser;
    bool m_parseDocument;
};

#endif // _CPP_COMPLETION_H_
