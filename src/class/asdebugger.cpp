#include "asdebugger.h"

#include <QApplication>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QTextStream>
#include <QThread>

asDebugger::asDebugger() : QObject() {
    _thread = new QThread;
    moveToThread(_thread);
    _thread->start();

    m_action = CONTINUE;
    m_lastFunction = nullptr;
    m_engine = nullptr;
}

asDebugger::~asDebugger() {
    setEngine(nullptr);
    _thread->deleteLater();
}

void asDebugger::registerToStringCallback(const asITypeInfo *ti,
                                          ToStringCallback callback) {
    if (!m_toStringCallbacks.contains(ti))
        m_toStringCallbacks.insert(ti, callback);
}

void asDebugger::registerBreakPointHitCallback(BreakPointHitCallback callback) {
    m_bphitCallback = callback;
}

void asDebugger::takeCommands(asIScriptContext *ctx) {
    while (true) {
        emit onPullVariables(globalVariables(ctx), localVariables(ctx));
        emit onPullCallStack(retriveCallstack(ctx));
        qApp->processEvents();
    }
}

void asDebugger::lineCallback(asIScriptContext *ctx) {
    Q_ASSERT(ctx);

    // This should never happen, but it doesn't hurt to validate it
    if (ctx == nullptr)
        return;

    // By default we ignore callbacks when the context is not active.
    // An application might override this to for example disconnect the
    // debugger as the execution finished.
    if (ctx->GetState() != asEXECUTION_ACTIVE)
        return;

    if (m_action == CONTINUE) {
        if (!checkBreakPoint(ctx))
            return;
    } else if (m_action == STEP_OVER) {
        if (ctx->GetCallstackSize() > m_lastCommandAtStackLevel) {
            if (!checkBreakPoint(ctx))
                return;
        }
    } else if (m_action == STEP_OUT) {
        if (ctx->GetCallstackSize() >= m_lastCommandAtStackLevel) {
            if (!checkBreakPoint(ctx))
                return;
        }
    } else if (m_action == STEP_INTO) {
        checkBreakPoint(ctx);

        // Always break, but we call the check break point anyway
        // to tell user when break point has been reached
    }

    // stringstream s;
    // const char *file = 0;
    // int lineNbr = ctx->GetLineNumber(0, 0, &file);
    // s << (file ? file : "{unnamed}") << ":" << lineNbr << "; "
    //   << ctx->GetFunction()->GetDeclaration() << endl;
    // Output(s.str());

    takeCommands(ctx);
}

void asDebugger::addFileBreakPoint(const QString &file, int lineNbr) {
    // Store just file name, not entire path
    QFileInfo info(file);
    // Trim the file name
    QString actual = info.fileName().trimmed();

    BreakPoint bp(actual, lineNbr, false);
    m_breakPoints.push_back(bp);
}

void asDebugger::removeFileBreakPoint(const QString &file, int lineNbr) {
    QFileInfo info(file);
    QString actual = info.fileName().trimmed();
    m_breakPoints.removeIf([=](const BreakPoint &bp) {
        return bp.name == actual && bp.lineNbr == lineNbr && bp.func == false;
    });
}

void asDebugger::addFuncBreakPoint(const QString &func) {
    // Trim the function name
    QString actual = func.trimmed();

    BreakPoint bp(actual, 0, true);
    m_breakPoints.push_back(bp);
}

void asDebugger::removeFuncBreakPoint(const QString &func) {
    QString actual = func.trimmed();
    m_breakPoints.removeIf([=](const BreakPoint &bp) {
        return bp.name == actual && bp.func == true;
    });
}

void asDebugger::clearBreakPoint() { m_breakPoints.clear(); }

const QVector<asDebugger::BreakPoint> &asDebugger::breakPoints() {
    return m_breakPoints;
}

QVector<asDebugger::VariablesInfo>
asDebugger::localVariables(asIScriptContext *ctx) {
    QVector<VariablesInfo> vars;
    if (ctx == nullptr) {
        return vars;
    }

    asIScriptFunction *func = ctx->GetFunction();
    if (!func) {
        return vars;
    }

    for (asUINT n = 0; n < func->GetVarCount(); n++) {
        // Skip temporary variables
        // TODO: Should there be an option to view temporary variables too?
        const char *name;
        func->GetVar(n, &name);
        if (name == 0 || strlen(name) == 0)
            continue;

        if (ctx->IsVarInScope(n)) {
            int typeId;
            ctx->GetVar(n, 0, 0, &typeId);

            VariablesInfo var;
            var.name = func->GetVarDecl(n);
            var.value = toString(ctx->GetAddressOfVar(n), typeId,
                                 _expandMembers, ctx->GetEngine());
            vars << var;
        }
    }
    return vars;
}

