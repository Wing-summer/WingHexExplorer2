#ifndef SCRIPTMACHINE_H
#define SCRIPTMACHINE_H

#include "AngelScript/add_on/contextmgr/contextmgr.h"
#include "AngelScript/add_on/scriptbuilder/scriptbuilder.h"
#include "AngelScript/angelscript/include/angelscript.h"

#include "asdebugger.h"

#include <QObject>

class ScriptMachine : public QObject {
    Q_OBJECT
private:
    typedef QString (*TranslateFunc)(const QStringList &contents);

public:
    enum class MessageType { Info, Warn, Error };

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

public:
    explicit ScriptMachine(std::function<QString ()> &getInputFn,
                           QObject *parent = nullptr);

    virtual ~ScriptMachine();

    bool inited();

    bool isRunning() const;

public slots:
    virtual bool executeScript(const QString &script, bool isInDebug = false);

protected:
    virtual bool configureEngine(asIScriptEngine *engine);

    virtual bool compileScript(const QString &script);

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

    static int pragmaCallback(const std::string &pragmaText,
                              CScriptBuilder &builder, void *userParam);

    static QString processTranslation(const char *content);

    Q_DECL_UNUSED void translation();

signals:
    void onOutput(MessageType type, const MessageInfo &message);

protected:
    asIScriptEngine *_engine = nullptr;
    asDebugger *_debugger = nullptr;
    CContextMgr *_ctxMgr = nullptr;
    QVector<asIScriptContext *> _ctxPool;

    QVector<asITypeInfo *> _rtypes;
    std::function<void(void *ref, int typeId)> _printFn;
    std::function<QString(void)> _getInputFn;
};

Q_DECLARE_METATYPE(ScriptMachine::MessageInfo)

#endif // SCRIPTMACHINE_H
