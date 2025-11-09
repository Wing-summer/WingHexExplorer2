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

#include "codeinfotip.h"

#include "class/angellsp.h"

#include <QHash>
#include <QMetaEnum>

using IconHash = QHash<LSP::CompletionItemKind, QIcon>;
Q_GLOBAL_STATIC(IconHash, q_icon_cache)

inline static QIcon getIcon(const QString &name) {
    return QIcon(QStringLiteral(":/completion/images/completion/") + name +
                 QStringLiteral(".svg"));
}

QIcon CodeInfoTip::getDisplayIcon(LSP::CompletionItemKind type) {
    static bool setup = false;

    if (!setup) {
        auto e = QMetaEnum::fromType<LSP::CompletionItemKind>();
        auto total = e.keyCount();
        for (int i = 0; i < total; ++i) {
            auto k = e.key(i);
            auto v = LSP::CompletionItemKind(e.value(i));
            q_icon_cache->insert(v, getIcon(QString::fromLatin1(k)));
        }
        setup = true;
    }

    return q_icon_cache->value(type);
}

QString CodeInfoTip::comment() const {
    if (_comment.isEmpty()) {
        resolve();
    }
    return _comment;
}

QString CodeInfoTip::completion() const {
    if (_completion.isEmpty()) {
        resolve();
    }
    return _completion;
}

void CodeInfoTip::setComment(const QString &comment) { _comment = comment; }

bool CodeInfoTip::isSnippet() const { return _isSnippet; }

void CodeInfoTip::resolve() const {
    if (!_comment.isEmpty() && !_completion.isEmpty()) {
        return;
    }

    if (value.isNull()) {
        _comment = name;
        _completion = name;
        return;
    }

    auto &lsp = AngelLsp::instance();
    auto v = lsp.requestResolve(value);
    auto label = v["label"].toString();

    if (_comment.isEmpty()) {
        _comment = v["detail"].toString();

        if (_comment.isEmpty()) {
            _comment = label;
        }
    }

    if (_completion.isEmpty()) {
        _completion = v["insertText"].toString();
        if (_completion.isEmpty()) {
            _completion = label;
        }
    }

    _isSnippet =
        v["insertTextFormat"].toInt() == int(LSP::InsertTextFormat::Snippet);
}
