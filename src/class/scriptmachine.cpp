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

#include "scriptmachine.h"

#include "AngelScript/sdk/add_on/scriptfile/scriptfilesystem.h"
#include "grammar/ASConsole/AngelscriptConsoleLexer.h"
#include "grammar/ASConsole/AngelscriptConsoleParser.h"

// reuse the listener
#include "structlib/cstructerrorlistener.h"

#include "AngelScript/sdk/add_on/autowrapper/aswrappedcall.h"
#include "AngelScript/sdk/add_on/datetime/datetime.h"
#include "AngelScript/sdk/add_on/scriptarray/scriptarray.h"
#include "AngelScript/sdk/add_on/scriptdictionary/scriptdictionary.h"
#include "AngelScript/sdk/add_on/scriptfile/scriptfile.h"
#include "AngelScript/sdk/add_on/scriptgrid/scriptgrid.h"
#include "AngelScript/sdk/add_on/scripthandle/scripthandle.h"
#include "AngelScript/sdk/add_on/scripthelper/scripthelper.h"
#include "AngelScript/sdk/add_on/scriptmath/scriptmath.h"
#include "AngelScript/sdk/add_on/scriptmath/scriptmathcomplex.h"
#include "AngelScript/sdk/add_on/weakref/weakref.h"
#include "AngelScript/sdk/angelscript/source/as_scriptengine.h"
#include "scriptaddon/scriptany.h"

#include "class/angelscriptconsolevisitor.h"
#include "class/appmanager.h"
#include "class/asbuilder.h"
#include "class/logger.h"
#include "class/pluginsystem.h"
#include "class/settingmanager.h"
#include "define.h"
#include "scriptaddon/scriptcolor.h"
#include "scriptaddon/scriptenv.h"
#include "scriptaddon/scriptjson.h"
#include "scriptaddon/scriptqstring.h"
#include "scriptaddon/scriptregex.h"

#include <QClipboard>
#include <QMimeData>
#include <QProcess>
#include <QScopeGuard>

namespace {
class FormatValue {
public:
    enum Type { Simple, Object };
    virtual ~FormatValue() = default;
    virtual Type type() const = 0;
    virtual QString rawText() const = 0;
};

class SimpleValue : public FormatValue {
public:
    SimpleValue(const QString &text) : m_text(text.trimmed()) {}
    Type type() const override { return Simple; }
    QString rawText() const override { return m_text; }

private:
    QString m_text;
};

class ObjectValue : public FormatValue {
public:
    Type type() const override { return Object; }
    QString rawText() const override { return QStringLiteral("{}"); }

    void addPair(const QString &key, QSharedPointer<FormatValue> value) {
        m_pairs.insert(key, value);
    }

    const QHash<QString, QSharedPointer<FormatValue>> &pairs() const {
        return m_pairs;
    }
    bool isEmpty() const { return m_pairs.isEmpty(); }

private:
    QHash<QString, QSharedPointer<FormatValue>> m_pairs;
};

class StringFormatter {
public:
    static QString format(const QString &input, uint indent) {
        qsizetype pos = 0;
        auto result = parseObject(input, pos);

        skipWhitespace(input, pos);
        if (pos < input.length() || !result) {
            return input;
        }

        if (result->type() == FormatValue::Object) {
            return formatObject(static_cast<ObjectValue *>(result.data()), 0,
                                indent);
        }

        return input;
    }

private:
    static QSharedPointer<FormatValue> parseObject(const QString &input,
                                                   qsizetype &pos) {
        skipWhitespace(input, pos);

        if (pos >= input.length() || input[pos] != '{') {
            return nullptr;
        }

        pos++; // Skip '{'
        skipWhitespace(input, pos);

        auto object = QSharedPointer<ObjectValue>(new ObjectValue());

        if (pos < input.length() && input[pos] == '}') {
            pos++;
            return object;
        }

        while (pos < input.length()) {
            skipWhitespace(input, pos);

            // Parse key-value pair
            QString key = parseKey(input, pos);
            if (key.isEmpty()) {
                return nullptr;
            }

            QSharedPointer<FormatValue> value = parseValue(input, pos);
            if (!value) {
                return nullptr;
            }

            object->addPair(key, value);

            skipWhitespace(input, pos);

            // Check for comma or end
            if (pos < input.length() && input[pos] == ',') {
                pos++;
                skipWhitespace(input, pos);
                if (pos < input.length() && input[pos] == '}') {
                    break;
                }
            } else if (pos < input.length() && input[pos] == '}') {
                break;
            } else if (pos >= input.length()) {
                return nullptr;
            }
        }

        if (pos >= input.length() || input[pos] != '}') {
            return nullptr;
        }

        pos++; // Skip '}'
        return object;
    }

    static QString parseKey(const QString &input, qsizetype &pos) {
        skipWhitespace(input, pos);

        if (pos >= input.length() || input[pos] != '[') {
            return QString();
        }

        pos++; // Skip '['
        skipWhitespace(input, pos);

        QString key;
        while (pos < input.length() && input[pos] != ']') {
            key += input[pos];
            pos++;
        }

        if (pos >= input.length() || input[pos] != ']') {
            return QString();
        }

        pos++; // Skip ']'
        return key.trimmed();
    }

    static QSharedPointer<FormatValue> parseValue(const QString &input,
                                                  qsizetype &pos) {
        skipWhitespace(input, pos);

        // Look for '='
        if (pos >= input.length() || input[pos] != '=') {
            return nullptr;
        }

        pos++; // Skip '='
        skipWhitespace(input, pos);

        // Check if it's an object
        if (pos < input.length() && input[pos] == '{') {
            return parseObject(input, pos);
        } else {
            QString simpleValue = parseSimpleValue(input, pos);
            if (simpleValue.isEmpty()) {
                return nullptr;
            }
            return QSharedPointer<FormatValue>(new SimpleValue(simpleValue));
        }
    }

