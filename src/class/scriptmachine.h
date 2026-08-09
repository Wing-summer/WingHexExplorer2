/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include "WingPlugin/iwingangel.h"

#include "class/luauscheduler.h"

#include <QObject>
#include <QQueue>

class CScriptArray;

class ScriptMachine {
    Q_GADGET
public:
    // we have three console modes
    enum ConsoleMode : int {
        Console_Begin = 0,
        Interactive, // in a shell
        Scripting,   // in scripting dialog
        Background,  // run codes from other way
        Console_End
    };

    static constexpr auto ConsoleModeCount = Console_End - Console_Begin - 1;

public:
    enum class MessageType { Unknown, Info, Warn, Error, Print, ExecInfo };

    struct MessageInfo {
        ConsoleMode mode = ConsoleMode::Background;
        QString section;
        qint64 row = -1;
        qint64 col = -1;
        MessageType type = MessageType::Info;
        QString message;
    };

public:
    struct RegCallBacks {
        std::function<QString()> getInputFn;
        std::function<void()> clearFn;
        std::function<void(const ScriptMachine::MessageInfo &)> printMsgFn;
    };

private:
private:
    explicit ScriptMachine();
    Q_DISABLE_COPY_MOVE(ScriptMachine)

    LuauThread *context(ConsoleMode mode) const;
    lua_State *contextState(ConsoleMode mode) const;
    LuauThreadData *contextData(ConsoleMode mode) const;

    static bool configureEngine(lua_State *l);

    static LuauThreadData *contextData(lua_State *l);
    static int consoleModeIdx(ConsoleMode mode);

public:
    static ScriptMachine &instance();
    void destoryMachine();

    void setCustomEvals(
        const QHash<std::string_view, WingHex::IWingAngel::Evaluator> &evals);

public:
    bool init();
    bool isInited() const;
    bool isRunning(ConsoleMode mode) const;
    bool checkEngineConfigError() const;

    // static void registerEngineAddon(asIScriptEngine *engine);
    // static void registerEngineClipboard(asIScriptEngine *engine);

    void registerCallBack(ConsoleMode mode, const RegCallBacks &callbacks);

public:
    void setFileEnableOverwrite(bool b);
    void setFileSystemWrite(bool b);

    bool fileEnableOverwrite() const;
    bool fileSystemWrite() const;

public:
    // asDebugger *debugger() const;

    void outputMessage(const MessageInfo &info);

    QString getGlobalDecls() const;

public:
    static void clip_setText(const QString &text);
    static void clip_setBinary(const CScriptArray &array);
    static QString clip_getText();
    static CScriptArray *clip_getBinary();

public:
    // debug or release?
    bool isDebugMode(ConsoleMode mode = Scripting);

public:
    // @return true if execution finished, else need more codes input
    void executeCode(ScriptMachine::ConsoleMode mode, const QString &code,
                     const std::function<void(bool)> &onFinished);

    // only scripting mode can be debugged
    void executeScript(ScriptMachine::ConsoleMode mode, const QString &fileName,
                       bool isInDebug,
                       const std::function<void(bool)> &onFinished);

    void abortDbgScript();
    void abortScript(ScriptMachine::ConsoleMode mode);

    std::string getAsTypeName(int typeId);

private:
    static int __output(MessageType type, lua_State *L);
    static int __outputln(MessageType type, lua_State *L);

    static int print(lua_State *L);
    static int println(lua_State *L);

    static int warnprint(lua_State *L);
    static int warnprintln(lua_State *L);

    static int errprint(lua_State *L);
    static int errprintln(lua_State *L);

    static int infoprint(lua_State *L);
    static int infoprintln(lua_State *L);

    QString input();

private:
    static void onLuauInterrupt(lua_State *L, int gc);
    static void onLuauThreadCreated(lua_State *LP, lua_State *L);

private:
    // void attachDebugBreak(asIScriptContext *ctx);

private:
    QVector<RegCallBacks> _regcalls;
    mutable QString _cachedGlobalStrs;

private:
    bool _inited = false;
    lua_State *_main = nullptr;
    mutable LuauThread _ctx[ConsoleModeCount]{};
    mutable LuauThreadData _tdata[ConsoleModeCount]{};
};

Q_DECLARE_METATYPE(ScriptMachine::MessageInfo)

#endif // SCRIPTMACHINE_H
