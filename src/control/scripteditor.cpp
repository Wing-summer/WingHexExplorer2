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

#include "scripteditor.h"

#include "Qt-Advanced-Docking-System/src/DockWidgetTab.h"
#include "class/editorlspevent.h"
#include "class/luaucompletion.h"
#include "class/scriptsettings.h"
#include "luau/lsp/luaulanguageserver.h"
#include "luau/luauformatter.h"
#include "utilities.h"

#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QPixmap>
#include <QScrollBar>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

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
    m_editor->setSyntax(m_editor->syntaxRepo().definitionForName("Luau"));
    connect(m_editor, &CodeEdit::textChanged, this, [this]() {
        syncDocChange();
        syncSemanticTokens();
    });
    m_editor->installEventFilter(this);

    auto completer = new LuauCompletion(this, m_editor);
    completer->setParent(m_editor);
    completer->setEnabled(true);

    connect(m_editor, &CodeEdit::symbolMarkLineMarginClicked, this,
            &ScriptEditor::onToggleMark);
    connect(
        m_editor, &CodeEdit::navigationRequested, this,
        [this](bool typeDefinition) {
            auto fileName = this->fileName();
            if (fileName.isEmpty()) {
                return;
            }

            const auto position = currentPosition();
            if (position.blockNumber < 0 || position.positionInBlock < 0) {
                return;
            }

            const lsp::Position lspPosition{uint(position.blockNumber),
                                            uint(position.positionInBlock)};
            auto &lsp = LuauLanguageServer::instance();
            const auto uri = lspFileNameURL();
            if (typeDefinition) {
                auto location =
                    lsp.gotoTypeDefinition({uri, lspPosition}, nullptr);
                if (location) {
                    Q_EMIT navigateToLocation(location->uri.toLocalFile(),
                                              location->range.start.line,
                                              location->range.start.character);
                }
            } else {
                const auto locations =
                    lsp.gotoDefinition({uri, lspPosition}, nullptr);
                if (!locations.empty()) {
                    const auto &location = locations.front();
                    Q_EMIT navigateToLocation(location.uri.toLocalFile(),
                                              location.range.start.line,
                                              location.range.start.character);
                }
            }
        });
    connect(m_editor, &CodeEdit::contentModified, this,
            [this]() { processTitle(); });

    connect(&_watcher, &QFileSystemWatcher::fileChanged, this,
            &ScriptEditor::need2Reload);

    this->setWidget(m_editor);

    _tokentimer = new ResettableTimer(this);
    connect(_tokentimer, &ResettableTimer::timeoutTriggered, this,
            &ScriptEditor::syncSemanticTokens);

    m_instances.append(this);
}

ScriptEditor::~ScriptEditor() {
    auto url = this->fileName();
    if (!url.isEmpty()) {
        auto &lsp = LuauLanguageServer::instance();
        lsp.onCloseDocument(url);
    }
    m_instances.removeOne(this);
}

QString ScriptEditor::fileName() const { return m_editor->windowFilePath(); }

const WingCodeEdit *ScriptEditor::editorPtr() const { return m_editor; }

lsp::DocumentUri ScriptEditor::lspFileNameURL() const {
    return QUrl::fromLocalFile(fileName());
}

