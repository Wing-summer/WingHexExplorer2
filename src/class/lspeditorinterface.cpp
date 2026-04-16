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

#include "lspeditorinterface.h"

#include "appmanager.h"
#include "editorlspevent.h"

LspEditorInterace::LspEditorInterace() {}

void LspEditorInterace::syncUpdate() {
    sendDocChange();
    auto app = AppManager::instance();
    auto curTime = app->currentMSecsSinceEpoch();
    while (isContentLspUpdated()) {
        // wait for a moment
        // timeout for 100ms
        auto nowTime = app->currentMSecsSinceEpoch();
        if (nowTime - curTime > 100) {
            sendDocChange();
            curTime = nowTime;
        }
        app->processEvents();
    }
}

void LspEditorInterace::applySemanticTokens() {
    auto editor = editorPtr();
    auto selections = EditorLspEvent::semanticTokensToExtraSelections(
        editor->document(), parseSemanticTokens(),
        [this](const QString &type,
               const QStringList &mods) -> QTextCharFormat {
            KSyntaxHighlighting::Theme::TextStyle style =
                KSyntaxHighlighting::Theme::Normal;
            auto editor = editorPtr();

            QTextCharFormat tf;
            if (type == "class" || type == "enum" || type == "interface" ||
                type == "namespace") {
                style = KSyntaxHighlighting::Theme::DataType;
                tf.setFontWeight(QFont::Bold);
            } else if (type == "variable") {
                style = KSyntaxHighlighting::Theme::Variable;
            } else if (type == "function" || type == "method") {
                style = KSyntaxHighlighting::Theme::Function;
            } else if (type == "type") {
                style = KSyntaxHighlighting::Theme::DataType;
            }
            auto theme = editor->theme();

            tf.setForeground(QColor::fromRgb(theme.textColor(style)));

            if (mods.contains("definition") || theme.isBold(style)) {
                tf.setFontWeight(QFont::Bold);
            }
            if (theme.isItalic(style)) {
                tf.setFontItalic(true);
            }
            if (theme.isUnderline(style)) {
                tf.setFontUnderline(true);
            }
            if (mods.contains("deprecated") || theme.isStrikeThrough(style)) {
                tf.setFontStrikeOut(true);
            }
            return tf;
        });
    const_cast<WingCodeEdit *>(editor)->setExtraSelections(selections);
}
