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

#ifndef _QCE_CONFIG_H_
#define _QCE_CONFIG_H_

/*!
        \file qce-config.h
        \brief Utility file for shared library creation
*/

#include <qglobal.h>

/*!
        \macro QCE_EXPORT
        \brief Macro needed for cross-platform shared libraries creation
*/
#ifdef QCE_EXPORT
// QCE_EXPORT manually defined, trust the user
#else
#ifdef _QCODE_EDIT_BUILD_
#ifdef _QCODE_EDIT_EMBED_
#define QCE_EXPORT
#else
#define QCE_EXPORT Q_DECL_EXPORT
#endif
#else
#define QCE_EXPORT Q_DECL_IMPORT
#endif
#endif

#include <QString>
#include <QStringList>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif

namespace QCE {
QString fetchDataFile(const QString &file);

QStringList dataPathes();
void addDataPath(const QString &path);

template <typename Registerable>
class Registar {
public:
    Registar() { Registerable::_register(); }
};

// added by wingsummer
[[maybe_unused]] static QStringList getEncodings() {
    static QStringList encodings;

    if (encodings.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        for (auto e = 0; e < int(QStringConverter::Encoding::LastEncoding);
             ++e) {
            auto ee = QStringConverter::nameForEncoding(
                QStringConverter::Encoding(e));
            if (ee == QStringLiteral("ISO-8859-1")) {
                encodings << QStringLiteral("ASCII");
            }
            encodings << ee;
        }
#else
        for (auto &e : QTextCodec::availableCodecs()) {
            if (e == QStringLiteral("ISO-8859-1")) {
                encodings << QStringLiteral("ASCII");
            } else {
                encodings << e;
            }
        }
#endif
    }

    return encodings;
}

[[maybe_unused]] static QString convertString(const QString &encoding,
                                              const QByteArray &data) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto enc = QStringConverter::encodingForName(encoding.toUtf8());
    if (enc) {
        QStringDecoder decoder(enc.value());
        auto unicode = decoder.decode(data);
        QString data = unicode.data;
        return data;
    } else {
        // Handle the case where the encoding is not recognized
        auto unicode = QString::fromUtf8(data); // Fallback to UTF-8
        return unicode;
    }
#else
    auto enc = QTextCodec::codecForName(encoding.toUtf8());
    auto d = enc->makeDecoder();
    auto unicode = d->toUnicode(data);
    return unicode;
#endif
}

[[maybe_unused]] static QByteArray convertByteArray(const QString &encoding,
                                                    const QString &data) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto enc = QStringConverter::encodingForName(encoding.toUtf8());
    if (enc) {
        QStringEncoder decoder(enc.value());
        return decoder.encode(data);
    } else {
        // Handle the case where the encoding is not recognized
        auto unicode = data.toUtf8(); // Fallback to UTF-8
        return unicode;
    }
#else
    auto enc = QTextCodec::codecForName(encoding.toUtf8());
    auto d = enc->makeEncoder();
    auto unicode = d->toUnicode(data);
    return unicode;
#endif
}

} // namespace QCE

#define QCE_AUTO_REGISTER(T) static QCE::Registar<T> _auto_##T##_registar;

#endif // !_QCE_CONFIG_H_
