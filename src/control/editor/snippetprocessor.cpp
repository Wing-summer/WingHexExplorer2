/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
** FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
** for more details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#include "snippetprocessor.h"

#include <algorithm>

#include <QHash>
#include <QMap>
#include <QRegularExpression>

namespace {
struct Tabstop {
    qsizetype position = -1;
    qsizetype length = 0;
};

class Parser {
public:
    Parser(const QString &source, const SnippetProcessor::Resolver &resolver)
        : m_source(source), m_resolver(resolver) {}

    SnippetResult parse() {
        const auto text = expand(0, m_source.size());
        qsizetype cursor = text.size();
        qsizetype selectionLength = 0;
        if (!m_tabstops.isEmpty()) {
            const auto first = m_tabstops.constBegin();
            cursor = first.value().position;
            selectionLength = first.value().length;
        } else if (m_finalCursor >= 0) {
            cursor = m_finalCursor;
        }
        return SnippetResult{text, cursor, selectionLength};
    }

private:
    const QString &m_source;
    const SnippetProcessor::Resolver &m_resolver;
    QHash<QString, QString> m_defaults;
    QMap<int, Tabstop> m_tabstops;
    qsizetype m_finalCursor = -1;

    static bool isIdentifier(QChar ch) {
        return ch.isLetterOrNumber() || ch == QLatin1Char('_');
    }

    qsizetype matchingBrace(qsizetype open, qsizetype end) const {
        int depth = 1;
        bool escaped = false;
        for (qsizetype i = open + 1; i < end; ++i) {
            const auto ch = m_source.at(i);
            if (escaped) {
                escaped = false;
            } else if (ch == QLatin1Char('\\')) {
                escaped = true;
            } else if (ch == QLatin1Char('{')) {
                ++depth;
            } else if (ch == QLatin1Char('}') && --depth == 0) {
                return i;
            }
        }
        return -1;
    }

    QString unescape(QString value) const {
        static const QRegularExpression escaped(
            QStringLiteral(R"(\\([$\\{}|,]))"));
        return value.replace(escaped, QStringLiteral("\\1"));
    }

    QString firstChoice(const QString &choices) const {
        bool escaped = false;
        for (qsizetype i = 0; i < choices.size(); ++i) {
            if (escaped) {
                escaped = false;
            } else if (choices.at(i) == QLatin1Char('\\')) {
                escaped = true;
            } else if (choices.at(i) == QLatin1Char(',')) {
                return choices.first(i);
            }
        }
        return choices;
    }

    QString expand(qsizetype begin, qsizetype end) {
        QString output;
        for (qsizetype i = begin; i < end;) {
            const auto ch = m_source.at(i);
            if (ch == QLatin1Char('\\') && i + 1 < end) {
                const auto escaped = m_source.at(i + 1);
                if (QStringLiteral("$\\{}|,").contains(escaped)) {
                    output.append(escaped);
                    ++m_currentOutputOffset;
                    i += 2;
                    continue;
                }
                output.append(ch);
                ++m_currentOutputOffset;
                ++i;
                continue;
            }
            if (ch != QLatin1Char('$') || i + 1 >= end) {
                output.append(ch);
                ++m_currentOutputOffset;
                ++i;
                continue;
            }

            if (m_source.at(i + 1).isDigit()) {
                qsizetype next = i + 1;
                while (next < end && m_source.at(next).isDigit()) {
                    ++next;
                }
                const auto index = m_source.mid(i + 1, next - i - 1).toInt();
                appendTabstop(output, QString::number(index), {});
                i = next;
                continue;
            }
            if (m_source.at(i + 1) == QLatin1Char('{')) {
                const auto close = matchingBrace(i + 1, end);
                if (close >= 0) {
                    output.append(expandBraced(i + 2, close));
                    i = close + 1;
                    continue;
                }
            } else if (isIdentifier(m_source.at(i + 1))) {
                qsizetype next = i + 2;
                while (next < end && isIdentifier(m_source.at(next))) {
                    ++next;
                }
                const auto value =
                    m_resolver(m_source.mid(i + 1, next - i - 1));
                output.append(value);
                m_currentOutputOffset += value.size();
                i = next;
                continue;
            }
            output.append(ch);
            ++m_currentOutputOffset;
            ++i;
        }
        return output;
    }

