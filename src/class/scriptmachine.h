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

#ifndef SCRIPTMACHINE_H
#define SCRIPTMACHINE_H

#include "AngelScript/sdk/angelscript/include/angelscript.h"
#include "class/aspreprocesser.h"

#include "asdebugger.h"
#include "class/ascontextmgr.h"

#include <QObject>

class ScriptMachine : public QObject {
    Q_OBJECT
private:
    using TranslateFunc = std::function<QString(const QStringList &)>;

public:
    enum class MessageType { Info, Warn, Error, Print };

    struct MessageInfo {
        QString section;
        int row = -1;
        int col = -1;
        asEMsgType type = asEMsgType::asMSGTYPE_INFORMATION;
        QString message;
    };

    enum RegisteredType {
        tString,
        tChar,
        tArray,
        tComplex,
        tWeakref,
        tConstWeakref,
        tAny,
        tDictionary,
        tDictionaryValue,
        tGrid,
        tRef,
        tColor,
        tMAXCOUNT
    };

    asITypeInfo *typeInfo(RegisteredType type) const;

public:
    // only for refection
    enum class asEContextState {
        asEXECUTION_FINISHED = ::asEContextState::asEXECUTION_FINISHED,
        asEXECUTION_SUSPENDED = ::asEContextState::asEXECUTION_SUSPENDED,
        asEXECUTION_ABORTED = ::asEContextState::asEXECUTION_ABORTED,
        asEXECUTION_EXCEPTION = ::asEContextState::asEXECUTION_EXCEPTION,
        asEXECUTION_PREPARED = ::asEContextState::asEXECUTION_PREPARED,
        asEXECUTION_UNINITIALIZED =
            ::asEContextState::asEXECUTION_UNINITIALIZED,
        asEXECUTION_ACTIVE = ::asEContextState::asEXECUTION_ACTIVE,
        asEXECUTION_ERROR = ::asEContextState::asEXECUTION_ERROR,
        asEXECUTION_DESERIALIZATION =
            ::asEContextState::asEXECUTION_DESERIALIZATION
    };
    Q_ENUM(asEContextState)

public:
    explicit ScriptMachine(const std::function<QString()> &getInputFn,
                           QObject *parent = nullptr);

    virtual ~ScriptMachine();

    bool inited();

    bool isRunning() const;

    asDebugger *debugger() const;

    asIScriptEngine *engine() const;

    bool insteadFoundDisabled() const;
    void setInsteadFoundDisabled(bool newInsteadFoundDisabled);

    static void registerEngineAddon(asIScriptEngine *engine);
    static void registerEngineAssert(asIScriptEngine *engine);

public:
    static void scriptAssert(bool b);
    static void scriptAssert_X(bool b, const QString &msg);

    static void scriptThrow(const QString &msg);

    static QString scriptGetExceptionInfo();

    static void registerExceptionRoutines(asIScriptEngine *engine);

public:
    // debug or release?
    bool isDebugMode() const;
    void setDebugMode(bool isDbg);

public slots:
    virtual bool executeCode(const QString &code);
    virtual bool executeScript(const QString &script, bool isInDebug = false);

    void abortScript();

private:
    bool execute(const QString &code, bool isInDebug = false);

protected:
    virtual bool configureEngine(asIScriptEngine *engine);

    QString getCallStack(asIScriptContext *context);

    void destoryMachine();

private:
    void print(void *ref, int typeId);

    QString getInput();

    bool isType(asITypeInfo *tinfo, RegisteredType type);

    static int execSystemCmd(QString &out, const QString &exe,
                             const QString &params, int timeout);

private:
    static void messageCallback(const asSMessageInfo *msg, void *param);

    static void cleanUpDbgContext(asIScriptContext *context);

    static void cleanUpPluginSysIDFunction(asIScriptFunction *fn);

    static asIScriptContext *requestContextCallback(asIScriptEngine *engine,
                                                    void *param);
    static void returnContextCallback(asIScriptEngine *engine,
                                      asIScriptContext *ctx, void *param);

    static int pragmaCallback(const QByteArray &pragmaText,
                              AsPreprocesser *builder,
                              const QString &sectionname, void *userParam);

    static int includeCallback(const QString &include, bool quotedInclude,
                               const QString &from, AsPreprocesser *builder,
                               void *userParam);

    static QString processTranslation(const char *content,
                                      ScriptMachine *machine);

    void exceptionCallback(asIScriptContext *context);

    Q_DECL_UNUSED void translation();

signals:
    void onOutput(MessageType type, const MessageInfo &message);

    void onDebugFinished();

private:
    asIScriptEngine *_engine = nullptr;
    asDebugger *_debugger = nullptr;
    asContextMgr *_ctxMgr = nullptr;
    QVector<asIScriptContext *> _ctxPool;
    std::function<void(void *ref, int typeId)> _printFn;

    QVector<asITypeInfo *> _rtypes;
    std::function<QString(void)> _getInputFn;

    bool m_insteadFoundDisabled = false;
};

Q_DECLARE_METATYPE(ScriptMachine::MessageInfo)

#endif // SCRIPTMACHINE_H
