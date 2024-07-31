#include "asdebugger.h"

asDebugger::asDebugger(QObject *parent) : QObject(parent), CDebugger() {}

void asDebugger::Output(const std::string &str) {
    emit onOutput(QString::fromStdString(str));
}

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
