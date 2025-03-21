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

#ifndef SCRIPTSETTINGS_H
#define SCRIPTSETTINGS_H

#include <QFont>
#include <QObject>

class ScriptSettings : public QObject {
    Q_OBJECT

public:
    enum SETTING { EDITOR = 1, CONSOLE = 2, ALL = EDITOR | CONSOLE };
    Q_DECLARE_FLAGS(SETTINGS, SETTING)

private:
    // Flags to indicate whether the modification has been made.
    // There are a maximum of 32 flags,
    // but it is impossible to have more than this.
    enum class SETTING_ITEM : quint32 {
        FONT = 1u,
        FONT_SIZE = 1u << 1,
        THEME = 1u << 2,
        TAB_WIDTH = 1u << 3,
        INDENTATION = 1u << 4,
        WORD_WRAP = 1u << 5,
        MATCH_BRACES = 1u << 6,
        SHOW_LINENUMBER = 1u << 7,
        SHOW_FOLDING = 1u << 8,
        SHOW_INDENTGUIDES = 1u << 9,
        SHOW_LONGLINEEDGE = 1u << 10,
        SHOW_WHITESPACE = 1u << 11,
        AUTO_CLOSE_CHAR = 1u << 12
    };
    Q_DECLARE_FLAGS(SETTING_ITEMS, SETTING_ITEM)

public:
    static ScriptSettings &instance();

    void load();
    void save(SETTINGS cat = SETTING::ALL);
    void reset(SETTINGS cat = SETTING::ALL);

public:
    QString editorFontFamily() const;
    void setEditorFontFamily(const QString &newEditorFontFamily);

    QString consoleFontFamily() const;
    void setConsoleFontFamily(const QString &newConsoleFontFamily);

    int editorFontSize() const;
    void setEditorFontSize(int newEditorfontSize);

    int consoleFontSize() const;
    void setConsoleFontSize(int newConsolefontSize);

    QString editorTheme() const;
    void setEditorTheme(const QString &newEditorTheme);

    QString consoleTheme() const;
    void setConsoleTheme(const QString &newConsoleTheme);

    int editorTabWidth() const;
    void setEditorTabWidth(int newEditorTabWidth);

    int consoleTabWidth() const;
    void setConsoleTabWidth(int newConsoleTabWidth);

    int editorInden() const;
    void setEditorInden(int newEditorInden);

    int consoleInden() const;
    void setConsoleInden(int newConsoleInden);

    bool editorMatchBraces() const;
    void setEditorMatchBraces(bool newEditorMatchBraces);

    bool consoleMatchBraces() const;
    void setConsoleMatchBraces(bool newConsoleMatchBraces);

    bool editorWordWrap() const;
    void setEditorWordWrap(bool newEditorWordWrap);

    bool editorShowLineNumber() const;
    void setEditorShowLineNumber(bool newEditorShowLineNumber);

    bool editorFolding() const;
    void setEditorFolding(bool newEditorFolding);

    bool editorShowGuideLine() const;
    void setEditorShowGuideLine(bool newEditorShowGuidLine);

    bool editorShowLineEdges() const;
    void setEditorShowLineEdges(bool newEditorShowLineEdges);

    bool editorShowWhiteSpace() const;
    void setEditorShowWhiteSpace(bool newEditorShowWhiteSpace);

    bool consoleShowWhiteSpace() const;
    void setConsoleShowWhiteSpace(bool newConsoleShowWhiteSpace);

    bool editorAutoCloseChar() const;
    void setEditorAutoCloseChar(bool newEditorAutoCloseChar);

    bool consoleAutoCloseChar() const;
    void setConsoleAutoCloseChar(bool newConsoleAutoCloseChar);

private:
    explicit ScriptSettings();

signals:
    void editorSettingsUpdate();
    void consoleSettingUpdate();

private:
    QFont _defaultFont;

    QString m_editorFontFamily;
    QString m_consoleFontFamily;

    int m_editorfontSize = 10;
    int m_consolefontSize = 10;

    QString m_editorTheme;
    QString m_consoleTheme;

    int m_editorTabWidth = 4;
    int m_consoleTabWidth = 4;

    int m_editorInden = 0;
    int m_consoleInden = 0;

    bool m_editorMatchBraces = true;
    bool m_consoleMatchBraces = true;

    bool m_editorWordWrap = false;
    bool m_editorShowLineNumber = true;
    bool m_editorFolding = true;
    bool m_editorShowGuideLine = true;
    bool m_editorShowLineEdges = false;

    bool m_editorShowWhiteSpace = false;
    bool m_consoleShowWhiteSpace = false;

    bool m_editorAutoCloseChar = true;
    bool m_consoleAutoCloseChar = true;

private:
    SETTING_ITEMS _setUnsavedEditor;
    SETTING_ITEMS _setUnsavedConsole;

private:
    Q_DISABLE_COPY_MOVE(ScriptSettings)
};

#endif // SCRIPTSETTINGS_H
