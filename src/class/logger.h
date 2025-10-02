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

#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QObject>
#include <QSharedPointer>
#include <QTextStream>

class Logger : public QObject {
    Q_OBJECT
public:
    // http://en.wikipedia.org/wiki/Java_logging_framework
    // http://commons.apache.org/logging/guide.html#Message%20Priorities/Levels
    //!
    enum Level : int {
        q0FATAL,
        q1ERROR,
        q2WARN,
        q3INFO,
        q4DEBUG,
        q5TRACE,
        LEVEL_BEGIN = q0FATAL,
        LEVEL_LAST = q5TRACE,
        LEVEL_END = LEVEL_LAST + 1
    };

public:
    static Logger &instance();

    Level logLevel() const;

    static QString getString(Level level);

    static Level defaultLevel();

signals:
    void log(const QString &message);

public slots:
    static void newLine();

    // external use
    static void trace(const QString &message);
    static void warning(const QString &message);
    static void info(const QString &message);
    static void debug(const QString &message);
    static void critical(const QString &message);
    static void logPrint(const QString &message);

    void setLogLevel(Logger::Level level);

    static QString packInfoStr(QString msg);
    static QString packDebugStr(QString msg);
    static QString packErrorStr(QString msg);
    static QString packWarnStr(QString msg);

private:
    explicit Logger(QObject *parent = nullptr);

    virtual ~Logger();

    Q_DISABLE_COPY_MOVE(Logger)

private:
    Level _level = Level::q4DEBUG;
    QSharedPointer<QFile> _file = nullptr;
    QSharedPointer<QTextStream> _stream = nullptr;
};

#endif // LOGGER_H
