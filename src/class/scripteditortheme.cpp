/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#include "scripteditortheme.h"

#include <QFile>
#include <QFontDatabase>
#include <QMetaEnum>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

ScriptEditorTheme::ScriptEditorTheme(QObject *parent) : QObject(parent) {}

ScriptEditorTheme::ScriptEditorTheme(const QString &filename, QObject *parent)
    : QObject(parent) {
    if (!filename.isEmpty() && QFile::exists(filename)) {
        loadTheme(filename);
    }
}

ScriptEditorTheme::ColorScheme
ScriptEditorTheme::colorScheme(Styles style) const {
    return m_colors.value(style);
}

void ScriptEditorTheme::setColorScheme(Styles style,
                                       const ColorScheme &scheme) {
    m_colors[style] = scheme;
}

bool ScriptEditorTheme::loadTheme(const QString &filename) {
    QFile xmlFile(filename);
    if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QXmlStreamReader xmlReader(&xmlFile);

    QString name;
    QString version;
    QString author;
    QString comment;

    auto e = QMetaEnum::fromType<Styles>();

    while (!xmlReader.atEnd() && !xmlReader.hasError()) {
        QXmlStreamReader::TokenType token = xmlReader.readNext();

        if (token == QXmlStreamReader::StartDocument) {
            if (xmlReader.name() != QStringLiteral("style-scheme")) {
                return false;
            }
            auto attr = xmlReader.attributes();
            if (readAttr(attr, QStringLiteral("name"), name)) {
                readAttr(attr, QStringLiteral("version"), version);
                readAttr(attr, QStringLiteral("author"), author);
                readAttr(attr, QStringLiteral("comment"), comment);
            } else {
                return false;
            }
            continue;
        }

        ColorScheme scheme;
        QString styleAttr;
        if (token == QXmlStreamReader::StartElement) {
            if (xmlReader.name() == QStringLiteral("style")) {
                auto attr = xmlReader.attributes();
                if (readAttr(attr, QStringLiteral("name"), styleAttr)) {
                    auto v = e.keyToValue(styleAttr.toUtf8());
                    if (v < 0) {
                        continue;
                    }

                    if (readAttr(attr, QStringLiteral("foreground"),
                                 styleAttr)) {
                        QColor color(styleAttr);
                        if (color.isValid()) {
                            scheme.foreground = color;
                        }
                    }

                    if (readAttr(attr, QStringLiteral("background"),
                                 styleAttr)) {
                        QColor color(styleAttr);
                        if (color.isValid()) {
                            scheme.background = color;
                        }
                    }

                    if (readAttr(attr, QStringLiteral("fontfamily"),
                                 styleAttr)) {
                        if (QFontDatabase().hasFamily(styleAttr)) {
                            scheme.fontFamily = styleAttr;
                        }
                    }

                    if (readAttr(attr, QStringLiteral("underLine"),
                                 styleAttr)) {
                        scheme.underLine =
                            QTextCharFormat::UnderlineStyle(styleAttr.toInt());
                    }

                    if (readAttr(attr, QStringLiteral("bold"), styleAttr)) {
                        bool ok;
                        scheme.isBold = string2Bool(styleAttr, ok);
                        if (!ok) {
                            scheme.isBold = false;
                        }
                    }

                    if (readAttr(attr, QStringLiteral("italic"), styleAttr)) {
                        bool ok;
                        scheme.isItalic = string2Bool(styleAttr, ok);
                        if (!ok) {
                            scheme.isItalic = false;
                        }
                    }

                } else {
                    continue;
                }
            }
        }
    }

    m_name = name;
    m_version = version;
    m_author = author;
    m_comment = comment;
    m_isValid = true;
    return true;
}

bool ScriptEditorTheme::saveTheme(const QString &filename) {
    QFile xmlFile(filename);
    if (!xmlFile.open(QFile::WriteOnly | QFile::Text)) {
        return false;
    }
    QXmlStreamWriter stream(&xmlFile);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    auto e = QMetaEnum::fromType<Styles>();
    for (auto p = m_colors.begin(); p != m_colors.end(); p++) {
        stream.writeStartElement("style");
        stream.writeAttribute(QStringLiteral("name"),
                              e.valueToKey(int(p.key())));
        auto v = p.value();
        if (v.foreground.isValid()) {
            stream.writeAttribute(QStringLiteral("foreground"),
                                  v.foreground.name());
        }
        if (v.background.isValid()) {
            stream.writeAttribute(QStringLiteral("background"),
                                  v.background.name());
        }
        if (!v.fontFamily.isEmpty()) {
            stream.writeAttribute(QStringLiteral("fontfamily"), v.fontFamily);
        }
        if (v.underLine != QTextCharFormat::NoUnderline) {
            stream.writeAttribute(QStringLiteral("underLine"),
                                  QString::number(v.underLine));
        }
        if (v.isBold) {
            stream.writeAttribute(QStringLiteral("bold"),
                                  bool2String(v.isBold));
        }
        if (v.isItalic) {
            stream.writeAttribute(QStringLiteral("italic"),
                                  bool2String(v.isBold));
        }
        stream.writeEndElement();
    }

    stream.writeEndDocument();

    xmlFile.close();
    return true;
}

bool ScriptEditorTheme::isValid() const { return m_isValid; }

bool ScriptEditorTheme::readAttr(const QXmlStreamAttributes &attrs,
                                 const QString &name, QString &output) {
    if (attrs.hasAttribute(name)) {
        output = attrs.value(name).toString();
        return true;
    }
    return false;
}

bool ScriptEditorTheme::string2Bool(const QString &str, bool &ok) {
    if (str.compare(QStringLiteral("true"), Qt::CaseInsensitive) == 0) {
        ok = true;
        return true;
    } else if (str.compare(QStringLiteral("false"), Qt::CaseInsensitive) == 0) {
        ok = true;
        return false;
    } else {
        ok = false;
        return false;
    }
}

QString ScriptEditorTheme::bool2String(bool b) {
    return b ? QStringLiteral("true") : QStringLiteral("false");
}

QString ScriptEditorTheme::comment() const { return m_comment; }

QString ScriptEditorTheme::author() const { return m_author; }

QString ScriptEditorTheme::version() const { return m_version; }

QString ScriptEditorTheme::name() const { return m_name; }
