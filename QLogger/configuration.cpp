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

#include "configuration.h"

namespace qlogger {

Configuration::Configuration(QString logOwner, Level lvl, QString textMask,
                             QString timestampFormat, QString fileMask,
                             QString fileTimestampFormat, QString path,
                             qint64 maxSizeInBytes)
    : logOwner(std::move(logOwner)), logLevel(lvl),
      logTextMask(std::move(textMask)),
      timestampFormat(std::move(timestampFormat)),
      fileNameMask(std::move(fileMask)),
      fileNameTimestampFormat(std::move(fileTimestampFormat)),
      filePath(std::move(path)), fileMaxSizeInBytes(maxSizeInBytes) {}

bool Configuration::operator==(const Configuration &rh) {
  return (this->logOwner == rh.logOwner && this->logLevel == rh.logLevel);
}

bool Configuration::validate() {
  return !(logOwner.isEmpty() || logTextMask.isEmpty() ||
           timestampFormat.isEmpty());
}

qint64 Configuration::getFileMaxSizeInBytes() const {
  return fileMaxSizeInBytes;
}

void Configuration::setFileMaxSizeInBytes(qint64 value) {
  fileMaxSizeInBytes = value;
}

QString Configuration::getFilePath() const { return filePath; }

void Configuration::setFilePath(const QString &value) { filePath = value; }

QString Configuration::getFileNameTimestampFormat() const {
  return fileNameTimestampFormat;
}

void Configuration::setFileNameTimestampFormat(const QString &value) {
  fileNameTimestampFormat = value;
}

QString Configuration::getFileNameMask() const { return fileNameMask; }

void Configuration::setFileNameMask(const QString &value) {
  fileNameMask = value;
}

QString Configuration::getLogTextMask() const { return logTextMask; }

void Configuration::setLogTextMask(const QString &value) {
  logTextMask = value;
}

QString Configuration::getTimestampFormat() const { return timestampFormat; }

void Configuration::setTimestampFormat(const QString &value) {
  timestampFormat = value;
}

Level Configuration::getLogLevel() const { return logLevel; }

void Configuration::setLogLevel(const Level &value) { logLevel = value; }

QString Configuration::getLogOwner() const { return logOwner; }

void Configuration::setLogOwner(const QString &value) { logOwner = value; }

} // namespace qlogger
