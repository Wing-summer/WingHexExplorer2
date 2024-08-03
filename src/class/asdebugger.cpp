#include "asdebugger.h"

asDebugger::asDebugger(QObject *parent) : QObject(parent), CDebugger() {}

void asDebugger::Output(const std::string &str) {
    emit onOutput(QString::fromStdString(str));
}

void asDebugger::Output(const QString &str) { emit onOutput(str); }

void asDebugger::PrintHelp() {
    emit onOutput(tr(" c - Continue\n"
                     " s - Step into\n"
                     " n - Next step\n"
                     " o - Step out\n"
                     " b - Set break point\n"
                     " l - List various things\n"
                     " r - Remove break point\n"
                     " p - Print value\n"
                     " w - Where am I?\n"
                     " a - Abort execution\n"
                     " h - Print this help text\n"));
}

void asDebugger::AddFileBreakPoint(const std::string &file, int lineNbr) {
    // Store just file name, not entire path
    size_t r = file.find_last_of("\\/");
    std::string actual;
    if (r != std::string::npos)
        actual = file.substr(r + 1);
    else
        actual = file;

    // Trim the file name
    size_t b = actual.find_first_not_of(" \t");
    size_t e = actual.find_last_not_of(" \t");
    actual = actual.substr(b, e != std::string::npos ? e - b + 1
                                                     : std::string::npos);

    auto s = tr("Setting break point in file '%1' at line %2")
                 .arg(QString::fromStdString(actual))
                 .arg(lineNbr);
    Output(s);

    BreakPoint bp(actual, lineNbr, false);
    m_breakPoints.push_back(bp);
}

void asDebugger::AddFuncBreakPoint(const std::string &func) {
    // Trim the function name
    size_t b = func.find_first_not_of(" \t");
    size_t e = func.find_last_not_of(" \t");
    std::string actual =
        func.substr(b, e != std::string::npos ? e - b + 1 : std::string::npos);

    auto s = tr("Adding deferred break point for function '%1'")
                 .arg(QString::fromStdString(actual));
    Output(s);

    BreakPoint bp(actual, 0, true);
    m_breakPoints.push_back(bp);
}

void asDebugger::ListBreakPoints() {
    // ok to call from super class
    CDebugger::ListBreakPoints();
}

bool asDebugger::CheckBreakPoint(asIScriptContext *ctx) {
    if (ctx == 0)
        return false;

    const char *tmp = 0;
    int lineNbr = ctx->GetLineNumber(0, 0, &tmp);

    // Consider just filename, not the full path
    std::string file = tmp ? tmp : "";
    size_t r = file.find_last_of("\\/");
    if (r != std::string::npos)
        file = file.substr(r + 1);

    // Did we move into a new function?
    asIScriptFunction *func = ctx->GetFunction();
    if (m_lastFunction != func) {
        // Check if any breakpoints need adjusting
        for (size_t n = 0; n < m_breakPoints.size(); n++) {
            // We need to check for a breakpoint at entering the function
            if (m_breakPoints[n].func) {
                if (m_breakPoints[n].name == func->GetName()) {
                    auto s =
                        tr("Entering function '%1'. Transforming it into break "
                           "point")
                            .arg(QString::fromStdString(m_breakPoints[n].name));
                    Output(s);

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
                        auto s = tr("Moving break point %1 in file '%2' to "
                                    "next line with code at line %3")
                                     .arg(n)
                                     .arg(QString::fromStdString(file))
                                     .arg(line);
                        Output(s);

                        // Move the breakpoint to the next line
                        m_breakPoints[n].lineNbr = line;
                    }
                }
            }
        }
    }
    m_lastFunction = func;

    // Determine if there is a breakpoint at the current line
    for (size_t n = 0; n < m_breakPoints.size(); n++) {
        auto bpName = m_breakPoints[n].name;
        // Should we break?
        if (!m_breakPoints[n].func && m_breakPoints[n].lineNbr == lineNbr &&
#ifdef Q_OS_WIN
            strcasecmp(bpName, file)) {
#else
            bpName == file) {
#endif
            auto s = tr("Reached break point %1 in file '%2' at line %3")
                         .arg(n)
                         .arg(QString::fromStdString(file))
                         .arg(lineNbr);
            Output(s);
            return true;
        }
    }

    return false;
}

void asDebugger::ListStatistics(asIScriptContext *ctx) {
    if (ctx == nullptr) {
        Output(tr("No script is running"));
        return;
    }

    asIScriptEngine *engine = ctx->GetEngine();

    asUINT gcCurrSize, gcTotalDestr, gcTotalDet, gcNewObjects, gcTotalNewDestr;
    engine->GetGCStatistics(&gcCurrSize, &gcTotalDestr, &gcTotalDet,
                            &gcNewObjects, &gcTotalNewDestr);

    auto s = tr("Garbage collector:") + tr("\n current size:          ") +
             QString::number(gcCurrSize) + tr("\n total destroyed:       ") +
             QString::number(gcTotalDestr) + tr("\n total detected:        ") +
             QString::number(gcTotalDet) + tr("\n new objects:           ") +
             QString::number(gcNewObjects) + tr("\n new objects destroyed: ") +
             QString::number(gcTotalNewDestr);

    Output(s);
}

void asDebugger::PrintValue(const std::string &expr, asIScriptContext *ctx) {
    if (ctx == 0) {
        Output(tr("No script is running"));
        return;
    }

    asIScriptEngine *engine = ctx->GetEngine();

    // Tokenize the input string to get the variable scope and name
    asUINT len = 0;
    std::string scope;
    std::string name;
    std::string str = expr;
    asETokenClass t = engine->ParseToken(str.c_str(), 0, &len);
    while (t == asTC_IDENTIFIER ||
           (t == asTC_KEYWORD && len == 2 && str.compare(0, 2, "::") == 0)) {
        if (t == asTC_KEYWORD) {
            if (scope == "" && name == "")
                scope = "::"; // global scope
            else if (scope == "::" || scope == "")
                scope = name; // namespace
            else
                scope += "::" + name; // nested namespace
            name = "";
        } else if (t == asTC_IDENTIFIER)
            name.assign(str.c_str(), len);

        // Skip the parsed token and get the next one
        str = str.substr(len);
        t = engine->ParseToken(str.c_str(), 0, &len);
    }

    if (name.size()) {
        // Find the variable
        void *ptr = 0;
        int typeId = 0;

        asIScriptFunction *func = ctx->GetFunction();
        if (!func)
            return;

        // skip local variables if a scope was informed
        if (scope == "") {
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
            if (scope == "") {
                // If no explicit scope was informed then use the namespace of
                // the current function by default
                scope = func->GetNamespace();
            } else if (scope == "::") {
                // The global namespace will be empty
                scope = "";
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
            if (str != "") {
                Output(tr("Invalid expression. Expression doesn't end after "
                          "symbol"));
            } else {
                Output(QString::fromStdString(
                    ToString(ptr, typeId, _expandMembers, engine)));
            }
        } else {
            Output(tr("Invalid expression. No matching symbol"));
        }
    } else {
        Output(tr("Invalid expression. Expected identifier"));
    }
}

bool asDebugger::strcasecmp(const std::string &str1, const std::string &str2) {
    if (str1.length() != str2.length()) {
        return false;
    }
    return _stricmp(str1.c_str(), str2.c_str()) == 0;
}

int asDebugger::getExpandMembers() const { return _expandMembers; }

void asDebugger::setExpandMembers(int newExpandMembers) {
    _expandMembers = qMax(newExpandMembers, 3);
}
