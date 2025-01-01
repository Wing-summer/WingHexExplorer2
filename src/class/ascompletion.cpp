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
#include "qcalltip.h"
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
Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, SEMI_COLON_TRIGGER, ("::"))
Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, LEFT_PARE_TRIGGER, ("("))

AsCompletion::AsCompletion(asIScriptEngine *engine, QObject *p)
    : QCodeCompletionEngine(p), parser(engine), _engine(engine),
      pPopup(new QCodeCompletionWidget()) {
    Q_ASSERT(engine);

    addTrigger(*DOT_TRIGGER);
    addTrigger(*SEMI_COLON_TRIGGER);

    // unleash the power of call tips
    addTrigger(*LEFT_PARE_TRIGGER);

    setTrigWordLen(3);
}

AsCompletion::~AsCompletion() {}

QCodeCompletionEngine *AsCompletion::clone() {
    AsCompletion *e = new AsCompletion(_engine, pPopup->editor());

    for (auto &t : triggers())
        e->addTrigger(t);

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
        // TODO parse current code
        auto codes = c.document()->text(true, false);
        parser.parse(codes, this->editor()->fileName());

        // QList<QCodeNode *> nodes = parser.codeNodes();
    }

    auto code = txt.mid(off, c.columnNumber() - off).toUtf8();

    auto p = code.data();
    auto len = code.length();
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
        Token token;
        token.pos = pos;
        token.type = tt;
        token.content = QByteArray(p, tokenLen);
        tokens << token;
        p += tokenLen;
        pos += tokenLen;
    }

    auto r =
        std::find_if(tokens.rbegin(), tokens.rend(), [](const Token &token) {
            return token.type != asTC_WHITESPACE;
        });
    if (r == tokens.rend()) {
        return;
    }

    QByteArray fn;
    QList<QCodeNode *> nodes;

    QCodeCompletionWidget::Filter filter =
        QCodeCompletionWidget::FilterFlag::KeepAll;
    auto &_headerNodes = parser.headerNodes();

    if (trigger.isEmpty() && trigWordLen() <= r->content.length()) {
        auto eb = tokens.back();
        if (eb.type == asTC_KEYWORD) {
            // only support these
            if (eb.content == *SEMI_COLON_TRIGGER) {
                complete(c, *SEMI_COLON_TRIGGER);
            } else if (eb.content == *DOT_TRIGGER) {
                complete(c, *DOT_TRIGGER);
            } else {
                pPopup->hide();
            }
            return;
        } else if (eb.type == asTC_IDENTIFIER) {
            fn = eb.content;

            // completion for a.b.c or a::b.c or a::b::c.d or ::a::b.c
            // first validate
            auto rbegin = tokens.rbegin();
            auto rend = tokens.rend();

            rbegin++;
            if (rbegin == rend) {
                applyEmptyNsNode(nodes);
            } else {
                if (rbegin->content == SEMI_COLON_TRIGGER) {
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
                            if (p->content != SEMI_COLON_TRIGGER) {
                                break;
                            }
                            semiFlag = true;
                        }
                    }

                    nodes = lookupNamespace(nss);
                    if (nodes.isEmpty()) {
                        return;
                    }
                } else if (rbegin->content == DOT_TRIGGER) {
                    // TODO only PR
                } else {
                    applyEmptyNsNode(nodes);
                }
            }
        } else {
            return;
        }

        // pPopup->setTemporaryNodes(temp);
        pPopup->setPrefix(fn);
        pPopup->setFilter(filter);
        pPopup->setCompletions(nodes);
        pPopup->popup();
    } else {
        if (trigger == *SEMI_COLON_TRIGGER) {
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
                    if (p->content != *SEMI_COLON_TRIGGER) {
                        break;
                    }
                    semiFlag = true;
                }
            }

            nodes = lookupNamespace(nss);
            if (nodes.isEmpty()) {
                return;
            }
        } else if (trigger == *LEFT_PARE_TRIGGER) {
            if (r != tokens.rend()) {
                auto pr = std::next(r);
                if (pr != tokens.rend()) {
                    if (pr->content == *SEMI_COLON_TRIGGER) {
                        if (pr != tokens.rend()) {
                            auto prr = std::next(pr);
                            auto ns = prr->content;
                            if (prr->type == asTC_IDENTIFIER) {
                                for (auto &n : _headerNodes) {
                                    auto name = n->qualifiedName();
                                    if (name == ns) {
                                        nodes << n;
                                        break;
                                    }
                                }
                            } else {
                                return;
                            }
                        }
                    }
                }
            }
        }

        if (nodes.count()) {
            if (trigger == *LEFT_PARE_TRIGGER) {
                QStringList tips;

                // qDebug("fn %s", fn.constData());

                for (auto &n : nodes) {
                    for (auto &f : n->children()) {
                        if (f->type() != QCodeNode::Function ||
                            f->role(QCodeNode::Name) != fn) {
                            continue;
                        }

                        auto tip =
                            QString::fromUtf8(f->role(QCodeNode::Arguments))
                                .prepend('(')
                                .append(')');

                        if (!tips.contains(tip))
                            tips << tip;
                    }
                }

                if (!tips.isEmpty()) {
                    QRect r = editor()->cursorRect();
                    QDocumentCursor cursor = editor()->cursor();
                    QDocumentLine line = cursor.line();

                    int hx = editor()->horizontalOffset(),
                        cx = line.cursorToX(cursor.columnNumber());

                    auto ct = new QCallTip(editor()->viewport());
                    ct->move(cx - hx, r.y() + r.height());
                    ct->setTips(tips);
                    ct->show();
                    ct->setFocus();

#ifdef TRACE_COMPLETION
                    qDebug("parsing + scoping + search + pre-display : elapsed "
                           "%i ms",
                           time.elapsed());
#endif
                }
            } else {
                // pPopup->setTemporaryNodes(temp);
                pPopup->setPrefix({});
                pPopup->setFilter(QCodeCompletionWidget::Filter(filter));
                pPopup->setCompletions(nodes);

#ifdef TRACE_COMPLETION
                qDebug(
                    "parsing + scoping + search + pre-display : elapsed %i ms",
                    time.elapsed());
#endif

                pPopup->popup();
            }
        } else {
            // qDeleteAll(temp);
            // qDebug("completion failed");
        }
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
