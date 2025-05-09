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

class AsPreprocesser;

class AsCompletion : public WingCompleter {
    Q_OBJECT
public:
    explicit AsCompletion(WingCodeEdit *p);

    virtual ~AsCompletion();

    // WingCompleter interface
public:
    virtual QString wordSeperators() const override;

    void clearFunctionTip();

protected:
    virtual bool processTrigger(const QString &trigger,
                                const QString &content) override;

    virtual QList<CodeInfoTip> parseDocument();

    virtual QList<CodeInfoTip> parseMarcos();

    QList<CodeInfoTip> parseScriptData(qsizetype offset,
                                       const QByteArray &code);

signals:
    void onFunctionTip(const QString &content);

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void applyEmptyNsNode(QList<CodeInfoTip> &nodes,
                          const QList<CodeInfoTip> &docNodes);
    void applyClassNodes(QList<CodeInfoTip> &nodes);

private:
    static int includeCallBack(const QString &include, bool quotedInclude,
                               const QString &from, AsPreprocesser *builder,
                               void *userParam);

private:
    struct CompleteDB {
        QList<CodeInfoTip> data;
        QByteArray md5;
        uint time = 0;
    };

    QHash<QString, CompleteDB> comdb;

    void pushCompleteDBData(const QString &fileName,
                            const QList<CodeInfoTip> &data);
    std::optional<CompleteDB> getCompleteDBData(const QString &fileName);
    void remoteCompleteDBData(const QString &fileName);
    void clearCompleteDBUnused();

private:
    ASDataBase parser;
};

#endif // _CPP_COMPLETION_H_
