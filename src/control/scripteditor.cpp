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

#include "scripteditor.h"
#include "qcodeeditwidget/qdocumentswaptextcommand.h"
#include "qeditor.h"

#include <QAction>
#include <QFile>
#include <QPixmap>

#include "qpanellayout.h"

#include "class/clangformatmanager.h"

ScriptEditor::ScriptEditor(QWidget *parent)
    : ads::CDockWidget(nullptr, QString(), parent) {
    this->setFeatures(
        CDockWidget::DockWidgetFocusable | CDockWidget::DockWidgetMovable |
        CDockWidget::DockWidgetClosable | CDockWidget::DockWidgetPinnable |
        CDockWidget::CustomCloseHandling);
    this->setFocusPolicy(Qt::StrongFocus);
    this->setObjectName(QStringLiteral("ScriptEditor"));

    m_editor = new CodeEdit(true, this);
    connect(m_editor, &CodeEdit::onToggleMark, this,
            &ScriptEditor::onToggleMark);

    auto editor = m_editor->editor();
    editor->setFlag(QEditor::AutoCloseChars, true);
    connect(editor, &QEditor::titleChanged, this, &ScriptEditor::processTitle);
    connect(editor, &QEditor::contentModified, this,
            &ScriptEditor::processTitle);

    this->setWidget(editor);
}

ScriptEditor::~ScriptEditor() {
    auto e = editor();
    e->disconnect();
    e->document()->disconnect();
}

QString ScriptEditor::fileName() const { return editor()->fileName(); }

bool ScriptEditor::openFile(const QString &filename) {
    auto e = editor();
    return e->load(filename);
}

bool ScriptEditor::save(const QString &path) {
    auto e = editor();
    auto clang = ClangFormatManager::instance();
    if (clang.exists() && clang.autoFormat()) {
        formatCode();
    }

    if (path.isEmpty()) {
        return e->save();
    }
    return e->save(path);
}

bool ScriptEditor::reload() {
    auto e = m_editor->editor();
    return e->load(e->fileName());
}

void ScriptEditor::processTitle() {
    auto e = m_editor->editor();
    if (e->isContentModified()) {
        setWindowTitle(e->windowTitle());
    } else {
        setWindowTitle(e->name());
    }
}

QEditor *ScriptEditor::editor() const { return m_editor->editor(); }

bool ScriptEditor::formatCode() {
    bool ok;
    auto e = editor();
    auto fmtcodes = ClangFormatManager::instance().formatCode(e->text(), ok);
    if (ok) {
        auto doc = e->document();
        doc->execute(new QDocumentSwapTextCommand(fmtcodes, doc));
        return true;
    }
    return false;
}
