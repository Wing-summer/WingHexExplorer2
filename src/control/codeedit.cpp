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

#include "codeedit.h"
#include "class/scriptsettings.h"
#include "class/skinmanager.h"
#include "model/codecompletionmodel.h"

#include <QApplication>
#include <QModelIndex>
#include <QShortcut>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

CodeEdit::CodeEdit(QWidget *parent) : WingCodeEdit(parent) {
    connect(this->document(), &QTextDocument::modificationChanged, this,
            &CodeEdit::contentModified);
    addMoveLineShortCut();

    connect(&ScriptSettings::instance(), &ScriptSettings::editorSettingsUpdate,
            this, &CodeEdit::applyEditorSetStyle);
    applyEditorSetStyle();

    m_searchWidget = new SearchWidget(this, parent);
    showSearchBar(false);

    connect(this, &CodeEdit::textChanged, this, [this] {
        if (m_searchWidget->isVisible())
            showSearchBar(false);
    });
}

void CodeEdit::showSearchBar(bool show) {
    m_searchWidget->setVisible(show);
    m_searchWidget->setEnabled(show);
    if (show) {
        const QTextCursor cursor = textCursor();
        if (cursor.hasSelection())
            m_searchWidget->setSearchText(cursor.selectedText());
        m_searchWidget->activate();
    } else {
        clearLiveSearch();
        setFocus(Qt::OtherFocusReason);
    }
}

void CodeEdit::onCompletion(const QModelIndex &index) {
    WingCodeEdit::onCompletion(index);
    auto selfdata = index.data(Qt::SelfDataRole).value<CodeInfoTip>();
    if (selfdata.type == CodeInfoTip::Type::Function ||
        selfdata.type == CodeInfoTip::Type::ClsFunction) {
        auto args = selfdata.addinfo.value(CodeInfoTip::Args);
        auto cursor = textCursor();
        cursor.insertText(QStringLiteral("()"));
        if (!args.isEmpty()) {
            cursor.movePosition(QTextCursor::Left);
            setTextCursor(cursor);
        }
    }
}

void CodeEdit::addMoveLineShortCut() {
    auto upLines = new QShortcut(
        QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_Up), this);
    upLines->setContext(Qt::WidgetShortcut);
    connect(upLines, &QShortcut::activated, this,
            [this]() { moveLines(QTextCursor::PreviousBlock); });
    auto downLines = new QShortcut(
        QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_Down),
        this);
    downLines->setContext(Qt::WidgetShortcut);
    connect(downLines, &QShortcut::activated, this,
            [this]() { moveLines(QTextCursor::NextBlock); });
}

void CodeEdit::applyEditorSetStyle() {
    auto &set = ScriptSettings::instance();
    auto dfont = QFont(set.editorFontFamily());
    dfont.setPointSize(set.editorFontSize());

    auto thname = set.editorTheme();
    if (thname.isEmpty()) {
        switch (SkinManager::instance().currentTheme()) {
        case SkinManager::Theme::Dark:
            this->setTheme(syntaxRepo().defaultTheme(
                KSyntaxHighlighting::Repository::DarkTheme));
            break;
        case SkinManager::Theme::Light:
            this->setTheme(syntaxRepo().defaultTheme(
                KSyntaxHighlighting::Repository::LightTheme));
            break;
        }
    } else {
        this->setTheme(syntaxRepo().theme(thname));
    }

    this->setFont(dfont);
    this->setTabWidth(set.editorTabWidth());
    this->setIndentationMode(WingCodeEdit::IndentationMode(set.editorInden()));
    this->setShowLineNumbers(set.editorShowLineNumber());
    this->setShowFolding(set.editorFolding());
    this->setShowIndentGuides(set.editorShowGuideLine());
    this->setWordWrap(set.editorWordWrap());
    this->setShowLongLineEdge(set.editorShowLineEdges());
    this->setMatchBraces(set.editorMatchBraces());
    this->setShowWhitespace(set.editorShowWhiteSpace());
    this->setAutoCloseChar(set.editorAutoCloseChar());
}

void CodeEdit::resizeEvent(QResizeEvent *event) {
    if (event)
        WingCodeEdit::resizeEvent(event);

    // Move the search widget to the upper-right corner
    const QPoint editorPos = this->pos();
    QSize searchSize = m_searchWidget->sizeHint();
    m_searchWidget->resize(searchSize);
    m_searchWidget->move(editorPos.x() + this->viewport()->width() -
                             searchSize.width() - 16,
                         editorPos.y());
}
