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

#include "class/angellsp.h"
#include "class/ascompletion.h"

ScriptEditor::ScriptEditor(QWidget *parent)
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
    connect(m_editor, &CodeEdit::incrementalDidChange, this,
            &ScriptEditor::processContentsChange);

    auto cm = new AsCompletion(m_editor);
    connect(cm, &AsCompletion::onFunctionTip, this,
            &ScriptEditor::onFunctionTip);

    connect(m_editor, &CodeEdit::symbolMarkLineMarginClicked, this,
            &ScriptEditor::onToggleMark);
    connect(m_editor, &CodeEdit::contentModified, this,
            [this]() { processTitle(); });

    connect(&_watcher, &QFileSystemWatcher::fileChanged, this,
            &ScriptEditor::need2Reload);

    this->setWidget(m_editor);
}

ScriptEditor::~ScriptEditor() {
    if (!m_fileName.isEmpty()) {
        AngelLsp::instance().closeDocument(Utilities::getUrlString(m_fileName));
    }

    auto e = editor();
    e->document()->disconnect();
    e->disconnect();
}

QString ScriptEditor::fileName() const { return m_fileName; }

bool ScriptEditor::openFile(const QString &filename) {
    auto &lsp = AngelLsp::instance();

    QFile f(filename);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        return false;
    }

    auto txt = QString::fromUtf8(f.readAll());
    m_editor->setPlainText(txt);
    f.close();

    if (!m_fileName.isEmpty()) {
        _watcher.removePath(m_fileName);
    }

    m_fileName = filename;
    lsp.openDocument(Utilities::getUrlString(m_fileName), 0, txt);
    _watcher.addPath(m_fileName);

    processTitle();
    return true;
}

bool ScriptEditor::save(const QString &path) {
    auto &lsp = AngelLsp::instance();

    if (!m_fileName.isEmpty()) {
        _watcher.removePath(m_fileName);
        lsp.closeDocument(Utilities::getUrlString(m_fileName));
    }
    QScopeGuard guard([this, path]() {
        if (path.isEmpty()) {
            _watcher.addPath(m_fileName);
        } else {
            _watcher.addPath(path);
        }
    });

#ifdef Q_OS_LINUX
    auto needAdjustFile = !QFile::exists(path);
#endif

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

void ScriptEditor::find() { m_editor->showSearchReplaceBar(true, false); }

void ScriptEditor::replace() { m_editor->showSearchReplaceBar(true, true); }

void ScriptEditor::gotoLine() { m_editor->showGotoBar(true); }

void ScriptEditor::processContentsChange(
    const LSP::TextDocumentContentChangeEvent &e) {
    if (m_fileName.isEmpty()) {
        return;
    }
    auto url = Utilities::getUrlString(m_fileName);
    auto &lsp = AngelLsp::instance();
    auto txt = m_editor->toPlainText();
    version++;
    if (version < 0) { // test overflow
        lsp.closeDocument(url);
        lsp.openDocument(url, 0, txt);
        version = 0;
    } else {
        lsp.changeDocument(url, version, e);
    }
}

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
    // TODO
    return false;
}
