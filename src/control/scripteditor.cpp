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
    connect(m_editor, &CodeEdit::textChanged, this, [this]() {
        if (!_ok) {
            _lastSent = false;
            return;
        }
        sendDocChange();
        _ok = false;
        _timer->reset(300);
    });
    connect(m_editor, &CodeEdit::tryShowToolTip, this,
            [this](int line, int character) {
                auto &lsp = AngelLsp::instance();
                // lsp.requestSignatureHelp(fileName(), , );
                // TODO
            });

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

    _timer = new ResettableTimer(this);
    connect(_timer, &ResettableTimer::timeoutTriggered, this,
            &ScriptEditor::onSendFullTextChangeCompleted);
}

ScriptEditor::~ScriptEditor() {
    auto fileName = this->fileName();
    if (!fileName.isEmpty()) {
        AngelLsp::instance().closeDocument(Utilities::getUrlString(fileName));
    }
}

QString ScriptEditor::fileName() const { return m_editor->windowFilePath(); }

bool ScriptEditor::openFile(const QString &filename) {
    auto &lsp = AngelLsp::instance();

    QFile f(filename);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        return false;
    }

    auto txt = QString::fromUtf8(f.readAll());
    m_editor->blockSignals(true); // don't triiger textChanged()
    m_editor->setPlainText(txt);
    m_editor->blockSignals(false);
    f.close();

    auto oldFileName = this->fileName();

    if (!oldFileName.isEmpty()) {
        _watcher.removePath(oldFileName);
    }

    m_editor->setWindowFilePath(filename);
    lsp.openDocument(Utilities::getUrlString(filename), 0, txt);
    _watcher.addPath(filename);

    processTitle();
    return true;
}

bool ScriptEditor::save(const QString &path) {
    auto &lsp = AngelLsp::instance();

    auto oldFileName = fileName();
    if (!oldFileName.isEmpty()) {
        _watcher.removePath(oldFileName);
        lsp.closeDocument(Utilities::getUrlString(oldFileName));
    }
    QScopeGuard guard([this, path]() {
        if (path.isEmpty()) {
            _watcher.addPath(fileName());
        } else {
            _watcher.addPath(path);
        }
    });

#ifdef Q_OS_LINUX
    auto needAdjustFile = !QFile::exists(path);
#endif

    if (path.isEmpty()) {
        QFile f(oldFileName);
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

    m_editor->setWindowFilePath(path);
    m_editor->document()->setModified(false);
    return true;
}

bool ScriptEditor::reload() {
    auto &lsp = AngelLsp::instance();
    auto fileName = this->fileName();
    lsp.closeDocument(Utilities::getUrlString(fileName));
    return openFile(fileName);
}

void ScriptEditor::find() { m_editor->showSearchReplaceBar(true, false); }

void ScriptEditor::replace() { m_editor->showSearchReplaceBar(true, true); }

void ScriptEditor::gotoLine() { m_editor->showGotoBar(true); }

bool ScriptEditor::increaseVersion() {
    version++;
    if (version == 0) { // test overflow
        version = 1;
        return true;
    }
    return false;
}

void ScriptEditor::onFunctionTip(LSP::CompletionItemKind kind,
                                 const QString &content) {
    if (kind != LSP::CompletionItemKind::Function) {
        return;
    }
}

void ScriptEditor::onSendFullTextChangeCompleted() {
    if (!_lastSent) {
        sendDocChange();
        _lastSent = true;
    }
    _ok = true;
}

void ScriptEditor::setReadOnly(bool b) {
    m_editor->setReadOnly(b);
    this->tabWidget()->setIcon(b ? ICONRES("lockon") : QIcon());
}

void ScriptEditor::processTitle() {
    QString filename = QFileInfo(fileName()).fileName();
    if (m_editor->document()->isModified()) {
        setWindowTitle(filename.prepend(QStringLiteral("* ")));
    } else {
        setWindowTitle(filename);
    }
}

void ScriptEditor::sendDocChange() {
    auto url = Utilities::getUrlString(fileName());
    auto &lsp = AngelLsp::instance();

    auto txt = m_editor->toPlainText();
    // test overflow
    if (increaseVersion()) {
        lsp.closeDocument(url);
        lsp.openDocument(url, 0, txt);
    } else {
        lsp.changeDocument(url, getVersion(), txt);
    }
}

bool ScriptEditor::isContentLspUpdated() const { return _ok; }

quint64 ScriptEditor::getVersion() const { return version; }

CodeEdit *ScriptEditor::editor() const { return m_editor; }

bool ScriptEditor::formatCode() {
    // TODO
    return false;
}
