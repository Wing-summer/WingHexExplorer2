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
#ifndef OUTPUT_H
#define OUTPUT_H

#include <QSharedPointer>

#include "configuration.h"

using namespace std;

namespace qlogger {

//!
//! \brief The Output class -  Abstract interface class used to define the
//! behaviour of the classes that will log
//!  the messages on all types io.
//!
class Output {
public:
  //!
  //! \brief write -  implement this method in order to be called to write the
  //! log on the io \param message \param owner \param lvl \param timestamp
  //! \param functionName
  //! \param lineNumber
  //!
  virtual void write(const QString message, const QString owner,
                     const Level lvl, const QDateTime timestamp,
                     const QString functionName, const int lineNumber) = 0;

  //!
  //! \brief close -  implement if any cleanup will be done after all logs were
  //! written
  //!
  virtual void close() = 0;

  //!
  //! \brief getConfiguration - retrives the configuration associated with this
  //! output \return
  //!
  QSharedPointer<Configuration> getConfiguration() const;

  //!
  //! \brief Output - the standard constructor for an output
  //! \param conf - the obligatory configuration that this output will hold
  //!
  Output(Configuration *conf);

  //! virtual destructor to avoid any bad behavior on child classes
  virtual ~Output() = default;

  //! no standard constructor, all outputs should receive a configuration
  Output() = delete;
  //! forbid copying (a safer way to define interfaces)
  Output(const Output &) = delete;
  Output &operator=(const Output &) = delete;
  Output(Output &&) = delete;
  Output &operator=(Output &&) = delete;

protected:
  //!
  //! \brief configuration - the actual configuration for the log/owner and this
  //! output
  //!
  QSharedPointer<Configuration> configuration;
};

} // namespace qlogger

#endif // OUTPUT_H
