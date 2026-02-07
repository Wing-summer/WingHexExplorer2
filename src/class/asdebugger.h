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

#ifndef ASDEBUGGER_H
#define ASDEBUGGER_H

#include "WingPlugin/iwingangel.h"
#include "angelscript.h"
#include "as-debugger/as_debugger.h"

#include <QApplication>
#include <QList>
#include <QMap>
#include <QObject>

class asDebugger : public QObject, public asIDBDebugger {
    Q_OBJECT

    friend class asWingCache;

public:
    struct VariablesInfo {
        QString name;
        QString value;
    };

public:
    explicit asDebugger(asIDBWorkspace *workspace);
    virtual ~asDebugger();

public:
    void setCustomEvals(
        const QHash<std::string_view, WingHex::IWingAngel::Evaluator> &evals);

public:
    void addFileBreakPoint(const QString &file, int lineNbr);
    void removeFileBreakPoint(const QString &file, int lineNbr);
    void clearBreakPoint();

    virtual void Suspend() override;
    virtual void Resume() override;

    void reset();
signals:
    void onAdjustBreakPointLine(const QString &file, int oldLineNbr,
                                int newLineNbr);
    void onPullVariables();
    void onPullCallStack();
    void onRunCurrentLine(const QString &file, int lineNr);

    void onDebugActionExec();

protected:
    bool resume = false;

    virtual std::unique_ptr<asIDBCache>
    CreateCache(asIScriptContext *ctx) override;

private:
    QHash<std::string_view, WingHex::IWingAngel::Evaluator> _customEvals;
};

#endif // ASDEBUGGER_H
