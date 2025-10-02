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

#ifndef SNIPPETPROCESSOR_H
#define SNIPPETPROCESSOR_H

#include <QMap>
#include <QObject>
#include <QString>
#include <QVector>

class SnippetResult {
public:
    QString expandedText;
    qsizetype cursorOffset;

    SnippetResult(const QString &text = {}, qsizetype offset = -1)
        : expandedText(text), cursorOffset(offset) {}
};

class SnippetProcessor {
    Q_GADGET
public:
    enum class TM_CODE {
        TM_SELECTED_TEXT,
        TM_CURRENT_LINE,
        TM_CURRENT_WORD,
        TM_LINE_INDEX,
        TM_LINE_NUMBER,
        TM_FILENAME,
        TM_FILENAME_BASE,
        TM_DIRECTORY,
        TM_FILEPATH,
        RELATIVE_FILEPATH, // same as TM_FILENAME_BASE
        CLIPBOARD,
        WORKSPACE_NAME,   // always ""
        WORKSPACE_FOLDER, // always ""
        CURRENT_YEAR,
        CURRENT_YEAR_SHORT,
        CURRENT_MONTH,
        CURRENT_MONTH_NAME,
        CURRENT_MONTH_NAME_SHORT,
        CURRENT_DATE,
        CURRENT_DAY_NAME,
        CURRENT_DAY_NAME_SHORT,
        CURRENT_HOUR,
        CURRENT_MINUTE,
        CURRENT_SECOND,
        CURRENT_SECONDS_UNIX,
        RANDOM,
        RANDOM_HEX,
        UUID,
        BLOCK_COMMENT_START,
        BLOCK_COMMENT_END,
        LINE_COMMENT
    };
    Q_ENUM(TM_CODE)

public:
    using Resolver = std::function<QString(const QString &)>;

    explicit SnippetProcessor(const Resolver &resolver);

public:
    // Parse and expand from snippet string
    SnippetResult process(const QString &snippet);

private:
    Resolver _resolver;
};

#endif // SNIPPETPROCESSOR_H
