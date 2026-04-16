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
#include "class/editorlspevent.h"
#include "utilities.h"

#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QPixmap>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

#include "class/angellsp.h"
#include "class/ascompletion.h"

constexpr auto SCRIPT_LIMIT = 1024 * 1024;

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
        syncSemanticTokens();
    });
    m_editor->installEventFilter(this);

    auto cm = new AsCompletion(m_editor);
    cm->setParent(m_editor);
    cm->setEnabled(AngelLsp::instance().isActive());

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

    _tokentimer = new ResettableTimer(this);
    connect(_timer, &ResettableTimer::timeoutTriggered, this,
            &ScriptEditor::syncSemanticTokens);

    m_instances.append(this);
}

ScriptEditor::~ScriptEditor() {
    auto fileName = this->fileName();
    if (!fileName.isEmpty()) {
        auto &lsp = AngelLsp::instance();
        if (lsp.isActive()) {
            lsp.closeDocument(Utilities::getUrlString(fileName));
        }
    }
    m_instances.removeOne(this);
}

QString ScriptEditor::fileName() const { return m_editor->windowFilePath(); }

const WingCodeEdit *ScriptEditor::editorPtr() const { return m_editor; }

QString ScriptEditor::lspFileNameURL() const {
    return Utilities::getUrlString(fileName());
}

bool ScriptEditor::openFile(const QString &filename) {
    auto oldFileName = this->fileName();
    auto &lsp = AngelLsp::instance();
    if (lsp.isActive() && !Utilities::isTextFile(QFileInfo(filename))) {
        if (!oldFileName.isEmpty()) {
            lsp.openDocument(lspFileNameURL(), 0, m_editor->toPlainText());
        }
        return false;
    }

    QFile f(filename);
    if (f.size() > SCRIPT_LIMIT) {
        return false;
    }

    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        return false;
    }

    auto txt = QString::fromUtf8(f.readAll());
    m_editor->blockSignals(true); // don't triiger textChanged()
    m_editor->setPlainText(txt);
    m_editor->blockSignals(false);
    m_editor->zoomReset();
    f.close();

    if (!oldFileName.isEmpty()) {
        _watcher.removePath(oldFileName);
    }

    m_editor->setWindowFilePath(filename);
    lsp.openDocument(lspFileNameURL(), 0, txt);
    _watcher.addPath(filename);
    m_editor->document()->setModified(false);
    processTitle();
    syncSemanticTokens();
    return true;
}

bool ScriptEditor::save(const QString &path) {
    auto &lsp = AngelLsp::instance();

    auto oldFileName = fileName();
    if (!oldFileName.isEmpty()) {
        _watcher.removePath(oldFileName);
        if (lsp.isActive()) {
            lsp.closeDocument(lspFileNameURL());
        }
    }
    QScopeGuard guard([this, path]() {
        if (path.isEmpty()) {
            _watcher.addPath(fileName());
        } else {
            _watcher.addPath(path);
        }
    });

    if (path.isEmpty()) {
        auto doc = m_editor->document();
        if (doc->isModified()) {
            QFile f(oldFileName);
            if (!f.open(QFile::WriteOnly | QFile::Text)) {
                return false;
            }

            auto data = m_editor->toPlainText().toUtf8();
            if (data.size() > SCRIPT_LIMIT) {
                return false;
            }
            f.write(data);
            doc->setModified(false);
        }
        return true;
    }

    QFile f(path);
    if (!f.open(QFile::WriteOnly | QFile::Text)) {
        return false;
    }
    auto data = m_editor->toPlainText().toUtf8();
    if (data.size() > SCRIPT_LIMIT) {
        return false;
    }
    f.write(data);

    m_editor->setWindowFilePath(path);
    processTitle();
    m_editor->document()->setModified(false);
    return true;
}

bool ScriptEditor::reload() {
    auto &lsp = AngelLsp::instance();
    auto fileName = this->fileName();
    if (lsp.isActive()) {
        lsp.closeDocument(Utilities::getUrlString(fileName));
    }
    return openFile(fileName);
}

void ScriptEditor::find() { m_editor->showSearchReplaceBar(true, false); }

void ScriptEditor::replace() { m_editor->showSearchReplaceBar(true, true); }

void ScriptEditor::gotoLine() { m_editor->showGotoBar(true); }

void ScriptEditor::onReconnectLsp() {
    auto &lsp = AngelLsp::instance();
    if (lsp.isActive()) {
        lsp.openDocument(lspFileNameURL(), 0, m_editor->toPlainText());
    }
    version = 1;
}

void ScriptEditor::setCompleterEnabled(bool b) {
    m_editor->completer()->setEnabled(b);
}

bool ScriptEditor::increaseVersion() {
    version++;
    if (version == 0) { // test overflow
        version = 1;
        return true;
    }
    return false;
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
    this->setIcon(b ? ICONRES("lockon") : QIcon());
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
    auto &lsp = AngelLsp::instance();
    if (lsp.isActive()) {
        auto url = lspFileNameURL();
        auto txt = m_editor->toPlainText();
        // test overflow
        if (increaseVersion()) {
            lsp.closeDocument(url);
            lsp.openDocument(url, 0, txt);
        } else {
            lsp.changeDocument(url, getVersion(), txt);
        }

        _ok = false;
        _timer->reset(300);
    }
}