QVector<asDebugger::VariablesInfo>
asDebugger::globalVariables(asIScriptContext *ctx) {
    QVector<VariablesInfo> vars;
    if (ctx == nullptr) {
        return vars;
    }

    // Determine the current module from the function
    asIScriptFunction *func = ctx->GetFunction();
    if (!func) {
        return vars;
    }

    asIScriptModule *mod = func->GetModule();
    if (!mod) {
        return vars;
    }

    for (asUINT n = 0; n < mod->GetGlobalVarCount(); n++) {
        int typeId = 0;
        mod->GetGlobalVar(n, 0, 0, &typeId);

        VariablesInfo var;
        var.name = mod->GetGlobalVarDeclaration(n);
        var.value = toString(mod->GetAddressOfGlobalVar(n), typeId,
                             _expandMembers, ctx->GetEngine());
    }

    return vars;
}

void asDebugger::listMemberProperties(asIScriptContext *ctx) {
    if (ctx == nullptr) {
        return;
    }

    void *ptr = ctx->GetThisPointer();
    if (ptr) {
        QString str;
        QTextStream s(&str);

        s << QStringLiteral("this = ")
          << toString(ptr, ctx->GetThisTypeId(), _expandMembers,
                      ctx->GetEngine())
          << Qt::endl;
    }
}

bool asDebugger::checkBreakPoint(asIScriptContext *ctx) {
    if (ctx == 0)
        return false;

    const char *tmp = 0;
    int lineNbr = ctx->GetLineNumber(0, 0, &tmp);

    QString file = tmp ? QString(tmp) : QString();

    // Did we move into a new function?
    asIScriptFunction *func = ctx->GetFunction();
    if (m_lastFunction != func) {
        // Check if any breakpoints need adjusting
        for (QVector<BreakPoint>::size_type n = 0; n < m_breakPoints.size();
             n++) {
            // We need to check for a breakpoint at entering the function
            if (m_breakPoints[n].func) {
                if (m_breakPoints[n].name == func->GetName()) {
                    // Entering function. Transforming it into break point.

                    // Transform the function breakpoint into a file breakpoint
                    m_breakPoints[n].name = file;
                    m_breakPoints[n].lineNbr = lineNbr;
                    m_breakPoints[n].func = false;
                    m_breakPoints[n].needsAdjusting = false;
                }
            }
            // Check if a given breakpoint fall on a line with code or else
            // adjust it to the next line
            else if (m_breakPoints[n].needsAdjusting &&
                     m_breakPoints[n].name == file) {
                int line = func->FindNextLineWithCode(m_breakPoints[n].lineNbr);
                if (line >= 0) {
                    m_breakPoints[n].needsAdjusting = false;
                    if (line != m_breakPoints[n].lineNbr) {
                        // Moving break point to next line with code

                        auto old = m_breakPoints[n];
                        // Move the breakpoint to the next line
                        m_breakPoints[n].lineNbr = line;
                        emit onAdjustBreakPointLine(old, line);
                    }
                }
            }
        }
    }
    m_lastFunction = func;

    // Determine if there is a breakpoint at the current line
    for (QVector<BreakPoint>::size_type n = 0; n < m_breakPoints.size(); n++) {
        auto bpName = m_breakPoints[n].name;
        // Should we break?
        if (!m_breakPoints[n].func && m_breakPoints[n].lineNbr == lineNbr &&
#ifdef Q_OS_WIN
            bpName.compare(file, Qt::CaseInsensitive) == 0) {
#else
            bpName == file) {
#endif
            m_bphitCallback(m_breakPoints[n], this);
            return true;
        }
    }

    return false;
}

