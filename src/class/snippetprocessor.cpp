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

#include <QHash>
#include <QMap>
#include <QRegularExpression>

#include <algorithm>

namespace {
struct Tabstop {
    qsizetype position = -1;
    qsizetype length = 0;
};

class Parser {
public:
    Parser(const QString &source, const SnippetProcessor::Resolver &resolver)
        : source_(source), resolver_(resolver) {}

    SnippetResult parse() {
        const auto text = expand(0, source_.size());
        qsizetype cursor = text.size();
        qsizetype selectionLength = 0;
        if (!tabstops_.isEmpty()) {
            const auto first = tabstops_.constBegin();
            cursor = first.value().position;
            selectionLength = first.value().length;
        } else if (finalCursor_ >= 0) {
            cursor = finalCursor_;
        }
        return SnippetResult{text, cursor, selectionLength};
    }

private:
    const QString &source_;
    const SnippetProcessor::Resolver &resolver_;
    QHash<QString, QString> defaults_;
    QMap<int, Tabstop> tabstops_;
    qsizetype finalCursor_ = -1;

    static bool isIdentifier(QChar ch) {
        return ch.isLetterOrNumber() || ch == QLatin1Char('_');
    }

    qsizetype matchingBrace(qsizetype open, qsizetype end) const {
        int depth = 1;
        bool escaped = false;
        for (qsizetype i = open + 1; i < end; ++i) {
            const auto ch = source_.at(i);
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
            const auto ch = source_.at(i);
            if (ch == QLatin1Char('\\') && i + 1 < end) {
                const auto escaped = source_.at(i + 1);
                if (QStringLiteral("$\\{}|,").contains(escaped)) {
                    output.append(escaped);
                    ++currentOutputOffset_;
                    i += 2;
                    continue;
                }
                output.append(ch);
                ++currentOutputOffset_;
                ++i;
                continue;
            }
            if (ch != QLatin1Char('$') || i + 1 >= end) {
                output.append(ch);
                ++currentOutputOffset_;
                ++i;
                continue;
            }

            if (source_.at(i + 1).isDigit()) {
                qsizetype next = i + 1;
                while (next < end && source_.at(next).isDigit()) {
                    ++next;
                }
                const auto index = source_.mid(i + 1, next - i - 1).toInt();
                appendTabstop(output, QString::number(index), {});
                i = next;
                continue;
            }
            if (source_.at(i + 1) == QLatin1Char('{')) {
                const auto close = matchingBrace(i + 1, end);
                if (close >= 0) {
                    output.append(expandBraced(i + 2, close));
                    i = close + 1;
                    continue;
                }
            } else if (isIdentifier(source_.at(i + 1))) {
                qsizetype next = i + 2;
                while (next < end && isIdentifier(source_.at(next))) {
                    ++next;
                }
                const auto value = resolver_(source_.mid(i + 1, next - i - 1));
                output.append(value);
                currentOutputOffset_ += value.size();
                i = next;
                continue;
            }
            output.append(ch);
            ++currentOutputOffset_;
            ++i;
        }
        return output;
    }

    QString expandBraced(qsizetype begin, qsizetype end) {
        const auto content = source_.mid(begin, end - begin);
        const auto colon = content.indexOf(QLatin1Char(':'));
        const auto comma = content.indexOf(QLatin1Char(','));
        const auto pipe = content.indexOf(QLatin1Char('|'));
        const auto separator = std::min({colon < 0 ? content.size() : colon,
                                         comma < 0 ? content.size() : comma,
                                         pipe < 0 ? content.size() : pipe});
        const auto key = content.first(separator);
        bool numeric = !key.isEmpty();
        for (const auto ch : key)
            numeric = numeric && ch.isDigit();

        if (numeric) {
            const auto index = key.toInt();
            if (pipe >= 0 && content.endsWith(QLatin1Char('|'))) {
                const auto options =
                    content.mid(pipe + 1, content.size() - pipe - 2);
                const auto value = unescape(firstChoice(options));
                appendTabstopValue(key, value);
                recordTabstop(index, currentOutputOffset_, value.size());
                currentOutputOffset_ += value.size();
                return value;
            }
            if (colon >= 0) {
                const auto start = currentOutputOffset_;
                if (defaults_.contains(key)) {
                    const auto value = defaults_.value(key);
                    recordTabstop(index, start, value.size());
                    currentOutputOffset_ += value.size();
                    return value;
                }
                const auto value = expand(begin + colon + 1, end);
                appendTabstopValue(key, value);
                recordTabstop(index, start, value.size());
                return value;
            }
            const auto value = defaults_.value(key);
            appendTabstopValue(key, value);
            recordTabstop(index, currentOutputOffset_, value.size());
            currentOutputOffset_ += value.size();
            return value;
        }

        if (!key.isEmpty()) {
            auto value = resolver_(key);
            const bool usedDefault = value.isEmpty() && colon >= 0;
            if (usedDefault) {
                value = expand(begin + colon + 1, end);
            }
            if (!usedDefault) {
                currentOutputOffset_ += value.size();
            }
            return value;
        }
        currentOutputOffset_ += 3 + content.size();
        return QStringLiteral("${") + content + QLatin1Char('}');
    }

    void appendTabstop(QString &output, const QString &key,
                       const QString &value) {
        const auto index = key.toInt();
        const auto start = currentOutputOffset_;
        auto text = value;
        if (text.isEmpty()) {
            text = defaults_.value(key);
        }
        output.append(text);
        appendTabstopValue(key, text);
        recordTabstop(index, start, text.size());
        currentOutputOffset_ += text.size();
    }

    void appendTabstopValue(const QString &key, const QString &value) {
        if (!defaults_.contains(key) && !value.isEmpty()) {
            defaults_.insert(key, value);
        }
    }

    void recordTabstop(int index, qsizetype position, qsizetype length) {
        if (index == 0) {
            if (finalCursor_ < 0) {
                finalCursor_ = position;
            }
        } else if (index > 0 && !tabstops_.contains(index)) {
            tabstops_.insert(index, {position, length});
        }
    }

    qsizetype currentOutputOffset_ = 0;
};
} // namespace

SnippetProcessor::SnippetProcessor(const Resolver &resolver)
    : _resolver(resolver) {
    Q_ASSERT(resolver);
}

SnippetResult SnippetProcessor::process(const QString &snippet) {
    Parser parser(snippet, _resolver);
    return parser.parse();
}
