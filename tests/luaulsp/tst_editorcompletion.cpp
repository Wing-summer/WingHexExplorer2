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

#include <algorithm>
#include <memory>

#include <QAbstractItemView>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#include "control/editor/codeinfotip.h"
#include "control/editor/luaucompletion.h"
#include "luau/lsp/luaulanguageserver.h"
#include "model/codecompletionmodel.h"

class TestEditor final : public WingCodeEdit, public LspEditorInterace {
public:
    explicit TestEditor(const QUrl &uri) : m_uri(uri) {
        m_completion = new LuauCompletion(this, this);
        LuauLanguageServer::instance().onOpenDocument(m_uri, document());
    }

    virtual ~TestEditor() override {
        setCompleter(nullptr);
        delete m_completion;
        LuauLanguageServer::instance().onCloseDocument(m_uri);
    }

    virtual const WingCodeEdit *editorPtr() const override { return this; }

    virtual lsp::DocumentUri lspFileNameURL() const override { return m_uri; }

    virtual CursorPos currentPosition() const override {
        return cursorPosition(textCursor());
    }

    virtual CursorPos cursorPosition(const QTextCursor &cursor) const override {
        return {cursor.blockNumber(), cursor.positionInBlock()};
    }

    virtual void showFunctionTip(
        const QList<WingSignatureTooltip::Signature> &signatures) override {
        m_label = signatures.front().label;
        showHelpTooltip(signatures);
    }

    virtual void clearFunctionTip() override {
        m_label.clear();
        hideHelpTooltip();
    }

    virtual void syncDocChange() override {
        ++m_queries;
        LuauLanguageServer::instance().onUpdateDocument(m_uri, document());
    }

    virtual void syncSemanticTokens() override {}

    QString label() const { return m_label; }

    int queries() const { return m_queries; }

    LuauCompletion *completion() const { return m_completion; }

    void setSource(const QString &source, int position = -1) {
        setPlainText(source);
        auto cursor = textCursor();
        cursor.setPosition(position < 0 ? source.size() : position);
        setTextCursor(cursor);
    }

protected:
    virtual QVector<lsp::SemanticToken> parseSemanticTokens() override {
        return {};
    }

    virtual void onCompletion(const QModelIndex &index) override {
        CodeInfoTip(index.data(Qt::SelfDataRole).value<CodeInfoTip>())
            .applyEdit(this, m_completion->completionPrefix(), true);
    }

private:
    QUrl m_uri;
    LuauCompletion *m_completion = nullptr;
    QString m_label;
    int m_queries = 0;
};

class EditorCompletionTests : public QObject {
    Q_OBJECT

private slots:

    void initTestCase() {
        QVERIFY(m_directory.isValid());
        QFile module(m_directory.filePath(QStringLiteral("module.luau")));
        QVERIFY(module.open(QIODevice::WriteOnly));
        module.write("return {}\n");
        module.close();
        LuauLanguageServer::InitializationOptions options;
        options.builtinModuleFiles.insert(QStringLiteral("core/net"),
                                          module.fileName());
        LuauLanguageServer::instance().initialize(options);
    }

    void init() {
        m_editor = std::make_unique<TestEditor>(QUrl::fromLocalFile(
            m_directory.filePath(QStringLiteral("main.luau"))));
        m_editor->resize(640, 480);
        m_editor->show();
        m_editor->activateWindow();
        m_editor->setFocus();
        QTRY_VERIFY(m_editor->hasFocus());
    }

    void cleanup() { m_editor.reset(); }

    void signatureFollowsCursorAndNestedCalls() {
        const auto source =
            definitions() + QStringLiteral("outer(inner(1), 2)");
        m_editor->setSource(source,
                            source.indexOf(QStringLiteral("inner(1)")) + 6);
        QTRY_VERIFY(m_editor->label().contains(QStringLiteral("inner(")));
        auto cursor = m_editor->textCursor();
        cursor.setPosition(source.indexOf(QStringLiteral(", 2)")) + 2);
        m_editor->setTextCursor(cursor);
        QTRY_VERIFY(m_editor->label().contains(QStringLiteral("outer(")));
        cursor.movePosition(QTextCursor::End);
        m_editor->setTextCursor(cursor);
        QTRY_VERIFY(!m_editor->isHelpTooltipVisible());
        cursor.setPosition(source.indexOf(QStringLiteral("inner(1)")) + 6);
        m_editor->setTextCursor(cursor);
        QTRY_VERIFY(m_editor->isHelpTooltipVisible());
    }

