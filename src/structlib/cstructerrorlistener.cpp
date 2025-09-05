#include "cstructerrorlistener.h"
#include <antlr4-runtime.h>

CStructErrorListener::CStructErrorListener(const ErrorHandler &handler)
    : antlr4::BaseErrorListener(), _handler(handler) {
    Q_ASSERT(handler);
}

void CStructErrorListener::syntaxError(antlr4::Recognizer *recognizer,
                                       antlr4::Token *offendingSymbol,
                                       size_t line, size_t charPositionInLine,
                                       const std::string &msg,
                                       std::exception_ptr e) {
    reportError(line, charPositionInLine, QString::fromStdString(msg));
}

void CStructErrorListener::reportError(size_t line, size_t charPositionInLine,
                                       const QString &info) {
    reportMsg(MsgType::Error, line, charPositionInLine, info);
}

void CStructErrorListener::reportWarn(size_t line, size_t charPositionInLine,
                                      const QString &info) {
    reportMsg(MsgType::Warn, line, charPositionInLine, info);
}

size_t CStructErrorListener::lineOffset() const { return _line; }

void CStructErrorListener::reportMsg(MsgType type, size_t line,
                                     size_t charPositionInLine,
                                     const QString &info) {
    if (_handler) {
        MsgInfo inf;
        inf.type = type;
        inf.line = line + _line;
        inf.charPositionInLine = charPositionInLine + _charPosOffset;
        inf.info = info;
        _handler(inf);
    }
}

size_t CStructErrorListener::charPosOffset() const { return _charPosOffset; }

void CStructErrorListener::setPositionOffset(size_t line, size_t charPosOff) {
    _line = line;
    _charPosOffset = charPosOff;
}

void CStructErrorListener::resetPositionOffset() {
    _line = 0;
    _charPosOffset = 0;
}
