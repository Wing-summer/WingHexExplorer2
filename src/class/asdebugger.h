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

#include "angelscript.h"
#include <QList>
#include <QMap>
#include <QObject>

// from AngelScript CDebugger, I modify it for Qt intergration and
// and add some TODO features that easy to implement
class asDebugger : public QObject {
    Q_OBJECT
public:
    struct BreakPoint {
        BreakPoint(QString f, int n, bool _func)
            : name(f), lineNbr(n), func(_func), needsAdjusting(true) {}
        QString name;
        int lineNbr;
        bool func;
        bool needsAdjusting;
    };

    struct VariablesInfo {
        QString name;
        QString value;
    };

    struct StackTraceInfo {
        QString file;
        int lineNr;
        QString fndecl;
    };

    struct GCStatistic {
        asUINT currentSize = 0;
        asUINT totalDestroyed = 0;
        asUINT totalDetected = 0;
        asUINT newObjects = 0;
        asUINT totalNewDestroyed = 0;
    };

    struct CallStackItem {
        QString file;
        int lineNbr;
        QString decl;
    };

    enum class WatchExpError {
        NoError,
        Error,
        NotEndAfterSymbol,
        NoMatchingSymbol,
        ExpectedIdentifier
    };

    enum DebugAction {
        ABORT,     // try to abort the script
        PAUSE,     // try to pause
        CONTINUE,  // continue until next break point
        STEP_INTO, // stop at next instruction
        STEP_OVER, // stop at next instruction, skipping called functions
        STEP_OUT   // run until returning from current function
    };

public:
    explicit asDebugger(QObject *parent = nullptr);
    virtual ~asDebugger();

    // Register callbacks to handle to-string conversions of application types
    typedef QString (*ToStringCallback)(void *obj, asDebugger *dbg);

    void registerToStringCallback(const asITypeInfo *ti,
                                  ToStringCallback callback);

    // Commands
    void addFileBreakPoint(const QString &file, int lineNbr);
    void removeFileBreakPoint(const QString &file, int lineNbr);
    void addFuncBreakPoint(const QString &func);
    void removeFuncBreakPoint(const QString &func);
    void clearBreakPoint();
    const QVector<BreakPoint> &breakPoints();

    // Optionally set the engine pointer in the debugger so it can be retrieved
    // by callbacks that need it. This will hold a reference to the engine.
    void setEngine(asIScriptEngine *engine);
    asIScriptEngine *getEngine();

    int expandMembers() const;
    void setExpandMembers(int newExpandMembers);

    // Line callback invoked by context
    void lineCallback(asIScriptContext *ctx);

    QString toString(void *value, asUINT typeId,
                     asIScriptEngine *engine = nullptr);

    GCStatistic gcStatistics();

    void runDebugAction(DebugAction action);

    DebugAction currentState() const;

    static void deleteDbgContextInfo(void *info);

private:
    QVector<VariablesInfo> globalVariables(asIScriptContext *ctx);
    QVector<VariablesInfo> localVariables(asIScriptContext *ctx);
    QList<CallStackItem> retriveCallstack(asIScriptContext *ctx);

    QString printValue(const QString &expr, asIScriptContext *ctx,
                       WatchExpError &error);

    void takeCommands(asIScriptContext *ctx);

    bool checkBreakPoint(asIScriptContext *ctx);
    void listMemberProperties(asIScriptContext *ctx);

signals:
    void breakPointChanged();
    void onAdjustBreakPointLine(const BreakPoint &old, int newLineNr);
    void onPullVariables(const QVector<VariablesInfo> &globalvars,
                         const QVector<VariablesInfo> &localvars);
    void onPullCallStack(const QList<CallStackItem> &callstacks);
    void onRunCurrentLine(const QString &file, int lineNr);

    void onDebugActionExec();

private:
    struct ContextDbgInfo {
        QString file;
        int line = -1;
        int col = -1;
        asUINT stackCount = 0;
    };

private:
    DebugAction m_action;

    asUINT m_lastCommandAtStackLevel;
    asIScriptFunction *m_lastFunction;

    QVector<BreakPoint> m_breakPoints;

    asIScriptEngine *m_engine = nullptr;

    // Registered callbacks for converting types to strings
    QMap<const asITypeInfo *, ToStringCallback> m_toStringCallbacks;

    QStringList m_watchVars;

    int _expandMembers = 3;
};

#endif // ASDEBUGGER_H
