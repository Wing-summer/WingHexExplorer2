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
}

ScriptEditor::~ScriptEditor() {
    auto fileName = this->fileName();
    if (!fileName.isEmpty()) {
        AngelLsp::instance().closeDocument(Utilities::getUrlString(fileName));
    }
}

QString ScriptEditor::fileName() const { return m_editor->windowFilePath(); }

const WingCodeEdit *ScriptEditor::editorPtr() const { return m_editor; }

QString ScriptEditor::lspFileNameURL() const {
    return Utilities::getUrlString(fileName());
}

bool ScriptEditor::openFile(const QString &filename) {
    auto oldFileName = this->fileName();
    auto &lsp = AngelLsp::instance();
    if (!Utilities::isTextFile(QFileInfo(filename))) {
        if (!oldFileName.isEmpty()) {
            lsp.openDocument(lspFileNameURL(), 0, m_editor->toPlainText());
        }
        return false;
    }

    QFile f(filename);
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
    return true;
}

bool ScriptEditor::save(const QString &path) {
    auto &lsp = AngelLsp::instance();

    auto oldFileName = fileName();
    if (!oldFileName.isEmpty()) {
        _watcher.removePath(oldFileName);
        lsp.closeDocument(lspFileNameURL());
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
            f.write(m_editor->toPlainText().toUtf8());
            doc->setModified(false);
        }
        return true;
    }

    QFile f(path);
    if (!f.open(QFile::WriteOnly | QFile::Text)) {
        return false;
    }
    f.write(m_editor->toPlainText().toUtf8());

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
    lsp.openDocument(lspFileNameURL(), 0, m_editor->toPlainText());
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

bool ScriptEditor::isContentLspUpdated() const { return _ok; }

LspEditorInterace::CursorPos ScriptEditor::currentPosition() const {
    auto tc = m_editor->textCursor();
    LspEditorInterace::CursorPos pos;
    pos.blockNumber = tc.blockNumber();
    pos.positionInBlock = tc.positionInBlock();
    return pos;
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

    auto r = lsp.requestFormat(lspFileNameURL());

    struct TextEdit {
        qint64 start = -1;
        qint64 end = -1;
        QString newText;

        bool isValid() const { return start >= 0 && end >= start; }
    };

    QTextDocument *document = m_editor->document();
    QVector<TextEdit> textEdits;

    auto calculatePosition = [](QTextDocument *document,
                                const LSP::Location &loc) -> qint64 {
        QTextBlock block = document->findBlockByLineNumber(loc.line);
        if (!block.isValid()) {
            return -1;
        }
        return block.position() + qMin(loc.character, block.length() - 1);
    };

    for (auto &&vj : r.toArray()) {
        QJsonValue v = vj;
        auto range = AngelLsp::readLSPDocRange(v.toObject());
        TextEdit edit;
        edit.start = calculatePosition(document, range.start);
        edit.end = calculatePosition(document, range.end);
        edit.newText = v["newText"].toString();
        if (edit.isValid()) {
            textEdits.append(edit);
        }
    }

    std::sort(
        textEdits.begin(), textEdits.end(),
        [](const TextEdit &a, const TextEdit &b) { return a.start > b.start; });

    QTextCursor cursor(document);
    cursor.beginEditBlock();
    for (const TextEdit &edit : textEdits) {
        cursor.setPosition(edit.start);
        cursor.setPosition(edit.end, QTextCursor::KeepAnchor);
        cursor.insertText(edit.newText);
    }
    cursor.endEditBlock();

    return true;
}

bool ScriptEditor::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_editor) {
        if (event->type() == QEvent::KeyPress) {
            auto e = static_cast<QKeyEvent *>(event);
            if (e->modifiers() == Qt::NoModifier && e->key() == Qt::Key_Comma) {
                auto &lsp = AngelLsp::instance();
                auto url = lspFileNameURL();
                auto tc = currentPosition();
                auto line = tc.blockNumber;
                auto character = tc.positionInBlock;

                sendDocChange();
                while (isContentLspUpdated()) {
                    // wait for a moment
                }

                auto r = lsp.requestSignatureHelp(url, line, character);
                auto sigs = r["signatures"].toArray();
                QList<WingSignatureTooltip::Signature> ss;
                for (auto &&sig : sigs) {
                    QJsonValue js = sig;
                    WingSignatureTooltip::Signature s;
                    s.label = js["label"].toString();
                    s.doc = js["documentation"].toString();
                    ss.append(s);
                }
                showFunctionTip(ss);
            }
        }
    }
    return ads::CDockWidget::eventFilter(watched, event);
}
