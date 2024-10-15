#include "qasparser.h"

#include "qcodemodel.h"
#include "qcodenode.h"
#include "qcodestream.h"

#include "qaslexer.h"

#include <QFileInfo>
#include <QLatin1String>
#include <QMessageBox>
#include <QStack>
#include <QTextStream>

//#define _TRACE_PARSING_

QByteArray join(const QList<QByteArray> &l, QByteArray glue, int max = -1) {
    int n = 0;
    QByteArray out;
    bool check = max != -1;

    foreach (const QByteArray &item, l) {
        if (check && (++n > max))
            break;

        out += item;
        out += glue;
    }

    if (out.length())
        out.chop(glue.length());

    return out;
}

static void prettify(QByteArray &s) {
    bool pwrd = false;

    for (int i = 0; i < s.length(); ++i) {
        char prev = i ? s.at(i - 1) : 0;
        char next = (i + 1) < s.length() ? s.at(i + 1) : 0;
        bool cwrd = (i + 1) < s.length()
                        ? (isalnum(s.at(i + 1)) || (s.at(i) == '_'))
                        : false;

        if ((s.at(i) == ' ') && !(pwrd && cwrd) && (next != '=') &&
            (prev != '*') && (prev != '=') && (prev != '&')) {
            s.remove(i, 1);
            --i;
        }

        pwrd = (i == -1) ? false : (isalnum(s.at(i)) || (s.at(i) == '_'));
    }
}

static QAsParser::KeywordId keyword(const char *s);

QAsParser::QAsParser() : QCodeParser() {}

QAsParser::~QAsParser() {}

QString QAsParser::language() const { return "AngelScript"; }

bool QAsParser::canParse(const QString &fn) const {
    QString x = QFileInfo(fn).suffix();

    return (x == "h") || (x == "hpp") || (x == "hxx");
}

bool isReserved(const QToken &tok) {
    static QTokenList reserved;

    if (reserved.isEmpty()) {
        reserved << "auto"
                 << "bool"
                 << "break"
                 << "case"
                 << "catch"
                 << "char"
                 << "class"
                 << "const"
                 << "continue"
                 << "do"
                 << "default"
                 << "delete"
                 << "double"
                 << "else"
                 << "enum"
                 << "explicit"
                 << "extern"
                 << "float"
                 << "for"
                 << "friend"
                 << "goto"
                 << "if"
                 << "inline"
                 << "int"
                 << "long"
                 << "mutable"
                 << "namespace"
                 << "new"
                 << "operator"
                 << "private"
                 << "protected"
                 << "public"
                 << "register"
                 << "return"
                 << "short"
                 << "signed"
                 << "static"
                 << "struct"
                 << "switch"
                 << "sizeof"
                 << "template"
                 //	<< "this"
                 << "throw"
                 << "try"
                 << "typedef"
                 << "typename"
                 << "union"
                 << "unsigned"
                 << "using"
                 << "virtual"
                 << "void"
                 << "while";
    }

    return reserved.contains(tok);
}

bool maybeIdentifier(const QToken &tok) {
    bool isId = true;

    if (isId) {
        for (int idx = 0; idx < tok.length(); idx++) {
            char c = tok.at(idx);

            isId &= isalnum(c) || (c == '_');
        }
    }

    return isId;
}

bool isIdentifier(const QToken &tok) {
    return maybeIdentifier(tok) && !isReserved(tok);
}

template <typename T>
static void flush(T &buffer, QList<T> &data) {
    if (buffer.length()) {
        data << buffer;
        buffer.clear();
    }
}

static void match(const QTokenList &list, int &l, int l_end, char beg, char end,
                  QTokenList endCond = QTokenList()) {
    int count = 0;

    --l;

    while (++l < l_end) {
        QToken t = list.at(l);
        char tc = t.at(0);

        if (endCond.contains(t)) {
            break;
        } else if (t.length() != 1) {
            continue;
        } else if (tc == beg) {
            ++count;
        } else if (tc == end) {
            --count;

            if (count <= 0)
                break;
        }
    }
}

static void dump(QTokenList &l) {
    QToken dump_array;

    for (const QToken &ba : l)
        dump_array += " " + ba;

    l.clear();

#ifdef _TRACE_PARSING_
    qDebug("buffer dump : {%s}", dump_array.constData());
#endif
}

inline bool filter(int &i, int end) { return i < end; }

QCodeLexer *QAsParser::lexer(QCodeStream *s) { return new QAsLexer(s); }

/*!
        \brief Updates the content of a code node according to a code source.
        This function can be very handy when dealing with large set of source
        data whose representation shall be dynamically modified when the source
        itself changes. However it is <b>*VERY IMPORTANT*</b> to provide the
        relevant source and only it...
*/
void QAsParser::update(QCodeNode *n, QCodeLexer *l, bool check) {
    if (!n || !l)
        return;

    sContext = l->contextFile();

#ifdef _TRACE_PARSING_
    qDebug("updating context %s in %s", qPrintable(sContext),
           n->role(QCodeNode::Name).constData());
#endif

    if (true) {
        QByteArray cxt = sContext.toLocal8Bit();

        QStack<QCodeNode *> nodes;
        nodes.push(n);

        while (nodes.count()) {
            QCodeNode *node = nodes.pop();
            int t = node->type();

            // qDebug("  * %s, context : %s",
            // node->role(QCodeNode::Name).constData(),
            // node->context().constData());

            if (node->context() == cxt) {
                if (!check) {
                    // qDebug("skipping duplicate cxt %s", cxt.constData());
                    return;
                }

                // qDebug("removed node %s (in {0x%x, 0x%x})",
                // node->role(QCodeNode::Name).constData(), node->parent,
                // node->model);

                // node->clear();
                node->detach();

                if (node != n) {
                    // qDebug("deleted node %s",
                    // node->role(QCodeNode::Name).constData());

                    delete node;
                }
            } else if ((t == QCodeNode::Group) || (t == QCodeNode::Language) ||
                       (t == QCodeNode::Namespace)) {

                foreach (QCodeNode *child, node->children) {
                    if (child)
                        nodes.push(child);
                }
            }
        }
        // qDebug("updating cxt %s", cxt.constData());
    } else {
#ifdef _TRACE_PARSING_
        qWarning("no check...");
#endif
    }

    QTokenList data = l->tokens();

#ifdef _TRACE_PARSING_
    /*	QTextStream out(stdout);

            out << "Tokens after filtering..." << endl << endl;

            foreach ( const QToken& token, data )
                    out << token << endl;
    */
#endif

    update(n, l, data, 0, data.count());
}