    void signatureInsideExpressionsAndIncompleteOuterCall() {
        const auto expression =
            definitions() + QStringLiteral("outer((1 + 2), 3)");
        m_editor->setSource(expression,
                            expression.indexOf(QStringLiteral("1 + 2")) + 1);
        QTRY_VERIFY(m_editor->label().contains(QStringLiteral("outer(")));
        m_editor->setSource(definitions() + QStringLiteral("outer(inner(1)"));
        QTRY_VERIFY(m_editor->label().contains(QStringLiteral("outer(")));
        const auto callback =
            definitions() +
            QStringLiteral("outer(function() local value = 1 end, 2)");
        m_editor->setSource(callback,
                            callback.indexOf(QStringLiteral("value = 1")) + 9);
        QTRY_VERIFY(!m_editor->isHelpTooltipVisible());
    }

    void typingAndEscape() {
        // Isolate signature triggers; popup priority is tested separately.
        auto *completion = m_editor->completion();
        completion->setTriggerList({});
        completion->setTriggerAmount(100000);
        m_editor->setSource(definitions() + QStringLiteral("outer"));
        QTest::keyClicks(m_editor.get(), "(");
        QTRY_VERIFY(m_editor->label().contains(QStringLiteral("outer(")));
        QTest::keyClick(m_editor.get(), Qt::Key_Escape);
        QVERIFY(!m_editor->isHelpTooltipVisible());
        QVERIFY(!completion->popup()->isVisible());
        QTest::keyClicks(m_editor.get(), "1");
        QTest::qWait(150);
        QVERIFY(!m_editor->isHelpTooltipVisible());
        QTest::keyClicks(m_editor.get(), ",");
        QTRY_VERIFY(m_editor->isHelpTooltipVisible());
    }

    void failedQueryRecovers() {
        m_editor->setSource(definitions() + QStringLiteral("outer"));
        QTest::qWait(150);
        QVERIFY(!m_editor->isHelpTooltipVisible());
        // Paste and programmatic edits do not deliver a '(' key event.
        m_editor->insertPlainText(QStringLiteral("(1, 2)"));
        QTest::keyClick(m_editor.get(), Qt::Key_Left);
        QTRY_VERIFY(m_editor->label().contains(QStringLiteral("outer(")));
    }

    void pendingUpdateStopsOnFocusLoss() {
        m_editor->setSource(definitions() + QStringLiteral("outer(1, 2)"));
        QTest::keyClick(m_editor.get(), Qt::Key_Left);
        m_editor->clearFocus();
        QTest::qWait(150);
        QVERIFY(!m_editor->isHelpTooltipVisible());
        m_editor->setFocus();
        QTRY_VERIFY(m_editor->isHelpTooltipVisible());
    }

    void refreshesAreCoalesced() {
        const auto source = definitions() + QStringLiteral("outer(1, 2)");
        m_editor->setSource(source, source.size() - 1);
        QTRY_VERIFY(m_editor->isHelpTooltipVisible());
        const int previousQueries = m_editor->queries();
        for (int i = 0; i < 10; ++i) {
            m_editor->insertPlainText(QStringLiteral(" "));
        }
        QCOMPARE(m_editor->queries(), previousQueries);
        QTRY_VERIFY(m_editor->queries() > previousQueries);
        QCOMPARE(m_editor->queries(), previousQueries + 1);
    }

    void completionRefreshesWithSelectedPlaceholder() {
        m_editor->setSource(definitions() + QStringLiteral("out"));
        lsp::CompletionItem item;
        item.label = QStringLiteral("outer");
        item.insertText = QStringLiteral("outer(${1:0}, ${2:0})");
        item.insertTextFormat = lsp::InsertTextFormat::Snippet;
        auto *completion = m_editor->completion();
        auto *model = new CodeCompletionModel({CodeInfoTip(item)}, completion);
        completion->setModel(model);
        completion->setCompletionPrefix(QStringLiteral("out"));
        completion->activated(model->index(0, 0));
        QVERIFY(m_editor->textCursor().hasSelection());
        QTRY_VERIFY(m_editor->label().contains(QStringLiteral("outer(")));
    }