QString asDebugger::toString(void *value, asUINT typeId, int expandMembersLevel,
                             asIScriptEngine *engine) {
    if (value == nullptr)
        return QStringLiteral("<null>");

    // If no engine pointer was provided use the default
    if (engine == nullptr)
        engine = m_engine;

    QString str;
    QTextStream s(&str);
    if (typeId == asTYPEID_VOID)
        return QStringLiteral("<void>");
    else if (typeId == asTYPEID_BOOL)
        return *(bool *)value ? QStringLiteral("true")
                              : QStringLiteral("false");
    else if (typeId == asTYPEID_INT8)
        s << (int)*(signed char *)value;
    else if (typeId == asTYPEID_INT16)
        s << (int)*(signed short *)value;
    else if (typeId == asTYPEID_INT32)
        s << *(signed int *)value;
    else if (typeId == asTYPEID_INT64)
        s << *(asINT64 *)value;
    else if (typeId == asTYPEID_UINT8)
        s << (unsigned int)*(unsigned char *)value;
    else if (typeId == asTYPEID_UINT16)
        s << (unsigned int)*(unsigned short *)value;
    else if (typeId == asTYPEID_UINT32)
        s << *(unsigned int *)value;
    else if (typeId == asTYPEID_UINT64)
        s << *(asQWORD *)value;
    else if (typeId == asTYPEID_FLOAT)
        s << *(float *)value;
    else if (typeId == asTYPEID_DOUBLE)
        s << *(double *)value;
    else if ((typeId & asTYPEID_MASK_OBJECT) == 0) {
        // The type is an enum
        s << *(asUINT *)value;

        // Check if the value matches one of the defined enums
        if (engine) {
            asITypeInfo *t = engine->GetTypeInfoById(typeId);
            for (int n = t->GetEnumValueCount(); n-- > 0;) {
                int enumVal;
                const char *enumName = t->GetEnumValueByIndex(n, &enumVal);
                if (enumVal == *(int *)value) {
                    s << QStringLiteral(", ") << enumName;
                    break;
                }
            }
        }
    } else if (typeId & asTYPEID_SCRIPTOBJECT) {
        // Dereference handles, so we can see what it points to
        if (typeId & asTYPEID_OBJHANDLE)
            value = *(void **)value;

        asIScriptObject *obj = (asIScriptObject *)value;

        // Print the address of the object
        s << QStringLiteral("{") << obj << QStringLiteral("}");

        // Print the members
        if (obj && _expandMembers > 0) {
            asITypeInfo *type = obj->GetObjectType();
            for (asUINT n = 0; n < obj->GetPropertyCount(); n++) {
                if (n == 0)
                    s << QStringLiteral(" ");
                else
                    s << QStringLiteral(", ");

                s << type->GetPropertyDeclaration(n) << QStringLiteral(" = ")
                  << toString(obj->GetAddressOfProperty(n),
                              obj->GetPropertyTypeId(n), _expandMembers - 1,
                              type->GetEngine());
            }
        }
    } else {
        // Dereference handles, so we can see what it points to
        if (typeId & asTYPEID_OBJHANDLE)
            value = *(void **)value;

        // Print the address for reference types so it will be
        // possible to see when handles point to the same object
        if (engine) {
            asITypeInfo *type = engine->GetTypeInfoById(typeId);
            if (type->GetFlags() & asOBJ_REF)
                s << QStringLiteral("{") << value << QStringLiteral("}");

            if (value) {
                // Check if there is a registered to-string callback
                auto it = m_toStringCallbacks.find(type);
                if (it == m_toStringCallbacks.end()) {
                    // If the type is a template instance, there might be a
                    // to-string callback for the generic template type
                    if (type->GetFlags() & asOBJ_TEMPLATE) {
                        asITypeInfo *tmplType =
                            engine->GetTypeInfoByName(type->GetName());
                        it = m_toStringCallbacks.find(tmplType);
                    }
                }

                if (it != m_toStringCallbacks.end()) {
                    if (type->GetFlags() & asOBJ_REF)
                        s << QStringLiteral(" ");

                    // Invoke the callback to get the string representation of
                    // this type
                    s << it.value()(value, _expandMembers, this);
                }
            }
        } else
            s << tr("{no engine}");
    }

    return str;
}

asDebugger::GCStatistic asDebugger::gcStatistics() {
    if (m_engine == nullptr) {
        return {};
    }

    GCStatistic sta;
    m_engine->GetGCStatistics(&sta.currentSize, &sta.totalDestroyed,
                              &sta.totalDetected, &sta.newObjects,
                              &sta.totalNewDestroyed);
    return sta;
}

void asDebugger::setEngine(asIScriptEngine *engine) {
    if (m_engine != engine) {
        if (m_engine)
            m_engine->Release();
        m_engine = engine;
        if (m_engine)
            m_engine->AddRef();
        m_breakPoints.clear();
    }
}

asIScriptEngine *asDebugger::getEngine() { return m_engine; }

