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

#include "logger.h"
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QRegularExpression>

#include "settingmanager.h"
#include "utilities.h"

#define INFOLOG(msg) "<font color=\"green\">" + msg + "</font>"
#define ERRLOG(msg) "<font color=\"red\">" + msg + "</font>"
#define WARNLOG(msg) "<font color=\"gold\">" + msg + "</font>"

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

    // clean up log files if too much
    auto logs = logDir.entryInfoList({"*.log"}, QDir::Files, QDir::Time);
    auto total = logs.size();
    for (decltype(total) i = SettingManager::instance().logCount() - 1;
         i < total; ++i) {
        QFile::remove(logs.at(i).absoluteFilePath());
    }

    auto path = logDir.absoluteFilePath(newFileName);
    _file = QSharedPointer<QFile>(new QFile(path));
    if (_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        _stream->setDevice(_file.get());
    }

    connect(this, &Logger::log, this, [this](const QString &message) {
        static QRegularExpression exp("<[^>]*>");
        auto str = message;
        *_stream << str.remove(exp) << Qt::endl;
    });

    if (qEnvironmentVariableIntValue("WING_DEBUG")) {
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

Logger &Logger::instance() {
    static Logger ins;
    return ins;
}

Logger::Level Logger::logLevel() const { return _level; }

void Logger::logPrint(const QString &message) { emit instance().log(message); }

void Logger::newLine() { logPrint({}); }

void Logger::trace(const QString &message) {
    if (instance()._level >= q5TRACE) {
        QString str = message;
        emit instance().log(packDebugStr(tr("[Trace]") + str));
    }
}

void Logger::warning(const QString &message) {
    if (instance()._level >= q2WARN) {
        QString str = message;
        emit instance().log(packWarnStr(tr("[Warn]") + str));
    }
}

void Logger::info(const QString &message) {
    if (instance()._level >= q3INFO) {
        QString str = message;
        emit instance().log(packInfoStr(tr("[Info]") + str));
    }
}

void Logger::debug(const QString &message) {
    if (instance()._level >= q4DEBUG) {
        QString str = message;
        emit instance().log(packDebugStr(tr("[Debug]") + str));
    }
}

void Logger::critical(const QString &message) {
    if (instance()._level >= q0FATAL) {
        QString str = message;
        emit instance().log(packErrorStr(tr("[Error]") + str));
    }
}

void Logger::setLogLevel(Level level) { _level = level; }

QString Logger::packInfoStr(QString msg) {
    return INFOLOG(msg.replace("\n", "<br />"));
}

QString Logger::packDebugStr(QString msg) {
    return msg.replace("\n", "<br />");
}

QString Logger::packErrorStr(QString msg) {
    return ERRLOG(msg.replace("\n", "<br />"));
}

QString Logger::packWarnStr(QString msg) {
    return WARNLOG(msg.replace("\n", "<br />"));
}

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