bool ScriptEditor::openFile(const QString &filename) {
    auto oldFileName = this->fileName();
    auto &lsp = LuauLanguageServer::instance();
    if (!Utilities::isTextFile(QFileInfo(filename))) {
        if (!oldFileName.isEmpty()) {
            lsp.onOpenDocument(QUrl::fromLocalFile(oldFileName),
                               m_editor->document());
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
    lsp.onOpenDocument(QUrl::fromLocalFile(filename), m_editor->document());
    _watcher.addPath(filename);
    m_editor->document()->setModified(false);
    processTitle();
    syncSemanticTokens();
    return true;
}

bool ScriptEditor::save(const QString &path) {
    auto &lsp = LuauLanguageServer::instance();
    auto oldFileName = fileName();
    if (!oldFileName.isEmpty()) {
        _watcher.removePath(oldFileName);
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
            if (f.write(data) != data.size()) {
                return false;
            }
            doc->setModified(false);
        }

        lsp.onSaveDocument(QUrl::fromLocalFile(oldFileName));
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
    if (f.write(data) != data.size()) {
        return false;
    }

    m_editor->setWindowFilePath(path);
    auto newUrl = QUrl::fromLocalFile(path);
    if (oldFileName != path) {
        _watcher.removePath(oldFileName);
        lsp.onCloseDocument(QUrl::fromLocalFile(oldFileName));
        lsp.onOpenDocument(newUrl, m_editor->document());
    }
    lsp.onSaveDocument(newUrl);
    processTitle();
    m_editor->document()->setModified(false);
    return true;
}

bool ScriptEditor::reload() {
    auto &lsp = LuauLanguageServer::instance();
    auto fileName = this->fileName();
    lsp.onCloseDocument(QUrl::fromLocalFile(fileName));
    return openFile(fileName);
}

void ScriptEditor::find() { m_editor->showSearchReplaceBar(true, false); }

void ScriptEditor::replace() { m_editor->showSearchReplaceBar(true, true); }

void ScriptEditor::gotoLine() { m_editor->showGotoBar(true); }

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

void ScriptEditor::syncDocChange() {
    LuauLanguageServer::instance().onUpdateDocument(lspFileNameURL(),
                                                    m_editor->document());
}

void ScriptEditor::saveState(QXmlStreamWriter &Stream) const {
    Q_UNUSED(Stream);
    // do nothing
}

void ScriptEditor::syncSemanticTokens() {
    applySemanticTokens();
    _tokentimer->reset(500);
}

QVector<lsp::SemanticToken> ScriptEditor::parseSemanticTokens() {
    const auto serverTokens = LuauLanguageServer::instance().semanticTokens(
        lsp::SemanticTokensParams{lspFileNameURL()}, nullptr);
    QVector<lsp::SemanticToken> tokens;
    tokens.reserve(serverTokens.size());

    auto typeName = [](lsp::SemanticTokenTypes type) -> QString {
        switch (type) {
        case lsp::SemanticTokenTypes::Namespace:
            return QStringLiteral("namespace");
        case lsp::SemanticTokenTypes::Type:
            return QStringLiteral("type");
        case lsp::SemanticTokenTypes::Class:
            return QStringLiteral("class");
        case lsp::SemanticTokenTypes::Enum:
            return QStringLiteral("enum");
        case lsp::SemanticTokenTypes::EnumMember:
            return QStringLiteral("enumMember");
        case lsp::SemanticTokenTypes::Variable:
            return QStringLiteral("variable");
        case lsp::SemanticTokenTypes::Property:
            return QStringLiteral("property");
        case lsp::SemanticTokenTypes::Function:
            return QStringLiteral("function");
        case lsp::SemanticTokenTypes::Method:
            return QStringLiteral("method");
        case lsp::SemanticTokenTypes::Parameter:
            return QStringLiteral("parameter");
        case lsp::SemanticTokenTypes::Event:
            return QStringLiteral("event");
        }
        return QStringLiteral("variable");
    };

    for (const auto &token : serverTokens) {
        if (token.start.line != token.end.line ||
            token.end.column <= token.start.column)
            continue;

        QStringList modifiers;
        if ((token.tokenModifiers &
             lsp::SemanticTokenModifiers::DefaultLibrary) !=
            lsp::SemanticTokenModifiers::None)
            modifiers.append(QStringLiteral("defaultLibrary"));
        if ((token.tokenModifiers & lsp::SemanticTokenModifiers::Readonly) !=
            lsp::SemanticTokenModifiers::None)
            modifiers.append(QStringLiteral("readonly"));

        tokens.append({int(token.start.line), int(token.start.column),
                       int(token.end.column - token.start.column),
                       typeName(token.tokenType), modifiers});
    }
    return tokens;
}

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

CodeEdit *ScriptEditor::editor() const { return m_editor; }

bool ScriptEditor::formatCode() {
    auto &s = ScriptSettings::instance();

    LuauFormat::FormatOptions options;
    options.indentSize = s.fmtIndentSpace();
    options.useTabs = s.fmtUseTabIndent();
    options.preserveBlockNewlineGaps = s.fmtKeepNewLineGap();
    options.quoteStyle = LuauFormat::QuoteStyle(s.fmtStrQuoteStyle());

    auto tc = m_editor->textCursor();
    auto c = LuauFormat::formatWithCursor(m_editor->toPlainText(), tc.anchor(),
                                          tc.position(), options);
    if (!c) {
        return false;
    }

    tc.beginEditBlock();
    tc.select(QTextCursor::Document);
    tc.insertText(c->formatted);
    tc.endEditBlock();

    tc.clearSelection();
    tc.setPosition(c->cursorAnchor);
    tc.setPosition(c->cursorPosition, QTextCursor::KeepAnchor);
    m_editor->setTextCursor(tc);

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
        if (event->type() == QEvent::FocusOut) {
            m_editor->hideHelpTooltip();
        }
    }
    return ads::CDockWidget::eventFilter(watched, event);
}

bool ScriptEditor::reloadLater() const { return _reloadLater; }

void ScriptEditor::setReloadLater(bool newReloadLater) {
    _reloadLater = newReloadLater;
}

const QList<ScriptEditor *> &ScriptEditor::instances() { return m_instances; }

bool ScriptEditor::isAllClosed() {
    return m_instances.isEmpty() ||
           std::all_of(
               m_instances.begin(), m_instances.end(),
               [](ScriptEditor *editor) -> bool { return editor->isClosed(); });
}

void ScriptEditor::scrollView(const QPoint &p) {
    m_editor->horizontalScrollBar()->setValue(p.x());
    m_editor->verticalScrollBar()->setValue(p.y());
}

QPoint ScriptEditor::scrollViewValue() const {
    return {m_editor->horizontalScrollBar()->value(),
            m_editor->verticalScrollBar()->value()};
}

void ScriptEditor::setCursorPos(const QPair<qsizetype, qsizetype> &p) {
    auto [line, col] = p;
    if (line < 0 || line >= m_editor->blockCount()) {
        return;
    }

    auto doc = m_editor->document();
    auto block = doc->findBlockByLineNumber(line);
    if (col < 0 || col >= block.length() - 1) {
        return;
    }

    QTextCursor cursor(block);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                        col);
    m_editor->setTextCursor(cursor);
}

QPair<qsizetype, qsizetype> ScriptEditor::cursorPosValue() const {
    auto pos = currentPosition();
    return {pos.blockNumber, pos.positionInBlock};
}

QIcon ScriptEditor::editorIcon() const { return ICONRES("angellsp"); }

QString ScriptEditor::infoFileName() const { return fileName(); }

QString ScriptEditor::infoTooltip() const { return fileName(); }

void ScriptEditor::focusInEvent(QFocusEvent *event) {
    m_editor->setFocus(event->reason());
    ads::CDockWidget::focusInEvent(event);
}