QList<asDebugger::CallStackItem>
asDebugger::retriveCallstack(asIScriptContext *ctx) {
    if (ctx == nullptr) {
        return {};
    }

    QList<CallStackItem> callstack;

    QString str;
    QTextStream s(&str);
    const char *file = nullptr;
    int lineNbr = 0;
    for (asUINT n = 0; n < ctx->GetCallstackSize(); n++) {
        lineNbr = ctx->GetLineNumber(n, 0, &file);

        CallStackItem item;
        item.file = file;
        item.lineNbr = lineNbr;
        item.decl = ctx->GetFunction(n)->GetDeclaration();

        callstack << item;
    }

    return callstack;
}

QString asDebugger::printValue(const QString &expr, asIScriptContext *ctx,
                               WatchExpError &error) {
    error = WatchExpError::Error;

    if (ctx == nullptr) {
        return {};
    }

    asIScriptEngine *engine = ctx->GetEngine();

    // Tokenize the input string to get the variable scope and name
    asUINT len = 0;
    QString scope;
    QString name;
    QString str = expr;
    asETokenClass t = engine->ParseToken(str.toUtf8(), 0, &len);
    while (t == asTC_IDENTIFIER ||
           (t == asTC_KEYWORD && len == 2 && str.startsWith("::"))) {
        if (t == asTC_KEYWORD) {
            if (scope.isEmpty() && name.isEmpty())
                scope = "::"; // global scope
            else if (scope == "::" || scope.isEmpty())
                scope = name; // namespace
            else
                scope += "::" + name; // nested namespace
            name.clear();
        } else if (t == asTC_IDENTIFIER)
            name = str.left(len);

        // Skip the parsed token and get the next one
        str = str.mid(len);
        t = engine->ParseToken(str.toUtf8(), 0, &len);
    }

    if (name.size()) {
        // Find the variable
        void *ptr = 0;
        int typeId = 0;

        asIScriptFunction *func = ctx->GetFunction();
        if (!func) {
            return {};
        }

        // skip local variables if a scope was informed
        if (scope.isEmpty()) {
            // We start from the end, in case the same name is reused in
            // different scopes
            for (asUINT n = func->GetVarCount(); n-- > 0;) {
                const char *varName = 0;
                ctx->GetVar(n, 0, &varName, &typeId);
                if (ctx->IsVarInScope(n) && varName != 0 && name == varName) {
                    ptr = ctx->GetAddressOfVar(n);
                    break;
                }
            }

            // Look for class members, if we're in a class method
            if (!ptr && func->GetObjectType()) {
                if (name == "this") {
                    ptr = ctx->GetThisPointer();
                    typeId = ctx->GetThisTypeId();
                } else {
                    asITypeInfo *type =
                        engine->GetTypeInfoById(ctx->GetThisTypeId());
                    for (asUINT n = 0; n < type->GetPropertyCount(); n++) {
                        const char *propName = 0;
                        int offset = 0;
                        bool isReference = 0;
                        int compositeOffset = 0;
                        bool isCompositeIndirect = false;
                        type->GetProperty(n, &propName, &typeId, 0, 0, &offset,
                                          &isReference, 0, &compositeOffset,
                                          &isCompositeIndirect);
                        if (name == propName) {
                            ptr = (void *)(((asBYTE *)ctx->GetThisPointer()) +
                                           compositeOffset);
                            if (isCompositeIndirect)
                                ptr = *(void **)ptr;
                            ptr = (void *)(((asBYTE *)ptr) + offset);
                            if (isReference)
                                ptr = *(void **)ptr;
                            break;
                        }
                    }
                }
            }
        }

        // Look for global variables
        if (!ptr) {
            if (scope.isEmpty()) {
                // If no explicit scope was informed then use the namespace of
                // the current function by default
                scope = func->GetNamespace();
            } else if (scope == "::") {
                // The global namespace will be empty
                scope.clear();
            }

            asIScriptModule *mod = func->GetModule();
            if (mod) {
                for (asUINT n = 0; n < mod->GetGlobalVarCount(); n++) {
                    const char *varName = 0, *nameSpace = 0;
                    mod->GetGlobalVar(n, &varName, &nameSpace, &typeId);

                    // Check if both name and namespace match
                    if (name == varName && scope == nameSpace) {
                        ptr = mod->GetAddressOfGlobalVar(n);
                        break;
                    }
                }
            }
        }

        if (ptr) {
            // TODO: If there is a . after the identifier, check for members
            // TODO: If there is a [ after the identifier try to call the
            // 'opIndex(expr) const' method
            if (!str.isEmpty()) {
                error = WatchExpError::NotEndAfterSymbol;
                return {};
            } else {
                return toString(ptr, typeId, _expandMembers, engine);
            }
        } else {
            error = WatchExpError::NoMatchingSymbol;
            return {};
        }
    } else {
        error = WatchExpError::ExpectedIdentifier;
        return {};
    }
}

