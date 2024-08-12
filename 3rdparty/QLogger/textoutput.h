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
#ifndef TEXTOUTPUT_H
#define TEXTOUTPUT_H

#include <QFile>

#include "consoleoutput.h"

using namespace std;

namespace qlogger {

//!
//! \brief The TextFileOutput class is responsible for write plain text log
//! messages for file or stdout (console)
//!
class PlainTextOutput : public ConsoleOutput {

public:
  //!
  //! \brief PlainTextOutput -  This constructor sets the class to operate in
  //! text file mode, all logs will be written in a file \param conf
  //!
  PlainTextOutput(Configuration *conf);
  PlainTextOutput() = delete;
  //! this destructor also releases any i/o resources associated with this
  //! operation
  virtual ~PlainTextOutput();

  //!
  //! \brief write - this method is responsible for write the log text in the
  //! selected output \param message \param owner \param lvl \param timestamp
  //! \param functionName
  //! \param lineNumber
  //!
  virtual void write(const QString message, const QString owner,
                     const Level lvl, const QDateTime timestamp,
                     const QString functionName, const int lineNumber);

  //!
  //! \brief close
  //!
  virtual void close();

private:
  void _close();

protected:
  //!
  //! \brief createNextFile -  this method is called when a new file needs to be
  //! created due to size limitation
  //!
  virtual void createNextFile();

protected:
  //!
  //! \brief outputFile - the file handler to save on the output on the file
  //! system
  //!
  QSharedPointer<QFile> outputFile;
};

} // namespace qlogger

#endif // TEXTOUTPUT_H
