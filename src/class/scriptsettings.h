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

public:
    static ScriptSettings &instance();

    void load();
    void reset(SETTINGS cat = SETTING::ALL);

    void __reset(SETTINGS cat);

public:
    QString editorFontFamily() const;
    QString consoleFontFamily() const;
    int editorFontSize() const;
    int consoleFontSize() const;
    QString editorTheme() const;
    QString consoleTheme() const;
    int editorTabWidth() const;
    int consoleTabWidth() const;
    int editorInden() const;
    int consoleInden() const;
    bool editorMatchBraces() const;
    bool consoleMatchBraces() const;
    bool editorWordWrap() const;
    bool editorShowLineNumber() const;
    bool editorFolding() const;
    bool editorShowGuideLine() const;
    bool editorShowLineEdges() const;
    bool editorShowWhiteSpace() const;
    bool consoleShowWhiteSpace() const;
    bool editorAutoCloseChar() const;
    bool consoleAutoCloseChar() const;
    bool editorAutoIden() const;

public slots:
    void setEditorFontFamily(const QString &newEditorFontFamily);
    void setConsoleFontFamily(const QString &newConsoleFontFamily);
    void setEditorFontSize(int newEditorfontSize);
    void setConsoleFontSize(int newConsolefontSize);
    void setEditorTheme(const QString &newEditorTheme);
    void setConsoleTheme(const QString &newConsoleTheme);
    void setEditorTabWidth(int newEditorTabWidth);
    void setConsoleTabWidth(int newConsoleTabWidth);
    void setEditorInden(int newEditorInden);
    void setConsoleInden(int newConsoleInden);
    void setEditorMatchBraces(bool newEditorMatchBraces);
    void setConsoleMatchBraces(bool newConsoleMatchBraces);
    void setEditorWordWrap(bool newEditorWordWrap);
    void setEditorShowLineNumber(bool newEditorShowLineNumber);
    void setEditorFolding(bool newEditorFolding);
    void setEditorShowGuideLine(bool newEditorShowGuidLine);
    void setEditorShowLineEdges(bool newEditorShowLineEdges);
    void setEditorShowWhiteSpace(bool newEditorShowWhiteSpace);
    void setConsoleShowWhiteSpace(bool newConsoleShowWhiteSpace);
    void setEditorAutoCloseChar(bool newEditorAutoCloseChar);
    void setConsoleAutoCloseChar(bool newConsoleAutoCloseChar);
    void setEditorAutoIden(bool newEditorAutoIden);

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
    bool m_editorAutoIden = true;

    bool m_editorShowWhiteSpace = false;
    bool m_consoleShowWhiteSpace = false;

    bool m_editorAutoCloseChar = true;
    bool m_consoleAutoCloseChar = true;

private:
    Q_DISABLE_COPY_MOVE(ScriptSettings)
};

#endif // SCRIPTSETTINGS_H
