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

#define WRITE_CONFIG_EDITOR_SET(config, flag, dvalue)                          \
    if (this->_setUnsavedEditor.testFlag(flag)) {                              \
        WRITE_CONFIG(config, dvalue);                                          \
        _setUnsavedEditor.setFlag(flag, false);                                \
    }

#define WRITE_CONFIG_EDITOR_RESET(config, flag, dvalue)                        \
    do {                                                                       \
        WRITE_CONFIG(config, dvalue);                                          \
        _setUnsavedEditor.setFlag(flag, false);                                \
    } while (0);

#define WRITE_CONFIG_CONSOLE_SET(config, flag, dvalue)                         \
    if (this->_setUnsavedConsole.testFlag(flag)) {                             \
        WRITE_CONFIG(config, dvalue);                                          \
        _setUnsavedConsole.setFlag(flag, false);                               \
    }

#define WRITE_CONFIG_CONSOLE_RESET(config, flag, dvalue)                       \
    do {                                                                       \
        WRITE_CONFIG(config, dvalue);                                          \
        _setUnsavedConsole.setFlag(flag, false);                               \
    } while (0);

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

void ScriptSettings::save(SETTINGS cat) {
    HANDLE_CONFIG;
    if (cat.testFlag(SETTING::EDITOR)) {
        WRITE_CONFIG_EDITOR_SET(CODEEDIT_FONT, SETTING_ITEM::FONT,
                                m_editorFontFamily);
        WRITE_CONFIG_EDITOR_SET(CODEEDIT_FONT_SIZE, SETTING_ITEM::FONT_SIZE,
                                m_editorfontSize);
        WRITE_CONFIG_EDITOR_SET(CODEEDIT_THEME, SETTING_ITEM::THEME,
                                m_editorTheme);
        WRITE_CONFIG_EDITOR_SET(CODEEDIT_TABS_WIDTH, SETTING_ITEM::TAB_WIDTH,
                                m_editorTabWidth);
        WRITE_CONFIG_EDITOR_SET(CODEEDIT_INDENTATION, SETTING_ITEM::INDENTATION,
                                m_editorInden);
        WRITE_CONFIG_EDITOR_SET(CODEEDIT_MATCH_BRACES,
                                SETTING_ITEM::MATCH_BRACES,
                                m_editorMatchBraces);
        WRITE_CONFIG_EDITOR_SET(CODEEDIT_WORD_WRAP, SETTING_ITEM::WORD_WRAP,
                                m_editorWordWrap);
        WRITE_CONFIG_EDITOR_SET(CODEEDIT_SHOW_LINENUMBER,
                                SETTING_ITEM::SHOW_LINENUMBER,
                                m_editorShowLineNumber);
        WRITE_CONFIG_EDITOR_SET(CODEEDIT_SHOW_FOLDING,
                                SETTING_ITEM::SHOW_FOLDING, m_editorFolding);
        WRITE_CONFIG_EDITOR_SET(CODEEDIT_SHOW_INDENTGUIDES,
                                SETTING_ITEM::SHOW_INDENTGUIDES,
                                m_editorShowGuideLine);
        WRITE_CONFIG_EDITOR_SET(CODEEDIT_SHOW_LONGLINEEDGE,
                                SETTING_ITEM::SHOW_LONGLINEEDGE,
                                m_editorShowLineEdges);
        WRITE_CONFIG_EDITOR_SET(CODEEDIT_SHOW_WHITESPACE,
                                SETTING_ITEM::SHOW_WHITESPACE,
                                m_editorShowWhiteSpace);
        WRITE_CONFIG_EDITOR_SET(CODEEDIT_AUTO_CLOSE_CHAR,
                                SETTING_ITEM::AUTO_CLOSE_CHAR,
                                m_editorAutoCloseChar);
        WRITE_CONFIG_EDITOR_SET(CODEEDIT_AUTO_IDEN, SETTING_ITEM::AUTO_IDEN,
                                m_editorAutoIden);
        Q_EMIT editorSettingsUpdate();
    }

    if (cat.testFlag(SETTING::CONSOLE)) {
        WRITE_CONFIG_CONSOLE_SET(CONSOLE_FONT, SETTING_ITEM::FONT,
                                 m_consoleFontFamily);
        WRITE_CONFIG_CONSOLE_SET(CONSOLE_FONT_SIZE, SETTING_ITEM::FONT_SIZE,
                                 m_consolefontSize);
        WRITE_CONFIG_CONSOLE_SET(CONSOLE_THEME, SETTING_ITEM::THEME,
                                 m_consoleTheme);
        WRITE_CONFIG_CONSOLE_SET(CONSOLE_TABS_WIDTH, SETTING_ITEM::TAB_WIDTH,
                                 m_consoleTabWidth);
        WRITE_CONFIG_CONSOLE_SET(CONSOLE_INDENTATION, SETTING_ITEM::INDENTATION,
                                 m_consoleInden);
        WRITE_CONFIG_CONSOLE_SET(CONSOLE_MATCH_BRACES,
                                 SETTING_ITEM::MATCH_BRACES,
                                 m_consoleMatchBraces);
        WRITE_CONFIG_CONSOLE_SET(CONSOLE_SHOW_WHITESPACE,
                                 SETTING_ITEM::SHOW_WHITESPACE,
                                 m_consoleShowWhiteSpace);
        WRITE_CONFIG_CONSOLE_SET(CONSOLE_AUTO_CLOSE_CHAR,
                                 SETTING_ITEM::AUTO_CLOSE_CHAR,
                                 m_consoleAutoCloseChar);
        Q_EMIT consoleSettingUpdate();
    }
}

