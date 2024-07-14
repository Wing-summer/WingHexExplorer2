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
#ifndef QLOGGERLIB_GLOBAL_H
#define QLOGGERLIB_GLOBAL_H

#include <QCoreApplication>
#include <QString>
#include <QtCore/qglobal.h>

// evaluate if a dynamic library would be better... it constrains the use of
// macro for log
/*
#if defined(QLOGGERLIB_LIBRARY)
#  define QLOGGER_EXPORT Q_DECL_EXPORT
#else
#  define QLOGGER_EXPORT Q_DECL_IMPORT
#endif
*/

//! the common global consts and methods used across qlogger anything that must
//! be added or configured along the qlogger project must be placed here.
//!
namespace qlogger {

// http://en.wikipedia.org/wiki/Java_logging_framework
// http://commons.apache.org/logging/guide.html#Message%20Priorities/Levels
//!
enum Level { q0FATAL, q1ERROR, q2WARN, q3INFO, q4DEBUG, q5TRACE };

//!
static const QString FATAL_LEVEL = "FATAL";
static const QString ERROR_LEVEL = "ERROR";
static const QString WARN_LEVEL = "WARN";
static const QString INFO_LEVEL = "INFO";
static const QString DEBUG_LEVEL = "DEBUG";
static const QString TRACE_LEVEL = "TRACE";
static const int NUM_LEVEL = 6;
static const QString levelsbuf[NUM_LEVEL] = {
    FATAL_LEVEL, ERROR_LEVEL, WARN_LEVEL, INFO_LEVEL, DEBUG_LEVEL, TRACE_LEVEL};

//!
enum OutputType { CONSOLE, TEXTFILE, XMLFILE, SIGNAL, JSON };

//!
static const QString CONSOLE_OUTPUT = "CONSOLE";
static const QString TEXTFILE_OUTPUT = "TEXT";
static const QString XMLFILE_OUTPUT = "XML";
static const QString SIGNAL_OUTPUT = "SIGNAL";
static const QString JSON_OUTPUT = "JSON";
static const int NUM_OUTPUT = 5;
static const QString outsbuf[NUM_OUTPUT] = {CONSOLE_OUTPUT, TEXTFILE_OUTPUT,
                                            XMLFILE_OUTPUT, SIGNAL_OUTPUT,
                                            JSON_OUTPUT};

//!
static const QString CH_LEVEL = "level";
static const QString CH_OUTPUT_TYPE = "outputType";
static const QString CH_LOG_MASK = "logMask";
static const QString CH_MAX_FILE_SIZE = "maxFileSize";
static const QString CH_PATH = "path";
static const QString CH_TIMESTAMP_FORMAT = "timestampFormat";
static const QString CH_FILE_NAME = "fileName";
static const QString CH_FILE_NAME_TIMESTAMP = "fileNameTimeStamp";

//!
static const QString DEFAULT_TIMESTAMP_FORMAT =
    "MM/dd/yyyy hh:mm:ss"; //! default log timestamp output format

//! %m - message %l - level %o - owner %t - datetime
static const QString DEFAULT_TEXT_MASK = "%t [%o] <%l> (%f) {line:%n} - %m";

//! default as the application path
static const QString DEFAULT_LOG_PATH = ".";

//! default size of the output log file = 1Mb
static const qint64 DEFAULT_FILE_SIZE_MB = 1000000;

//! will be used on the file name mask
static const QString FILE_NAME_TIMESTAMP_FORMAT = "yyyyMMdd_hhmmss";

//! log_appname_logname_datetime.txt %1 = application name , %2 = owner , %3 =
//! timestamp
static const QString TEXT_FILE_NAME_MASK = "log_%1_%2_%3.txt";

//! log_appname_logname_datetime.txt %1 = application name , %2 = owner , %3 =
//! timestamp
static const QString XML_FILE_NAME_MASK = "log_%1_%2_%3.xml";

//! XML tags
static const QString XML_TAG = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
static const QString ROOT_OPEN_TAG = "<LOGS>";
static const QString ROOT_CLOSE_TAG = "</LOGS>";
static const QString LOG_TAG_OPEN = "<LOG>";
static const QString LOG_TAG_CLOSE = "</LOG>";
static const QString DATE_TIME_TAG = "<DATE_TIME>%1</DATE_TIME>";
static const QString OWNER_TAG = "<OWNER>%1</OWNER>";
static const QString MESSAGE_TAG = "<MESSAGE>%1</MESSAGE>";
static const QString LINE_TAG = "<LINE_NUMBER>%1</LINE_NUMBER>";
static const QString FUNCTION_TAG = "<FUNCTION>%1</FUNCTION>";
static const QString LEVEL_TAG = "<LEVEL>%1</LEVEL>";

//! log_appname_logname_datetime.txt %1 = application name , %2 = owner , %3 =
//! timestamp
static const QString JSON_FILE_NAME_MASK = "log_%1_%2_%3.json";

static const QString JSON_LOG_ENTRY = "{ "
                                      "\"owner:\" \"%1\","
                                      "\"level:\" \"%2\","
                                      "\"message:\" \"%3\","
                                      "\"dateTime:\" \"%4\","
                                      "\"function:\" \"%5\","
                                      "\"line:\" \"%6\" "
                                      " }";
static QString JSON_FILE_START = "{ \"logs\": [ ";
static QString JSON_FILE_END = " ] }";

//! utility to convert the level enum to string
inline static QString levelToString(const Level level) {
  return levelsbuf[level];
}

//! convert to enum level from string
inline static Level levelFromString(const QString level) {
  for (int i = 0; i < NUM_LEVEL; i++)
    if (0 == level.trimmed().compare(levelsbuf[i], Qt::CaseInsensitive))
      return static_cast<Level>(i);
  return q1ERROR;
}

//! convert to enum output type from string
inline static OutputType ouputFromString(const QString out) {
  for (int i = 0; i < NUM_OUTPUT; i++)
    if (0 == out.trimmed().compare(outsbuf[i], Qt::CaseInsensitive))
      return static_cast<OutputType>(i);
  return CONSOLE;
}

//! do a plain text format based on the provided log format mask
inline static QString formatLogText(const QString logFormatMask,
                                    const QString message, const QString owner,
                                    const QString lvl, const QString timestamp,
                                    const QString functionName,
                                    const int lineNumber) {
  QString text = QString(logFormatMask);
  text = text.replace("%t", timestamp);
  text = text.replace("%m", message);
  text = text.replace("%l", lvl);
  text = text.replace("%o", owner);
  text = text.replace("%f", functionName);
  text = text.replace("%n", QString::number(lineNumber));
  return text;
}

} // namespace qlogger

#endif // QLOGGERLIB_GLOBAL_H
