#ifndef ASDEBUGGER_H
#define ASDEBUGGER_H

#include "angelscript.h"
#include <QMap>
#include <QObject>

// from AngelScript CDebugger, I modify it for Qt intergration and
// and add some TODO features that easy to implement
class asDebugger : public QObject {
    Q_OBJECT
public:
    asDebugger(QObject *parent = nullptr);
    virtual ~asDebugger();

    // Register callbacks to handle to-string conversions of application types
    // The expandMembersLevel is a counter for how many recursive levels the
    // members should be expanded. If the object that is being converted to a
    // string has members of its own the callback should call the debugger's
    // ToString passing in expandMembersLevel - 1.
    typedef QString (*ToStringCallback)(void *obj, int expandMembersLevel,
                                        asDebugger *dbg);

    void RegisterToStringCallback(const asITypeInfo *ti,
                                  ToStringCallback callback);

    // User interaction
    void TakeCommands(asIScriptContext *ctx);
    void Output(const QString &str);

    // Line callback invoked by context
    void LineCallback(asIScriptContext *ctx);

    // Commands
    void PrintHelp();
    void AddFileBreakPoint(const QString &file, int lineNbr);
    void AddFuncBreakPoint(const QString &func);
    void ListBreakPoints();
    void ListLocalVariables(asIScriptContext *ctx);
    void ListGlobalVariables(asIScriptContext *ctx);
    void ListMemberProperties(asIScriptContext *ctx);
    void ListStatistics(asIScriptContext *ctx);
    void PrintCallstack(asIScriptContext *ctx);
    void PrintValue(const QString &expr, asIScriptContext *ctx);

    // Helpers
    bool InterpretCommand(const QString &cmd, asIScriptContext *ctx);
    bool CheckBreakPoint(asIScriptContext *ctx);
    QString ToString(void *value, asUINT typeId, int expandMembersLevel,
                     asIScriptEngine *engine);

    // Optionally set the engine pointer in the debugger so it can be retrieved
    // by callbacks that need it. This will hold a reference to the engine.
    void SetEngine(asIScriptEngine *engine);
    asIScriptEngine *GetEngine();

    int expandMembers() const;
    void setExpandMembers(int newExpandMembers);

signals:
    void onOutput(const QString &message);

private:
    enum DebugAction {
        CONTINUE,  // continue until next break point
        STEP_INTO, // stop at next instruction
        STEP_OVER, // stop at next instruction, skipping called functions
        STEP_OUT   // run until returning from current function
    };
    DebugAction m_action;
    asUINT m_lastCommandAtStackLevel;
    asIScriptFunction *m_lastFunction;

    struct BreakPoint {
        BreakPoint(QString f, int n, bool _func)
            : name(f), lineNbr(n), func(_func), needsAdjusting(true) {}
        QString name;
        int lineNbr;
        bool func;
        bool needsAdjusting;
    };
    QVector<BreakPoint> m_breakPoints;

    asIScriptEngine *m_engine;

    // Registered callbacks for converting types to strings
    QMap<const asITypeInfo *, ToStringCallback> m_toStringCallbacks;

    int _expandMembers = 1;
};

#endif // ASDEBUGGER_H
