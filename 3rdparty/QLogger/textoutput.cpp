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

#include "textoutput.h"

#include "configuration.h"

#include <QDateTime>
#include <QDir>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif

using namespace std;

namespace qlogger {

PlainTextOutput::PlainTextOutput(Configuration *conf) : ConsoleOutput(conf) {}

PlainTextOutput::~PlainTextOutput() { _close(); }

void PlainTextOutput::createNextFile() {
    // close any open i/o
    close();

    // COMPOSE NEXT FILE NAME
    QString newFileName = configuration->getFileNameMask().arg(
        QCoreApplication::applicationName(), configuration->getLogOwner(),
        QDateTime::currentDateTime().toString(
            configuration->getFileNameTimestampFormat()));

    // TEXT FILE MODE
    QDir dir(configuration->getFilePath());
    QString myFile = dir.absoluteFilePath(newFileName);
    outputFile.clear(); // release the memory from preovious files (if any)
    outputFile = QSharedPointer<QFile>(new QFile(myFile));

    if (outputFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        outputStream->setDevice((outputFile.get()));
    }

    // enables the output to text mode and have correct line breaks
    outputStream->device()->setTextModeEnabled(true);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    outputStream->setEncoding(QStringConverter::Utf8);
#else
    outputStream->setCodec(QTextCodec::codecForName("UTF-8"));
#endif
}

void PlainTextOutput::write(const QString message, const QString owner,
                            const Level lvl, const QDateTime timestamp,
                            const QString functionName, const int lineNumber) {
    if (outputFile.isNull()      // if there is no file
        || !outputFile->isOpen() // or the file is not opened for writing
        ||
        (outputFile->size() >
         configuration
             ->getFileMaxSizeInBytes())) // or the file is already at max size
    {
        createNextFile(); // create a new file
    }

    ConsoleOutput::write(message, owner, lvl, timestamp, functionName,
                         lineNumber);
}

void PlainTextOutput::close() { _close(); }

void PlainTextOutput::_close() {
    if (!outputStream.isNull())
        outputStream->flush();

    if (!outputFile.isNull())
        outputFile->close();

    ConsoleOutput::close();
}

} // namespace qlogger
