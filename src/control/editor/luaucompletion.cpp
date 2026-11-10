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

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QPointer>

#include "editorlspevent.h"
#include "luau/lsp/lsp.h"
#include "luau/lsp/luaulanguageserver.h"
#include "model/codecompletionmodel.h"

LuauCompletion::LuauCompletion(LspEditorInterace *inter, WingCodeEdit *parent)
    : WingCompleter(parent), m_timer(new ResettableTimer(this)),
      m_signatureTimer(new ResettableTimer(this)), m_inter(inter) {
    Q_ASSERT(parent);
    Q_ASSERT(inter);

    setTriggerList({".", "(", ",", ":", "@", "\"", "'", "/"});
    setTriggerAmount(3);

    connect(this,
            QOverload<const QModelIndex &>::of(&LuauCompletion::activated),
            this, &LuauCompletion::onActivatedCodeComplete);
    parent->installEventFilter(this);
    popup()->installEventFilter(this);
    connect(m_timer, &ResettableTimer::sigOnTimeout, this,
            &LuauCompletion::onCodeComplete);
    connect(m_signatureTimer, &ResettableTimer::sigOnTimeout, this,
            &LuauCompletion::onSignatureHelpUpdate);
    connect(parent, &QPlainTextEdit::textChanged, this,
            &LuauCompletion::onSignatureHelpRefreshRequested);
    connect(parent, &QPlainTextEdit::cursorPositionChanged, this,
            &LuauCompletion::onSignatureHelpRefreshRequested);
}

void LuauCompletion::clearFunctionTip() {
    m_signatureTimer->stop();
    m_inter->clearFunctionTip();
}

QString LuauCompletion::wordSeperators() const {
    return QStringLiteral("~!$%^&*()+{}|<>?,:;[]\\-=\"'/@.");
}

bool LuauCompletion::processTrigger(const QString &trigger,
                                    const QString &content) {
    auto lspURL = m_inter->lspFileNameURL();
    if (lspURL.isEmpty() || (trigger.isEmpty() && content.isEmpty())) {
        popup()->hide();
        return false;
    }

    const auto cursor = m_inter->currentPosition();
    if (cursor.blockNumber < 0 || cursor.positionInBlock < 0) {
        return false;
    }

    const auto separators = wordSeperators();
    auto split = std::find_if(
        content.crbegin(), content.crend(),
        [separators](const QChar &ch) { return separators.contains(ch); });
    const auto prefixStart = std::distance(split, content.crend());
    const auto prefix = content.sliced(prefixStart);

    if (trigger.isEmpty() && popup()->isVisible()) {
        setCompletionPrefix(prefix);
        return true;
    }
    if (trigger.isEmpty() && !m_ok) {
        return false;
    }

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

    QPointer<QAbstractItemModel> oldModel = model();
    auto newModel = new CodeCompletionModel(items, this);
    setModel(newModel);
    if (oldModel && oldModel->parent() == this) {
        oldModel->deleteLater();
    }
    if (items.isEmpty()) {
        popup()->hide();
    }
    setCompletionPrefix(trigger.isEmpty() ? prefix : QString());
    m_ok = false;
    m_timer->reset(300);
    return !items.isEmpty();
}

bool LuauCompletion::eventFilter(QObject *watched, QEvent *event) {
    if (watched == popup()) {
        if (event->type() == QEvent::Show) {
            clearFunctionTip();
        } else if (event->type() == QEvent::Hide && !m_signatureHelpDismissed) {
            // Restore help after focus and completion edits have settled.
            m_signatureTimer->reset(1);
        }
        if (event->type() != QEvent::KeyPress ||
            static_cast<QKeyEvent *>(event)->key() == Qt::Key_Escape) {
            return WingCompleter::eventFilter(watched, event);
        }
    }

    if (event->type() == QEvent::KeyPress) {
        const auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            const bool dismissingHelp =
                !popup()->isVisible() &&
                (m_inter->editorPtr()->isHelpTooltipVisible() ||
                 m_signatureTimer->isActive());
            m_signatureHelpDismissed = true;
            clearFunctionTip();
            if (dismissingHelp) {
                return true;
            }
        } else {
            const auto text = keyEvent->text();
            if (text == QLatin1String("(") || text == QLatin1String(",") ||
                text == QLatin1String(")")) {
                m_signatureHelpDismissed = false;
                // Query after the editor has applied the key, including any
                // auto-inserted closing parenthesis.
                m_signatureTimer->reset(1);
            } else {
                switch (keyEvent->key()) {
                case Qt::Key_Left:
                case Qt::Key_Right:
                case Qt::Key_Up:
                case Qt::Key_Down:
                case Qt::Key_Home:
                case Qt::Key_End:
                case Qt::Key_PageUp:
                case Qt::Key_PageDown:
                    m_signatureHelpDismissed = false;
                    break;
                default:
                    break;
                }
            }
        }
    } else if (event->type() == QEvent::MouseButtonRelease ||
               event->type() == QEvent::FocusIn) {
        m_signatureHelpDismissed = false;
        onSignatureHelpRefreshRequested();
    } else if (event->type() == QEvent::FocusOut ||
               event->type() == QEvent::Hide ||
               event->type() == QEvent::Wheel) {
        clearFunctionTip();
    }
    return WingCompleter::eventFilter(watched, event);
}

void LuauCompletion::onCodeComplete() { m_ok = true; }

bool LuauCompletion::canShowSignatureHelp() const {
    const auto *editor = m_inter->editorPtr();
    return enabled() && !editor->isReadOnly() && editor->hasFocus() &&
           !popup()->isVisible();
}

void LuauCompletion::onSignatureHelpRefreshRequested() {
    if (!canShowSignatureHelp()) {
        clearFunctionTip();
        return;
    }
    // Coalesce text/cursor changes without postponing updates indefinitely
    // during continuous typing. A failed query does not disable later ones.
    if (!m_signatureHelpDismissed && !m_signatureTimer->isActive()) {
        m_signatureTimer->reset(80);
    }
}

void LuauCompletion::onSignatureHelpUpdate() {
    if (m_signatureHelpDismissed || !canShowSignatureHelp()) {
        clearFunctionTip();
        return;
    }
    EditorLspEvent::showSignatureHelp(m_inter);
}

void LuauCompletion::onActivatedCodeComplete(const QModelIndex &index) {
    Q_UNUSED(index);
    m_signatureHelpDismissed = false;
    // Activation can be delivered before the completion edit is inserted.
    m_signatureTimer->reset(1);
}
