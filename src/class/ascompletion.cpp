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

Q_GLOBAL_STATIC_WITH_ARGS(QString, DOT_TRIGGER, ("."))
Q_GLOBAL_STATIC_WITH_ARGS(QString, SEMI_COLON_TRIGGER, ("::"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, LEFT_PARE_TRIGGER, ("("))

AsCompletion::AsCompletion(asIScriptEngine *engine, QObject *p)
    : QCodeCompletionEngine(p), parser(engine), _engine(engine),
      pPopup(new QCodeCompletionWidget()) {
    Q_ASSERT(engine);

    addTrigger(*DOT_TRIGGER);
    addTrigger(*SEMI_COLON_TRIGGER);

    // unleash the power of call tips
    addTrigger(*LEFT_PARE_TRIGGER);

    // TODO parse the std aslib

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
    if (pPopup->isVisible()) {
        return;
    }

    // TODO parse current code
    // auto codes = c.document()->text(true, false);
    // parser.parse(codes, this->editor()->fileName());

    // QList<QCodeNode *> nodes = parser.codeNodes();
    auto txt = c.line().text().left(c.columnNumber());
    auto code = txt.toUtf8();

    auto p = code.data();
    auto len = code.length();
    auto end = p + len;

    struct Token {
        qsizetype pos;
        asETokenClass type;
        QByteArray content;
    };

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

    QByteArray fn;
    QList<QCodeNode *> nodes;

    auto r =
        std::find_if(tokens.rbegin(), tokens.rend(), [](const Token &token) {
            return token.type != asTC_WHITESPACE;
        });
    if (r == tokens.rend()) {
        return;
    }

    QCodeCompletionWidget::Filter filter =
        QCodeCompletionWidget::FilterFlag::KeepAll;
    auto &_headerNodes = parser.headerNodes();
    fn = r->content;

    if (trigger.isEmpty()) {
        auto eb = tokens.back();
        if (eb.type == asTC_KEYWORD) {
            // only support these
            if (eb.content == *SEMI_COLON_TRIGGER) {
                complete(c, *SEMI_COLON_TRIGGER);
            } else if (eb.content == *DOT_TRIGGER) {
                complete(c, *DOT_TRIGGER);
            }
            return;
        } else if (eb.type == asTC_IDENTIFIER) {
            if (r != tokens.rend()) {
                auto pr = std::next(r);
                if (pr->content == *SEMI_COLON_TRIGGER) {
                    if (pr != tokens.rend()) {
                        auto prr = std::next(pr);
                        auto ns = prr->content;

                        if (prr->type == asTC_IDENTIFIER) {
                            for (auto &n : _headerNodes) {
                                auto name = n->qualifiedName();
                                if (name == ns) {
                                    nodes << n;
                                }
                            }

                            auto cur = c;
                            cur.movePosition(pr->pos + pr->content.length() -
                                             txt.length());
                            pPopup->setCursor(cur);
                        } else {
                            return;
                        }
                    }
                }
            }
        } else {
            return;
        }

        // pPopup->setTemporaryNodes(temp);
        pPopup->setFilter(filter);
        pPopup->setCompletions(nodes);
        pPopup->popup();
    } else {
        if (trigger == *SEMI_COLON_TRIGGER) {
            for (auto &n : _headerNodes) {
                auto name = n->qualifiedName();
                if (name == fn) {
                    nodes << n;
                }
            }
        } else if (trigger == *LEFT_PARE_TRIGGER) {
            if (r != tokens.rend()) {
                auto pr = std::next(r);
                if (pr->content == *SEMI_COLON_TRIGGER) {
                    if (pr != tokens.rend()) {
                        auto prr = std::next(pr);
                        auto ns = prr->content;
                        if (prr->type == asTC_IDENTIFIER) {
                            for (auto &n : _headerNodes) {
                                auto name = n->qualifiedName();
                                if (name == ns) {
                                    nodes << n;
                                }
                            }
                        } else {
                            return;
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
            qDebug("completion failed");
        }
    }
}

void AsCompletion::setEditor(QEditor *e) {
    QCodeCompletionEngine::setEditor(e);
    pPopup->setEditor(e);
}