void ScriptSettings::reset(SETTINGS cat) {
    __reset(cat);
    load();
}

void ScriptSettings::__reset(SETTINGS cat) {
    HANDLE_CONFIG;
    if (cat.testFlag(SETTING::EDITOR)) {
        WRITE_CONFIG_EDITOR_RESET(CODEEDIT_FONT, SETTING_ITEM::FONT,
                                  _defaultFont.defaultFamily());
        WRITE_CONFIG_EDITOR_RESET(CODEEDIT_FONT_SIZE, SETTING_ITEM::FONT_SIZE,
                                  10);
        WRITE_CONFIG_EDITOR_RESET(CODEEDIT_THEME, SETTING_ITEM::THEME, {});
        WRITE_CONFIG_EDITOR_RESET(CODEEDIT_TABS_WIDTH, SETTING_ITEM::TAB_WIDTH,
                                  4);
        WRITE_CONFIG_EDITOR_RESET(CODEEDIT_INDENTATION,
                                  SETTING_ITEM::INDENTATION, 0);
        WRITE_CONFIG_EDITOR_RESET(CODEEDIT_MATCH_BRACES,
                                  SETTING_ITEM::MATCH_BRACES, true);
        WRITE_CONFIG_EDITOR_RESET(CODEEDIT_WORD_WRAP, SETTING_ITEM::WORD_WRAP,
                                  false);
        WRITE_CONFIG_EDITOR_RESET(CODEEDIT_SHOW_LINENUMBER,
                                  SETTING_ITEM::SHOW_LINENUMBER, true);
        WRITE_CONFIG_EDITOR_RESET(CODEEDIT_SHOW_FOLDING,
                                  SETTING_ITEM::SHOW_FOLDING, true);
        WRITE_CONFIG_EDITOR_RESET(CODEEDIT_SHOW_INDENTGUIDES,
                                  SETTING_ITEM::SHOW_INDENTGUIDES, true);
        WRITE_CONFIG_EDITOR_RESET(CODEEDIT_SHOW_LONGLINEEDGE,
                                  SETTING_ITEM::SHOW_LONGLINEEDGE, false);
        WRITE_CONFIG_EDITOR_RESET(CODEEDIT_SHOW_WHITESPACE,
                                  SETTING_ITEM::SHOW_WHITESPACE, false);
        WRITE_CONFIG_EDITOR_RESET(CODEEDIT_AUTO_CLOSE_CHAR,
                                  SETTING_ITEM::AUTO_CLOSE_CHAR, true);
        WRITE_CONFIG_EDITOR_RESET(CODEEDIT_AUTO_IDEN, SETTING_ITEM::AUTO_IDEN,
                                  true);
    }

    if (cat.testFlag(SETTING::CONSOLE)) {
        WRITE_CONFIG_CONSOLE_RESET(CONSOLE_FONT, SETTING_ITEM::FONT,
                                   _defaultFont.defaultFamily());
        WRITE_CONFIG_CONSOLE_RESET(CONSOLE_FONT_SIZE, SETTING_ITEM::FONT_SIZE,
                                   10);
        WRITE_CONFIG_CONSOLE_RESET(CONSOLE_THEME, SETTING_ITEM::THEME, {});
        WRITE_CONFIG_CONSOLE_RESET(CONSOLE_TABS_WIDTH, SETTING_ITEM::TAB_WIDTH,
                                   4);
        WRITE_CONFIG_CONSOLE_RESET(CONSOLE_INDENTATION,
                                   SETTING_ITEM::INDENTATION, 0);
        WRITE_CONFIG_CONSOLE_RESET(CONSOLE_MATCH_BRACES,
                                   SETTING_ITEM::MATCH_BRACES, true);
        WRITE_CONFIG_CONSOLE_RESET(CONSOLE_SHOW_WHITESPACE,
                                   SETTING_ITEM::SHOW_WHITESPACE, false);
        WRITE_CONFIG_CONSOLE_RESET(CONSOLE_AUTO_CLOSE_CHAR,
                                   SETTING_ITEM::AUTO_CLOSE_CHAR, true);
    }
}

