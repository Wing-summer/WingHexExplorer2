/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
** FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
** for more details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#include "luaucompletion.h"

#include <QKeyEvent>

#include "editorlspevent.h"
#include "luau/lsp/lsp.h"
#include "luau/lsp/luaulanguageserver.h"
#include "model/codecompletionmodel.h"

LuauCompletion::LuauCompletion(LspEditorInterace *inter, WingCodeEdit *parent)
    : WingCompleter(parent), m_timer(new ResettableTimer(this)),
      m_inter(inter) {
    Q_ASSERT(parent);
    Q_ASSERT(inter);

    setTriggerList({".", "(", ",", ":", "@"});
    setTriggerAmount(3);

    connect(this,
            QOverload<const QModelIndex &>::of(&LuauCompletion::activated),
            this, &LuauCompletion::onActivatedCodeComplete);
    parent->installEventFilter(this);
    connect(m_timer, &ResettableTimer::sigOnTimeout, this,
            &LuauCompletion::onCodeComplete);
}

void LuauCompletion::clearFunctionTip() { m_inter->clearFunctionTip(); }

QString LuauCompletion::wordSeperators() const {
    return QStringLiteral("~!@$%^&*()+{}|\"<>?,/;'[]\\-=");
}

bool LuauCompletion::processTrigger(const QString &trigger,
                                    const QString &content) {
    auto lspURL = m_inter->lspFileNameURL();
    if (!m_ok || lspURL.isEmpty() || content.isEmpty()) {
        return false;
    }

    const auto cursor = m_inter->currentPosition();
    if (cursor.blockNumber < 0 || cursor.positionInBlock < 0) {
        return false;
    }

    QString prefix;
    auto separators = wordSeperators();
    auto split = std::find_if(
        content.crbegin(), content.crend(),
        [separators](const QChar &ch) { return separators.contains(ch); });
    const auto prefixStart = std::distance(split, content.crend());
    const auto trailingText = content.sliced(prefixStart);
    const auto dot = trailingText.lastIndexOf(QLatin1Char('.'));
    prefix = dot < 0 ? trailingText : trailingText.sliced(dot + 1);

    lsp::CompletionParams params{
        lspURL, {uint(cursor.blockNumber), uint(cursor.positionInBlock)}};
    if (!trigger.isEmpty()) {
        params.context = lsp::CompletionParams::Context{
            lsp::CompletionTriggerKind::TriggerCharacter, trigger};
    }

    const auto serverItems =
        LuauLanguageServer::instance().completion(params, nullptr);
    QList<CodeInfoTip> items;
    items.reserve(serverItems.size());
    for (const auto &serverItem : serverItems) {
        CodeInfoTip item(serverItem);
        items.append(std::move(item));
    }

    setModel(new CodeCompletionModel(items, this));
    setCompletionPrefix(prefix);
    m_ok = false;
    m_timer->reset(300);
    return !items.isEmpty();
}

bool LuauCompletion::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        auto e = static_cast<QKeyEvent *>(event);
        auto key = e->key();
        if (key == Qt::Key_Escape || key == Qt::Key_Return ||
            key == Qt::Key_Enter) {
            clearFunctionTip();
        }
    }
    return WingCompleter::eventFilter(watched, event);
}

void LuauCompletion::onCodeComplete() { m_ok = true; }

void LuauCompletion::onActivatedCodeComplete(const QModelIndex &index) {
    const auto item = index.data(Qt::SelfDataRole).value<CodeInfoTip>();
    if (item.kind == lsp::CompletionItemKind::Function ||
        item.kind == lsp::CompletionItemKind::Method ||
        item.kind == lsp::CompletionItemKind::Event ||
        item.kind == lsp::CompletionItemKind::Operator ||
        item.kind == lsp::CompletionItemKind::Interface ||
        item.kind == lsp::CompletionItemKind::Constructor) {
        EditorLspEvent::showSignatureHelp(m_inter);
    }
}