void ScriptEditor::saveState(QXmlStreamWriter &Stream) const {
    Q_UNUSED(Stream);
    // do nothing
}

void ScriptEditor::syncSemanticTokens() {
    auto &lsp = AngelLsp::instance();
    if (lsp.isActive()) {
        applySemanticTokens();
        _tokentimer->reset(500);
    }
}

QVector<LSP::SemanticToken> ScriptEditor::parseSemanticTokens() {
    auto &lsp = AngelLsp::instance();
    if (lsp.isActive()) {
        auto url = lspFileNameURL();
        auto reply = lsp.requestSemanticTokensFull(url);
        return lsp.parseSemanticTokens(url, reply);
    }
    return {};
}

bool ScriptEditor::isContentLspUpdated() const { return _ok; }

LspEditorInterace::CursorPos
ScriptEditor::cursorPosition(const QTextCursor &cursor) const {
    LspEditorInterace::CursorPos pos;
    pos.blockNumber = cursor.blockNumber();
    pos.positionInBlock = cursor.positionInBlock();
    return pos;
}

LspEditorInterace::CursorPos ScriptEditor::currentPosition() const {
    auto tc = m_editor->textCursor();
    return cursorPosition(tc);
}

void ScriptEditor::showFunctionTip(
    const QList<WingSignatureTooltip::Signature> &sigs) {
    m_editor->showHelpTooltip(sigs);
}

void ScriptEditor::clearFunctionTip() { m_editor->hideHelpTooltip(); }

quint64 ScriptEditor::getVersion() const { return version; }

CodeEdit *ScriptEditor::editor() const { return m_editor; }

bool ScriptEditor::formatCode() {
    auto &lsp = AngelLsp::instance();
    if (!lsp.isActive()) {
        return false;
    }

    struct TextEdit {
        LSP::Range range;
        QString newText;

        bool isValid() const {
            auto validLoc = [](const LSP::Location &p) {
                return p.line >= 0 && p.character >= 0;
            };

            auto beforeOrEqual = [](const LSP::Location &a,
                                    const LSP::Location &b) {
                return a.line < b.line ||
                       (a.line == b.line && a.character <= b.character);
            };

            if (!validLoc(range.start) || !validLoc(range.end)) {
                return false;
            }
            if (!beforeOrEqual(range.start, range.end)) {
                return false;
            }
            if (range.start.line == range.end.line &&
                range.start.character == range.end.character &&
                newText.isEmpty()) {
                return false;
            }
            return true;
        }
    };

    auto r = lsp.requestFormat(lspFileNameURL());

    QVector<TextEdit> textEdits;
    const auto mods = r.toArray();
    for (const auto &&vj : mods) {
        QJsonValue v = vj;
        TextEdit edit;
        edit.range = AngelLsp::readLSPDocRange(v.toObject());
        edit.newText = v["newText"].toString();
        if (edit.isValid()) {
            textEdits.append(edit);
        }
    }

    QTextDocument *document = m_editor->document();
    QTextCursor cursor(document);

    std::sort(textEdits.begin(), textEdits.end(),
              [](const TextEdit &a, const TextEdit &b) {
                  if (a.range.start.line != b.range.start.line) {
                      return a.range.start.line > b.range.start.line;
                  }
                  if (a.range.start.character != b.range.start.character) {
                      return a.range.start.character > b.range.start.character;
                  }
                  if (a.range.end.line != b.range.end.line) {
                      return a.range.end.line > b.range.end.line;
                  }
                  return a.range.end.character > b.range.end.character;
              });

    cursor.beginEditBlock();
    for (const TextEdit &edit : textEdits) {
        const auto &start = edit.range.start;
        const auto &end = edit.range.end;

        QTextBlock startBlock = document->findBlockByLineNumber(start.line);
        QTextBlock endBlock = document->findBlockByLineNumber(end.line);
        if (!startBlock.isValid() || !endBlock.isValid()) {
            continue;
        }

        const qint64 startPos = startBlock.position() + start.character;
        const qint64 endPos = endBlock.position() + end.character;

        if (startPos < 0 || endPos < startPos) {
            continue;
        }

        cursor.setPosition(startPos);
        cursor.setPosition(endPos, QTextCursor::KeepAnchor);
        cursor.insertText(edit.newText);
    }
    cursor.endEditBlock();

    syncUpdate();
    return true;
}

bool ScriptEditor::isModified() const {
    return m_editor->document()->isModified();
}

bool ScriptEditor::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_editor) {
        if (EditorLspEvent::processEvent(event, this)) {
            return true;
        }
    }
    return ads::CDockWidget::eventFilter(watched, event);
}

const QList<ScriptEditor *> &ScriptEditor::instances() { return m_instances; }

bool ScriptEditor::isAllClosed() {
    return m_instances.isEmpty() ||
           std::all_of(
               m_instances.begin(), m_instances.end(),
               [](ScriptEditor *editor) -> bool { return editor->isClosed(); });
}

QIcon ScriptEditor::editorIcon() const { return ICONRES("angellsp"); }

QString ScriptEditor::infoFileName() const { return fileName(); }

QString ScriptEditor::infoTooltip() const { return fileName(); }
