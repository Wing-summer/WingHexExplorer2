/****************************************************************************
**
** Copyright (C) 2006-2009 fullmetalcoder <fullmetalcoder@hotmail.fr>
**
** This file is part of the Edyuk project <http://edyuk.org>
**
** This file may be used under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation and appearing in the
** file GPL.txt included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcodedevice.h"

/*!
        \file qcodedevice.cpp
        \brief Implementation of the QcodeDevice class.
*/

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>

QString stripped(const QString &s) { return QDir::cleanPath(s); }

QString makeAbsolute(const QString &rel, const QString &abs) {
    if (QFileInfo(rel).isAbsolute())
        return stripped(rel);

    QFileInfo info(abs);
    QStringList ref, mov = QString(rel).replace("\\", "/").split("/");
    ref = (info.isFile() ? info.absolutePath() : info.absoluteFilePath())
              .replace("\\", "/")
              .split("/");

    foreach (QString s, mov) {
        if (s == ".")
            continue;

        if (s == "..") {
            if (ref.count())
                ref.removeLast();
        } else {
            ref << s;
        }
    }

    return stripped(ref.join("/"));
}

/*!
        \class QCodeDevice
        \brief Code stream that operates on local files
*/

/*!

*/
QCodeDevice::QCodeDevice(const QString &f) : m_pos(0), m_beg(0) {
    setContextFile(f);

    QFile file(f);

    if (file.open(QFile::ReadOnly | QFile::Text)) {
        buffer = file.readAll();
    } else {
        qWarning("Device not found : %s", qPrintable(f));
    }
}

bool QCodeDevice::tryFetchOtherContext(const QString &f) {
    m_pos = 0;
    m_beg = 0;
    buffer.clear();

    QString fn = makeAbsolute(f, contextFile());
    QFile file(fn);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning("Unable to read %s", qPrintable(fn));
        return false;
    }

    buffer = file.readAll();
    setContextFile(fn);

    return true;
}

char QCodeDevice::getChar() {
    return (m_pos < buffer.length()) ? buffer.at(m_pos++) : 0;
}

void QCodeDevice::ungetChar(char) {
    if (m_beg >= m_pos)
        return;

    --m_pos;
}

QByteArray QCodeDevice::readLine() {
    QByteArray b = buffer.mid(buffer.indexOf('\n', m_pos) - m_pos);

    m_pos += b.length();
    return b;
}