    QString expandBraced(qsizetype begin, qsizetype end) {
        const auto content = m_source.mid(begin, end - begin);
        const auto colon = content.indexOf(QLatin1Char(':'));
        const auto comma = content.indexOf(QLatin1Char(','));
        const auto pipe = content.indexOf(QLatin1Char('|'));
        const auto separator = std::min({colon < 0 ? content.size() : colon,
                                         comma < 0 ? content.size() : comma,
                                         pipe < 0 ? content.size() : pipe});
        const auto key = content.first(separator);
        bool numeric = !key.isEmpty();
        for (const auto ch : key) {
            numeric = numeric && ch.isDigit();
        }

        if (numeric) {
            const auto index = key.toInt();
            if (pipe >= 0 && content.endsWith(QLatin1Char('|'))) {
                const auto options =
                    content.mid(pipe + 1, content.size() - pipe - 2);
                const auto value = unescape(firstChoice(options));
                appendTabstopValue(key, value);
                recordTabstop(index, m_currentOutputOffset, value.size());
                m_currentOutputOffset += value.size();
                return value;
            }
            if (colon >= 0) {
                const auto start = m_currentOutputOffset;
                if (m_defaults.contains(key)) {
                    const auto value = m_defaults.value(key);
                    recordTabstop(index, start, value.size());
                    m_currentOutputOffset += value.size();
                    return value;
                }
                const auto value = expand(begin + colon + 1, end);
                appendTabstopValue(key, value);
                recordTabstop(index, start, value.size());
                return value;
            }
            const auto value = m_defaults.value(key);
            appendTabstopValue(key, value);
            recordTabstop(index, m_currentOutputOffset, value.size());
            m_currentOutputOffset += value.size();
            return value;
        }

        if (!key.isEmpty()) {
            auto value = m_resolver(key);
            const bool usedDefault = value.isEmpty() && colon >= 0;
            if (usedDefault) {
                value = expand(begin + colon + 1, end);
            }
            if (!usedDefault) {
                m_currentOutputOffset += value.size();
            }
            return value;
        }
        m_currentOutputOffset += 3 + content.size();
        return QStringLiteral("${") + content + QLatin1Char('}');
    }

    void appendTabstop(QString &output, const QString &key,
                       const QString &value) {
        const auto index = key.toInt();
        const auto start = m_currentOutputOffset;
        auto text = value;
        if (text.isEmpty()) {
            text = m_defaults.value(key);
        }
        output.append(text);
        appendTabstopValue(key, text);
        recordTabstop(index, start, text.size());
        m_currentOutputOffset += text.size();
    }

    void appendTabstopValue(const QString &key, const QString &value) {
        if (!m_defaults.contains(key) && !value.isEmpty()) {
            m_defaults.insert(key, value);
        }
    }

    void recordTabstop(int index, qsizetype position, qsizetype length) {
        if (index == 0) {
            if (m_finalCursor < 0) {
                m_finalCursor = position;
            }
        } else if (index > 0 && !m_tabstops.contains(index)) {
            m_tabstops.insert(index, {position, length});
        }
    }

    qsizetype m_currentOutputOffset = 0;
};
} // namespace

SnippetProcessor::SnippetProcessor(const Resolver &resolver)
    : m_resolver(resolver) {
    Q_ASSERT(resolver);
}

SnippetResult SnippetProcessor::process(const QString &snippet) {
    Parser parser(snippet, m_resolver);
    return parser.parse();
}
