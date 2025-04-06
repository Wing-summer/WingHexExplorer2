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

#include "asdatabase.h"
#include "class/aspreprocesser.h"
#include "class/qascodeparser.h"
#include "model/codecompletionmodel.h"
#include "wingcodeedit.h"

#include <QAbstractItemView>
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

AsCompletion::AsCompletion(asIScriptEngine *engine, WingCodeEdit *p)
    : WingCompleter(p), parser(engine), _engine(engine), m_parseDocument(true) {
    Q_ASSERT(engine);

    setTriggerList({*DOT_TRIGGER, *DBL_COLON_TRIGGER,
                    // unleash the power of call tips
                    *LEFT_PARE_TRIGGER,
                    // clear the tips
                    *SEMI_COLON_TRIGGER});
    setTriggerAmount(3);

    connect(this, QOverload<const QModelIndex &>::of(&AsCompletion::activated),
            this, [this](const QModelIndex &index) {
                auto v = index.data(Qt::SelfDataRole).value<CodeInfoTip>();
                if (v.type == CodeInfoTip::Type::Function ||
                    v.type == CodeInfoTip::Type::ClsFunction) {
                    emit onFunctionTip(v.getTooltip());
                }
            });

    p->installEventFilter(this);
}

AsCompletion::~AsCompletion() {}

void AsCompletion::applyEmptyNsNode(QList<CodeInfoTip> &nodes) {
    static QList<CodeInfoTip> emptyNsNodes;
    if (emptyNsNodes.isEmpty()) {
        auto &hn = parser.headerNodes();
        for (auto p = hn.constKeyValueBegin(); p != hn.constKeyValueEnd();
             ++p) {
            if (Q_UNLIKELY(p->first.isEmpty())) {
                emptyNsNodes.append(p->second);
            } else {
                CodeInfoTip tip;
                tip.type = CodeInfoTip::Type::Group;
                tip.name = p->first;
                emptyNsNodes.append(tip);
            }
        }
        emptyNsNodes.append(parser.keywordNodes());
    }
    nodes.append(emptyNsNodes);
}

void AsCompletion::applyClassNodes(QList<CodeInfoTip> &nodes) {
    static QList<CodeInfoTip> clsNodes;
    if (clsNodes.isEmpty()) {
        auto &hn = parser.headerNodes();
        for (auto &n : hn) {
            for (auto &item : n) {
                if (item.type == CodeInfoTip::Type::Class) {
                    for (auto &c : item.children) {
                        if (c.type == CodeInfoTip::Type::Function) {
                            if (!c.addinfo.contains(CodeInfoTip::RetType)) {
                                continue;
                            }
                        }
                        clsNodes.append(c);
                    }
                }
            }
        }
    }
    nodes = clsNodes;
}

bool AsCompletion::parseDocument() const { return m_parseDocument; }

void AsCompletion::setParseDocument(bool newParseDocument) {
    m_parseDocument = newParseDocument;
}

void AsCompletion::clearFunctionTip() { emit onFunctionTip({}); }

QString AsCompletion::wordSeperators() const {
    static QString eow(QStringLiteral("~!@#$%^&*()_+{}|\"<>?,/;'[]\\-="));
    return eow;
}

void AsCompletion::processTrigger(const QString &trigger,
                                  const QString &content) {
    if (content.isEmpty()) {
        return;
    }

    if (trigger == *SEMI_COLON_TRIGGER) {
        clearFunctionTip();
        return;
    }

    auto len = content.length();
    auto code = content.toUtf8();

    QList<CodeInfoTip> nodes;

    if (m_parseDocument) {
        nodes.append(parseDocument());
    }

    if (!trigger.isEmpty() && trigger != *DOT_TRIGGER) {
        clearFunctionTip();
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

    auto getNamespace = [](const QVector<Token> &tokens) -> QString {
        auto rbegin = tokens.rbegin();
        auto rend = tokens.rend();

        QStringList nss;
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
        return nss.join(QStringLiteral("::"));
    };

    QByteArray fn;
    if (tokens.isEmpty()) {
        popup()->hide();
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
            if (etoken.content == *DBL_COLON_TRIGGER) {
                processTrigger(*DBL_COLON_TRIGGER, content.left(etoken.pos));
                setCompletionPrefix(prefix);
                return;
            } else if (etoken.content == *DOT_TRIGGER) {
                processTrigger(*DOT_TRIGGER, content.left(etoken.pos));
                setCompletionPrefix(prefix);
                return;
            } else {
                applyEmptyNsNode(nodes);
            }
        } else if (etoken.type != asTC_IDENTIFIER) {
            popup()->hide();
            return;
        }

        if (trigger == *DOT_TRIGGER) {
            applyClassNodes(nodes);
        } else if (etoken.content.length() >= triggerAmount()) {
            // completion for a.b.c or a::b.c or a::b::c.d or ::a::b.c
            if (trigger == *DBL_COLON_TRIGGER) {
                auto ns = getNamespace(tokens);
                auto idx = tokens.length() - ns.length() * 2;
                if (idx >= 0) {
                    if (tokens.at(idx).content == *DOT_TRIGGER) {
                        popup()->hide();
                        return;
                    }
                }
                nodes = parser.headerNodes().value(ns);
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
                        popup()->hide();
                        return;
                    }
                }
                nodes = parser.headerNodes().value(ns);
                if (nodes.isEmpty()) {
                    applyEmptyNsNode(nodes);
                }
            }
        }
    }

    setModel(new CodeCompletionModel(nodes, this));
    setCompletionPrefix(prefix);
}

QList<CodeInfoTip> AsCompletion::parseDocument() {
    auto editor = qobject_cast<WingCodeEdit *>(widget());
    if (editor == nullptr) {
        return {};
    }

    auto code = editor->toPlainText();

    // first preprocess the code
    AsPreprocesser prepc(_engine);
    // TODO: set include callback
    // prepc.setIncludeCallback();

    auto r = prepc.loadSectionFromMemory(QStringLiteral("ASCOMPLETION"),
                                         code.toUtf8());
    if (r <= 0) {
        return {};
    }

    auto data = prepc.scriptData();
    QList<CodeInfoTip> ret;

    for (auto &d : data) {
        QAsCodeParser parser(_engine);
        auto syms =
            parser.parseAndIntell(editor->textCursor().position(), d.script);

        for (auto &sym : syms) {
            CodeInfoTip tip;
            tip.name = sym.name;
            tip.nameSpace = QString::fromUtf8(sym.scope.join("::"));

            switch (sym.symtype) {
            case QAsCodeParser::SymbolType::Function:
                tip.type = CodeInfoTip::Type::Function;
                tip.addinfo.insert(CodeInfoTip::RetType,
                                   QString::fromUtf8(sym.type));
                tip.addinfo.insert(CodeInfoTip::Args,
                                   QString::fromUtf8(sym.additonalInfo));
                break;
            case QAsCodeParser::SymbolType::Enum:
                tip.type = CodeInfoTip::Type::Enum;
                break;
            case QAsCodeParser::SymbolType::Variable:
            case QAsCodeParser::SymbolType::Class:
            case QAsCodeParser::SymbolType::TypeDef:
            case QAsCodeParser::SymbolType::FnDef:
            case QAsCodeParser::SymbolType::VarsDecl:
            case QAsCodeParser::SymbolType::Invalid:
                continue;
            }

            ret.append(tip);
        }
    }

    return ret;
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
