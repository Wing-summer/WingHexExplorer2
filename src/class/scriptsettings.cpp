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

#include "scriptsettings.h"

#include <QApplication>
#include <QFontInfo>

#include "settings/settings.h"
#include "wingcodeedit.h"

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_FONT, ("codeedit.font"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_FONT_SIZE, ("codeedit.fontsize"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_THEME, ("codeedit.theme"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_TABS_WIDTH, ("codeedit.tab_width"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_INDENTATION,
                          ("codeedit.indentation"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_WORD_WRAP, ("codeedit.word_wrap"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_MATCH_BRACES,
                          ("codeedit.match_braces"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_SHOW_LINENUMBER,
                          ("codeedit.linenumber"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_SHOW_FOLDING, ("codeedit.folding"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_SHOW_INDENTGUIDES,
                          ("codeedit.indentguides"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_SHOW_LONGLINEEDGE,
                          ("codeedit.longlineedge"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_SHOW_WHITESPACE,
                          ("codeedit.whitespace"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_AUTO_CLOSE_CHAR,
                          ("codeedit.auto_close_char"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_AUTO_IDEN, ("codeedit.auto_iden"))

Q_GLOBAL_STATIC_WITH_ARGS(QString, CONSOLE_FONT, ("console.font"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CONSOLE_FONT_SIZE, ("console.fontsize"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CONSOLE_THEME, ("console.theme"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CONSOLE_TABS_WIDTH, ("console.tab_width"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CONSOLE_INDENTATION, ("console.indentation"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CONSOLE_MATCH_BRACES,
                          ("console.match_braces"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CONSOLE_SHOW_WHITESPACE,
                          ("console.whitespace"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CONSOLE_AUTO_CLOSE_CHAR,
                          ("console.auto_close_char"))

ScriptSettings &ScriptSettings::instance() {
    static ScriptSettings ins;
    return ins;
}

void ScriptSettings::load() {
    auto defaultFontSize = _defaultFont.pointSize();
    Q_ASSERT(defaultFontSize > 0);
    HANDLE_CONFIG;

    READ_CONFIG_STRING(m_editorFontFamily, CODEEDIT_FONT,
                       _defaultFont.family());
    if (!QFontInfo(QFont(m_editorFontFamily)).exactMatch()) {
        m_editorFontFamily = _defaultFont.family();
    }
    READ_CONFIG_STRING(m_consoleFontFamily, CONSOLE_FONT,
                       _defaultFont.family());
    if (!QFontInfo(QFont(m_consoleFontFamily)).exactMatch()) {
        m_consoleFontFamily = _defaultFont.family();
    }

    READ_CONFIG_INT_POSITIVE(m_editorfontSize, CODEEDIT_FONT_SIZE, 10);
    m_editorfontSize = qBound(6, m_editorfontSize, 50);
    READ_CONFIG_INT_POSITIVE(m_consolefontSize, CONSOLE_FONT_SIZE, 10);
    m_consolefontSize = qBound(6, m_consolefontSize, 50);

    READ_CONFIG_STRING(m_editorTheme, CODEEDIT_THEME, {});
    if (!WingCodeEdit::syntaxRepo().theme(m_editorTheme).isValid()) {
        m_editorTheme.clear();
    }
    READ_CONFIG_STRING(m_consoleTheme, CONSOLE_THEME, {});
    if (!WingCodeEdit::syntaxRepo().theme(m_consoleTheme).isValid()) {
        m_consoleTheme.clear();
    }

    READ_CONFIG_INT_POSITIVE(m_editorTabWidth, CODEEDIT_TABS_WIDTH, 4);
    m_editorTabWidth = qBound(2, m_editorTabWidth, 32);
    READ_CONFIG_INT_POSITIVE(m_consoleTabWidth, CONSOLE_TABS_WIDTH, 4);
    m_consoleTabWidth = qBound(2, m_consoleTabWidth, 32);

    READ_CONFIG_INT(m_editorInden, CODEEDIT_INDENTATION, 0);
    m_editorInden = qBound(0, m_editorInden, 2);
    READ_CONFIG_INT(m_consoleInden, CONSOLE_INDENTATION, 0);
    m_consoleInden = qBound(0, m_consoleInden, 2);

    READ_CONFIG_BOOL(m_editorMatchBraces, CODEEDIT_MATCH_BRACES, true);
    READ_CONFIG_BOOL(m_consoleMatchBraces, CONSOLE_MATCH_BRACES, true);

    READ_CONFIG_BOOL(m_editorWordWrap, CODEEDIT_WORD_WRAP, false);
    READ_CONFIG_BOOL(m_editorShowLineNumber, CODEEDIT_SHOW_LINENUMBER, true);
    READ_CONFIG_BOOL(m_editorFolding, CODEEDIT_SHOW_FOLDING, true);
    READ_CONFIG_BOOL(m_editorShowGuideLine, CODEEDIT_SHOW_INDENTGUIDES, true);
    READ_CONFIG_BOOL(m_editorShowLineEdges, CODEEDIT_SHOW_LONGLINEEDGE, false);
    READ_CONFIG_BOOL(m_editorAutoIden, CODEEDIT_AUTO_IDEN, true);

    READ_CONFIG_BOOL(m_editorShowWhiteSpace, CODEEDIT_SHOW_WHITESPACE, false);
    READ_CONFIG_BOOL(m_consoleShowWhiteSpace, CONSOLE_SHOW_WHITESPACE, false);

    READ_CONFIG_BOOL(m_editorAutoCloseChar, CODEEDIT_AUTO_CLOSE_CHAR, true);
    READ_CONFIG_BOOL(m_consoleAutoCloseChar, CONSOLE_AUTO_CLOSE_CHAR, true);
}

void ScriptSettings::reset(SETTINGS cat) {
    __reset(cat);
    load();
}

void ScriptSettings::__reset(SETTINGS cat) {
    HANDLE_CONFIG;
    if (cat.testFlag(SETTING::EDITOR)) {
        WRITE_CONFIG(CODEEDIT_FONT, _defaultFont.defaultFamily());
        WRITE_CONFIG(CODEEDIT_FONT_SIZE, 10);
        WRITE_CONFIG(CODEEDIT_THEME, {});
        WRITE_CONFIG(CODEEDIT_TABS_WIDTH, 4);
        WRITE_CONFIG(CODEEDIT_INDENTATION, 0);
        WRITE_CONFIG(CODEEDIT_MATCH_BRACES, true);
        WRITE_CONFIG(CODEEDIT_WORD_WRAP, false);
        WRITE_CONFIG(CODEEDIT_SHOW_LINENUMBER, true);
        WRITE_CONFIG(CODEEDIT_SHOW_FOLDING, true);
        WRITE_CONFIG(CODEEDIT_SHOW_INDENTGUIDES, true);
        WRITE_CONFIG(CODEEDIT_SHOW_LONGLINEEDGE, false);
        WRITE_CONFIG(CODEEDIT_SHOW_WHITESPACE, false);
        WRITE_CONFIG(CODEEDIT_AUTO_CLOSE_CHAR, true);
        WRITE_CONFIG(CODEEDIT_AUTO_IDEN, true);
    }

    if (cat.testFlag(SETTING::CONSOLE)) {
        WRITE_CONFIG(CONSOLE_FONT, _defaultFont.defaultFamily());
        WRITE_CONFIG(CONSOLE_FONT_SIZE, 10);
        WRITE_CONFIG(CONSOLE_THEME, {});
        WRITE_CONFIG(CONSOLE_TABS_WIDTH, 4);
        WRITE_CONFIG(CONSOLE_INDENTATION, 0);
        WRITE_CONFIG(CONSOLE_MATCH_BRACES, true);
        WRITE_CONFIG(CONSOLE_SHOW_WHITESPACE, false);
        WRITE_CONFIG(CONSOLE_AUTO_CLOSE_CHAR, true);
    }
}

ScriptSettings::ScriptSettings() : QObject() {
    _defaultFont = qApp->font();
    load();
}

bool ScriptSettings::editorAutoIden() const { return m_editorAutoIden; }

void ScriptSettings::setEditorAutoIden(bool newEditorAutoIden) {
    if (m_editorAutoIden != newEditorAutoIden) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CODEEDIT_AUTO_IDEN, newEditorAutoIden);
        m_editorAutoIden = newEditorAutoIden;
    }
}

bool ScriptSettings::consoleAutoCloseChar() const {
    return m_consoleAutoCloseChar;
}

void ScriptSettings::setConsoleAutoCloseChar(bool newConsoleAutoCloseChar) {
    if (m_consoleAutoCloseChar != newConsoleAutoCloseChar) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CONSOLE_AUTO_CLOSE_CHAR, newConsoleAutoCloseChar);
        m_consoleAutoCloseChar = newConsoleAutoCloseChar;
    }
}

bool ScriptSettings::editorAutoCloseChar() const {
    return m_editorAutoCloseChar;
}

void ScriptSettings::setEditorAutoCloseChar(bool newEditorAutoCloseChar) {
    if (m_editorAutoCloseChar != newEditorAutoCloseChar) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CODEEDIT_AUTO_CLOSE_CHAR, newEditorAutoCloseChar);
        m_editorAutoCloseChar = newEditorAutoCloseChar;
    }
}

bool ScriptSettings::consoleShowWhiteSpace() const {
    return m_consoleShowWhiteSpace;
}

void ScriptSettings::setConsoleShowWhiteSpace(bool newConsoleShowWhiteSpace) {
    if (m_consoleShowWhiteSpace != newConsoleShowWhiteSpace) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CONSOLE_SHOW_WHITESPACE, newConsoleShowWhiteSpace);
        m_consoleShowWhiteSpace = newConsoleShowWhiteSpace;
    }
}

bool ScriptSettings::editorShowWhiteSpace() const {
    return m_editorShowWhiteSpace;
}

void ScriptSettings::setEditorShowWhiteSpace(bool newEditorShowWhiteSpace) {
    if (m_editorShowWhiteSpace != newEditorShowWhiteSpace) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CODEEDIT_SHOW_WHITESPACE, newEditorShowWhiteSpace);
        m_editorShowWhiteSpace = newEditorShowWhiteSpace;
    }
}

bool ScriptSettings::editorShowLineEdges() const {
    return m_editorShowLineEdges;
}

void ScriptSettings::setEditorShowLineEdges(bool newEditorShowLineEdges) {
    if (m_editorShowLineEdges != newEditorShowLineEdges) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CODEEDIT_SHOW_LONGLINEEDGE, newEditorShowLineEdges);
        m_editorShowLineEdges = newEditorShowLineEdges;
    }
}

bool ScriptSettings::editorShowGuideLine() const {
    return m_editorShowGuideLine;
}

void ScriptSettings::setEditorShowGuideLine(bool newEditorShowGuidLine) {
    if (m_editorShowGuideLine != newEditorShowGuidLine) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CODEEDIT_SHOW_INDENTGUIDES, newEditorShowGuidLine);
        m_editorShowGuideLine = newEditorShowGuidLine;
    }
}

bool ScriptSettings::editorFolding() const { return m_editorFolding; }

void ScriptSettings::setEditorFolding(bool newEditorFolding) {
    if (m_editorFolding != newEditorFolding) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CODEEDIT_SHOW_FOLDING, newEditorFolding);
        m_editorFolding = newEditorFolding;
    }
}

bool ScriptSettings::editorShowLineNumber() const {
    return m_editorShowLineNumber;
}

void ScriptSettings::setEditorShowLineNumber(bool newEditorShowLineNumber) {
    if (m_editorShowLineNumber != newEditorShowLineNumber) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CODEEDIT_SHOW_LINENUMBER, newEditorShowLineNumber);
        m_editorShowLineNumber = newEditorShowLineNumber;
    }
}

bool ScriptSettings::editorWordWrap() const { return m_editorWordWrap; }

void ScriptSettings::setEditorWordWrap(bool newEditorWordWrap) {
    if (m_editorWordWrap != newEditorWordWrap) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CODEEDIT_WORD_WRAP, newEditorWordWrap);
        m_editorWordWrap = newEditorWordWrap;
    }
}

