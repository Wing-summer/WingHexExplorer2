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
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "qloggerlib_global.h"

namespace qlogger {

//!
//! \brief The Configuration class - This class stores the configuration details
//! for each log to be created. Default configuration can be created with the
//! constructor and filling only the owner.
//!
class Configuration final {
public:
  //!
  //! \brief Configuration - no empty configs are allowed
  //!
  Configuration() = delete;

  //!
  //! \brief Configuration - the actual constructor to be used with this class
  //! \param logOwner - the only required parameter, name of the log owner
  //! \param lvl - the level were this log should be triggered
  //! \param logTextMask -  the actual log text mask, used only for plain text
  //! logs outputs \param timestampFormat - the date time format to be displayed
  //! on the log text it must use Qt Date Time format convention \param
  //! fileNameMask - the mask for a file name of the log file \param
  //! fileNameTimestampFormat - the timestamp to be used on the mask for a file
  //! name of the log \param filePath - the path were this log file will be
  //! saved \param fileMaxSizeInKb - the max sizes of the log output file
  //!
  Configuration(QString logOwner, Level lvl = q1ERROR,
                QString logTextMask = DEFAULT_TEXT_MASK,
                QString timestampFormat = DEFAULT_TIMESTAMP_FORMAT,
                QString fileNameMask = TEXT_FILE_NAME_MASK,
                QString fileNameTimestampFormat = FILE_NAME_TIMESTAMP_FORMAT,
                QString filePath = DEFAULT_LOG_PATH,
                qint64 maxSizeInBytes = DEFAULT_FILE_SIZE_MB);

  //! destructor as default since not much to do here.
  ~Configuration() = default;

  //!
  //! \brief operator == equals operator is used to check if a configuration
  //! have the same level and owner \param rh \return
  //!
  bool operator==(const Configuration &rh);

  //!
  //! \brief validate - check the class attributes and inform if the class is
  //! valid, true = valid, false = invalid \return
  //!
  bool validate();

  //! just getters and setters to access class attributes

  QString getLogOwner() const;
  void setLogOwner(const QString &value);

  Level getLogLevel() const;
  void setLogLevel(const Level &value);

  QString getTimestampFormat() const;
  void setTimestampFormat(const QString &value);

  QString getLogTextMask() const;
  void setLogTextMask(const QString &value);

  QString getFileNameMask() const;
  void setFileNameMask(const QString &value);

  QString getFileNameTimestampFormat() const;
  void setFileNameTimestampFormat(const QString &value);

  QString getFilePath() const;
  void setFilePath(const QString &value);

  qint64 getFileMaxSizeInBytes() const;
  void setFileMaxSizeInBytes(qint64 value);

private:
  //!
  //! \brief logOwner - the name of the log owner to pass it to the output
  //! classes
  //!
  QString logOwner;

  //!
  //! \brief logLevel - level of this configuration
  //!
  Level logLevel;

  //!
  //! \brief logTextMask - the actual log text mask, used only for plain text
  //! logs outputs
  //!
  QString logTextMask;

  //!
  //! \brief timestampFormat - the date time format to be displayed on the log
  //! text it must use Qt Date Time format convention
  //!
  QString timestampFormat;

  //!
  //! \brief fileNameMask - the mask for a file name of the log file
  //!
  QString fileNameMask;

  //!
  //! \brief fileNameTimestampFormat - the timestamp to be used on the mask for
  //! a file name of the log
  //!
  QString fileNameTimestampFormat;

  //!
  //! \brief filePath - the path were this log file will be saved
  //!
  QString filePath;

  //!
  //! \brief fileMaxSizeInBytes - the max size in bytes of the log output file
  //!
  qint64 fileMaxSizeInBytes;
};

} // namespace qlogger

#endif // CONFIGURATION_H