bool asDebugger::interpretCommand(const QString &cmd, asIScriptContext *ctx) {
    if (cmd.length() == 0)
        return true;

    switch (cmd[0].toLatin1()) {
    case 'c':
        m_action = CONTINUE;
        break;

    case 's':
        m_action = STEP_INTO;
        break;

    case 'n':
        m_action = STEP_OVER;
        m_lastCommandAtStackLevel = ctx ? ctx->GetCallstackSize() : 1;
        break;

    case 'o':
        m_action = STEP_OUT;
        m_lastCommandAtStackLevel = ctx ? ctx->GetCallstackSize() : 0;
        break;

    case 'b': {
        // Set break point
        // auto p = cmd.indexOf(cmdRegExp, 1);
        // auto div = cmd.indexOf(':');
        // if (div > 2 && p > 1) {
        //     auto file = cmd.mid(2, div - 2);
        //     auto line = cmd.mid(div + 1);

        //     int nbr = line.toInt();

        //     addFileBreakPoint(file, nbr);
        // } else if (div < 0 && p > 1) {
        //     auto func = cmd.mid(p);
        //     addFuncBreakPoint(func);
        // } else {
        //     outputString(tr(
        //         "Incorrect format for setting break point, expected one
        //         of:\n" " b <file name>:<line number>\n" " b <function
        //         name>"));
        // }
    }
        // take more commands
        return false;

    case 'r': {
        // Remove break point
        // auto p = cmd.indexOf(cmdRegExp, 1);
        // if (cmd.length() > 2 && p > 1) {
        //     auto br = cmd.mid(2);
        //     if (br == QStringLiteral("all")) {
        //         m_breakPoints.clear();
        //         outputString(tr("All break points have been removed"));
        //     } else {
        //         int nbr = br.toInt();
        //         if (nbr >= 0 && nbr < m_breakPoints.size())
        //             m_breakPoints.erase(m_breakPoints.begin() + nbr);
        //     }
        // } else {
        //     outputString(
        //         "Incorrect format for removing break points, expected:\n"
        //         " r <all|number of break point>\n");
        // }
    }
        // take more commands
        return false;

    case 'l': {
        // List something
        // bool printHelp = false;
        // auto p = cmd.indexOf(cmdRegExp, 1);
        // if (p > 1) {
        //     auto c = cmd.at(2);
        //     if (c == 'b') {
        //         // listBreakPoints();
        //     } else if (c == 'v') {
        //         // listLocalVariables(ctx);
        //     } else if (c == 'g') {
        //         // listGlobalVariables(ctx);
        //     } else if (c == 'm') {
        //         // listMemberProperties(ctx);
        //     } else if (c == 's') {
        //         // listStatistics(ctx);
        //     } else {
        //         outputString(tr("Unknown list option."));
        //         printHelp = true;
        //     }
        // } else {
        //     outputString(tr("Incorrect format for list command."));
        //     printHelp = true;
        // }

        // if (printHelp) {
        //     outputString(tr("Expected format: \n"
        //                     " l <list option>\n"
        //                     "Available options: \n"
        //                     " b - breakpoints\n"
        //                     " v - local variables\n"
        //                     " m - member properties\n"
        //                     " g - global variables\n"
        //                     " s - statistics"));
        // }
    }
        // take more commands
        return false;

    case 'h':
        // printHelp();
        // take more commands
        return false;

    case 'p': {
        // Print a value
        // auto p = cmd.indexOf(cmdRegExp, 1);
        // if (p > 1) {
        //     printValue(cmd.mid(p), ctx);
        // } else {
        //     outputString(tr("Incorrect format for print, expected:\n"
        //                     " p <expression>"));
        // }
    }
        // take more commands
        return false;

    case 'a':
        // abort the execution
        if (ctx == nullptr) {
            // outputString(tr("No script is running"));
            return false;
        }
        ctx->Abort();
        break;
    }

    // Continue execution
    return true;
}

int asDebugger::expandMembers() const { return _expandMembers; }

void asDebugger::setExpandMembers(int newExpandMembers) {
    _expandMembers = qMax(newExpandMembers, 3);
}
