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

#include "qasparser.h"
#include "qdocumentcursor.h"
#include "qdocumentline.h"

#include <QDir>
#include <QLibraryInfo>
#include <QQueue>
#include <QTextStream>
#include <QTime>
#include <QTimer>
#include <QtDebug>

#include "control/qcodecompletionwidget.h"

Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, DOT_TRIGGER, ("."))
Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, DBL_COLON_TRIGGER, ("::"))
Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, LEFT_PARE_TRIGGER, ("("))
Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, SEMI_COLON_TRIGGER, (";"))

AsCompletion::AsCompletion(asIScriptEngine *engine, QObject *p)
    : QCodeCompletionEngine(p), parser(engine), _engine(engine),
      pPopup(new QCodeCompletionWidget()) {
    Q_ASSERT(engine);

    addTrigger(*DOT_TRIGGER);
    addTrigger(*DBL_COLON_TRIGGER);

    // unleash the power of call tips
    addTrigger(*LEFT_PARE_TRIGGER);
    // clear the tips
    addTrigger(*SEMI_COLON_TRIGGER);

    setTrigWordLen(3);
}

AsCompletion::~AsCompletion() {}

QCodeCompletionEngine *AsCompletion::clone() {
    AsCompletion *e = new AsCompletion(_engine, pPopup->editor());

    for (auto &t : triggers())
        e->addTrigger(t);

    connect(e, &AsCompletion::onFunctionTip, this,
            &AsCompletion::onFunctionTip);

    emit cloned(e);

    return e;
}

QString AsCompletion::language() const { return QStringLiteral("AngelScript"); }

QStringList AsCompletion::extensions() const {
    QStringList l;

    l << "as"
      << "angelscript";

    return l;
}

void AsCompletion::complete(const QDocumentCursor &c, const QString &trigger) {
    auto txt = c.line().text();
    if (txt.isEmpty()) {
        emit onFunctionTip(this, {});
        return;
    }

    if (trigger == *SEMI_COLON_TRIGGER) {
        emit onFunctionTip(this, {});
        return;
    }

    auto off = 0;
    auto offcur = c.document()->property("console");
    if (!offcur.isNull() && offcur.isValid()) {
        auto cur = offcur.value<QDocumentCursor>();
        if (cur.isValid()) {
            off = cur.columnNumber();
        }
    } else {
        // parse current code
        // auto codes = c.document()->text(true, false);
        // parser.parse(codes, this->editor()->fileName());

        // QList<QCodeNode *> nodes = parser.codeNodes();
    }

    auto code = txt.mid(off, c.columnNumber() - off).toUtf8();

    auto len = code.length();
    if (len < trigWordLen()) {
        emit onFunctionTip(this, {});
        pPopup->hide();
        return;
    }

    auto p = code.data();
    auto end = p + len;

    struct Token {
        qsizetype pos;
        asETokenClass type;
        QByteArray content;
    };

    // parse the tokens
    QVector<Token> tokens;
    qsizetype pos = 0;
    for (; p < end;) {
        asUINT tokenLen = 0;
        auto tt = _engine->ParseToken(p, len, &tokenLen);
        if (tt == asTC_WHITESPACE) {
            p += tokenLen;
            pos += tokenLen;
            continue;
        }
        Token token;
        token.pos = pos;
        token.type = tt;
        token.content = QByteArray(p, tokenLen);
        tokens << token;
        p += tokenLen;
        pos += tokenLen;
    }

    auto getNamespace = [](const QVector<Token> &tokens) -> QByteArrayList {
        auto rbegin = tokens.rbegin();
        auto rend = tokens.rend();

        QByteArrayList nss;
        bool semiFlag = true;
        auto p = rbegin;
        for (; p != rend; ++p) {
            if (semiFlag) {
                if (p->type != asTC_IDENTIFIER) {
                    break;
                } else {
                    nss.prepend(p->content);
                    semiFlag = false;
                }
            } else {
                if (p->content != *DBL_COLON_TRIGGER) {
                    break;
                }
                semiFlag = true;
            }
        }
        return nss;
    };

    QByteArray fn;
    QList<QCodeNode *> nodes;

    QFlags<QCodeCompletionWidget::FilterFlag> filter(
        QCodeCompletionWidget::FilterFlag::Public |
        QCodeCompletionWidget::FilterFlag::KeepAllFunctions |
        QCodeCompletionWidget::FilterFlag::KeepAllSub);

    if (tokens.isEmpty()) {
        pPopup->hide();
        return;
    }

    QString prefix;
    auto etoken = tokens.back();

    // if trigger is empty, it's making editing
    if (trigger.isEmpty()) {
        // it can not be any trigger, so take the last as prefix
        prefix = etoken.content;
        tokens.removeLast();
        if (tokens.isEmpty()) {
            applyEmptyNsNode(nodes);
        } else {
            etoken = tokens.back(); // checking later
        }
    }

    if (nodes.isEmpty()) {
        if (etoken.type == asTC_KEYWORD) {
            // only support these
            auto cur = c;
            cur.setColumnNumber(off + etoken.pos);
            if (etoken.content == *DBL_COLON_TRIGGER) {
                complete(cur, *DBL_COLON_TRIGGER);
                pPopup->setCursor(c);
                pPopup->setPrefix(prefix);
                return;
            } else if (etoken.content == *DOT_TRIGGER) {
                complete(cur, *DOT_TRIGGER);
                pPopup->setCursor(c);
                pPopup->setPrefix(prefix);
                return;
            } else {
                applyEmptyNsNode(nodes);
            }
        } else if (etoken.type != asTC_IDENTIFIER) {
            pPopup->hide();
            return;
        }

        auto &_headerNodes = parser.headerNodes();

        if (etoken.content.length() >= trigWordLen()) {
            // completion for a.b.c or a::b.c or a::b::c.d or ::a::b.c
            if (trigger == *DBL_COLON_TRIGGER) {
                auto ns = getNamespace(tokens);
                auto idx = tokens.length() - ns.length() * 2;
                if (idx >= 0) {
                    if (tokens.at(idx).content == *DOT_TRIGGER) {
                        pPopup->hide();
                        return;
                    }
                }
                nodes = lookupNamespace(ns);
                if (nodes.isEmpty()) {
                    return;
                }
            } else if (trigger == *LEFT_PARE_TRIGGER) {
                // the first is function name, an identifier
                fn = etoken.content;
                tokens.removeLast();
                if (!tokens.isEmpty()) {
                    if (tokens.back().content == *DBL_COLON_TRIGGER) {
                        tokens.removeLast();
                    }
                }
                auto ns = getNamespace(tokens);
                auto idx = tokens.length() - ns.length() * 2 + 1;
                if (idx >= 0 && idx < tokens.length()) {
                    if (tokens.at(idx).content == *DOT_TRIGGER) {
                        pPopup->hide();
                        return;
                    }
                }
                nodes = lookupNamespace(ns);
                if (nodes.isEmpty()) {
                    applyEmptyNsNode(nodes);
                }
                QString tip;
                for (auto &n : nodes) {
                    for (auto &f : n->children()) {
                        if (f->type() != QCodeNode::Function ||
                            f->role(QCodeNode::Name) != fn) {
                            continue;
                        }

                        tip = QString::fromUtf8(f->role(QCodeNode::Return)) +
                              QLatin1Char(' ') +
                              QString::fromUtf8(f->role(QCodeNode::Name)) +
                              QString::fromUtf8(f->role(QCodeNode::Arguments))
                                  .prepend('(')
                                  .append(')');
                        break;
                    }
                    if (!tip.isEmpty()) {
                        break;
                    }
                }

                emit onFunctionTip(this, tip);
            }
        }
    }

    auto cur = c;
    cur.movePosition(trigger.length());
    pPopup->setCursor(cur);
    pPopup->setPrefix(prefix);
    pPopup->setFilter(filter);
    pPopup->setCompletions(nodes);

    if (!pPopup->isCompleting()) {
        pPopup->popup();
    }
}