bool ScriptSettings::consoleMatchBraces() const { return m_consoleMatchBraces; }

void ScriptSettings::setConsoleMatchBraces(bool newConsoleMatchBraces) {
    if (m_consoleMatchBraces != newConsoleMatchBraces) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CONSOLE_MATCH_BRACES, newConsoleMatchBraces);
        m_consoleMatchBraces = newConsoleMatchBraces;
    }
}

bool ScriptSettings::editorMatchBraces() const { return m_editorMatchBraces; }

void ScriptSettings::setEditorMatchBraces(bool newEditorMatchBraces) {
    if (m_editorMatchBraces != newEditorMatchBraces) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CODEEDIT_MATCH_BRACES, newEditorMatchBraces);
        m_editorMatchBraces = newEditorMatchBraces;
    }
}

int ScriptSettings::consoleInden() const { return m_consoleInden; }

void ScriptSettings::setConsoleInden(int newConsoleInden) {
    if (m_consoleInden != newConsoleInden) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CONSOLE_INDENTATION, newConsoleInden);
        m_consoleInden = newConsoleInden;
    }
}

int ScriptSettings::editorInden() const { return m_editorInden; }

void ScriptSettings::setEditorInden(int newEditorInden) {
    if (m_editorInden != newEditorInden) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CODEEDIT_INDENTATION, newEditorInden);
        m_editorInden = newEditorInden;
    }
}