    void completionPopupHasPriorityOverSignatureHelp() {
        const auto source = definitions() + QStringLiteral("outer(1, 2)");
        m_editor->setSource(source, source.size() - 1);
        QTRY_VERIFY(m_editor->isHelpTooltipVisible());
        auto *completion = m_editor->completion();
        lsp::CompletionItem item;
        item.label = QStringLiteral("argument");
        completion->setModel(
            new CodeCompletionModel({CodeInfoTip(item)}, completion));
        completion->setCompletionPrefix({});
        completion->complete(m_editor->cursorRect());
        QTRY_VERIFY(completion->popup()->isVisible());
        QVERIFY(!m_editor->isHelpTooltipVisible());
        m_editor->insertPlainText(QStringLiteral(" "));
        QTest::qWait(150);
        QVERIFY(!m_editor->isHelpTooltipVisible());
        // Escape closes the list first; a second Escape dismisses help.
        QTest::keyClick(completion->popup(), Qt::Key_Escape);
        QTRY_VERIFY(!completion->popup()->isVisible());
        QTRY_VERIFY(m_editor->isHelpTooltipVisible());
        QTest::keyClick(m_editor.get(), Qt::Key_Escape);
        QVERIFY(!m_editor->isHelpTooltipVisible());
    }

    void typedRequireParentActivation() {
        m_editor->setSource(QStringLiteral("require(\"@b/\")"), 12);
        m_editor->syncDocChange();
        // Mirror the host's document update before completion is triggered.
        connect(m_editor.get(), &QPlainTextEdit::textChanged, m_editor.get(),
                [this] { m_editor->syncDocChange(); });
        auto *completion = m_editor->completion();
        completion->trigger(QStringLiteral("/"), QStringLiteral("require(\"@b"),
                            m_editor->cursorRect());
        QTest::keyClicks(m_editor.get(), ".");
        auto *model = completion->completionModel();
        QModelIndex parent;
        for (int row = 0; row < model->rowCount(); ++row) {
            const auto index = model->index(row, 0);
            if (index.data().toString() == QStringLiteral("..")) {
                parent = index;
                break;
            }
        }
        QVERIFY(parent.isValid());
        QVERIFY(completion->popup()->isVisible());
        completion->popup()->setCurrentIndex(parent);
        QTest::keyClick(completion->popup(), Qt::Key_Tab);
        QCOMPARE(m_editor->toPlainText(), QStringLiteral("require(\"@b\")"));
    }

    void requireNavigationEdits_data() {
        QTest::addColumn<QString>("path");
        QTest::addColumn<QString>("label");
        QTest::addColumn<QString>("expectedPath");
        QTest::newRow("relative-current")
            << QString() << QStringLiteral(".") << QStringLiteral(".");
        QTest::newRow("relative-parent")
            << QString() << QStringLiteral("..") << QStringLiteral("..");
        QTest::newRow("builtin-root-parent")
            << QStringLiteral("@b/") << QStringLiteral("..")
            << QStringLiteral("@b");
        QTest::newRow("builtin-nested-parent")
            << QStringLiteral("@b/core/") << QStringLiteral("..")
            << QStringLiteral("@b/core");
    }

    void requireNavigationEdits() {
        QFETCH(QString, path);
        QFETCH(QString, label);
        QFETCH(QString, expectedPath);
        const auto source =
            QStringLiteral("require(\"") + path + QStringLiteral("\")");
        m_editor->setSource(source, 9 + path.size());
        m_editor->syncDocChange();
        const auto items = LuauLanguageServer::instance().completion(
            {m_editor->lspFileNameURL(),
             {0, uint(9 + path.size())},
             std::nullopt},
            nullptr);
        const auto item =
            std::find_if(items.cbegin(), items.cend(),
                         [&](const lsp::CompletionItem &candidate) {
                             return candidate.label == label;
                         });
        QVERIFY2(item != items.cend(), qPrintable(label));
        CodeInfoTip(*item).applyEdit(m_editor.get(), {}, true);
        QCOMPARE(m_editor->toPlainText(), QStringLiteral("require(\"") +
                                              expectedPath +
                                              QStringLiteral("\")"));
        QCOMPARE(m_editor->textCursor().position(), 9 + expectedPath.size());
    }

private:
    static QString definitions() {
        return QStringLiteral(
            "local function outer(a: number, b: number): number return a end\n"
            "local function inner(a: number): number return a end\n");
    }

    QTemporaryDir m_directory;
    std::unique_ptr<TestEditor> m_editor;
};

QTEST_MAIN(EditorCompletionTests)
#include "tst_editorcompletion.moc"
