/*
 * QLogger - A tiny Qt logging framework.
 *
 * MIT License
 * Copyright (c) 2013 sandro fadiga
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef QLOGGER_H
#define QLOGGER_H

#include <QHash>
#include <QMutex>
#include <QSharedPointer>

#include "configuration.h"
#include "qloggerlib_global.h"

#include "signaloutput.h"

namespace qlogger {

//!
//! \brief The QLogger class is the main QLogger class, its a singleton
//! responsible for register the log messages to its respective owners and also
//! load the runtime configurations wich defines how each log message will be
//! displayed.
//!
class QLogger final {

public:
  //!
  //! \brief instance - retrives the global instance of the QLogger class, this
  //! is private so no ones need to call this directly \return a global instance
  //! of qlogger
  //!
  static QLogger &instance();

  //!
  //! \brief addLogger - adds a logger (configuration) and takes ownership of
  //! the pointer with output \param configuration - a non null configuration
  //! (that will be owned by qlogger) \param output - a non null output (that
  //! will be owned by qlogger)
  //!
  static void addLogger(Configuration *configuration, Output *output);

  //!
  //! \brief addLogger -  adds a logger (configuration) and takes ownership of
  //! the pointer with default console output \param configuration - a non null
  //! configuration (that will be owned by qlogger) \param type - only the type
  //! of output, the actual object will be created based on this information
  //!
  static void addLogger(Configuration *configuration,
                        OutputType type = CONSOLE);

  //!
  //! \brief addLogger - adds a logger with a default configuration to the list
  //! of loggers in runtime \param logOwner - the name of the log owner
  //!
  static void addLogger(QString logOwner);

  //!
  //! \brief addLogger - adds a logger passing a plain attribute configuration
  //! to the list of loggers in runtime \param logOwner - name of the owner
  //! \param lvl - the desired level for the log
  //! \param ouputType - the type of output for the log
  //! \param logTextMask - the log text mask, if applicable
  //! \param timestampFormat - the timestamp format of the log time
  //! \param fileNameMask - the file name of the log, if applicable
  //! \param fileNameTimestampFormat - the timestamp format of the filename
  //! \param filePath - the file path if applicable
  //! \param fileMaxSizeInBytes - the max size per file in bytes
  //!
  static void
  addLogger(QString logOwner, Level lvl, OutputType ouputType = CONSOLE,
            QString logTextMask = DEFAULT_TEXT_MASK,
            QString timestampFormat = DEFAULT_TIMESTAMP_FORMAT,
            QString fileNameMask = TEXT_FILE_NAME_MASK,
            QString fileNameTimestampFormat = FILE_NAME_TIMESTAMP_FORMAT,
            QString filePath = DEFAULT_LOG_PATH,
            qint64 fileMaxSizeInBytes = DEFAULT_FILE_SIZE_MB);

  //!
  //! \brief log - the generic method to log a message to a specific level
  //! \param message - the actual log message to be, er.. logged
  //! \param owner - the owner of this log
  //! \param functionName - the function (if any) where this log was called
  //! \param lineNumber - the line number were this log was called
  //!
  static void log(Level lvl, const QString &message,
                  const QString &functionName = QString(), int lineNumber = -1,
                  const QString &owner = "root", const QString &cls = "");

  //!
  //! \brief fatal - logs directly to fatal level
  //! \param message - the actual log message to be, er.. logged
  //! \param owner - the owner of this log
  //! \param functionName - the function (if any) where this log was called
  //! \param lineNumber - the line number were this log was called
  //!
  static void fatal(const QString &message, const QString &owner = "root",
                    const QString &functionName = QString(),
                    const QString &cls = "", int lineNumber = -1);

  //!
  //! \brief error - logs directly to error level
  //! \param message - the actual log message to be, er.. logged
  //! \param owner - the owner of this log
  //! \param functionName - the function (if any) where this log was called
  //! \param lineNumber - the line number were this log was called
  //!
  static void error(const QString &message, const QString &owner = "root",
                    const QString &functionName = QString(),
                    const QString &cls = "", int lineNumber = -1);

  //!
  //! \brief warn - logs directly to warn level
  //! \param message - the actual log message to be, er.. logged
  //! \param owner - the owner of this log
  //! \param functionName - the function (if any) where this log was called
  //! \param lineNumber - the line number were this log was called
  //!
  static void warn(const QString &message, const QString &owner = "root",
                   const QString &functionName = QString(),
                   const QString &cls = "", int lineNumber = -1);

  //!
  //! \brief info - logs directly to info level
  //! \param message - the actual log message to be, er.. logged
  //! \param owner - the owner of this log
  //! \param functionName - the function (if any) where this log was called
  //! \param lineNumber - the line number were this log was called
  //!
  static void info(const QString &message, const QString &owner = "root",
                   const QString &functionName = QString(),
                   const QString &cls = "", int lineNumber = -1);

  //!
  //! \brief debug - logs directly to debug level
  //! \param message - the actual log message to be, er.. logged
  //! \param owner - the owner of this log
  //! \param functionName - the function (if any) where this log was called
  //! \param lineNumber - the line number were this log was called
  //!
  static void debug(const QString &message, const QString &owner = "root",
                    const QString &functionName = QString(),
                    const QString &cls = "", int lineNumber = -1);

  //!
  //! \brief trace - logs directly to trace level
  //! \param message - the actual log message to be, er.. logged
  //! \param owner - the owner of this log
  //! \param functionName - the function (if any) where this log was called
  //! \param lineNumber - the line number were this log was called
  //!
  static void trace(const QString &message, const QString &owner = "root",
                    const QString &functionName = QString(),
                    const QString &cls = "", int lineNumber = -1);

  //!
  //! \brief getSignal - return an Signal output class that uses QT Signal/Slot
  //! system, this class "emits" a SIGNAL when an log is written \param owner -
  //! the owner of the logger with SignalOutput \return - an reference to the
  //! SignalOutput object if it exists , otherwise nullptr.
  //!
  static SignalOutput *getSignal(const QString &owner);

protected:
  //!
  //! \brief getConfigFilePath - return the path (if exists) to the
  //! configuration file (.ini) see @readConfigurationFile \param startingPath -
  //! were the qlogger will start to search for the settings \param fileName -
  //! the name of the setting file \return
  //!
  static QString getConfigFilePath(const QString &startingPath,
                                   const QString &fileName);

  //!
  //! \brief readConfigurationFile uses the QSettings standard as configuration
  //! file (force to a .ini file)
  //!
  //! groups are defined as the log owners, a complete configuration with its
  //! descriptions as follows:
  //!
  //! [owner]
  //! level = { FATAL, ERROR, WARN, INFO, DEBUG, TRACE }
  //! outputType = { CONSOLE, TEXT, XML }
  //! logMask = { for console and text can use the symbols %t %o %l %m , if does
  //! not contain %t %l and %m will use the default } maxFileSize = {in kb just
  //! a number 100, 1000... only for TEXT and XML} path = { a valid absolut path
  //! on the system, if invalid path is given then will default to app path }
  //! timestampFormat = { the Qt format for datetime used to format the %t part
  //! of console and text and date_time tag of XML, it defaults to platform
  //! short format } fileName = { file name mask, must contain all %1 %2 %3
  //! params, example: log_%1_%2_%3.txt fileNameTimeStamp = { the timestamp that
  //! will be written in param %3 of the file name mask, must follow QTimeDate
  //! string format.
  //!
  //! [another_owner]
  //! ...
  static void readConfigurationFile(QString cfg);

private:
  //! protected constructor to avoid instatiation outside the class
  QLogger() = default;
  ~QLogger() = default;
  QLogger(const QLogger &) = delete;
  QLogger(QLogger &&) = delete;
  QLogger &operator=(const QLogger &) = delete;
  QLogger &operator=(QLogger &&) = delete;

private:
  //!
  //! \brief writex - log writing mutex to be (used with threads)
  //!
  static QMutex writex;

  //!
  //! \brief instanceFlag - controls if the instance was called once
  //!
  static bool instanceFlag;

  //!
  //! \brief loggers - the owner / output relationship, it stores loggers based
  //! on owner name, tha may have more than one type of output/configuration
  //!
  QMultiHash<QString, QSharedPointer<Output>> loggers;
};

//! MACROS FOR THE PEOPLE!
#define QLOG_FATAL(message, ...)                                               \
  QLogger::log(q0FATAL, message, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define QLOG_ERROR(message, ...)                                               \
  QLogger::log(q1ERROR, message, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define QLOG_WARN(message, ...)                                                \
  QLogger::log(q2WARN, message, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define QLOG_INFO(message, ...)                                                \
  QLogger::log(q3INFO, message, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define QLOG_DEBUG(message, ...)                                               \
  QLogger::log(q4DEBUG, message, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define QLOG_TRACE(message, ...)                                               \
  QLogger::log(q5TRACE, message, __FUNCTION__, __LINE__, ##__VA_ARGS__);

} // namespace qlogger

#endif // QLOGGER_H