int ScriptSettings::consoleTabWidth() const { return m_consoleTabWidth; }

void ScriptSettings::setConsoleTabWidth(int newConsoleTabWidth) {
    if (m_consoleTabWidth != newConsoleTabWidth) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CONSOLE_TABS_WIDTH, newConsoleTabWidth);
        m_consoleTabWidth = newConsoleTabWidth;
    }
}

int ScriptSettings::editorTabWidth() const { return m_editorTabWidth; }

void ScriptSettings::setEditorTabWidth(int newEditorTabWidth) {
    if (m_editorTabWidth != newEditorTabWidth) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CODEEDIT_TABS_WIDTH, newEditorTabWidth);
        m_editorTabWidth = newEditorTabWidth;
    }
}

QString ScriptSettings::consoleTheme() const { return m_consoleTheme; }

void ScriptSettings::setConsoleTheme(const QString &newConsoleTheme) {
    if (m_consoleTheme != newConsoleTheme) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CONSOLE_THEME, newConsoleTheme);
        m_consoleTheme = newConsoleTheme;
    }
}

QString ScriptSettings::editorTheme() const { return m_editorTheme; }

void ScriptSettings::setEditorTheme(const QString &newEditorTheme) {
    if (m_editorTheme != newEditorTheme) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CODEEDIT_THEME, newEditorTheme);
        m_editorTheme = newEditorTheme;
    }
}

