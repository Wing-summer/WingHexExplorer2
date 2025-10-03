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
            q_icon_cache->insert(v, getIcon(k));
        }
        setup = true;
    }

    return q_icon_cache->value(type);
}

void CodeInfoTip::resolve() {
    if (!comment.isEmpty() && !completion.isEmpty()) {
        return;
    }

    if (value.isNull()) {
        comment = name;
        completion = name;
        return;
    }

    auto &lsp = AngelLsp::instance();
    auto v = lsp.requestResolve(value);
    if (comment.isEmpty()) {
        comment = v["detail"].toString();
    }

    if (completion.isEmpty()) {
        completion = v["insertText"].toString();
    }
}