/*!
        \brief Updates a
*/
void QAsParser::update(QCodeNode *n, QCodeLexer *l, QTokenList &tokens, int id,
                       int end, bool bNeedCxt, QTokenList *ns) {
    if (!n || id == end) {
#ifdef _TRACE_PARSING_
        qWarning("QCppParser::update() => bad input...");
#endif

        return;
    }

    QByteArray sContext = this->sContext.toLocal8Bit();

    int line = l ? 0 : -1;
    int type = n->type();
    QCodeNode *pScope = n;
    QStack<QCodeNode *> nest;
    QTokenList buffer_unused, toSkip, ltemplates;
    int first, last = id;
    QCodeNode::NodeVisibility visibility = QCodeNode::VISIBILITY_DEFAULT;

    if (type == QCodeNode::Class) {
        visibility = QCodeNode::VISIBILITY_PRIVATE;
    } else if ((type == QCodeNode::Enum) || (type == QCodeNode::Namespace)) {
        visibility = QCodeNode::VISIBILITY_PUBLIC;
    }

    while (id < end) {
        // if ( !filter(i, end) )
        //	break;

        QToken token = tokens.at(id);
        KeywordId k = keyword(token.constData());

        if (l)
            line = l->lineForToken(id, line);

        // qDebug("\"%s\"", token.constData());

        switch (k) {
        case KEYWORD_CLASS: {
            if (ltemplates.count() == 1 && ltemplates.at(0).isEmpty()) {
                // specialization : discard
                buffer_unused.clear();

                do {
                    ++id;
                } while (id < end && tokens.at(id) != "{");

                break;
            }

            last = id;

            while (filter(++id, end)) {
                if (tokens.at(id) == ":" || tokens.at(id) == "{" ||
                    tokens.at(id) == ";")
                    break;
            }

            if (id >= end || tokens.at(id) == ";") {
                if ((id - last) == 2) {
#ifdef _TRACE_PARSING_
                    qDebug("forward declaration of %s.",
                           tokens.at(id - 1).constData());
#endif
                    last = id + 1;
                } else {
#ifdef _TRACE_PARSING_
                    qDebug("twisted decl.");
#endif
                    id = ++last;
                }

                buffer_unused.clear();
                break;
            }

            if (buffer_unused.count()) {
                qWarning(
                    "Syntax error : expecting \';\' before \"class\" in %s",
                    sContext.constData());
                dump(buffer_unused);
                ++id;
                break;
            }

            QToken name;

            if ((id - last) > 1) {
                if ((id - last) > 2 && tokens.at(id - 1) == ">") {
#ifdef _TRACE_PARSING_
                    qWarning("skipped %s", (tokens.at(id - 1) == ">")
                                               ? "template specialization"
                                               : "malformed class decl");
#endif
                    break;
                }

                name = tokens.at(id - 1);

                if (!isIdentifier(name)) {
                    qWarning("Syntax error : class name is a reserved keyword");
                    break;
                }

            } else {
                name = "<anonymous>";
            }

            QByteArray roles = "c@";

#ifdef _TRACE_PARSING_
            qDebug("%s %s", (k == KEYWORD_STRUCT) ? "struct" : "class",
                   name.constData());
#endif

            roles += name;
            roles += "@";

            // QList<QByteArray> ancestors;
            QByteArray rule, drule = "private";

            if (tokens.at(id) == ":") {
                while (filter(++id, end)) {
                    if (tokens.at(id) == "{")
                        break;
                    // else if ( tokens.at(id) == "," )
                    //	continue;

                    if (tokens.at(id) == "public") {
                        ++id;
                        rule = "public";
                    } else if (tokens.at(id) == "protected") {
                        ++id;
                        rule = "protected";
                    } else if (tokens.at(id) == "private") {
                        ++id;
                        rule = "private";
                    } else {
                        rule = drule;
                    }

                    if (id >= end || tokens.at(id) == "{")
                        break;

                    rule += " " + tokens.at(id);
                    int tpl_nest = 0;

                    while (++id < end) {
                        if (tokens.at(id) == "{") {
                            break;
                        } else if (!tpl_nest && tokens.at(id) == ",") {
                            rule += tokens.at(id);
                            ++id;
                            break;
                        } else {
                            rule += " " + tokens.at(id);

                            if (tokens.at(id) == "<")
                                ++tpl_nest;
                            else if (tokens.at(id) == ">")
                                --tpl_nest;
                        }
                    }

                    // ancestors << rule;

                    prettify(rule);

#ifdef _TRACE_PARSING_
                    qDebug("ancestor : %s", rule.constData());
#endif

                    roles += rule;

                    if (tokens.at(id) == "{")
                        break;
                }
            }

            // room for friends...
            roles += "@";

            // templates
            roles += "@";

            if (ltemplates.count()) {
                // qDebug("class %s", name.constData());
                roles += ::join(ltemplates, "$");
                ltemplates.clear();
            }

            // context
            if (bNeedCxt)
                roles += "@" + sContext;

            // TODO : this is an update func, check for existing node instead
            // or clear all nodes before processing code...

            QCodeNode *c = getNode();
            c->line = line;
            c->roles = roles;

            // if ( l ) qDebug("class at line %i", line);

            c->attach(pScope);

            if (tokens.at(id) == "{") {
                // parse members
                last = id;

                match(tokens, id, end, '{', '}');

                if (id >= end)
                    id = end - 1;

#ifdef _TRACE_PARSING_
                qDebug("{");
#endif
                update(c, l, tokens, last + 1, id, false);
#ifdef _TRACE_PARSING_
                qDebug("}");
#endif

                if (tokens.at(id) == "}") {
                    if ((id + 1 >= end) || (tokens.at(id + 1) == ";")) {
                        ++id;
                    } else if (id < end) {
                        // subsequent variable decl
                        buffer_unused.clear();
                        buffer_unused << name;

                        ++id;
                        break;
                    } else {
                        qWarning("Syntax error in class definition : "
                                 "missing semi-colon at end of input in %s",
                                 sContext.constData());
                    }
                } else {
                    qWarning("Syntax error in class definition : "
                             "missing close bracket at end of input in %s",
                             sContext.constData());
                }
            }

            last = ++id;
            buffer_unused.clear();
            break;
        }

        case KEYWORD_ENUM: {
            ltemplates.clear();

            QByteArray name;

            if (((id + 1) < end && tokens.at(id + 1) != "{") &&
                ((id + 2) < end && tokens.at(id + 2) != "{")) {
                // twisted variable declaration...
                ++id;
                break;
            }

            if (tokens.at(id + 1) == "{") {
                // anonymous enum
                name = "<anonymous>";
            } else if (tokens.at(id + 2) == "{") {
                // named enum
                name = tokens.at(++id);
            } else {
                qWarning("syntax error : expected \'{\' after \"enum\" in %s",
                         sContext.constData());
                buffer_unused.clear();
                ++id;
                break;
            }

            ++id;
            first = id;

            match(tokens, id, end, '{', '}', QList<QByteArray>() << ";");

            if (buffer_unused.count()) {
                qWarning("syntax error : expecting \';\' before \"enum\" in %s",
                         sContext.constData());
                dump(buffer_unused);
                break;
            }

#ifdef _TRACE_PARSING_
            qDebug("enum %s", name.constData());
#endif

            QCodeNode *en = getNode();
            en->line = line;
            en->roles =
                QByteArray("e@") + name + "@@" + QByteArray::number(visibility);

            // if ( l ) qDebug("enum at line %i", line);

            if (bNeedCxt)
                en->roles += "@" + sContext;

            bool ok, gettingName = true;
            QByteArray lastVal, eid, val;
            int lastNum = -1, lastValCount = 0;

            while (++first < id) {
                if (tokens.at(first) == "," || tokens.at(first) == "}") {
                    if (eid.isEmpty())
                        continue;

                    if (val.isEmpty()) {
                        if (lastVal.length())
                            val = lastVal + " + " +
                                  QByteArray::number(++lastValCount);
                        else
                            val = QByteArray::number(++lastNum);
                    } else {
                        int num = val.toInt(&ok);

                        if (ok) {
                            lastNum = num;
                            lastVal.clear();
                        } else {
                            lastVal = val;
                            lastValCount = 0;
                        }
                    }

                    QCodeNode *enumerator = getNode();

                    enumerator->roles = QByteArray("r@") + eid + "@" + val;
                    enumerator->attach(en);

#ifdef _TRACE_PARSING_
                    qDebug("  %s = %s", eid.constData(), val.constData());
#endif

                    eid.clear();
                    val.clear();

                    gettingName = true;

                } else if ((tokens.at(first) == "=") && gettingName) {
                    gettingName = false;
                } else if (gettingName) {
                    eid = tokens.at(first);
                } else {
                    if (val.length())
                        val += ' ';

                    val += tokens.at(first);
                }
            }

            if (eid.length()) {
                if (val.isEmpty()) {
                    if (lastVal.length())
                        val = lastVal + " + " +
                              QByteArray::number(++lastValCount);
                    else
                        val = QByteArray::number(++lastNum);
                } else {
                    int num = val.toInt(&ok);

                    if (ok) {
                        lastNum = num;
                        lastVal.clear();
                    } else {
                        lastVal = val;
                        lastValCount = 0;
                    }
                }

                QCodeNode *enumerator = getNode();

                enumerator->roles = QByteArray("r@") + eid + "@" + val;
                enumerator->attach(en);

#ifdef _TRACE_PARSING_
                qDebug("  %s = %s", eid.constData(), val.constData());
#endif
            }

            en->attach(pScope);
            ++id;

            break;
        }

        case KEYWORD_NAMESPACE: {
            ltemplates.clear();

            if (buffer_unused.count()) {
                qWarning(
                    "Syntax error : expecting \';\' before \"namespace\" in %s",
                    sContext.constData());
                buffer_unused.clear();

                break;
            }

            QList<QByteArray> names;

            do {
                ++id;

                names << tokens.at(id);

                ++id;
            } while (id < end && tokens.at(id) == "::");

            if (tokens.at(id) != "{") {
                qDebug(
                    "Syntax error : expected \'{\' after \"namespace\" in %s",
                    sContext.constData());
                break;
            }

            first = id + 1;
            buffer_unused.clear();
            match(tokens, id, end, '{', '}');

#ifdef _TRACE_PARSING_
            qDebug("namespace(s) :");

            foreach (const QByteArray &n, names)
                qDebug("\t%s", n.constData());

// qDebug("namespace : %s", qPrintable(names.join("::")));
#endif

            QCodeNode *p = pScope, *n = 0;

            while (names.count()) {
                foreach (n, p->children) {
                    if ((n->type() == QCodeNode::Namespace) &&
                        (n->role(QCodeNode::Name) == names.first()))
                        break;

                    n = 0;
                }

                QByteArray name = names.takeAt(0);

                if (name.endsWith("Private")) {
// suffixing class/namespaces name is a convention used
// by Qt to kinda hide all dirty code
// for this reason (and mostly because it confused the
// parser when modelizing Qt4 sources) such namespaces are
// skipped...
#ifdef _TRACE_PARSING_
                    qDebug("skipped private namespace %s.", name.constData());
#endif

                    ++id;
                    break;
                }

                if (!n) {
                    n = getNode();
                    n->roles = QByteArray("n@") + name;

                    n->attach(pScope);
                }
            }

#ifdef _TRACE_PARSING_
            qDebug("{");
#endif
            update(n, l, tokens, first, id, true);
#ifdef _TRACE_PARSING_
            qDebug("}");
#endif
            ++id;

            break;
        }

        case KEYWORD_TYPEDEF: {
            ltemplates.clear();

            if (buffer_unused.count()) {
                qWarning(
                    "Syntax error : expecting \';\' before \"typedef\" in %s",
                    sContext.constData());
                dump(buffer_unused);
                ++id;
                break;
            }

            ++id;

            switch (keyword(tokens.at(id))) {
            case KEYWORD_ENUM: {
                ++id;

                QByteArray name;

                if (tokens.at(id) == "{") {
                    // typedef anonymous as something

                    first = id + 1;

                    match(tokens, id, end, '{', '}');

                    last = id;

                    if (id < end && tokens.at(id) != ";")
                        name = tokens.at(id);
                    else
                        ++id;

                } else {
                    // typedef something as itself
                    // => weird construct but can happen...

                    name = tokens.at(id);

                    first = id + 1;

                    // match(tokens, id, end, '{', '}');

                    last = id;
                    ++id;
                }

                // at this point :
                // -> first : first definition elem
                // -> last : closing bracket after last definition elem
                // -> i : end token : should be ';'
                // name : struct/enum/union name

                /*
                if ( !filter(i, end) || tokens.at(id) != ";" )
                {
                        qWarning("expected \';\' after \"typedef\"");
                }
                */

                while ((id < end) && (tokens.at(id) != ";"))
                    ++id;

                break;
            }

            default:
                // regular typedef
                QByteArray name, alias;

                while ((id < (end - 1)) && (tokens.at(id + 1) != ";")) {
                    if (alias.length())
                        alias += ' ';

                    alias += tokens.at(id);
                    ++id;
                }

                name = tokens.at(id);

                if (name == ")") {
                    // special case : pointer to function
                    break;
                }

#ifdef _TRACE_PARSING_
                qDebug("typedef %s", name.constData());
#endif

                QCodeNode *td = getNode();
                td->line = line;
                td->roles = QByteArray("t@") + name + "@" + alias + "@" +
                            QByteArray::number(visibility);

                // if ( l ) qDebug("typedef at line %i", line);

                if (bNeedCxt)
                    td->roles += "@" + sContext;

                td->attach(pScope);

                break;
            }

            ++id;
            buffer_unused.clear();

            break;
        }

        case KEYWORD_FRIEND: {
            ltemplates.clear();

            if (buffer_unused.count()) {
                qWarning(
                    "Syntax error : expecting \';\' before \"friend\" in %s",
                    sContext.constData());
                dump(buffer_unused);
                ++id;
                break;
            }

#ifdef _TRACE_PARSING_
            qDebug("friendship.");
#endif

            QByteArray friendship_target;

            ++id;

            while ((id < end) && (tokens.at(id) != ";")) {
                if (friendship_target.length())
                    friendship_target += ' ';

                friendship_target += tokens.at(id);
                ++id;
            }

            if (pScope->type() == QCodeNode::Class) {
                QByteArray role = pScope->role(QCodeNode::Friends);

                if (role.length())
                    role += "#";

                prettify(friendship_target);

                role += friendship_target;

                pScope->setRole(QCodeNode::Friends, role);
            }

            if (id != end)
                ++id;

            break;
        }

        case KEYWORD_USING: {
            ltemplates.clear();

            if (buffer_unused.count()) {
                qWarning(
                    "Syntax error : expecting \';\' before \"using\" in %s",
                    sContext.constData());
                dump(buffer_unused);

                while ((id < end) && (tokens.at(id) != ";"))
                    ++id;

                break;
            }

#ifdef _TRACE_PARSING_
            qDebug("using directive.");
#endif

            if (ns && ((id + 1) < end) && (tokens.at(id + 1) == "namespace")) {
                id += 2;
                QByteArray tmpns;

                while ((id < end) && (tokens.at(id) != ";")) {
                    tmpns += tokens.at(id);
                    ++id;
                }

                ns->append(tmpns);
            } else {
                while ((id < end) && (tokens.at(id) != ";"))
                    ++id;
            }

            if (id < end)
                ++id;

            break;
        }

        case KEYWORD_TEMPLATE: {
            ltemplates.clear();

            if (buffer_unused.count()) {
                qWarning(
                    "Syntax error : expecting \';\' before \"template\" in %s",
                    sContext.constData());
                dump(buffer_unused);

                match(tokens, ++id, end, '<', '>');
                ++id;
                break;
            }

#ifdef _TRACE_PARSING_
            qDebug("template directive.");
#endif

            QToken buffer;
            int tmp = ++id;

            match(tokens, tmp, end, '<', '>');

            if (tmp == end)
                break;

            int nest = 0;
            bool skipFirst = true;

            while (id != tmp) {
                ++id;

                if (tokens.at(id) == "<") {
                    ++nest;
                } else if (tokens.at(id) == ">") {
                    if (nest) {
                        --nest;
                    } else {
#ifdef _TRACE_PARSING_
                        qDebug("  %s", buffer.constData());
#endif

                        ltemplates << buffer;
                        buffer.clear();
                        break;
                    }
                } else if (!nest && (tokens.at(id) == ",")) {
#ifdef _TRACE_PARSING_
                    qDebug("  %s", buffer.constData());
#endif

                    ltemplates << buffer;
                    buffer.clear();
                    skipFirst = true;
                } else if (skipFirst) {
                    skipFirst = false;
                } else {
                    if (buffer.length())
                        buffer += " ";

                    buffer += tokens.at(id);
                }
            }

            id = ++tmp;
            break;
        }

        case KEYWORD_EXTERN: {
            ltemplates.clear();

            if (buffer_unused.count()) {
                qWarning(
                    "Syntax error : expecting \';\' before \"extern\" in %s",
                    sContext.constData());
                dump(buffer_unused);

                ++id;
                break;
            }

#ifdef _TRACE_PARSING_
            qDebug("extern.");
#endif

            if ((id + 1) < end && tokens.at(id + 1).startsWith("\"")) {
                // extern "C/C++/..." { ... }
                ++id;

                last = id + 1;
                match(tokens, last, end, '{', '}');

                if (tokens.at(id) == "\"C\"" || tokens.at(id) == "\"C++\"") {
                    update(pScope, l, tokens, id + 2, last, true);
                }

                id = last + 1;

            } else {
                // extern variable/function...
                ++id;
            }

            break;
        }

        default: {
            if (token.endsWith("_EXPORT") || token.contains("_EXPORT_")) {
#ifdef _TRACE_PARSING_
                qDebug("skipped annoying export token : %s", token.constData());
#endif

                ++id;
                continue;
            } else if (toSkip.contains(token)) {
#ifdef _TRACE_PARSING_
                qDebug("skipped problematic token : %s", token.constData());
#endif

                ltemplates.clear();
                buffer_unused.clear();

                ++id;
                continue;
            } else if (token == "{") {

                ltemplates.clear();
                // buffer_unused.clear();

                if (buffer_unused.count() && buffer_unused.last() == "=") {
#ifdef _TRACE_PARSING_
                    qDebug("skipped problematic data block");
#endif
                    buffer_unused.removeLast();
                } else {
#ifdef _TRACE_PARSING_
                    qDebug("skipped problematic code block");
#endif
                    buffer_unused.clear();
                }

                match(tokens, id, end, '{', '}');
                ++id;

                continue;

            } else if (token == ";") {
                if (buffer_unused.count()) {
                    if (keyword(buffer_unused.last()) == KEYWORD_NONE) {
                        QByteArray type;

                        int templateCount = 0;

                        QTokenList::const_iterator
                            ba = buffer_unused.constBegin(),
                            buffer_end = buffer_unused.constEnd() /*,
                             buffer_beg = buffer_unused.constBegin()*/
                            ;

                        QCodeNode::TypeSpecifier ts = QCodeNode::SPECIFIER_NONE;

                        int nodeType = pScope->type();

                        bool cont = false,
                             inClass = (nodeType == QCodeNode::Class);

                        if (!inClass) {
                            if (*ba == "extern")
                                ts |= QCodeNode::SPECIFIER_EXTERN;
                            else if (*ba == "auto")
                                ts |= QCodeNode::SPECIFIER_AUTO;
                            else if (*ba == "const")
                                ts |= QCodeNode::SPECIFIER_CONST;
                            else
                                --ba;
                        } else {
                            if (*ba == "const")
                                ts |= QCodeNode::SPECIFIER_CONST;
                            else
                                --ba;
                        }

                        ++ba;

                        if (ba >= buffer_end) {
                            buffer_unused.clear();
                            break;
                        }

                        do {
                            if (*ba == "<") {
                                cont = true;
                                ++templateCount;
                            } else if (*ba == ">") {
                                --templateCount;
                                cont = templateCount;
                            } else if ((*ba == "::") ||
                                       (((ba + 1) < buffer_end) &&
                                        *(ba + 1) == "::") ||
                                       (((ba + 1) < buffer_end) &&
                                        *(ba + 1) == "<")) {
                                cont = true;
                            } else if (inClass &&
                                       (*ba == "const" || *ba == "volatile")) {
                                cont = true;
                            } else if (*ba == "unsigned" || *ba == "signed") {
                                cont = true;
                            } else if (*ba == "long" &&
                                       ((((ba + 1) < buffer_end) &&
                                         *(ba + 1) == "int") ||
                                        (((ba + 1) < buffer_end) &&
                                         *(ba + 1) == "long") ||
                                        (((ba + 1) < buffer_end) &&
                                         *(ba + 1) == "double"))) {
                                cont = true;
                            } else {
                                cont = templateCount;
                            }

                            if (type.length())
                                type += " ";

                            type += *ba;

                        } while (++ba < buffer_end && cont);

                        --ba;
                        QByteArray tmp;
                        QByteArray tok;
                        bool b_val_skip = false;

                        while (++ba < buffer_end) {
                            if (*ba == "::" || *ba == "<") {
                                // syntax pitfall...
                                break;
                            } else if (*ba == "=" || *ba == ":" || *ba == "(") {
                                // ctor/init/assignment
                                int assign_nest = 0;

                                do {
                                    if (*ba == "<" || *ba == "(")
                                        ++assign_nest;
                                    else if (*ba == ">" || *ba == ")")
                                        --assign_nest;

                                    ++ba;
                                } while ((ba + 1) < buffer_end &&
                                         (*ba != "," || assign_nest) &&
                                         *ba != ";");

                                tok = ",";
                            } else if ((ba + 1) == buffer_end) {
                                if (!b_val_skip) {
                                    if (tmp.isEmpty())
                                        tmp += *ba;
                                    else
                                        tmp += " " + *ba;
                                }

                                tok = ",";
                            } else {
                                tok = *ba;
                            }

                            if (tok == ",") {
                                int refs = tmp.count('&');
                                int stars = tmp.count('*');

                                tmp.replace('*', QByteArray());
                                tmp.replace('&', QByteArray());

                                type += QByteArray(stars, '*');
                                type += QByteArray(refs, '&');

                                prettify(type);

#ifdef _TRACE_PARSING_
                                qDebug("%s %s;", type.constData(),
                                       tmp.trimmed().constData());
#endif

                                QCodeNode *var = getNode();
                                var->line = line;
                                var->roles = QByteArray("v@") + tmp.trimmed() +
                                             "@" + type + "@" +
                                             QByteArray::number(visibility) +
                                             "@" + QByteArray::number(ts);

                                // if ( l ) qDebug("variable at line %i", line);

                                if (bNeedCxt)
                                    var->roles += "@" + sContext;

                                var->attach(pScope);

                                tmp.clear();
                                b_val_skip = false;
                            } else if (b_val_skip) {
                            } else if (tok == "=") {
                                b_val_skip = true;
                            } else if (tmp.isEmpty()) {
                                tmp += tok;
                            } else {
                                tmp += " " + tok;
                            }
                        }
                    }

                    ltemplates.clear();
                    buffer_unused.clear();
                }
            } else if (buffer_unused.count()) {

                int equ_idx = buffer_unused.indexOf("=");
                bool assign = equ_idx != -1;

                if (assign && buffer_unused.count() >= 1)
                    assign = equ_idx != buffer_unused.count() - 1;

                if (assign && buffer_unused.count() >= 2)
                    assign = buffer_unused.at(buffer_unused.count() - 2) !=
                             "operator";

                if (token == "(") {
                    // make sure we don't mistake var decl fro func decl...
                    first = last = id;
                    match(tokens, last, end, '(', ')',
                          QList<QByteArray>() << ";");

                    // variable decl with ctor (most probably...)
                    if (last == end || (last + 1) == end ||
                        tokens.at(last + 1) == "," || tokens.at(last) == ";")
                        assign = true;
                }

                if (token == ":") {
                    bool visa = false;

                    if (buffer_unused.last() == "slots") {
                        // TODO : slots visibility flag...
                        visa = true;
                        buffer_unused.removeLast();
                    }

                    if (buffer_unused.last() == "public") {
                        visa = true;
                        visibility = QCodeNode::VISIBILITY_PUBLIC;
                    } else if (buffer_unused.last() == "protected") {
                        visa = true;
                        visibility = QCodeNode::VISIBILITY_PROTECTED;
                    } else if (buffer_unused.last() == "private") {
                        visa = true;
                        visibility = QCodeNode::VISIBILITY_PRIVATE;
                    } else {
                        // qWarning("weird syntactic construct in %s",
                        // sContext.constData());
                    }

                    if (visa) {
#ifdef _TRACE_PARSING_
                        qDebug("[visibility changed to : %s]",
                               buffer_unused.last().constData());
#endif

                        buffer_unused.clear();
                    } else {
                        // assumme it's all about in-decl variable init
                    }
                } else if ((token == "(") && ((id + 1) < end) &&
                           (tokens.at(id + 1) != "*") && !assign) {
                    // parse members
                    first = id + 1;

                    // qDebug("looking for matching paren...");

                    match(tokens, id, end, '(', ')',
                          QList<QByteArray>() << ";");

                    if (id == end) {
                        buffer_unused.clear();

                        break;
                    }

                    last = id;
                    ++id;

                    // qDebug("recursive argument parsing");

                    if ((id >= end) // out of range
                        || (tokens.at(last) ==
                            ";") // out of range and/or malformed code
                        || (     // get rid of macros
                               (tokens.at(id) != ";")    // !decl
                               && (tokens.at(id) != "{") // !impl
                               && (tokens.at(id) != ":") // !ctor impl
                               && (buffer_unused.count() == 1)) ||
                        buffer_unused.isEmpty()) {
                        buffer_unused.clear();

                        break;
                    }

                    QCodeNode::FunctionQualifier fq = QCodeNode::QUALIFIER_NONE;

                    int pref_idx = buffer_unused.indexOf("explicit");

                    if (pref_idx != -1)
                        buffer_unused.removeAt(pref_idx);

                    QByteArray name;
                    QByteArray retval;
                    QByteArray arg_buff;
                    QList<QByteArray> args;
                    bool isCtorDtor = false;
                    int idx = buffer_unused.indexOf("operator");

                    if (idx != -1) {
                        name = "operator";

                        buffer_unused.removeAt(idx);

                        if (idx) {
                            while (buffer_unused.count() > idx) {
                                name += ' ';
                                name += buffer_unused.takeAt(idx);
                            }
                        } else {
                            // conversion operator e.g :
                            // operator bool () const;
                            // leave buffer as is so that retval computation
                            // succeeds
                            while (idx < buffer_unused.count()) {
                                name += ' ';
                                name += buffer_unused.at(idx);
                                ++idx;
                            }
                        }
                    } else if (buffer_unused.last() == ">") {
                        int templates = 1;

                        while (buffer_unused.count()) {
                            QByteArray s = buffer_unused.takeLast();

                            name.prepend(s + " ");

                            if (!templates)
                                break;

                            if (s == ">")
                                ++templates;
                            else if (s == "<")
                                --templates;
                        }

                        if (ltemplates.count() == 1 &&
                            ltemplates.at(0).isEmpty()) {
                            // specialization : discard
                            buffer_unused.clear();

                            break;
                        }
                    } else {
                        name = buffer_unused.takeLast();
                    }

                    // maybe ctor or dtor

                    if ((pScope->type() == QCodeNode::Class) &&
                        (name == pScope->role(QCodeNode::Name))) {
                        isCtorDtor = true;

                        if (buffer_unused.count() &&
                            buffer_unused.last() == "~") {
                            name.prepend("~");
                            buffer_unused.removeAt(buffer_unused.count() - 1);
                        }

                        if (buffer_unused.count() &&
                            buffer_unused.at(0) == "abstract") {
                            fq |= QCodeNode::QUALIFIER_ABSTRACT;
                            buffer_unused.removeAt(0);
                        }

                        buffer_unused.clear();
                    }

                    if (!isCtorDtor) {
                        if (buffer_unused.isEmpty()) {
// probably macro stuff...
#ifdef _TRACE_PARSING_
                            qDebug("skipped supposed macro : %s",
                                   name.constData());
#endif
                            break;
                        }

                        // anything else
                        if (buffer_unused.at(0) == "extern")
                            fq |= QCodeNode::QUALIFIER_EXTERN;
                        else
                            goto skip_qualifiers;

                        buffer_unused.removeAt(0);
                    }

                skip_qualifiers:

                    foreach (const QToken &ba, buffer_unused)
                        retval += (retval.length() ? " " + ba : ba);

                    if ((last - first) >= 1) {
                        bool bParamAbort = false;
                        bool bFirstParamTok = true;
                        int tpl_nest = 0;

                        do {
                            if (bFirstParamTok &&
                                !maybeIdentifier(tokens.at(first)) &&
                                tokens.at(first) != "...") {
                                bParamAbort = true;
                                break;
                            }

                            if (!tpl_nest && tokens.at(first) == ",") {
                                prettify(arg_buff);
                                flush(arg_buff, args);
                                bFirstParamTok = true;
                            } else {
                                bFirstParamTok = false;
                                arg_buff += (arg_buff.isEmpty()
                                                 ? tokens.at(first)
                                                 : " " + tokens.at(first));

                                if (tokens.at(first) == "<")
                                    ++tpl_nest;
                                else if (tokens.at(first) == ">")
                                    --tpl_nest;
                            }
                        } while (++first < last);

                        if (bParamAbort) {
                            break;
                        }

                        prettify(arg_buff);
                        flush(arg_buff, args);
                    }

                    while (id < end) {
                        if (tokens.at(id) == ";") {
                            break; // end of decl
                        } else if (tokens.at(id) == ":") {
                            // handle ctor special construct
                            while ((id < end) && (tokens.at(id) != "{"))
                                ++id;

                            match(tokens, id, end, '{', '}'); // end of impl
                            break;
                        } else if (tokens.at(id) == "{") {
                            match(tokens, id, end, '{', '}'); // end of impl
                            break;
                        } else if (tokens.at(id) == "const")
                            fq |= QCodeNode::QUALIFIER_CONST;

                        ++id;
                    }

                    if (!retval.endsWith("::") &&
                        !retval.endsWith(":: ~") // small quirk handling...
                    ) {

                        prettify(name);
                        prettify(retval);

#ifdef _TRACE_PARSING_
                        qDebug("func %s", name.constData());

                        qDebug("(");

                        foreach (const QByteArray &a, args)
                            qDebug("  %s", a.constData());

                        qDebug(")");
                        qDebug("-> %s", retval.constData());

#endif

                        QCodeNode *fct = getNode();
                        fct->line = line;
                        fct->roles = QByteArray("f@") + name + "@" + retval +
                                     "@" + QByteArray::number(visibility) + "@";

                        // if ( l ) qDebug("function at line %i", line);

                        // templates

                        if (ltemplates.count())
                            fct->roles += ::join(ltemplates, "$");

                        fct->roles +=
                            QByteArray("@") + QByteArray::number(fq) + "@";

                        foreach (const QByteArray &a, args)
                            fct->roles += a + ", ";

                        if (args.count())
                            fct->roles.chop(2);

                        if (bNeedCxt)
                            fct->roles += "@" + sContext;

                        fct->attach(pScope);
                    }

                    ltemplates.clear();
                    buffer_unused.clear();

                } else {
                    buffer_unused << token;
                }
            } else {
                buffer_unused << token;
            }

            ++id;
            break;
        }
        }
    }
}

