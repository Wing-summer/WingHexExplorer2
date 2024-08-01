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

    auto s = tr("Setting break point in file '") +
             QString::fromStdString(actual) + tr("' at line ") +
             QString::number(lineNbr);
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

    auto s = tr("Adding deferred break point for function '") +
             QString::fromStdString(actual) + QStringLiteral("'");
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
                    auto s = tr("Entering function '") +
                             QString::fromStdString(m_breakPoints[n].name) +
                             tr("'. Transforming it into break point");
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
                        auto s = tr("Moving break point ") +
                                 QString::number(n) + tr(" in file '") +
                                 QString::fromStdString(file) +
                                 tr("' to next line with code at line ") +
                                 QString::number(line);
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
            auto s = tr("Reached break point ") + QString::number(n) +
                     tr(" in file '") + QString::fromStdString(file) +
                     tr("' at line ") + QString::number(lineNbr);
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

bool asDebugger::strcasecmp(const std::string &str1, const std::string &str2) {
    if (str1.length() != str2.length()) {
        return false;
    }
    return _stricmp(str1.c_str(), str2.c_str()) == 0;
}
