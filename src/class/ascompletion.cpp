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

#include "ascompletion.h"

#include "class/angellsp.h"
#include "control/scripteditor.h"
#include "model/codecompletionmodel.h"
#include "utilities.h"
#include "wingcodeedit.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QEvent>
#include <QLibraryInfo>
#include <QQueue>
#include <QTextStream>
#include <QTime>
#include <QTimer>
#include <QtDebug>

Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, DOT_TRIGGER, ("."))
Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, DBL_COLON_TRIGGER, ("::"))
Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, LEFT_PARE_TRIGGER, ("("))
Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, SEMI_COLON_TRIGGER, (";"))
Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, SHARP_TRIGGER, ("#"))

AsCompletion::AsCompletion(WingCodeEdit *p) : WingCompleter(p) {
    QStringList kws{
        "const",     "in",        "inout",    "out",    "auto",     "public",
        "protected", "private",   "void",     "int8",   "int16",    "int",
        "int64",     "uint8",     "uint16",   "uint",   "uint64",   "float",
        "double",    "bool",      "enum",     "string", "array",    "any",
        "for",       "while",     "do",       "if",     "else",     "switch",
        "break",     "continue",  "try",      "catch",  "throw",    "abstract",
        "delete",    "cast",      "class",    "final",  "property", "external",
        "function",  "interface", "shared",   "this",   "explicit", "override",
        "namespace", "get",       "set",      "super",  "mixin",    "false",
        "true",      "null",      "typename", "return", "typedef",  "funcdef",
        "from",      "import",    "not",      "xor",    "or",       "is",
        "co_await"};
    for (auto &k : kws) {
        CodeInfoTip t;
        t.type = CodeInfoTip::Type::KeyWord;
        t.name = k;
        _keywordNode.append(t);
    }

    setTriggerList({*DOT_TRIGGER, *DBL_COLON_TRIGGER,
                    // unleash the power of call tips
                    *LEFT_PARE_TRIGGER,
                    // clear the tips
                    *SEMI_COLON_TRIGGER,
                    // for marcos
                    *SHARP_TRIGGER});
    setTriggerAmount(3);

    connect(this, QOverload<const QModelIndex &>::of(&AsCompletion::activated),
            this, [this](const QModelIndex &index) {
                auto v = index.data(Qt::SelfDataRole).value<CodeInfoTip>();
                if (v.type == CodeInfoTip::Type::Function ||
                    v.type == CodeInfoTip::Type::ClsFunction) {
                    Q_EMIT onFunctionTip(v.getTooltip());
                }
            });

    p->installEventFilter(this);
}

AsCompletion::~AsCompletion() {}

void AsCompletion::clearFunctionTip() { Q_EMIT onFunctionTip({}); }

QString AsCompletion::wordSeperators() const {
    static QString eow(QStringLiteral("~!@$%^&*()_+{}|\"<>?,/;'[]\\-="));
    return eow;
}

bool AsCompletion::processTrigger(const QString &trigger,
                                  const QString &content) {
    QList<CodeInfoTip> nodes;

    if (trigger == *SHARP_TRIGGER) {
        setModel(new CodeCompletionModel(parseMarcos(), this));
        setCompletionPrefix({});
        return true;
    }

    if (content.isEmpty()) {
        return false;
    }

    if (trigger == *SEMI_COLON_TRIGGER) {
        clearFunctionTip();
        return false;
    }

    auto editor = qobject_cast<ScriptEditor *>(this->widget()->parent());
    if (editor == nullptr) {
        return false;
    }
    Q_ASSERT(editor);

    if (!trigger.isEmpty() && trigger != *DOT_TRIGGER) {
        clearFunctionTip();
    }

    auto url = Utilities::getUrlString(editor->fileName());
    auto tc = editor->editor()->textCursor();
    auto &lsp = AngelLsp::instance();
    auto line = tc.blockNumber();
    auto character = tc.positionInBlock() + trigger.size();
    auto ret = lsp.requestCompletion(url, line, character);

    setModel(new CodeCompletionModel(nodes, this));

    return true;
}

QList<CodeInfoTip> AsCompletion::parseMarcos() {
    static QList<CodeInfoTip> marcos;
    if (marcos.isEmpty()) {
        QStringList m{"define", "undef",  "if",      "else",  "endif",
                      "ifdef",  "ifndef", "include", "pragma"};
        for (auto &i : m) {
            CodeInfoTip tip;
            tip.name = i;
            tip.type = CodeInfoTip::Type::KeyWord;
            marcos.append(tip);
        }
    }
    return marcos;
}

bool AsCompletion::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        auto e = static_cast<QKeyEvent *>(event);
        if (e->key() == Qt::Key_Escape) {
            clearFunctionTip();
        }
    }
    return WingCompleter::eventFilter(watched, event);
}