/*
        full table kept for evolutivity but only directly parsed keyword
        have to be "visible"...
*/
static const QLatin1String KeywordTable[] = {
    // keyword
    /*
    QLatin1String("__attribute__"),
    QLatin1String("asm"),
    QLatin1String("auto"),
    QLatin1String("bool"),
    QLatin1String("break"),
    QLatin1String("case"),
    QLatin1String("catch"),
    QLatin1String("char"),
    */
    QLatin1String("class"),

    /*
    QLatin1String("const"),
    QLatin1String("continue"),
    QLatin1String("do"),
    QLatin1String("default"),
    QLatin1String("delete"),
    QLatin1String("double"),
    QLatin1String("else"),
    */

    QLatin1String("enum"),

    // QLatin1String("explicit"),

    QLatin1String("extern"),

    /*
    QLatin1String("float"),
    QLatin1String("for"),
    */

    QLatin1String("friend"),

    /*
    QLatin1String("goto"),
    QLatin1String("if"),
    QLatin1String("inline"),
    QLatin1String("int"),
    QLatin1String("long"),
    QLatin1String("mutable"),
    */

    QLatin1String("namespace"),

    /*
    QLatin1String("new"),
    QLatin1String("operator"),
    QLatin1String("private"),
    QLatin1String("protected"),
    QLatin1String("public"),
    QLatin1String("register"),
    QLatin1String("return"),
    QLatin1String("short"),
    QLatin1String("signed"),
    QLatin1String("static"),
    */

    QLatin1String("struct"),

    /*
    QLatin1String("switch"),
    QLatin1String("sizeof"),
    */

    QLatin1String("template"),

    /*
    QLatin1String("this"),
    QLatin1String("throw"),
    QLatin1String("try"),
    */

    QLatin1String("typedef"),

    // QLatin1String("typename"),

    QLatin1String("union"),

    // QLatin1String("unsigned"),
    QLatin1String("using")

    /*
    ,
    QLatin1String("virtual"),
    QLatin1String("void"),
    QLatin1String("volatile"),
    QLatin1String("wchar_t"),
    QLatin1String("while")
    */
};

