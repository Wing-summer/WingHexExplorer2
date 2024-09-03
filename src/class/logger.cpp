#include "logger.h"

#include "QLogger/qloggerlib/qlogger.h"

#define INFOLOG(msg) "<font color=\"green\">" + msg + "</font>"
#define ERRLOG(msg) "<font color=\"red\">" + msg + "</font>"
#define WARNLOG(msg) "<font color=\"gold\">" + msg + "</font>"

Logger *Logger::ins = new Logger;

Logger::Logger(QObject *parent) : QObject(parent) {}

Logger *Logger::instance() { return ins; }

void Logger::_log(const QString &message) { emit ins->log(message); }

void Logger::warning(const QString &message) {
    qlogger::QLogger::warn(message);
    QString str = message;
    emit ins->log(WARNLOG(tr("[Warn]") + str.replace("\n", "<br />")));
}

void Logger::info(const QString &message) {
    qlogger::QLogger::info(message);
    QString str = message;
    emit ins->log(INFOLOG(tr("[Info]") + str.replace("\n", "<br />")));
}

void Logger::debug(const QString &message) {
    qlogger::QLogger::debug(message);
    QString str = message;
    emit ins->log(tr("[Debug]") + str.replace("\n", "<br />"));
}

void Logger::critical(const QString &message) {
    qlogger::QLogger::error(message);
    QString str = message;
    emit ins->log(ERRLOG(tr("[Error]") + str.replace("\n", "<br />")));
}
