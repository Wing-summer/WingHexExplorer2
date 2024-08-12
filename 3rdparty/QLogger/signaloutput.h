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
#ifndef SIGNALOUTPUT_H
#define SIGNALOUTPUT_H

#include <QObject>

#include "configuration.h"
#include "output.h"

namespace qlogger {

//!
//! \brief The SignalOutput class - An output class tha 'writes' to a SLOT, this
//! class is also a QObject so it can emit a SIGNAL when a log is written
//!
class SignalOutput : public QObject, public Output {
  Q_OBJECT

public:
  //!
  //! \brief SignalOutput
  //! \param cfg
  //!
  SignalOutput(Configuration *cfg);
  SignalOutput() = delete;
  //!
  ~SignalOutput() = default;

  // Output interface
public:
  //!
  //! \brief write
  //! \param message
  //! \param owner
  //! \param lvl
  //! \param timestamp
  //! \param functionName
  //! \param lineNumber
  //!
  void write(const QString message, const QString owner, const Level lvl,
             const QDateTime timestamp, const QString functionName,
             const int lineNumber);

  //!
  //! \brief close
  //!
  void close();

signals:
  //!
  //! \brief qlogger - the signal emmited when 'write' is called
  //! \param logtext - the output text written by this log to the SLOT
  //!
  void qlogger(QString logtext);
};

} // namespace qlogger

#endif // SIGNALOUTPUT_H
