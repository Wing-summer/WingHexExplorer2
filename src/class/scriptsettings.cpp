#include "scriptsettings.h"

Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_FONT, ("codeedit.font"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_FONT_SIZE, ("codeedit.fontsize"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_SHOW_LEADING_WHITESPACE,
                          ("codeedit.leading_whitespace"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_SHOW_TRAILING_WHITESPACE,
                          ("codeedit.trailing_whitespace"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_SHOW_TABS, ("codeedit.show_tabs"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_TABS_WIDTH, ("codeedit.tab_width"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_REPLACE_TABS,
                          ("codeedit.replace_tabs"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_REMOVE_TRAILING,
                          ("codeedit.remove_trailing"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_PRESERVE_TRAILING_INDENT,
                          ("codeedit.preserve_trailing_indent"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CODEEDIT_LINE_ENDINGS,
                          ("codeedit.line_endings"))

ScriptSettings &ScriptSettings::instance() {
    static ScriptSettings ins;
    return ins;
}

void ScriptSettings::load() {}

void ScriptSettings::save(SETTINGS cat) {}

void ScriptSettings::reset(SETTINGS cat) {}

ScriptSettings::ScriptSettings() : QObject() {}
