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
#include "Qt-Advanced-Docking-System/src/DockWidgetTab.h"
#include "utilities.h"

#ifdef Q_OS_LINUX
#include "utilities.h"
#endif

#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QPixmap>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

#include "class/ascompletion.h"
#include "class/clangformatmanager.h"

ScriptEditor::ScriptEditor(asIScriptEngine *engine, QWidget *parent)
    : ads::CDockWidget(nullptr, QString(), parent) {
    this->setFeatures(
        CDockWidget::DockWidgetFocusable | CDockWidget::DockWidgetMovable |
        CDockWidget::DockWidgetClosable | CDockWidget::DockWidgetPinnable |
        CDockWidget::CustomCloseHandling);
    this->setFocusPolicy(Qt::StrongFocus);
    this->setObjectName(QStringLiteral("ScriptEditor"));

    m_editor = new CodeEdit(this);
    m_editor->setSyntax(
        m_editor->syntaxRepo().definitionForName("AngelScript"));

    auto cm = new AsCompletion(engine, m_editor);
    connect(cm, &AsCompletion::onFunctionTip, this,
            &ScriptEditor::onFunctionTip);

    connect(m_editor, &CodeEdit::symbolMarkLineMarginClicked, this,
            &ScriptEditor::onToggleMark);
    connect(m_editor, &CodeEdit::contentModified, this,
            [this]() { processTitle(); });

    this->setWidget(m_editor);
}

ScriptEditor::~ScriptEditor() {
    auto e = editor();
    e->document()->disconnect();
    e->disconnect();
}

QString ScriptEditor::fileName() const { return m_fileName; }

bool ScriptEditor::openFile(const QString &filename) {
    QFile f(filename);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        return false;
    }
    m_editor->setPlainText(QString::fromUtf8(f.readAll()));
    f.close();
    m_fileName = filename;
    processTitle();
    return true;
}

bool ScriptEditor::save(const QString &path) {
#ifdef Q_OS_LINUX
    auto needAdjustFile = !QFile::exists(path);
#endif

    auto &clang = ClangFormatManager::instance();
    if (clang.exists() && clang.autoFormat()) {
        formatCode();
    }

    if (path.isEmpty()) {
        QFile f(m_fileName);
        if (!f.open(QFile::WriteOnly | QFile::Text)) {
            return false;
        }
        f.write(m_editor->toPlainText().toUtf8());
        m_editor->document()->setModified(false);
        return true;
    }

    QFile f(path);
    if (!f.open(QFile::WriteOnly | QFile::Text)) {
        return false;
    }
    f.write(m_editor->toPlainText().toUtf8());

#ifdef Q_OS_LINUX
    if (Utilities::isRoot()) {
        // a trick off when root under linux OS
        // When new file created, change file's permission to 666.

        // Because you cannot open it when you use it in common user
        // after saving under root user.

        // It's a workaround and not eligent for permission system

        if (needAdjustFile) {
            if (Utilities::isFileOwnerRoot(path)) {
                Utilities::fixUpFilePermissions(path);
            }
        }
    }
#endif

    m_fileName = path;
    m_editor->document()->setModified(false);
    return true;
}

bool ScriptEditor::reload() { return openFile(m_fileName); }

void ScriptEditor::setReadOnly(bool b) {
    m_editor->setReadOnly(b);
    this->tabWidget()->setIcon(b ? ICONRES("lockon") : QIcon());
}

void ScriptEditor::processTitle() {
    QString filename = QFileInfo(m_fileName).fileName();
    if (m_editor->document()->isModified()) {
        setWindowTitle(filename.prepend(QStringLiteral("* ")));
    } else {
        setWindowTitle(filename);
    }
}

CodeEdit *ScriptEditor::editor() const { return m_editor; }

bool ScriptEditor::formatCode() {
    bool ok;
    auto e = editor();
    auto orign = e->toPlainText();
    auto fmtcodes = ClangFormatManager::instance().formatCode(orign, ok);
    if (ok && fmtcodes != orign) {
        auto cursor = e->textCursor();
        auto row = cursor.blockNumber();
        auto col = cursor.columnNumber();

        cursor.beginEditBlock();
        cursor.select(QTextCursor::Document);
        cursor.removeSelectedText();
        cursor.insertText(fmtcodes);
        cursor.endEditBlock();

        cursor = e->textCursor();
        cursor.setPosition(0);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, row);

        auto tmpcur = cursor;
        tmpcur.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
        auto len = tmpcur.position() - cursor.position();
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor,
                            qMin(len, col));
        e->setTextCursor(cursor);

        return true;
    }
    return false;
}