    static QString parseSimpleValue(const QString &input, qsizetype &pos) {
        qsizetype start = pos;
        bool inSingleQuote = false;
        bool inDoubleQuote = false;
        bool escaped = false;
        qsizetype bracketDepth = 0;
        qsizetype parenDepth = 0;
        qsizetype angleDepth = 0;

        while (pos < input.length()) {
            QChar c = input[pos];

            if (escaped) {
                escaped = false;
                pos++;
                continue;
            }

            if (c == '\\') {
                escaped = true;
                pos++;
                continue;
            }

            if (inSingleQuote) {
                if (c == '\'') {
                    inSingleQuote = false;
                }
                pos++;
            } else if (inDoubleQuote) {
                if (c == '"') {
                    inDoubleQuote = false;
                }
                pos++;
            } else {
                if (c == '\'') {
                    inSingleQuote = true;
                    pos++;
                } else if (c == '"') {
                    inDoubleQuote = true;
                    pos++;
                } else if (c == '[') {
                    bracketDepth++;
                    pos++;
                } else if (c == ']') {
                    if (bracketDepth > 0)
                        bracketDepth--;
                    pos++;
                } else if (c == '(') {
                    parenDepth++;
                    pos++;
                } else if (c == ')') {
                    if (parenDepth > 0)
                        parenDepth--;
                    pos++;
                } else if (c == '<') {
                    angleDepth++;
                    pos++;
                } else if (c == '>') {
                    if (angleDepth > 0)
                        angleDepth--;
                    pos++;
                } else if (bracketDepth == 0 && parenDepth == 0 &&
                           angleDepth == 0) {
                    if (c == ',' || c == '}') {
                        break;
                    } else {
                        pos++;
                    }
                } else {
                    pos++;
                }
            }
        }

        if (pos == start) {
            return {};
        }

        return input.sliced(start, pos - start).trimmed();
    }

    static QString formatObject(const ObjectValue *object, int indentLevel,
                                uint indentSize) {
        if (object->isEmpty()) {
            return QStringLiteral("{}");
        }

        QStringList formattedPairs;
        QString currentIndent = getIndent(indentLevel, indentSize);
        QString nextIndent = getIndent(indentLevel + 1, indentSize);

        qsizetype maxKeyLength = 0;
        const auto &pairs = object->pairs();
        for (auto it = pairs.begin(); it != pairs.end(); ++it) {
            maxKeyLength = qMax(maxKeyLength, it.key().length() + 2);
        }

        for (auto it = pairs.begin(); it != pairs.end(); ++it) {
            QString keyPart = it.key();
            keyPart.prepend('[').append(']');
            QString formattedValue;

            if (it.value()->type() == FormatValue::Object) {
                const ObjectValue *childObject =
                    static_cast<const ObjectValue *>(it.value().data());
                formattedValue =
                    formatObject(childObject, indentLevel + 1, indentSize);
            } else {
                formattedValue = formatSimple(
                    static_cast<const SimpleValue *>(it.value().data()));
            }

            QString paddedKeyPart = keyPart.leftJustified(maxKeyLength, ' ');
            formattedPairs.append(nextIndent + paddedKeyPart +
                                  QStringLiteral(" = ") + formattedValue);
        }

        return QStringLiteral("{\n") +
               formattedPairs.join(QStringLiteral(",\n")) + '\n' +
               currentIndent + '}';
    }

    static QString formatSimple(const SimpleValue *value) {
        return value->rawText();
    }

    static QString getIndent(int level, uint indentSize) {
        return QString(level * indentSize, ' ');
    }

    static void skipWhitespace(const QString &input, qsizetype &pos) {
        while (pos < input.length() && isWhitespace(input[pos])) {
            pos++;
        }
    }

    static bool isWhitespace(QChar c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }
};
} // namespace

bool ScriptMachine::init() {
    if (isInited()) {
        return true;
    }

    qRegisterMetaType<MessageInfo>();

    _engine = asCreateScriptEngine();
    if (!ScriptMachine::configureEngine()) {
        _engine->ShutDownAndRelease();
        _engine = nullptr;
        return false;
    }

    // create the debugger
    _workspace = new asIDBFileWorkspace("", _engine);
    _debugger = new asDebugger(_workspace);
    return true;
}

bool ScriptMachine::isInited() const { return _engine != nullptr; }

bool ScriptMachine::isRunning(ConsoleMode mode) const {
    return _ctx.value(mode) != nullptr;
}

bool ScriptMachine::checkEngineConfigError() const {
    if (_engine) {
        auto e = static_cast<asCScriptEngine *>(_engine);
        if (e) {
            e->PrepareEngine();
            return e->configFailed;
        }
    }
    return true;
}

