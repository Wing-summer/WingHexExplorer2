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

    void registerToStringCallback(const asITypeInfo *ti,
                                  ToStringCallback callback);

    // User interaction
    void takeCommands(asIScriptContext *ctx);
    void outputString(const QString &str);

    // Commands
    void printHelp();
    void addFileBreakPoint(const QString &file, int lineNbr);
    void removeFileBreakPoint(const QString &file, int lineNbr);
    void addFuncBreakPoint(const QString &func);
    void removeFuncBreakPoint(const QString &func);
    void clearBreakPoint();
    void listBreakPoints();
    const QVector<BreakPoint> &breakPoints();

    // Optionally set the engine pointer in the debugger so it can be retrieved
    // by callbacks that need it. This will hold a reference to the engine.
    void setEngine(asIScriptEngine *engine);
    asIScriptEngine *getEngine();

    int expandMembers() const;
    void setExpandMembers(int newExpandMembers);

    // Line callback invoked by context
    void lineCallback(asIScriptContext *ctx);

    QString toString(void *value, asUINT typeId, int expandMembersLevel,
                     asIScriptEngine *engine = nullptr);

private:
    bool checkBreakPoint(asIScriptContext *ctx);

    void listLocalVariables(asIScriptContext *ctx);
    QVector<VariablesInfo> localVariables(asIScriptContext *ctx);
    void listGlobalVariables(asIScriptContext *ctx);
    void listMemberProperties(asIScriptContext *ctx);
    void listStatistics(asIScriptContext *ctx);
    void printCallstack(asIScriptContext *ctx);
    void printValue(const QString &expr, asIScriptContext *ctx);

    // Helpers
    bool interpretCommand(const QString &cmd, asIScriptContext *ctx);

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

    QVector<BreakPoint> m_breakPoints;

    asIScriptEngine *m_engine;

    // Registered callbacks for converting types to strings
    QMap<const asITypeInfo *, ToStringCallback> m_toStringCallbacks;

    int _expandMembers = 1;
};

#endif // ASDEBUGGER_H