ScriptSettings::ScriptSettings() : QObject() {
    _defaultFont = qApp->font();
    load();
}

bool ScriptSettings::editorAutoIden() const { return m_editorAutoIden; }

void ScriptSettings::setEditorAutoIden(bool newEditorAutoIden) {
    m_editorAutoIden = newEditorAutoIden;
}

bool ScriptSettings::consoleAutoCloseChar() const {
    return m_consoleAutoCloseChar;
}

void ScriptSettings::setConsoleAutoCloseChar(bool newConsoleAutoCloseChar) {
    if (m_consoleAutoCloseChar != newConsoleAutoCloseChar) {
        m_consoleAutoCloseChar = newConsoleAutoCloseChar;
        _setUnsavedConsole.setFlag(SETTING_ITEM::AUTO_CLOSE_CHAR);
    }
}

bool ScriptSettings::editorAutoCloseChar() const {
    return m_editorAutoCloseChar;
}

void ScriptSettings::setEditorAutoCloseChar(bool newEditorAutoCloseChar) {
    if (m_editorAutoCloseChar != newEditorAutoCloseChar) {
        m_editorAutoCloseChar = newEditorAutoCloseChar;
        _setUnsavedEditor.setFlag(SETTING_ITEM::AUTO_CLOSE_CHAR);
    }
}

bool ScriptSettings::consoleShowWhiteSpace() const {
    return m_consoleShowWhiteSpace;
}

void ScriptSettings::setConsoleShowWhiteSpace(bool newConsoleShowWhiteSpace) {
    if (m_consoleShowWhiteSpace != newConsoleShowWhiteSpace) {
        m_consoleShowWhiteSpace = newConsoleShowWhiteSpace;
        _setUnsavedConsole.setFlag(SETTING_ITEM::SHOW_WHITESPACE);
    }
}

bool ScriptSettings::editorShowWhiteSpace() const {
    return m_editorShowWhiteSpace;
}

void ScriptSettings::setEditorShowWhiteSpace(bool newEditorShowWhiteSpace) {
    if (m_editorShowWhiteSpace != newEditorShowWhiteSpace) {
        m_editorShowWhiteSpace = newEditorShowWhiteSpace;
        _setUnsavedEditor.setFlag(SETTING_ITEM::SHOW_WHITESPACE);
    }
}

bool ScriptSettings::editorShowLineEdges() const {
    return m_editorShowLineEdges;
}

void ScriptSettings::setEditorShowLineEdges(bool newEditorShowLineEdges) {
    if (m_editorShowLineEdges != newEditorShowLineEdges) {
        m_editorShowLineEdges = newEditorShowLineEdges;
        _setUnsavedEditor.setFlag(SETTING_ITEM::SHOW_LONGLINEEDGE);
    }
}

bool ScriptSettings::editorShowGuideLine() const {
    return m_editorShowGuideLine;
}

void ScriptSettings::setEditorShowGuideLine(bool newEditorShowGuidLine) {
    if (m_editorShowGuideLine != newEditorShowGuidLine) {
        m_editorShowGuideLine = newEditorShowGuidLine;
        _setUnsavedEditor.setFlag(SETTING_ITEM::SHOW_INDENTGUIDES);
    }
}

bool ScriptSettings::editorFolding() const { return m_editorFolding; }

void ScriptSettings::setEditorFolding(bool newEditorFolding) {
    if (m_editorFolding != newEditorFolding) {
        m_editorFolding = newEditorFolding;
        _setUnsavedEditor.setFlag(SETTING_ITEM::SHOW_FOLDING);
    }
}

bool ScriptSettings::editorShowLineNumber() const {
    return m_editorShowLineNumber;
}

void ScriptSettings::setEditorShowLineNumber(bool newEditorShowLineNumber) {
    if (m_editorShowLineNumber != newEditorShowLineNumber) {
        m_editorShowLineNumber = newEditorShowLineNumber;
        _setUnsavedEditor.setFlag(SETTING_ITEM::SHOW_LINENUMBER);
    }
}

bool ScriptSettings::editorWordWrap() const { return m_editorWordWrap; }

void ScriptSettings::setEditorWordWrap(bool newEditorWordWrap) {
    if (m_editorWordWrap != newEditorWordWrap) {
        m_editorWordWrap = newEditorWordWrap;
        _setUnsavedEditor.setFlag(SETTING_ITEM::WORD_WRAP);
    }
}

