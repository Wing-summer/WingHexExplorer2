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

#ifndef EDITORLSPEVENT_H
#define EDITORLSPEVENT_H

#include <QEvent>
#include <QPlainTextEdit>

#include "class/lsp.h"
#include "lspeditorinterface.h"

class EditorLspEvent {
public:
    static bool processEvent(QEvent *event, LspEditorInterace *editor);

    static int absolutePositionForLineCharacter(const QTextDocument *doc,
                                                int line, int character);

    static QList<QTextEdit::ExtraSelection> semanticTokensToExtraSelections(
        QTextDocument *doc, const QVector<LSP::SemanticToken> &tokens,
        const std::function<QTextCharFormat(const QString &tokenType,
                                            const QStringList &modifiers)>
            &formatForToken);
};

#endif // EDITORLSPEVENT_H
