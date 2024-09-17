#ifndef SCRIPTMACHINE_H
#define SCRIPTMACHINE_H

#include "AngelScript/sdk/angelscript/include/angelscript.h"
#include "class/asbuilder.h"

#include "asdebugger.h"
#include "class/ascontextmgr.h"

#include <QObject>

class ScriptMachine : public QObject {
    Q_OBJECT
private:
    typedef QString (*TranslateFunc)(const QStringList &contents);

public:
    enum class MessageType { Info, Warn, Error, Print };

    struct MessageInfo {
        QString section;
        int row = -1;
        int col = -1;
        asEMsgType type = asEMsgType::asMSGTYPE_INFORMATION;
        QString message;
    };

protected:
    enum RegisteredType {
        tString,
        tArray,
        tComplex,
        tWeakref,
        tConstWeakref,
        tAny,
        tDictionary,
        tDictionaryValue,
        tGrid,
        tDateTime,
        tRef,
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
    explicit ScriptMachine(std::function<QString()> &getInputFn,
                           QObject *parent = nullptr);

    virtual ~ScriptMachine();

    bool inited();

    bool isRunning() const;

    bool isInDebugMode() const;

    asDebugger *debugger() const;

    asIScriptContext *immediateContext() const;

    asIScriptEngine *engine() const;

public slots:
    virtual bool executeCode(const QString &code);
    virtual bool executeScript(const QString &script, bool isInDebug = false);

protected:
    virtual bool configureEngine(asIScriptEngine *engine);

    QString getCallStack(asIScriptContext *context);

    void destoryMachine();

private:
    void print(void *ref, int typeId);

    QString getInput();

    bool isType(asITypeInfo *tinfo, RegisteredType type);

    static int execSystemCmd(const std::string &exe, const std::string &params,
                             std::string &out);

    static int execSystemCmd(const std::string &exe, const std::string &params);

private:
    static void messageCallback(const asSMessageInfo *msg, void *param);

    static asIScriptContext *requestContextCallback(asIScriptEngine *engine,
                                                    void *param);
    static void returnContextCallback(asIScriptEngine *engine,
                                      asIScriptContext *ctx, void *param);

    static int pragmaCallback(const QByteArray &pragmaText, asBuilder *builder,
                              void *userParam);

    static int includeCallback(const QString &include, bool quotedInclude,
                               const QString &from, asBuilder *builder,
                               void *userParam);

    static QString processTranslation(const char *content);

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

    asIScriptContext *_immediateContext = nullptr;
};

Q_DECLARE_METATYPE(ScriptMachine::MessageInfo)

#endif // SCRIPTMACHINE_H
