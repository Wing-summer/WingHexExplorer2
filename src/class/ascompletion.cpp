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
#include "model/codecompletionmodel.h"
#include "wingcodeedit.h"

#include <KSyntaxHighlighting/Format>

#include <QAbstractItemView>
#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QEvent>
#include <QJsonArray>
#include <QLibraryInfo>
#include <QQueue>
#include <QTextStream>
#include <QThread>
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
        "const",     "in",        "inout",    "out",      "auto",
        "public",    "protected", "private",  "void",     "int8",
        "int16",     "int",       "int64",    "uint8",    "uint16",
        "uint",      "uint64",    "float",    "double",   "bool",
        "enum",      "for",       "while",    "do",       "if",
        "else",      "switch",    "break",    "continue", "try",
        "catch",     "throw",     "abstract", "delete",   "cast",
        "class",     "final",     "property", "external", "function",
        "interface", "shared",    "this",     "explicit", "override",
        "namespace", "get",       "set",      "super",    "mixin",
        "false",     "true",      "null",     "typename", "return",
        "typedef",   "funcdef",   "from",     "import",   "not",
        "xor",       "or",        "is"};
    for (auto &k : kws) {
        CodeInfoTip t;
        t.type = LSP::CompletionItemKind::Keyword;
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
            this, &AsCompletion::onActivatedCodeComplete);

    p->installEventFilter(this);

    _timer = new ResettableTimer(this);
    connect(_timer, &ResettableTimer::timeoutTriggered, this,
            &AsCompletion::onCodeComplete);
}

AsCompletion::~AsCompletion() {}

void AsCompletion::clearFunctionTip() {
    auto editor = getEditor();
    if (editor == nullptr) {
        return;
    }

    editor->clearFunctionTip();
}

bool AsCompletion::isCursorInString() const {
    auto edit = getEditor();
    auto e = edit->editorPtr();
    const QTextCursor cur = e->textCursor();
    return _strdet.isInsideString(cur);
}

QList<CodeInfoTip> AsCompletion::parseCompletion(const QJsonValue &v) {
    if (!v.isArray()) {
        return {};
    }

    QList<CodeInfoTip> ret;
    auto arr = v.toArray();
    auto total = arr.size();
    for (int i = 0; i < total; ++i) {
        auto item = arr.at(i);
        CodeInfoTip tip;
        tip.value = item;
        tip.type = LSP::CompletionItemKind(item["kind"].toInt());
        tip.name = item["label"].toString();
        ret.append(tip);
    }

    return ret;
}

QString AsCompletion::wordSeperators() const {
    static QString eow(QStringLiteral("~!@$%^&*()_+{}|\"<>?,/;'[]\\-="));
    return eow;
}

bool AsCompletion::processTrigger(const QString &trigger,
                                  const QString &content) {
    if (isCursorInString()) {
        return false;
    }

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

    auto editor = getEditor();
    if (editor == nullptr) {
        return false;
    }
    Q_ASSERT(editor);

    if (!trigger.isEmpty() && trigger != *DOT_TRIGGER) {
        clearFunctionTip();
    }

    if (!_ok) {
        return false;
    }

    auto url = editor->lspFileNameURL();
    auto &lsp = AngelLsp::instance();

    auto tc = editor->currentPosition();
    auto line = tc.blockNumber;
    auto character = tc.positionInBlock;

    QString prefix;
    if (trigger.isEmpty()) {
        auto seps = wordSeperators();
        auto r =
            std::find_if(content.crbegin(), content.crend(),
                         [seps](const QChar &ch) { return seps.contains(ch); });
        auto stridx = std::distance(r, content.crend());
        auto str = content.sliced(stridx);

        if (str.startsWith('#')) {
            prefix = str.sliced(1);
            setModel(new CodeCompletionModel(parseMarcos(), this));
            setCompletionPrefix(prefix);
            _ok = false;
            _timer->reset(300);
            return true;
        }

        auto tg = *DOT_TRIGGER;
        auto idx = str.lastIndexOf(tg);
        if (idx < 0) {
            tg = *DBL_COLON_TRIGGER;
            idx = str.lastIndexOf(tg);
        }

        if (idx >= 0) {
            prefix = str.sliced(idx + tg.size());
        } else {
            prefix = str;
        }
    }

    while (editor->isContentLspUpdated()) {
        // wait for a moment
    }

    auto ret = lsp.requestCompletion(url, line, character, trigger);
    auto nodes = parseCompletion(ret);

    if (trigger.isEmpty()) {
        nodes.append(_keywordNode);
    }

    setModel(new CodeCompletionModel(nodes, this));
    setCompletionPrefix(prefix);
    _ok = false;
    _timer->reset(300);

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
            tip.type = LSP::CompletionItemKind::Keyword;
            marcos.append(tip);
        }
    }
    return marcos;
}

LspEditorInterace *AsCompletion::getEditor() const {
    return dynamic_cast<LspEditorInterace *>(this->widget()->parent());
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

void AsCompletion::onCodeComplete() { _ok = true; }

void AsCompletion::onActivatedCodeComplete(const QModelIndex &index) {
    auto editor = getEditor();
    if (editor == nullptr) {
        return;
    }
    auto v = index.data(Qt::SelfDataRole).value<CodeInfoTip>();
    if (v.type == LSP::CompletionItemKind::Function) {
        auto tc = editor->currentPosition();
        auto line = tc.blockNumber;
        auto character = tc.positionInBlock;
        auto &lsp = AngelLsp::instance();

        // textChanged will emit later so send now
        editor->syncUpdate();

        auto url = editor->lspFileNameURL();
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
        editor->showFunctionTip(ss);
    }
}

QList<CodeInfoTip> AsCompletion::keywordNode() const { return _keywordNode; }