int ScriptSettings::consoleFontSize() const { return m_consolefontSize; }

void ScriptSettings::setConsoleFontSize(int newConsolefontSize) {
    if (m_consolefontSize != newConsolefontSize) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CONSOLE_FONT_SIZE, newConsolefontSize);
        m_consolefontSize = newConsolefontSize;
    }
}

int ScriptSettings::editorFontSize() const { return m_editorfontSize; }

void ScriptSettings::setEditorFontSize(int newEditorfontSize) {
    if (m_editorfontSize != newEditorfontSize) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CODEEDIT_FONT_SIZE, newEditorfontSize);
        m_editorfontSize = newEditorfontSize;
    }
}

QString ScriptSettings::consoleFontFamily() const {
    return m_consoleFontFamily;
}

void ScriptSettings::setConsoleFontFamily(const QString &newConsoleFontFamily) {
    if (m_consoleFontFamily != newConsoleFontFamily) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CONSOLE_FONT, newConsoleFontFamily);
        m_consoleFontFamily = newConsoleFontFamily;
    }
}

QString ScriptSettings::editorFontFamily() const { return m_editorFontFamily; }

void ScriptSettings::setEditorFontFamily(const QString &newEditorFontFamily) {
    if (m_editorFontFamily != newEditorFontFamily) {
        HANDLE_CONFIG;
        WRITE_CONFIG(CODEEDIT_FONT, newEditorFontFamily);
        m_editorFontFamily = newEditorFontFamily;
    }
}
