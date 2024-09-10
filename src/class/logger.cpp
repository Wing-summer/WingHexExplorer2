#include "logger.h"
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QRegularExpression>

#include "utilities.h"

#define INFOLOG(msg) "<font color=\"green\">" + msg + "</font>"
#define ERRLOG(msg) "<font color=\"red\">" + msg + "</font>"
#define WARNLOG(msg) "<font color=\"gold\">" + msg + "</font>"

Logger *Logger::ins = new Logger;

Logger::Logger(QObject *parent)
    : QObject(parent), _stream(new QTextStream(stdout)) {
    _stream->device()->setTextModeEnabled(true);

    QString newFileName = QStringLiteral("%1_%2_%3.log")
                              .arg(APP_NAME, WINGHEX_VERSION,
                                   QDateTime::currentDateTime().toString(
                                       QStringLiteral("yyyyMMdd_hhmmsss")));
    auto logPath =
        Utilities::getAppDataPath() + QDir::separator() + QStringLiteral("log");

    QDir logDir;
    logDir.mkpath(logPath);
    logDir.setPath(logPath);
    auto path = logDir.absoluteFilePath(newFileName);
    _file = QSharedPointer<QFile>(new QFile(path));
    if (_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        _stream->setDevice(_file.get());
    }

    connect(this, &Logger::log, this, [this](const QString &message) {
        auto str = message;
        *_stream << str.remove(QRegularExpression("<[^>]*>")) << Qt::endl;
    });

    if (qEnvironmentVariableIntValue("debug")) {
        setLogLevel(Level::q4DEBUG);
    } else {
#ifdef QT_DEBUG
        setLogLevel(Level::q4DEBUG);
#else
        setLogLevel(Level::q3INFO);
#endif
    }
}

Logger::~Logger() {
    _stream->flush();
    _file->close();
}

Logger *Logger::instance() { return ins; }

Logger::Level Logger::logLevel() const { return _level; }

void Logger::_log(const QString &message) { emit ins->log(message); }

void Logger::trace(const QString &message) {
    if (instance()->_level >= q5TRACE) {
        QString str = message;
        emit ins->log(tr("[Trace]") + str.replace("\n", "<br />"));
    }
}

void Logger::warning(const QString &message) {
    if (instance()->_level >= q2WARN) {
        QString str = message;
        emit ins->log(WARNLOG(tr("[Warn]") + str.replace("\n", "<br />")));
    }
}

void Logger::info(const QString &message) {
    if (instance()->_level >= q3INFO) {
        QString str = message;
        emit ins->log(INFOLOG(tr("[Info]") + str.replace("\n", "<br />")));
    }
}

void Logger::debug(const QString &message) {
    if (instance()->_level >= q4DEBUG) {
        QString str = message;
        emit ins->log(tr("[Debug]") + str.replace("\n", "<br />"));
    }
}

void Logger::critical(const QString &message) {
    if (instance()->_level >= q0FATAL) {
        QString str = message;
        emit ins->log(ERRLOG(tr("[Error]") + str.replace("\n", "<br />")));
    }
}

void Logger::setLogLevel(Level level) { _level = level; }

QString Logger::getString(Level level) {
    switch (level) {
    case q0FATAL:
        return tr("Critial");
    case q1ERROR:
        return tr("Error");
    case q2WARN:
        return tr("Warning");
    case q3INFO:
        return tr("Info");
    case q4DEBUG:
        return tr("Debug");
    case q5TRACE:
        return tr("Trace");
    default:
        return QString();
    }
}

Logger::Level Logger::defaultLevel() {
#ifdef QT_DEBUG
    return Level::q4DEBUG;
#else
    return Level::q3INFO;
#endif
}