void AsCompletion::applyEmptyNsNode(QList<QCodeNode *> &nodes) {
    if (_emptyNsNodes.isEmpty()) {
        for (auto &n : parser.headerNodes()) {
            auto name = n->qualifiedName();
            if (name.isEmpty()) {
                // only one group for empty namespace
                _emptyNsNodes << n;
                break;
            }
        }
    }

    nodes = _emptyNsNodes;
}

void AsCompletion::parse(const QDocumentCursor &c) {
    auto codes = c.document()->text();
    // asBuilder builder;
}

QList<QCodeNode *> AsCompletion::lookupNamespace(const QByteArrayList &ns) {
    QList<QCodeNode *> nodes;

    if (ns.isEmpty()) {
        return nodes;
    }

    auto headers = parser.headerNodes();
    auto r =
        std::find_if(headers.begin(), headers.end(), [&ns](QCodeNode *node) {
            return node->role(QCodeNode::Name) == ns.first();
        });
    if (r != headers.end()) {
        auto n = *r;
        auto len = ns.size();

        bool found = true;
        for (qsizetype i = 1; i < len; ++i) {
            auto &c = n->children();
            r = std::find_if(c.begin(), c.end(), [&ns, i](QCodeNode *node) {
                return node->role(QCodeNode::Name) == ns.at(i);
            });
            if (r == c.end()) {
                found = false;
                break;
            }
        }

        if (found) {
            nodes.append(*r);
        }
    }

    return nodes;
}

QCodeCompletionWidget *AsCompletion::codeCompletionWidget() const {
    return pPopup;
}

void AsCompletion::setEditor(QEditor *e) {
    QCodeCompletionEngine::setEditor(e);
    pPopup->setEditor(e);
}