bool ScriptSettings::consoleMatchBraces() const { return m_consoleMatchBraces; }

void ScriptSettings::setConsoleMatchBraces(bool newConsoleMatchBraces) {
    if (m_consoleMatchBraces != newConsoleMatchBraces) {
        m_consoleMatchBraces = newConsoleMatchBraces;
        _setUnsavedConsole.setFlag(SETTING_ITEM::MATCH_BRACES);
    }
}

bool ScriptSettings::editorMatchBraces() const { return m_editorMatchBraces; }

void ScriptSettings::setEditorMatchBraces(bool newEditorMatchBraces) {
    if (m_editorMatchBraces != newEditorMatchBraces) {
        m_editorMatchBraces = newEditorMatchBraces;
        _setUnsavedEditor.setFlag(SETTING_ITEM::MATCH_BRACES);
    }
}

int ScriptSettings::consoleInden() const { return m_consoleInden; }

void ScriptSettings::setConsoleInden(int newConsoleInden) {
    if (m_consoleInden != newConsoleInden) {
        m_consoleInden = newConsoleInden;
        _setUnsavedConsole.setFlag(SETTING_ITEM::INDENTATION);
    }
}

int ScriptSettings::editorInden() const { return m_editorInden; }

void ScriptSettings::setEditorInden(int newEditorInden) {
    if (m_editorInden != newEditorInden) {
        m_editorInden = newEditorInden;
        _setUnsavedEditor.setFlag(SETTING_ITEM::INDENTATION);
    }
}

int ScriptSettings::consoleTabWidth() const { return m_consoleTabWidth; }

void ScriptSettings::setConsoleTabWidth(int newConsoleTabWidth) {
    if (m_consoleTabWidth != newConsoleTabWidth) {
        m_consoleTabWidth = newConsoleTabWidth;
        _setUnsavedConsole.setFlag(SETTING_ITEM::TAB_WIDTH);
    }
}

int ScriptSettings::editorTabWidth() const { return m_editorTabWidth; }

void ScriptSettings::setEditorTabWidth(int newEditorTabWidth) {
    if (m_editorTabWidth != newEditorTabWidth) {
        m_editorTabWidth = newEditorTabWidth;
        _setUnsavedEditor.setFlag(SETTING_ITEM::TAB_WIDTH);
    }
}

QString ScriptSettings::consoleTheme() const { return m_consoleTheme; }

void ScriptSettings::setConsoleTheme(const QString &newConsoleTheme) {
    if (m_consoleTheme != newConsoleTheme) {
        m_consoleTheme = newConsoleTheme;
        _setUnsavedConsole.setFlag(SETTING_ITEM::THEME);
    }
}

QString ScriptSettings::editorTheme() const { return m_editorTheme; }

void ScriptSettings::setEditorTheme(const QString &newEditorTheme) {
    if (m_editorTheme != newEditorTheme) {
        m_editorTheme = newEditorTheme;
        _setUnsavedEditor.setFlag(SETTING_ITEM::THEME);
    }
}

int ScriptSettings::consoleFontSize() const { return m_consolefontSize; }

void ScriptSettings::setConsoleFontSize(int newConsolefontSize) {
    if (m_consolefontSize != newConsolefontSize) {
        m_consolefontSize = newConsolefontSize;
        _setUnsavedConsole.setFlag(SETTING_ITEM::FONT_SIZE);
    }
}

int ScriptSettings::editorFontSize() const { return m_editorfontSize; }

void ScriptSettings::setEditorFontSize(int newEditorfontSize) {
    if (m_editorfontSize != newEditorfontSize) {
        m_editorfontSize = newEditorfontSize;
        _setUnsavedEditor.setFlag(SETTING_ITEM::FONT_SIZE);
    }
}

QString ScriptSettings::consoleFontFamily() const {
    return m_consoleFontFamily;
}

void ScriptSettings::setConsoleFontFamily(const QString &newConsoleFontFamily) {
    if (m_consoleFontFamily != newConsoleFontFamily) {
        m_consoleFontFamily = newConsoleFontFamily;
        _setUnsavedConsole.setFlag(SETTING_ITEM::FONT);
    }
}

QString ScriptSettings::editorFontFamily() const { return m_editorFontFamily; }

void ScriptSettings::setEditorFontFamily(const QString &newEditorFontFamily) {
    if (m_editorFontFamily != newEditorFontFamily) {
        m_editorFontFamily = newEditorFontFamily;
        _setUnsavedEditor.setFlag(SETTING_ITEM::FONT);
    }
}