static const QAsParser::KeywordId IdTable[] = {
    // keyword ID
    /*
    QCppParser::KEYWORD_ATTRIBUTE,
    QCppParser::KEYWORD_ASM,
    QCppParser::KEYWORD_AUTO,
    QCppParser::KEYWORD_BOOL,
    QCppParser::KEYWORD_BREAK,
    QCppParser::KEYWORD_CASE,
    QCppParser::KEYWORD_CATCH,
    QCppParser::KEYWORD_CHAR,
    */

    QAsParser::KEYWORD_CLASS,

    /*
    QCppParser::KEYWORD_CONST,
    QCppParser::KEYWORD_CONTINUE,
    QCppParser::KEYWORD_DO,
    QCppParser::KEYWORD_DEFAULT,
    QCppParser::KEYWORD_DELETE,
    QCppParser::KEYWORD_DOUBLE,
    QCppParser::KEYWORD_ELSE,
    */

    QAsParser::KEYWORD_ENUM,

    // QCppParser::KEYWORD_EXPLICIT,

    QAsParser::KEYWORD_EXTERN,

    /*
    QCppParser::KEYWORD_FLOAT,
    QCppParser::KEYWORD_FOR,
    */

    QAsParser::KEYWORD_FRIEND,

    /*
    QCppParser::KEYWORD_GOTO,
    QCppParser::KEYWORD_IF,
    QCppParser::KEYWORD_INLINE,
    QCppParser::KEYWORD_INT,
    QCppParser::KEYWORD_LONG,
    QCppParser::KEYWORD_MUTABLE,
    */

    QAsParser::KEYWORD_NAMESPACE,

    /*
    QCppParser::KEYWORD_NEW,
    QCppParser::KEYWORD_OPERATOR,
    QCppParser::KEYWORD_PRIVATE,
    QCppParser::KEYWORD_PROTECTED,
    QCppParser::KEYWORD_PUBLIC,
    QCppParser::KEYWORD_REGISTER,
    QCppParser::KEYWORD_RETURN,
    QCppParser::KEYWORD_SHORT,
    QCppParser::KEYWORD_SIGNED,
    QAsParser::KEYWORD_STRUCT,
    */

    /*
    QCppParser::KEYWORD_SWITCH,
    QCppParser::KEYWORD_SIZEOF,
    */

    QAsParser::KEYWORD_TEMPLATE,

    /*
    QCppParser::KEYWORD_THIS,
    QCppParser::KEYWORD_THROW,
    QCppParser::KEYWORD_TRY,
    */

    QAsParser::KEYWORD_TYPEDEF,

    // QCppParser::KEYWORD_TYPENAME,

    // QCppParser::KEYWORD_UNSIGNED,

    QAsParser::KEYWORD_USING,

    /*
    QCppParser::KEYWORD_VIRTUAL,
    QCppParser::KEYWORD_VOID,
    QCppParser::KEYWORD_VOLATILE,
    QCppParser::KEYWORD_WCHAR_T,
    QCppParser::KEYWORD_WHILE
    */
};

Q_DECL_UNUSED static const int ids = sizeof(IdTable) / sizeof(IdTable[0]);

static const int keywords = sizeof(KeywordTable) / sizeof(KeywordTable[0]);

static const QLatin1String *invalid_match = KeywordTable + keywords;

static QAsParser::KeywordId keyword(const char *s) {
    // if ( keywords != ids )
    //	qFatal("QCodeParser::update() : misconstructed keyword hash...");

    const QLatin1String *match =
        std::lower_bound(KeywordTable, invalid_match, s);

    return (match == invalid_match) ? QAsParser::KEYWORD_NONE
                                    : IdTable[match - KeywordTable];
}
