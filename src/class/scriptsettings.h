#ifndef SCRIPTSETTINGS_H
#define SCRIPTSETTINGS_H

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

private:
    explicit ScriptSettings();

signals:
    void editorSettingsUpdate();
    void consoleSettingUpdate();

private:
    Q_DISABLE_COPY_MOVE(ScriptSettings)
};

#endif // SCRIPTSETTINGS_H