bool ScriptMachine::configureEngine() {
    if (_engine == nullptr) {
        return false;
    }

    // we need utf8, the default is what we want
    _engine->SetEngineProperty(asEP_EXPAND_DEF_ARRAY_TO_TMPL, true);
    _engine->SetEngineProperty(asEP_DISALLOW_EMPTY_LIST_ELEMENTS, true);
    _engine->SetEngineProperty(asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE, false);
    _engine->SetEngineProperty(asEP_ALLOW_MULTILINE_STRINGS, false);
    _engine->SetEngineProperty(asEP_USE_CHARACTER_LITERALS, false);
    _engine->SetEngineProperty(asEP_DISABLE_INTEGER_DIVISION, false);
    _engine->SetEngineProperty(asEP_PRIVATE_PROP_AS_PROTECTED, false);
    _engine->SetEngineProperty(asEP_ALTER_SYNTAX_NAMED_ARGS, 0);
    _engine->SetEngineProperty(asEP_ALLOW_UNICODE_IDENTIFIERS, true);
    _engine->SetEngineProperty(asEP_REQUIRE_ENUM_SCOPE,
                               true); // enum class like

    // The script compiler will send any compiler messages to the callback
    auto r = _engine->SetMessageCallback(asFUNCTION(messageCallback), this,
                                         asCALL_CDECL);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    _engine->SetFunctionUserDataCleanupCallback(
        &ScriptMachine::cleanUpPluginSysIDFunction,
        AsUserDataType::UserData_PluginFn);

    registerEngineAddon(_engine);

    _engine->SetDefaultAccessMask(0x1);

    // Register a couple of extra functions for the scripts
    r = _engine->RegisterGlobalFunction("void print(const ? &in ...)",
                                        asFUNCTION(print), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction("void println(const ? &in ...)",
                                        asFUNCTION(println), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction("string getInput()",
                                        asMETHOD(ScriptMachine, getInput),
                                        asCALL_THISCALL_ASGLOBAL, this);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction("string stringify(? &in obj)",
                                        asMETHOD(ScriptMachine, stringify),
                                        asCALL_THISCALL_ASGLOBAL, this);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction(
        "string beautify(const string &in str, uint indent = 4)",
        asFUNCTION(beautify), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction(
        "int exec(string &out output, const string &in exe, "
        "const string &in params = \"\", int timeout = 3000)",
        asFUNCTION(execSystemCmd), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    _ctxMgr = new CContextMgr;
    _ctxMgr->RegisterCoRoutineSupport(_engine);
    _ctxMgr->SetGetTimeCallback([]() -> asUINT {
        return AppManager::instance()->currentMSecsSinceEpoch();
    });
    _ctxMgr->RegisterThreadSupport(_engine);

    // Tell the engine to use our context pool. This will also
    // allow us to debug internal script calls made by the engine
    r = _engine->SetContextCallbacks(requestContextCallback,
                                     returnContextCallback, this);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    PluginSystem::instance().angelApi()->installAPI(this);

    // create module for Console
    auto mod = _engine->GetModule("WINGCONSOLE", asGM_ALWAYS_CREATE);
    mod->SetAccessMask(0x1);

    return true;
}

QString ScriptMachine::getCallStack(asIScriptContext *context) {
    QString str = QStringLiteral("AngelScript callstack:\n");

    // Append the call stack
    for (asUINT i = 0; i < context->GetCallstackSize(); i++) {
        asIScriptFunction *func;
        const char *scriptSection;
        int line, column;
        func = context->GetFunction(i);
        line = context->GetLineNumber(i, &column, &scriptSection);
        str += (QStringLiteral("\t") + scriptSection + QStringLiteral(":") +
                func->GetDeclaration() + QStringLiteral(":") +
                QString::number(line) + QStringLiteral(",") +
                QString::number(column) + QStringLiteral("\n"));
    }

    return str;
}

void ScriptMachine::destoryMachine() {
    _ctxMgr->AbortAll();
    delete _ctxMgr;

    delete _debugger;
    delete _workspace;

    _engine->ShutDownAndRelease();
    _engine = nullptr;
}

void ScriptMachine::setCustomEvals(
    const QHash<std::string, WingHex::IWingAngel::Evaluator> &evals) {
    _debugger->setCustomEvals(evals);
}

void ScriptMachine::exceptionCallback(asIScriptContext *context) {
    if (context) {
        ConsoleMode mode = ConsoleMode(reinterpret_cast<asPWORD>(
            context->GetUserData(AsUserDataType::UserData_ContextMode)));
        QString message =
            QStringLiteral("- Exception '%1' in '%2'\n")
                .arg(context->GetExceptionString(),
                     context->GetExceptionFunction()->GetDeclaration()) +
            QStringLiteral("\n") + getCallStack(context);

        const char *section;
        int col;
        MessageInfo msg;
        msg.mode = mode;
        msg.row = context->GetExceptionLineNumber(&col, &section);
        msg.col = col;
        msg.type = MessageType::Error;
        msg.message = message;

        outputMessage(msg);
    }
}

void ScriptMachine::attachDebugBreak(asIScriptContext *ctx) {
    if (!ctx)
        ctx = asGetActiveContext();

    checkDebugger(ctx);

    if (_debugger)
        _debugger->DebugBreak(ctx);
}

void ScriptMachine::checkDebugger(asIScriptContext *ctx) {
    if (_debugger == nullptr) {
        return;
    }
    // hook the context
    _debugger->HookContext(ctx, _debugger->HasWork());
}

void ScriptMachine::print(asIScriptGeneric *args) {
    auto context = asGetActiveContext();
    if (context) {
        ConsoleMode mode = ConsoleMode(reinterpret_cast<asPWORD>(
            context->GetUserData(AsUserDataType::UserData_ContextMode)));

        auto &m = ScriptMachine::instance();

        MessageInfo info;
        info.mode = mode;
        info.type = MessageType::Print;

        for (int i = 0; i < args->GetArgCount(); ++i) {
            void *ref = args->GetArgAddress(i);
            int typeId = args->GetArgTypeId(i);
            info.message.append(m.stringify(ref, typeId));
        }

        m.outputMessage(info);
    }
}

void ScriptMachine::println(asIScriptGeneric *args) {
    auto context = asGetActiveContext();
    if (context) {
        ConsoleMode mode = ConsoleMode(reinterpret_cast<asPWORD>(
            context->GetUserData(AsUserDataType::UserData_ContextMode)));

        auto &m = ScriptMachine::instance();

        MessageInfo info;
        info.mode = mode;
        info.type = MessageType::Print;

        for (int i = 0; i < args->GetArgCount(); ++i) {
            void *ref = args->GetArgAddress(i);
            int typeId = args->GetArgTypeId(i);

            if (typeId) {
                info.message.append(m.stringify(ref, typeId)).append('\n');
            }
        }

        m.outputMessage(info);
    }
}

QString ScriptMachine::getInput() {
    auto context = asGetActiveContext();
    if (context) {
        ConsoleMode mode = ConsoleMode(reinterpret_cast<asPWORD>(
            context->GetUserData(AsUserDataType::UserData_ContextMode)));

        auto cbs = _regcalls.value(mode);
        if (cbs.getInputFn) {
            return cbs.getInputFn();
        }
    }
    return {};
}

void ScriptMachine::outputMessage(const MessageInfo &info) {
    auto cbs = _regcalls.value(info.mode);
    if (cbs.printMsgFn) {
        cbs.printMsgFn(info);
    }
}

QString ScriptMachine::getTypeNameWithNs(int typeId) const {
    auto type = _engine->GetTypeInfoById(typeId);
    QString name;

    if (!type) {
        // a primitive
        switch (typeId & asTYPEID_MASK_SEQNBR) {
        case asTYPEID_BOOL:
            name = QStringLiteral("bool");
            break;
        case asTYPEID_INT8:
            name = QStringLiteral("int8");
            break;
        case asTYPEID_INT16:
            name = QStringLiteral("int16");
            break;
        case asTYPEID_INT32:
            name = QStringLiteral("int32");
            break;
        case asTYPEID_INT64:
            name = QStringLiteral("int64");
            break;
        case asTYPEID_UINT8:
            name = QStringLiteral("uint8");
            break;
        case asTYPEID_UINT16:
            name = QStringLiteral("uint16");
            break;
        case asTYPEID_UINT32:
            name = QStringLiteral("uint32");
            break;
        case asTYPEID_UINT64:
            name = QStringLiteral("uint64");
            break;
        case asTYPEID_FLOAT:
            name = QStringLiteral("float");
            break;
        case asTYPEID_DOUBLE:
            name = QStringLiteral("double");
            break;
        default:
            break;
        }
    } else {
        auto ns = type->GetNamespace();
        if (ns && strlen(ns)) {
            name =
                QString::fromUtf8(ns) + QStringLiteral("::") + type->GetName();
        } else {
            name = type->GetName();
        }
    }

    return name;
}

QString ScriptMachine::getGlobalDecls() const {
    if (!_cachedGlobalStrs.isEmpty()) {
        return _cachedGlobalStrs;
    }

    auto mod = module(ScriptMachine::Interactive);
    if (mod == nullptr) {
        return {};
    }

    auto total = mod->GetGlobalVarCount();
    for (asUINT n = 0; n < total; n++) {
        const char *name;
        const char *nameSpace;
        int typeId;
        bool isConst;

        mod->GetGlobalVar(n, &name, &nameSpace, &typeId, &isConst);
        const auto viewType = getTypeNameWithNs(typeId);
        if (isConst) {
            _cachedGlobalStrs.append(QStringLiteral("const "));
        }
        _cachedGlobalStrs.append(viewType).append(' ').append(name).append(';');
    }
    total = _engine->GetGlobalPropertyCount();
    for (asUINT n = 0; n < total; n++) {
        const char *name;
        const char *nameSpace;
        int typeId;
        bool isConst;

        _engine->GetGlobalPropertyByIndex(n, &name, &nameSpace, &typeId,
                                          &isConst);
        const auto viewType = getTypeNameWithNs(typeId);
        if (isConst) {
            _cachedGlobalStrs.append(QStringLiteral("const "));
        }
        _cachedGlobalStrs.append(viewType).append(' ').append(name).append(';');
    }

    return _cachedGlobalStrs;
}

int ScriptMachine::execSystemCmd(QString &out, const QString &exe,
                                 const QString &params, int timeout) {
    if (Utilities::isRoot()) {
        auto ctx = asGetActiveContext();
        if (ctx) {
            auto err = QStringLiteral("ExecNotAllowedInRoot");
            ctx->SetException(err.toUtf8());
        }
        return -1;
    }
    QProcess ps;
    ps.setProgram(exe);
    ps.setArguments(QProcess::splitCommand(params));
    ps.start();
    if (ps.waitForFinished(timeout)) {
        out = ps.readAllStandardOutput();
        return ps.exitCode();
    } else {
        ps.kill();
        return -1;
    }
}

QString ScriptMachine::beautify(const QString &str, uint indent) {
    return StringFormatter::format(str, indent);
}

QString ScriptMachine::stringify(void *ref, int typeId) {
    if (_debugger && typeId) {
        auto var = std::make_shared<asIDBVariable>(*_debugger);
        var->ptr = var;
        var->address.typeId = typeId;
        var->address.address = ref;
        return stringify_helper(var);
    }
    return {};
}

QString
ScriptMachine::stringify_helper(const std::shared_ptr<asIDBVariable> &var) {
    Q_ASSERT(var);
    var->Evaluate();
    if (var->evaluated) {
        if (var->expandable) {
            var->Expand();
            if (var->expanded) {
                QStringList r;
                for (auto &item : var->indexedProps) {
                    auto istr = stringify_helper(item);
                    r.append(istr.prepend(QStringLiteral(" = "))
                                 .prepend(item->identifier.Combine()));
                }
                for (auto &item : var->namedProps) {
                    auto istr = stringify_helper(item);
                    r.append(istr.prepend(QStringLiteral(" = "))
                                 .prepend(item->identifier.Combine()));
                }
                return r.join(QStringLiteral(", ")).prepend('{').append('}');
            } else {
                return QStringLiteral("<expand-failed>");
            }
        } else {
            return QString::fromStdString(var->value);
        }
    } else {
        return QStringLiteral("<eval-failed>");
    }
}

bool ScriptMachine::executeScript(
    ConsoleMode mode, const QString &script, bool isInDebug, int *retCode,
    std::function<void(const QHash<QString, AsPreprocesser::Result> &)>
        sections) {
    Q_ASSERT(mode != Interactive);
    if (QThread::currentThread() != qApp->thread()) {
        Logger::warning(QStringLiteral("Code must be exec in the main thread"));
        return false;
    }

    if (script.isEmpty()) {
        return true;
    }

    // Compile the script
    auto mod = createModule(mode);
    // script-running is not allowed in interactive mode
    if (mod == nullptr) {
        return false;
    }

    QScopeGuard guard([mod, this]() {
        // Before leaving, allow the engine to clean up remaining objects by
        // discarding the module and doing a full garbage collection so that
        // this can also be debugged if desired
        mod->Discard();
        endEvaluateDefine();
    });

    beginEvaluateDefine();

    asPWORD isDbg = 0;
    if (mode == Scripting) {
        if (isInDebug) {
            isDbg = 1;
        }
    }
    _engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, isDbg == 0);

    asBuilder builder(_engine);
    for (auto &m : PluginSystem::instance().scriptMarcos()) {
        builder.defineMacroWord(m);
    }

    // Set the pragma callback so we can detect
    builder.setPragmaCallback(&ScriptMachine::pragmaCallback);
    builder.setErrorHandler(
        [this, mode](const AsPreprocesser::PreprocError &error) {
            MessageInfo info;
            info.mode = mode;
            info.row = error.line;
            info.col = error.column;
            info.message = error.message;
            info.type = MessageType::Error;
            outputMessage(info);
        });

    _curMsgMode = mode;
    auto r = builder.loadSectionFromFile(script.toUtf8());
    if (r < 0) {
        MessageInfo info;
        info.mode = mode;
        info.message = QStringLiteral("Script failed to pre-processed");
        info.type = MessageType::Error;
        outputMessage(info);
        return false;
    }

    r = builder.build(mod);
    if (r < 0) {
        MessageInfo info;
        info.mode = mode;
        info.message = QStringLiteral("Script failed to build");
        info.type = MessageType::Error;
        outputMessage(info);
        return false;
    }

    // Find the main function
    asIScriptFunction *func = mod->GetFunctionByDecl("int main()");
    if (func == nullptr) {
        // Try again with "void main()"
        func = mod->GetFunctionByDecl("void main()");
    }

    if (func == nullptr) {
        MessageInfo info;
        info.mode = mode;
        info.message =
            QStringLiteral("Cannot find 'int main()' or 'void main()'");
        info.type = MessageType::Error;
        outputMessage(info);
        return false;
    }

    if (sections) {
        auto data = builder.scriptData();
        sections(data);
    }

    if (isInDebug) {
        // Allow the user to initialize the debugging before moving on
        MessageInfo info;
        info.mode = mode;
        info.message = QStringLiteral("Debugging, waiting for commands.");
        info.type = MessageType::Info;
        outputMessage(info);
    }

    // Set up a context to execute the script
    // The context manager will request the context from the
    // pool, which will automatically attach the debugger
    asIScriptContext *ctx = _ctxMgr->AddContext(_engine, func, true);
    _ctx[mode] = ctx;

    if (mode == Background) {
        MessageInfo info;
        info.mode = mode;
        info.message = QStringLiteral("Run:") + script;
        info.type = MessageType::Info;
        outputMessage(info);
    }

    ctx->SetUserData(reinterpret_cast<void *>(
                         AppManager::instance()->currentMSecsSinceEpoch()),
                     AsUserDataType::UserData_Timer);
    ctx->SetUserData(reinterpret_cast<void *>(isDbg),
                     AsUserDataType::UserData_isDbg);

    auto timeOutRaw = SettingManager::instance().scriptTimeout();
    auto timeOut = asPWORD(timeOutRaw) * 6000; // min -> ms
    ctx->SetUserData(reinterpret_cast<void *>(timeOut),
                     AsUserDataType::UserData_TimeOut);

    mod->SetUserData(reinterpret_cast<void *>(isDbg),
                     AsUserDataType::UserData_isDbg);

    asPWORD umode = asPWORD(mode);
    ctx->SetUserData(reinterpret_cast<void *>(umode),
                     AsUserDataType::UserData_ContextMode);

    ctx->SetExceptionCallback(asMETHOD(ScriptMachine, exceptionCallback), this,
                              asCALL_THISCALL);

    // Execute the script until completion
    // The script may create co-routines. These will automatically
    // be managed by the context manager
    while (_ctxMgr->ExecuteScripts()) {
        qApp->processEvents();
    }

    _ctx[mode] = nullptr;

    // Check if the main script finished normally
    r = ctx->GetState();
    if (r != asEXECUTION_FINISHED) {
        if (r == asEXECUTION_EXCEPTION) {
            MessageInfo info;
            info.mode = mode;
            info.message =
                QStringLiteral("The script failed with an exception:\n") +
                QString::fromStdString(GetExceptionInfo(ctx, true));
            info.type = MessageType::Error;
            outputMessage(info);
            r = -1;
        } else if (r == asEXECUTION_ABORTED) {
            MessageInfo info;
            info.mode = mode;
            info.message = QStringLiteral("The script was aborted");
            info.type = MessageType::Error;
            outputMessage(info);
            r = -1;
        } else {
            auto e = QMetaEnum::fromType<asEContextState>();
            MessageInfo info;
            info.mode = mode;
            info.message =
                QStringLiteral("The script terminated unexpectedly (") +
                e.valueToKey(r) + QStringLiteral(")");
            info.type = MessageType::Error;
            outputMessage(info);
            r = -1;
        }
    } else {
        // Get the return value from the script
        if (func->GetReturnTypeId() == asTYPEID_INT32) {
            r = *(int *)ctx->GetAddressOfReturnValue();
        } else
            r = 0;
    }

    MessageInfo info;
    info.mode = mode;
    info.message =
        QStringLiteral("The script exited with ") + QString::number(r);
    info.type = MessageType::Info;
    outputMessage(info);

    // Return the context after retrieving the return value
    _ctxMgr->DoneWithContext(ctx);

    // Before leaving, allow the engine to clean up remaining objects by
    // discarding the module and doing a full garbage collection so that
    // this can also be debugged if desired
    _engine->GarbageCollect();

    if (isDbg) {
        _debugger->reset();
    }

    if (retCode) {
        *retCode = r;
    }

    return r >= 0;
}

void ScriptMachine::beginEvaluateDefine() {
    Q_ASSERT(_eMod == nullptr);
    if (_eMod == nullptr) {
        _eMod = _engine->GetModule("WINGDEF", asGM_ALWAYS_CREATE);
        _eMod->SetAccessMask(0x2);
    }
}

QVariant ScriptMachine::evaluateDefine(const QString &code) {
    Q_ASSERT(_eMod);
    if (!_eMod) {
        return {};
    }

    asIScriptFunction *func = nullptr;

    auto ccode = code;
    ccode.prepend("any f(){any ret;ret.store(").append(");return ret;}");
    // start to compile

    auto cr = _eMod->CompileFunction(nullptr, ccode.toUtf8(), 0, 0, &func);
    if (cr < 0) {
        return {};
    }

    // Set up a context to execute the script
    // The context manager will request the context from the
    // pool, which will automatically attach the debugger
    asIScriptContext *ctx = _ctxMgr->AddContext(_engine, func, true);

    ctx->SetUserData(reinterpret_cast<void *>(asPWORD(
                         AppManager::instance()->currentMSecsSinceEpoch())),
                     AsUserDataType::UserData_Timer);

    asPWORD isDbg = 0;
    ctx->SetUserData(reinterpret_cast<void *>(isDbg),
                     AsUserDataType::UserData_isDbg);
    _eMod->SetUserData(reinterpret_cast<void *>(isDbg),
                       AsUserDataType::UserData_isDbg);
    ctx->SetUserData(0, AsUserDataType::UserData_ContextMode);

    ctx->SetExceptionCallback(asMETHOD(ScriptMachine, exceptionCallback), this,
                              asCALL_THISCALL);

    // Execute the script until completion
    // The script may create co-routines. These will automatically
    // be managed by the context manager
    while (_ctxMgr->ExecuteScripts()) {
        qApp->processEvents();
    }

    QVariant result;
    // Check if the main script finished normally
    int r = ctx->GetState();
    if (r == asEXECUTION_FINISHED) {
        auto ret = reinterpret_cast<CScriptAny *>(ctx->GetReturnObject());
        int typeID = ret->GetTypeId();
        switch (typeID) {
        case asTYPEID_BOOL: {
            bool r;
            ret->Retrieve(&r, asTYPEID_BOOL);
            result = r;
        } break;
        case asTYPEID_INT8: {
            qint8 r;
            ret->Retrieve(&r, asTYPEID_INT8);
            result = r;
        } break;
        case asTYPEID_INT16: {
            qint16 r;
            ret->Retrieve(&r, asTYPEID_INT16);
            result = r;
        } break;
        case asTYPEID_INT32: {
            qint32 r;
            ret->Retrieve(&r, asTYPEID_INT32);
            result = r;
        } break;
        case asTYPEID_INT64: {
            qint64 r;
            ret->Retrieve(&r, asTYPEID_INT64);
            result = r;
        } break;
        case asTYPEID_UINT8: {
            quint8 r;
            ret->Retrieve(&r, asTYPEID_UINT8);
            result = r;
        } break;
        case asTYPEID_UINT16: {
            quint16 r;
            ret->Retrieve(&r, asTYPEID_UINT16);
            result = r;
        } break;
        case asTYPEID_UINT32: {
            quint32 r;
            ret->Retrieve(&r, asTYPEID_UINT32);
            result = r;
        } break;
        case asTYPEID_UINT64: {
            quint64 r;
            ret->Retrieve(&r, asTYPEID_UINT64);
            result = r;
        } break;
        case asTYPEID_FLOAT: {
            float r;
            ret->Retrieve(&r, asTYPEID_FLOAT);
            result = r;
        } break;
        case asTYPEID_DOUBLE: {
            double r;
            ret->Retrieve(r);
            result = r;
        } break;
        default:
            break;
        }
    }

    func->Release();

    // Return the context after retrieving the return value
    _ctxMgr->DoneWithContext(ctx);
    _engine->GarbageCollect();

    return result;
}

void ScriptMachine::endEvaluateDefine() {
    if (_eMod) {
        _eMod->Discard();
        _eMod = nullptr;
    }
}

void ScriptMachine::abortDbgScript() {
    if (_debugger) {
        abortScript(ConsoleMode::Scripting);
        _debugger->Resume();
    }
}

void ScriptMachine::abortScript(ConsoleMode mode) {
    auto ctx = _ctx.value(mode, nullptr);
    if (ctx) {
        ctx->Abort();
    }
}

void ScriptMachine::abortScript() {
    abortScript(ConsoleMode::Interactive);
    abortScript(ConsoleMode::Scripting);
    abortScript(ConsoleMode::Background);
}

void ScriptMachine::messageCallback(const asSMessageInfo *msg, void *param) {
    MessageType t = MessageType::Print;
    switch (msg->type) {
    case asMSGTYPE_ERROR:
        t = MessageType::Error;
        break;
    case asMSGTYPE_WARNING:
        t = MessageType::Warn;
        break;
    case asMSGTYPE_INFORMATION:
        t = MessageType::Info;
        break;
    }
    auto ins = static_cast<ScriptMachine *>(param);

    MessageInfo info;
    info.mode = ins->_curMsgMode;
    info.row = msg->row + ins->lineOffset;
    info.col = msg->col + ins->colOffset;
    info.section = msg->section;
    info.message = msg->message;
    info.type = t;
    ins->outputMessage(info);
}

void ScriptMachine::cleanUpPluginSysIDFunction(asIScriptFunction *) {
    // do nothing
    // UserData_API is readonly and it will delete later by its allocator
    // UserData_PluginFn is just an id, not a valid pointer to data
}

ScriptMachine &ScriptMachine::instance() {
    static ScriptMachine ins;
    return ins;
}

ScriptMachine::ScriptMachine() {}

asIScriptModule *ScriptMachine::createModule(ConsoleMode mode) {
    if (isModuleExists(mode)) {
        return nullptr;
    }

    asIScriptModule *mod = nullptr;

    switch (mode) {
    case Interactive:
        mod = nullptr;
    case Scripting:
        mod = _engine->GetModule("WINGSCRIPT", asGM_ALWAYS_CREATE);
        mod->SetAccessMask(0x1);
        break;
    case Background:
        mod = _engine->GetModule("WINGSRV", asGM_ALWAYS_CREATE);
        mod->SetAccessMask(0x1);
        break;
    }

    return mod;
}

asIScriptModule *ScriptMachine::createModuleIfNotExist(ConsoleMode mode) {
    asIScriptModule *mod = nullptr;
    switch (mode) {
    case Interactive:
        mod = _engine->GetModule("WINGCONSOLE", asGM_ONLY_IF_EXISTS);
        mod->SetAccessMask(0x1);
        break;
    case Scripting:
        mod = _engine->GetModule("WINGSCRIPT", asGM_CREATE_IF_NOT_EXISTS);
        mod->SetAccessMask(0x1);
        break;
    case Background:
        mod = _engine->GetModule("WINGSRV", asGM_CREATE_IF_NOT_EXISTS);
        mod->SetAccessMask(0x1);
        break;
    }

    return mod;
}

asIScriptModule *ScriptMachine::module(ConsoleMode mode) const {
    switch (mode) {
    case Interactive:
        return _engine->GetModule("WINGCONSOLE", asGM_ONLY_IF_EXISTS);
    case Scripting:
        return _engine->GetModule("WINGSCRIPT", asGM_ONLY_IF_EXISTS);
    case Background:
        return _engine->GetModule("WINGSRV", asGM_ONLY_IF_EXISTS);
    }
    return nullptr;
}

bool ScriptMachine::isModuleExists(ConsoleMode mode) {
    return module(mode) != nullptr;
}

asIScriptContext *ScriptMachine::requestContextCallback(asIScriptEngine *engine,
                                                        void *param) {
    asIScriptContext *ctx = nullptr;
    auto p = reinterpret_cast<ScriptMachine *>(param);
    Q_ASSERT(p);

    // Check if there is a free context available in the pool
    if (p->_ctxPool.size()) {
        ctx = p->_ctxPool.back();
        p->_ctxPool.pop_back();
    } else {
        // No free context was available so we'll have to create a new one
        ctx = engine->CreateContext();
    }

    // Attach the debugger
    p->checkDebugger(ctx);

    return ctx;
}

void ScriptMachine::returnContextCallback(asIScriptEngine *engine,
                                          asIScriptContext *ctx, void *param) {
    Q_UNUSED(engine);

    if (ctx) {
        // We can also check for possible script exceptions here if so desired

        // Unprepare the context to free any objects it may still hold (e.g.
        // return value) This must be done before making the context available
        // for re-use, as the clean up may trigger other script executions, e.g.
        // if a destructor needs to call a function.
        ctx->Unprepare();

        auto p = reinterpret_cast<ScriptMachine *>(param);
        Q_ASSERT(p);

        // Place the context into the pool for when it will be needed again
        p->_ctxPool.push_back(ctx);
    }
}

void ScriptMachine::debug_break() {
    auto ctx = asGetActiveContext();
    if (ctx) {
        auto &m = ScriptMachine::instance();
        if (m._ctx[ConsoleMode::Scripting] == ctx) {
            m._debugger->DebugBreak(ctx);
        } else {
            ctx->SetException("debug::setBreak can be only used in scripting");
        }
    }
}

QString ScriptMachine::debug_backtrace() {
    auto ctx = asGetActiveContext();
    if (ctx) {
        auto &m = ScriptMachine::instance();
        if (m._ctx[ConsoleMode::Interactive] == ctx) {
            ctx->SetException("debug::break cannot be used in console");
            return {};
        }

        auto cs = ctx->GetCallstackSize();

        QString ret;
        for (asUINT i = 0; i < cs; i++) {
            auto f = ctx->GetFunction(i);
            int col;
            const char *section;
            int row = ctx->GetLineNumber(i, &col, &section);
            ret.append(QStringLiteral("%1 %2[%3:%4]\n")
                           .arg(QString::fromUtf8(
                                    f->GetDeclaration(true, false, true)),
                                QString::fromUtf8(section))
                           .arg(row)
                           .arg(col));
        }

        return ret;
    }

    return {};
}

std::optional<PragmaResult>
ScriptMachine::pragmaCallback(const QString &pragmaText,
                              AsPreprocesser *builder,
                              const QString &sectionname) {
    asIScriptEngine *engine = builder->getEngine();

    // Filter the pragmaText so only what is of interest remains
    // With this the user can add comments and use different whitespaces
    // without affecting the result
    asUINT pos = 0;
    asUINT length = 0;
    QStringList tokens;
    auto pcodes = pragmaText.toUtf8();
    while (pos < pragmaText.size()) {
        asETokenClass tokenClass =
            engine->ParseToken(pcodes.data() + pos, 0, &length);
        if (tokenClass == asTC_IDENTIFIER || tokenClass == asTC_KEYWORD ||
            tokenClass == asTC_VALUE) {
            auto token = pcodes.mid(pos, length);
            tokens << QString::fromUtf8(token);
        }
        if (tokenClass == asTC_UNKNOWN)
            return {};
        pos += length;
    }

    auto pn = tokens.takeFirst();
    return PluginSystem::instance().processPragma(sectionname, pn, tokens);
}

void ScriptMachine::registerEngineAddon(asIScriptEngine *engine) {
    // all modules can access
    engine->SetDefaultAccessMask(0x3);
    RegisterScriptArray(engine, true);
    RegisterQString(engine);
    RegisterScriptRegex(engine);
    RegisterQStringUtils(engine);
    RegisterQStringRegExSupport(engine);
    RegisterScriptMath(engine);
    RegisterScriptMathComplex(engine);
    RegisterScriptWeakRef(engine);
    RegisterScriptAny(engine);
    RegisterScriptDictionary(engine);
    RegisterScriptGrid(engine);
    RegisterScriptHandle(engine);
    RegisterColor(engine);
    RegisterQJson(engine);
    RegisterEnv(engine);
    RegisterScriptFile(engine);
    RegisterScriptDateTime(engine);
    RegisterScriptFileSystem(engine);

    engine->SetDefaultAccessMask(0x1);
    registerExceptionRoutines(engine);
    registerEngineAssert(engine);
    registerEngineClipboard(engine);
    registerEngineDebug(engine);
}

void ScriptMachine::registerEngineAssert(asIScriptEngine *engine) {
    int r;

    // The string type must be available
    Q_ASSERT(engine->GetTypeInfoByDecl("string"));

    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") == 0) {
        r = engine->RegisterGlobalFunction("void assert(bool expression)",
                                           asFUNCTION(scriptAssert),
                                           asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);

        r = engine->RegisterGlobalFunction(
            "void assert_x(bool expression, const string &in msg)",
            asFUNCTION(scriptAssert_X), asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
    } else {
        r = engine->RegisterGlobalFunction("void assert(bool expression)",
                                           WRAP_FN(scriptAssert),
                                           asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);

        r = engine->RegisterGlobalFunction(
            "void assert_x(bool expression, const string &in msg)",
            WRAP_FN(scriptAssert_X), asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
    }
}

void ScriptMachine::registerEngineClipboard(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("clipboard");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // The string type must be available
    Q_ASSERT(engine->GetTypeInfoByDecl("string"));

    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") == 0) {
        r = engine->RegisterGlobalFunction(
            "void setText(const string &in text)", asFUNCTION(clip_setText),
            asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterGlobalFunction(
            "string text()", asFUNCTION(clip_getText), asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterGlobalFunction(
            "void setBinary(const uint8[]@ data)", asFUNCTION(clip_setBinary),
            asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterGlobalFunction(
            "uint8[]@ getBinary()", asFUNCTION(clip_getBinary), asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
    } else {
        r = engine->RegisterGlobalFunction(
            "void setText(const string &in text)", WRAP_FN(clip_setText),
            asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterGlobalFunction(
            "string text()", asFUNCTION(clip_getText), asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterGlobalFunction(
            "void setBinary(const uint8[]@ data)", WRAP_FN(clip_setBinary),
            asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterGlobalFunction(
            "uint8[]@ getBinary()", WRAP_FN(clip_getBinary), asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
    }

    engine->SetDefaultNamespace("");
}

void ScriptMachine::registerEngineDebug(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("debug");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction("void setBreak()",
                                       asFUNCTION(debug_break), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "string backtrace()", asFUNCTION(debug_backtrace), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    engine->SetDefaultNamespace("");
}

void ScriptMachine::registerCallBack(ConsoleMode mode,
                                     const RegCallBacks &callbacks) {
    _regcalls.insert(mode, callbacks);
}

void ScriptMachine::scriptAssert(bool b) {
    auto ctx = asGetActiveContext();
    if (ctx) {
        if (!b) {
            QString buffer = QStringLiteral("Assert failed");
            ctx->SetException(buffer.toUtf8(), false);
        }
    }
}

void ScriptMachine::scriptAssert_X(bool b, const QString &msg) {
    auto ctx = asGetActiveContext();
    if (ctx) {
        if (!b) {
            auto m = msg;
            if (m.isEmpty()) {
                m = QStringLiteral("Assert failed");
            }
            ctx->SetException(m.toUtf8(), false);
        }
    }
}

void ScriptMachine::clip_setText(const QString &text) {
    qApp->clipboard()->setText(text);
}

void ScriptMachine::clip_setBinary(const CScriptArray &array) {
    QByteArray buffer;
    buffer.reserve(array.GetSize());
    array.AddRef();
    for (asUINT i = 0; i < array.GetSize(); ++i) {
        auto item = reinterpret_cast<const asBYTE *>(array.At(i));
        buffer.append(*item);
    }
    array.Release();

    auto c = qApp->clipboard();
    auto mime = new QMimeData;
    mime->setData(QStringLiteral("application/octet-stream"),
                  buffer); // don't use setText()
    c->setMimeData(mime);
}

QString ScriptMachine::clip_getText() { return qApp->clipboard()->text(); }

CScriptArray *ScriptMachine::clip_getBinary() {
    QClipboard *c = qApp->clipboard();

    QByteArray data;
    auto d = c->mimeData();
    data = d->data(QStringLiteral("application/octet-stream"));

    auto engine = ScriptMachine::instance().engine();
    auto len = data.size();
    auto arr =
        CScriptArray::Create(engine->GetTypeInfoByDecl("array<uint8>"), len);
    arr->AddRef();
    for (int i = 0; i < len; ++i) {
        auto addr = arr->At(i);
        *reinterpret_cast<char *>(addr) = data.at(i);
    }
    arr->Release();
    return arr;
}

void ScriptMachine::scriptThrow(const QString &msg) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        ctx->SetException(msg.toUtf8());
    }
}

bool ScriptMachine::isDebugMode(ConsoleMode mode) {
    if (mode == Scripting) {
        auto mod = module(mode);
        if (mod) {
            return reinterpret_cast<asPWORD>(
                mod->GetUserData(AsUserDataType::UserData_isDbg));
        }
    }

    return false;
}

asIScriptEngine *ScriptMachine::engine() const { return _engine; }

asDebugger *ScriptMachine::debugger() const { return _debugger; }

bool ScriptMachine::executeCode(ConsoleMode mode, const QString &code) {
    if (QThread::currentThread() != qApp->thread()) {
        Logger::warning(QStringLiteral("Code must be exec in the main thread"));
        return false;
    }

    if (code.isEmpty()) {
        return true;
    }

    QScopeGuard guard([this]() {
        lineOffset = 0;
        colOffset = 0;
    });

    asIScriptModule *mod = createModuleIfNotExist(mode);
    _engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, false);

    // first, valid the input
    auto ccode = code.toUtf8();
    antlr4::ANTLRInputStream input(ccode.constData(), ccode.length());
    AngelscriptConsoleLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);

    // reuse the listener, ha!
    CStructErrorListener lis([this, mode](const MsgInfo &info) {
        MessageInfo inf;
        inf.mode = mode;
        inf.row = info.line;
        inf.col = info.charPositionInLine;
        inf.message = info.info;
        switch (info.type) {
        case MsgType::Error:
            inf.type = MessageType::Error;
            break;
        case MsgType::Warn:
            inf.type = MessageType::Warn;
            break;
        }
        outputMessage(inf);
    });

    AngelscriptConsoleParser parser(&tokens);
    parser.removeErrorListeners();
    parser.addErrorListener(&lis);
    parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());

    AngelScriptConsoleVisitor visitor(tokens);

    try {
        visitor.visit(parser.script());
    } catch (...) {
        return false;
    }

    asIScriptFunction *func = nullptr;
    _curMsgMode = mode;

    auto decls = visitor.declCode();
    if (!decls.isEmpty()) {
        for (auto &s : visitor.declCode()) {
            auto r = mod->CompileGlobalVar(nullptr, s, 0);
            if (r < 0) {
                MessageInfo info;
                info.mode = mode;
                info.message = QStringLiteral("BadDecl:") + s;
                info.type = MessageType::Error;
                outputMessage(info);
            }
        }
        if (mode == ScriptMachine::Interactive) {
            _cachedGlobalStrs.clear();
        }
    }

    ccode = visitor.execCode().trimmed();
    if (ccode.isEmpty()) {
        return true;
    }

    // ok, wrap the codes
    ccode.prepend("void f(){\n").append("\n}");

    auto off = decls.size();
    lineOffset = off - 1;

    // start to compile
    auto cr = mod->CompileFunction(nullptr, ccode, 0, 0, &func);
    if (cr < 0) {
        MessageInfo info;
        info.mode = mode;
        info.message = QStringLiteral("Script failed to build");
        info.type = MessageType::Error;
        outputMessage(info);
        return false;
    }

    // Set up a context to execute the script
    // The context manager will request the context from the
    // pool, which will automatically attach the debugger
    asIScriptContext *ctx = _ctxMgr->AddContext(_engine, func, true);
    _ctx[mode] = ctx;

    ctx->SetUserData(reinterpret_cast<void *>(asPWORD(
                         AppManager::instance()->currentMSecsSinceEpoch())),
                     AsUserDataType::UserData_Timer);

    asPWORD isDbg = 0;
    ctx->SetUserData(reinterpret_cast<void *>(isDbg),
                     AsUserDataType::UserData_isDbg);
    mod->SetUserData(reinterpret_cast<void *>(isDbg),
                     AsUserDataType::UserData_isDbg);

    auto timeOutRaw = SettingManager::instance().scriptTimeout();
    auto timeOut = asPWORD(timeOutRaw) * 6000; // min -> ms
    ctx->SetUserData(reinterpret_cast<void *>(timeOut),
                     AsUserDataType::UserData_TimeOut);

    asPWORD umode = asPWORD(mode);
    ctx->SetUserData(reinterpret_cast<void *>(umode),
                     AsUserDataType::UserData_ContextMode);

    ctx->SetExceptionCallback(asMETHOD(ScriptMachine, exceptionCallback), this,
                              asCALL_THISCALL);

    // Execute the script until completion
    // The script may create co-routines. These will automatically
    // be managed by the context manager
    while (_ctxMgr->ExecuteScripts()) {
        qApp->processEvents();
    }

    _ctx[mode] = nullptr;

    // Check if the main script finished normally
    int r = ctx->GetState();
    if (r != asEXECUTION_FINISHED) {
        if (r == asEXECUTION_EXCEPTION) {
            MessageInfo info;
            info.mode = mode;
            info.message =
                QStringLiteral("The script failed with an exception:\n") +
                QString::fromStdString(GetExceptionInfo(ctx, true));
            info.type = MessageType::Error;
            outputMessage(info);
            r = -1;
        } else if (r == asEXECUTION_ABORTED) {
            MessageInfo info;
            info.mode = mode;
            info.message = QStringLiteral("The script was aborted");
            info.type = MessageType::Error;
            outputMessage(info);
            r = -1;
        } else {
            auto e = QMetaEnum::fromType<asEContextState>();
            MessageInfo info;
            info.mode = mode;
            info.message =
                QStringLiteral("The script terminated unexpectedly (") +
                e.valueToKey(r) + QStringLiteral(")");
            info.type = MessageType::Error;
            outputMessage(info);
            r = -1;
        }
    } else {
        r = 0;
    }

    // Return the context after retrieving the return value
    _ctxMgr->DoneWithContext(ctx);

    // Before leaving, allow the engine to clean up remaining objects by
    // discarding the module and doing a full garbage collection so that
    // this can also be debugged if desired
    _engine->GarbageCollect();

    func->Release();
    return r >= 0;
}

QString ScriptMachine::scriptGetExceptionInfo() {
    asIScriptContext *ctx = asGetActiveContext();
    if (!ctx)
        return {};

    const char *msg = ctx->GetExceptionString();
    if (msg == 0)
        return {};

    return QString(msg);
}

void ScriptMachine::registerExceptionRoutines(asIScriptEngine *engine) {
    int r;

    // The string type must be available
    Q_ASSERT(engine->GetTypeInfoByDecl("string"));

    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") == 0) {
        r = engine->RegisterGlobalFunction("void throw(const string &in)",
                                           asFUNCTION(scriptThrow),
                                           asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);

        r = engine->RegisterGlobalFunction("string getExceptionInfo()",
                                           asFUNCTION(scriptGetExceptionInfo),
                                           asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);

    } else {
        r = engine->RegisterGlobalFunction("void throw(const string &in)",
                                           WRAP_FN(scriptThrow),
                                           asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);

        r = engine->RegisterGlobalFunction("string getExceptionInfo()",
                                           WRAP_FN(scriptGetExceptionInfo),
                                           asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
    }
}
