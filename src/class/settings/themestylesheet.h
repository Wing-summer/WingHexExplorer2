/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#ifndef THEMESTYLESHEET_H
#define THEMESTYLESHEET_H

#include <QFile>
#include <QHash>
#include <QList>
#include <QPair>
#include <QRegularExpression>
#include <QString>
#include <QStyle>

namespace ThemeStyleSheet {
inline qsizetype skipQuoted(const QString &text, qsizetype position) {
    const auto quote = text[position++];
    while (position < text.size()) {
        const auto ch = text[position++];
        if (ch == QLatin1Char('\\') && position < text.size()) {
            ++position;
        } else if (ch == quote) {
            break;
        }
    }
    return position;
}

// Keep offsets intact so only recognized declarations are removed from the
// original sheet. Comments and quoted braces must not create apparent rules.
inline QString maskComments(const QString &sheet) {
    QString result = sheet;
    qsizetype i = 0;
    while (i < sheet.size()) {
        if (sheet[i] == QLatin1Char('"') || sheet[i] == QLatin1Char('\'')) {
            i = skipQuoted(sheet, i);
        } else if (sheet[i] == QLatin1Char('/') && i + 1 < sheet.size() &&
                   sheet[i + 1] == QLatin1Char('*')) {
            const auto end = sheet.indexOf(QStringLiteral("*/"), i + 2);
            const auto stop = end < 0 ? sheet.size() : end + 2;
            while (i < stop) {
                result[i++] = QLatin1Char(' ');
            }
        } else {
            ++i;
        }
    }
    return result;
}

inline int standardIconRole(const QString &property) {
    struct Entry {
        const char *property;
        QStyle::StandardPixmap role;
    };
    // Qt property names are stable; resource paths remain solely in the QSS.
    static constexpr Entry entries[] = {
        {"dialog-cancel-icon", QStyle::SP_DialogCancelButton},
        {"dialog-close-icon", QStyle::SP_DialogCloseButton},
        {"dialog-ok-icon", QStyle::SP_DialogOkButton},
        {"dialog-open-icon", QStyle::SP_DialogOpenButton},
        {"dialog-reset-icon", QStyle::SP_DialogResetButton},
        {"dialog-save-icon", QStyle::SP_DialogSaveButton},
        {"dialog-yes-icon", QStyle::SP_DialogYesButton},
        {"dialog-help-icon", QStyle::SP_DialogHelpButton},
        {"dialog-no-icon", QStyle::SP_DialogNoButton},
        {"dialog-apply-icon", QStyle::SP_DialogApplyButton},
        {"dialog-discard-icon", QStyle::SP_DialogDiscardButton},
        {"filedialog-backward-icon", QStyle::SP_FileDialogBack},
        {"filedialog-contentsview-icon", QStyle::SP_FileDialogContentsView},
        {"filedialog-detailedview-icon", QStyle::SP_FileDialogDetailedView},
        {"filedialog-end-icon", QStyle::SP_FileDialogEnd},
        {"filedialog-infoview-icon", QStyle::SP_FileDialogInfoView},
        {"filedialog-listview-icon", QStyle::SP_FileDialogListView},
        {"filedialog-new-directory-icon", QStyle::SP_FileDialogNewFolder},
        {"filedialog-parent-directory-icon", QStyle::SP_FileDialogToParent},
        {"filedialog-start-icon", QStyle::SP_FileDialogStart},
        {"directory-closed-icon", QStyle::SP_DirClosedIcon},
        {"directory-icon", QStyle::SP_DirIcon},
        {"directory-link-icon", QStyle::SP_DirLinkIcon},
        {"directory-open-icon", QStyle::SP_DirOpenIcon},
        {"file-icon", QStyle::SP_FileIcon},
        {"file-link-icon", QStyle::SP_FileLinkIcon},
        {"home-icon", QStyle::SP_DirHomeIcon},
        {"messagebox-critical-icon", QStyle::SP_MessageBoxCritical},
        {"messagebox-information-icon", QStyle::SP_MessageBoxInformation},
        {"messagebox-question-icon", QStyle::SP_MessageBoxQuestion},
        {"messagebox-warning-icon", QStyle::SP_MessageBoxWarning},
        {"computer-icon", QStyle::SP_ComputerIcon},
        {"desktop-icon", QStyle::SP_DesktopIcon},
        {"cd-icon", QStyle::SP_DriveCDIcon},
        {"dvd-icon", QStyle::SP_DriveDVDIcon},
        {"floppy-icon", QStyle::SP_DriveFDIcon},
        {"harddisk-icon", QStyle::SP_DriveHDIcon},
        {"network-icon", QStyle::SP_DriveNetIcon},
        {"trash-icon", QStyle::SP_TrashIcon},
        {"uparrow-icon", QStyle::SP_ArrowUp},
        {"downarrow-icon", QStyle::SP_ArrowDown},
        {"leftarrow-icon", QStyle::SP_ArrowLeft},
        {"rightarrow-icon", QStyle::SP_ArrowRight},
        {"backward-icon", QStyle::SP_ArrowBack},
        {"forward-icon", QStyle::SP_ArrowForward},
        {"titlebar-close-icon", QStyle::SP_TitleBarCloseButton},
        {"titlebar-contexthelp-icon", QStyle::SP_TitleBarContextHelpButton},
        {"titlebar-maximize-icon", QStyle::SP_TitleBarMaxButton},
        {"titlebar-menu-icon", QStyle::SP_TitleBarMenuButton},
        {"titlebar-minimize-icon", QStyle::SP_TitleBarMinButton},
        {"titlebar-normal-icon", QStyle::SP_TitleBarNormalButton},
        {"titlebar-shade-icon", QStyle::SP_TitleBarShadeButton},
        {"titlebar-unshade-icon", QStyle::SP_TitleBarUnshadeButton},
        {"dockwidget-close-icon", QStyle::SP_DockWidgetCloseButton},
    };
    for (const auto &entry : entries) {
        if (property == QLatin1String(entry.property)) {
            return int(entry.role);
        }
    }
    return -1;
}

// Optimize only unconditional QWidget defaults with simple existing resource
// URLs. All scoped/state rules, unknown properties and complex values stay in
// the sheet for Qt's own cascade and parser to handle.
inline QHash<int, QString> extractStandardIcons(QString &sheet) {
    const auto masked = maskComments(sheet);
    static const QRegularExpression declaration(QStringLiteral(
        R"qss(^\s*([a-z-]+-icon)\s*:\s*url\(\s*(?:"([^"\\]*)"|'([^'\\]*)'|([^\s()'"\\]+))\s*\)\s*$)qss"));
    QHash<int, QString> icons;
    QList<QPair<qsizetype, qsizetype>> removals;
    qsizetype ruleStart = 0;
    qsizetype statementStart = 0;
    int braces = 0;
    int parentheses = 0;
    bool defaults = false;
    qsizetype i = 0;
    while (i < masked.size()) {
        const auto ch = masked[i];
        if (ch == QLatin1Char('"') || ch == QLatin1Char('\'')) {
            i = skipQuoted(masked, i);
            continue;
        }
        if (ch == QLatin1Char('{')) {
            if (braces == 0) {
                defaults = masked.mid(ruleStart, i - ruleStart).trimmed() ==
                           QStringLiteral("QWidget");
                statementStart = i + 1;
                parentheses = 0;
            } else {
                defaults = false;
            }
            ++braces;
        } else if (ch == QLatin1Char('}')) {
            if (braces > 0 && --braces == 0) {
                ruleStart = i + 1;
                defaults = false;
            }
        } else if (defaults && braces == 1) {
            if (ch == QLatin1Char('(')) {
                ++parentheses;
            } else if (ch == QLatin1Char(')')) {
                --parentheses;
            } else if (ch == QLatin1Char(';') && parentheses == 0) {
                const auto match = declaration.match(
                    masked.mid(statementStart, i - statementStart));
                if (match.hasMatch()) {
                    const auto role = standardIconRole(match.captured(1));
                    const auto path =
                        !match.captured(2).isNull()   ? match.captured(2)
                        : !match.captured(3).isNull() ? match.captured(3)
                                                      : match.captured(4);
                    if (role >= 0 && path.startsWith(QStringLiteral(":/")) &&
                        QFile::exists(path)) {
                        icons.insert(role, path);
                        removals.append(
                            {statementStart, i + 1 - statementStart});
                    }
                }
                statementStart = i + 1;
            }
        }
        ++i;
    }
    for (auto it = removals.crbegin(); it != removals.crend(); ++it) {
        sheet.remove(it->first, it->second);
    }
    return icons;
}
} // namespace ThemeStyleSheet

#endif // THEMESTYLESHEET_H
