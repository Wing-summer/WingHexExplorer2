#include "ascompletion.h"

#include "class/qaslexer.h"
#include "class/qasparser.h"
#include "qcalltip.h"
#include "qdocumentcursor.h"
#include "qdocumentline.h"

#include "qcodemodel.h"
#include "qcodenode.h"
#include "qcodeserializer.h"

// #include "plugin.h"

#include <QDir>
#include <QLibraryInfo>
#include <QQueue>
#include <QTextStream>
#include <QTime>
#include <QTimer>
#include <QtDebug>

//#define TRACE_COMPLETION

/*
        TODO :
                - better support for template methods based upon parameters ?
                - add support for some more operators ?
                - improve visibility guesses by taking friendship into account
                - improve visibility guesses by taking inheritance visibility
   into account
                - improve visibility guesses by taking context hierarchy into
   account
*/

static void remove(QByteArray &b, char c, int i = 0) {
    i = qMax(i, 0);
    while (i < b.length()) {
        int len = 0;

        while (((i + len) < b.length()) && (b.at(i + len) == c))
            ++len;

        if (!len)
            ++i;
        else
            b.remove(i, len);
    }
}

static void remove(QByteArray &b, const char *str, int i = 0) {
    i = qMax(i, 0);
    auto length = qstrlen(str);

    while (i < b.length()) {
        if (!qstrncmp(b.constData() + i, str, length))
            b.remove(i, length);
        else
            ++i;
    }
}

static bool isWord(char c) { return isalnum(c) || (c == '_'); }

static void substitute(QByteArray &b, const QByteArray &tpl,
                       const QByteArray &crt) {
    // qDebug() << "substituting " << tpl << " with " << crt << " in " << b;
    if (b.isEmpty() || tpl.isEmpty())
        return;

    int i = 0;
    bool _word = false;

    while (i < b.length()) {
        int j = i + tpl.length();
        if (!_word &&
            !qstrncmp(b.constData() + i, tpl.constData(), tpl.length()) &&
            ((j >= b.length()) || !isWord(b.at(j)))) {
            b.replace(i, tpl.length(), crt);
            i += crt.length();
        } else {
            ++i;
        }

        _word = i ? isWord(b.at(i - 1)) : false;
    }

    // qDebug() << b;
}

static void blockRemove(QByteArray &b, char in, char out) {
    int i = 0, s;

    while (i < b.length()) {
        if (b.at(i) == in) {
            s = i;

            int count = 0;

            do {
                if (b.at(i) == in)
                    ++count;
                else if (b.at(i) == out)
                    --count;

                ++i;

            } while (count && (i < b.length()));

            b.remove(s, i - s + 1);
            i = s;
        } else {
            ++i;
        }
    }
}

static QList<QByteArray> split(const QByteArray &b, const char *str, char in,
                               char out) {
    int i = 0, last = 0;
    QList<QByteArray> l;
    const int length = qstrlen(str);

    while (i < b.length()) {
        if (b.at(i) == in) {
            int count = 0;

            do {
                if (b.at(i) == in)
                    ++count;
                else if (b.at(i) == out)
                    --count;

                ++i;

            } while (count && (i < b.length()));

        } else if (!qstrncmp(b.constData() + i, str, length)) {
            l << QByteArray(b.constData() + last, i - last);
            i += length;
            last = i;
        } else {
            ++i;
        }
    }

    if (i > last)
        l << QByteArray(b.constData() + last, i - last);

    // qDebug("split \"%s\" at \"%s\" => %s", b.constData(), str,
    // QModel::join(l, ",").constData());

    return l;
}

static QList<QByteArray> split(const QByteArray &b, const char *str) {
    int i = 0, last = 0;
    QList<QByteArray> l;
    const int length = qstrlen(str);

    while (i < b.length()) {
        if (!qstrncmp(b.constData() + i, str, length)) {
            l << QByteArray(b.constData() + last, i - last);
            i += length;
            last = i;
        } else {
            ++i;
        }
    }

    if (i > last)
        l << QByteArray(b.constData() + last, i - last);

    // qDebug("split \"%s\" at \"%s\" => %s", b.constData(), str,
    // QModel::join(l, ",").constData());

    return l;
}

/*
static int contextBound(const QToken& unscoped, const QTokenList& context)
{
        int i = 0, max = context.length();
        QTokenList l = split(unscoped, "::", '<', '>');

        while ( (max > 0) && (i < l.length()) )
        {
                if ( context.at(max - 1) == l.at(i) )
                {
                        --max;
                        ++i;
                } else {
                        break;
                }
        }

//	qDebug() << "max scoping between " << l << " and " << context << " is "
<< max;

        return max;
}
*/

extern QByteArray join(const QList<QByteArray> &l, QByteArray glue,
                       int max = -1);

QCodeCompletionBackend *AsCompletion::pBackend = 0;
unsigned long AsCompletion::instances = 0;

AsCompletion::AsCompletion(QObject *p)
    : QCodeCompletionEngine(p), pPopup(0), pModel(0) {
    if (!pBackend)
        pBackend = new QCodeCompletionBackend;

    ++instances;

    addTrigger(".");
    addTrigger("->");
    addTrigger("::");

    // unleash the power of call tips
    addTrigger("(");

    // EDYUK_SHORTCUT(triggerAction(), "C++ completion", tr("Ctrl+Space"));
}

AsCompletion::AsCompletion(QCodeModel *m, QObject *p)
    : QCodeCompletionEngine(p), pPopup(0), pModel(m) {
    if (!pBackend)
        pBackend = new QCodeCompletionBackend;

    ++instances;

    addTrigger(".");
    addTrigger("->");
    addTrigger("::");

    // unleash the power of call tips
    addTrigger("(");

    // EDYUK_SHORTCUT(triggerAction(), "C++ completion", tr("Ctrl+Space"));
}

AsCompletion::~AsCompletion() {
    --instances;
    // qDebug("AsCompletion : %i remaining", instances);

    if (!instances && pBackend) {
        delete pBackend;
        pBackend = 0;
    }
}

QCodeCompletionEngine *AsCompletion::clone() {
    AsCompletion *e = new AsCompletion(pModel);

    foreach (QString t, triggers())
        e->addTrigger(t);

    emit cloned(e);

    return e;
}

QString AsCompletion::language() const { return "C++"; }

QStringList AsCompletion::extensions() const {
    QStringList l;

    l << "c"
      << "cc"
      << "cpp"
      << "cxx"
      << "h"
      << "hpp"
      << "hxx";

    return l;
}

void AsCompletion::setCodeModel(QCodeModel *m) { pModel = m; }

void AsCompletion::init() {
    if (pBackend)
        pBackend->init();
}

extern bool isReserved(const QToken &tok);
extern bool maybeIdentifier(const QToken &tok);
extern bool isIdentifier(const QToken &tok);

template <typename Iterator>
static bool matchBackward(Iterator &i, QToken open, QToken close,
                          const Iterator &beg) {
    int nest = 1;

    while (nest && --i >= beg) {
        if (*i == open)
            ++nest;
        else if (*i == close)
            --nest;
    }

    return !nest;
}

template <typename Iterator>
static bool matchForward(Iterator &i, QToken open, QToken close,
                         const Iterator &end) {
    int nest = 1;

    while (nest && ++i < end) {
        if (*i == open)
            ++nest;
        else if (*i == close)
            --nest;
    }

    return !nest;
}

static void flush(QTokenList::const_iterator beg,
                  QTokenList::const_iterator end,
                  QHash<QToken, QToken> &variables,
                  QTokenList stops = QTokenList() << ";") {
    QToken type;
    int tpl_nest = 0;
    bool stillType = true;
    QTokenList::const_iterator i = beg;

    // qDebug("%i{", end - beg);

    if (!maybeIdentifier(*i))
        return;

    while (i < end) {
        // qDebug("%s", i->constData());
        if (stillType) {
            if (*i == "(" || *i == "[" || *i == "{")
                break;

            if (type.length())
                type += ' ';

            type += *i;
        } else if (isIdentifier(*i)) {
            if (maybeIdentifier(*(i + 1)))
                break;

#ifdef TRACE_COMPLETION
            qDebug("\t{%s, %s}", i->constData(), type.constData());
#endif

            variables[*i] = type;
        } else if (stops.contains(*i)) {
            break;
        } else if (*i == "=") {
            while ((i < end) && (*i != ",")) {
                if (*i == "<")
                    matchForward(i, "<", ">", end);
                else if (*i == "(")
                    matchForward(i, "(", ")", end);
                else if (*i == "[")
                    matchForward(i, "[", "]", end);
                else if (stops.contains(*i))
                    break;
                else
                    ++i;
            }

            if (stops.contains(*i))
                break;

        } else if (*i != ",") {
            // qWarning("malformed input.");
            break;
        }

        if (*i == "<") {
            ++tpl_nest;
        } else if (*i == ">") {
            ++i;
            --tpl_nest;
            stillType =
                stillType && (tpl_nest || *i == "*" || *i == "&" || *i == "::");
            continue;
        }

        if (stillType) {
            if (tpl_nest || *i == "const" || *i == "static" || *i == "::") {
                ++i;
                continue;
            }

            if (++i == end)
                break;

            stillType = *i == "*" || *i == "&" || *i == "<" || *i == "::";
            continue;
        }

        ++i;
    }

    // if ( i != end )
    //	qDebug("@%i", end - i);

    // qDebug("};");
}

QTokenList concreteTemplates(const QToken &type) {
    QTokenList ltpl;

    int t_beg = type.indexOf('<'), t_end = type.lastIndexOf('>');

    if ((t_beg != -1) && (t_end != -1)) {
        QToken tpl;
        int t_idx = t_beg + 1, t_nest = 0;

        while (t_idx < t_end) {
            if (type.at(t_idx) == '<') {
                tpl += '<';

                ++t_nest;

            } else if (type.at(t_idx) == '>') {
                if (t_nest) {
                    --t_nest;
                    tpl += '>';

                } else {
                    ltpl << tpl.trimmed();
                    tpl.clear();
                    break;
                }
            } else if (!t_nest && (type.at(t_idx) == ',')) {
                ltpl << tpl.trimmed();
                tpl.clear();
            } else {
                tpl += type.at(t_idx);
            }

            ++t_idx;
        }

        if (tpl.length())
            ltpl << tpl.trimmed();
    }

    return ltpl;
}

static QByteArray trimmedType(const QByteArray &t, bool &ptr) {
    if (t.isEmpty())
        return QByteArray();

    QByteArray s = t;

    if (s.startsWith("static "))
        s.remove(0, 7);

    if (s.startsWith("const "))
        s.remove(0, 6);

    bool pwrd = false;

    for (int i = 0; i < s.length(); ++i) {
        if (s.at(i) == '*') {
            ptr = true;
            s.remove(i, 1);
            --i;
        } else if (s.at(i) == '&') {
            s.remove(i, 1);
            --i;
        } else if ((s.at(i) == ' ') &&
                   !(pwrd && (((i + 1) < s.length())
                                  ? (isalnum(s.at(i + 1)) || (s.at(i) == '_'))
                                  : false))) {
            s.remove(i, 1);
            --i;
        } else if (s.at(i) == '<') {
            int k = i;

            int count = 0;

            do {
                if (s.at(i) == '<')
                    ++count;
                else if (s.at(i) == '>')
                    --count;

                ++i;

            } while (count && (i < s.length()));

            s.remove(k, i - k);
            i = k - 1;
        }

        pwrd = (i == -1) ? false : (isalnum(s.at(i)) || (s.at(i) == '_'));
    }

    // qDebug() << "trimmed " << t << " into " << s;
    return s;
}

static QList<QCodeNode *> context(QCodeNode *p) {
    QList<QCodeNode *> cxt;

    if (!p)
        return cxt;

    while (p->parent) {
        p = p->parent;

        int t = p->type();

        if (t == QCodeNode::Group || t == QCodeNode::Language)
            break;

        cxt.prepend(p);
    }

    return cxt;
}

static void updateContext(QCodeNode *p, QList<QCodeNode *> *cxt,
                          QList<QByteArray> *scxt) {
    if (!p)
        return;

    if (cxt)
        cxt->clear();

    if (scxt)
        scxt->clear();

    while (p->parent) {
        p = p->parent;

        int t = p->type();

        if (t == QCodeNode::Group || t == QCodeNode::Language)
            break;

        if (cxt)
            cxt->prepend(p);

        if (scxt)
            scxt->prepend(p->role(QCodeNode::Name));
    }
}

static void updateTemplates(QCodeNode *n, const QByteArray &type,
                            QHash<QToken, QToken> &tpl) {
    if (!n)
        return;

    QTokenList abs, crt;

    abs << n->role(QCodeNode::Templates).split('$');
    crt = concreteTemplates(type);
    int max = qMin(abs.length(), crt.length());

    for (int i = 0; i < crt.length(); ++i)
        if (tpl.contains(crt.at(i)))
            crt[i] = tpl[crt.at(i)];

    for (int i = 0; i < max; ++i)
        tpl[abs.at(i)] = crt.at(i);

#ifdef TRACE_COMPLETION
    qDebug() << "concrete templates for " << type << ": <" << abs << "> : <"
             << crt << ">";
#endif
}

static void updateTemplates(const QList<QCodeNode *> &l, const QByteArray &type,
                            QHash<QToken, QToken> &tpl) {
    if (l.isEmpty())
        return;

    QTokenList abs, crt;

    foreach (QCodeNode *n, l) {
        if (!n)
            continue;

        QByteArray tmp = n->role(QCodeNode::Templates);

        if (tmp.isEmpty())
            continue;

        abs << tmp.split('$');
    }

    crt = concreteTemplates(type);
    int max = qMin(abs.length(), crt.length());

    for (int i = 0; i < crt.length(); ++i)
        if (tpl.contains(crt.at(i)))
            crt[i] = tpl[crt.at(i)];

    for (int i = 0; i < max; ++i)
        tpl[abs.at(i)] = crt.at(i);

#ifdef TRACE_COMPLETION
    qDebug() << "concrete templates for " << type << ": <" << abs << "> : <"
             << crt << ">";
#endif
}

QByteArray AsCompletion::functionLookup(QCodeNode *n, const QByteArray &s) {
    QStack<QCodeNode *> stack;
    stack.push(n);

    QByteArray sig = s;
    blockRemove(sig, '<', '>');

    QTokenList abs, crt;
    crt = concreteTemplates(s);

    while (stack.length()) {
        QCodeNode *c = stack.pop();
        int ctype = c->type();
        QByteArray cn = c->role(QCodeNode::Name), tpl;

        if (ctype == QCodeNode::Group ||
            (ctype == QCodeNode::Language && cn == "C++") ||
            (ctype == QCodeNode::Namespace && m_namespaces.contains(cn))) {
            foreach (QCodeNode *child, c->children)
                stack.push(child);
        } else if (ctype == QCodeNode::Function && cn == sig) {
            tpl = c->role(QCodeNode::Templates);
            abs = tpl.split('$');

#ifdef TRACE_COMPLETION
            qDebug("found near match : %s", cn.constData());
#endif
            // qDebug() << c->roles.split('@');

            // not necessarily syntactically sensible but technically sensible
            if (tpl.length() && (abs.length() != crt.length()))
                continue;

            QByteArray type = c->role(QCodeNode::Return);

            if (tpl.length()) {
#ifdef TRACE_COMPLETION
                qDebug() << "concrete templates for " << type << ": <" << abs
                         << "> : <" << crt << ">";
#endif

                for (int i = 0; i < crt.length(); ++i)
                    substitute(type, abs.at(i), crt.at(i));
            }

            // qDebug("match -> %s", type.constData());
            return type;
        }
    }

    return QByteArray();
}

QByteArray AsCompletion::functionLookup(QCodeModel *m, const QByteArray &s) {
    QList<QCodeNode *> l = m->topLevelNodes();

    foreach (QCodeNode *n, l) {
        QByteArray type = functionLookup(n, s);

        if (type.length())
            return type;
    }

    return QByteArray();
}

static QByteArray __indent;

struct IndentLevel {
    IndentLevel(QByteArray &data) : d(data) { d += ' '; }

    ~IndentLevel() { d.chop(1); }

    QByteArray &d;
};

#ifdef TRACE_COMPLETION
#define TRACE_UP IndentLevel __lvl(__indent);
#define TRACE(pattern, ...)                                                    \
    qDebug("%s" pattern, __indent.constData(), ##__VA_ARGS__);
#define TRACE_IF(cond, pattern, ...)                                           \
    if (cond)                                                                  \
        qDebug("%s" pattern, __indent.constData(), ##__VA_ARGS__);
#else
#define TRACE_UP
#define TRACE(pattern, ...)
#define TRACE_IF(cond, pattern, ...)
#endif

QCodeNode *AsCompletion::lookup(const QByteArray &t) {
    TRACE_UP

    if (t.isEmpty())
        return 0;

    QCodeNode *n = 0;

    TRACE("looking for %s in global db.", t.constData())

    if (scope_file && m_locals)
        n = lookup(m_locals, t, 0);

    TRACE_IF(n, "found in locals")

    if (!n && scope_local && pModel) {
        n = pModel->findNode("C++", t);

        TRACE_IF(n, "found in projects")
    }

    if (!n && scope_system && pBackend) {
        n = pBackend->findNode(t);

        TRACE_IF(n, "found in globals")
    }

    // qDebug("found!");
    TRACE_IF(!n, "failed")

    return n;
}

QCodeNode *AsCompletion::nsAwareLookup(const QByteArray &t) {
    TRACE_UP
    TRACE("nsAware for %s", t.constData())

    for (int k = 0; k < m_namespaces.length(); ++k) {
        const QByteArray &ns = m_namespaces.at(k);
        QCodeNode *n = ns.length() ? lookup(ns) : 0;

        n = n ? lookup(n, t) : lookup(t);

        TRACE_IF(n, "success")

        if (n)
            return n;
    }

    TRACE("failed")

    return 0;
}

QCodeNode *AsCompletion::lookup(QCodeNode *n, const QByteArray &t,
                                QList<QCodeNode *> *extra) {
    if (!n || t.isEmpty())
        return 0;

    TRACE_UP
    TRACE("looking for %s in %s [%s].", t.constData(),
          n->role(QCodeNode::Name).constData(), n->qualifiedName().constData())

    QCodeNode *p = 0, *q = 0;
    QTokenList lt = split(t, "::", '<', '>');
    QToken tok = lt.takeFirst();

    foreach (QCodeNode *child, n->children) {
        if (child && (child->role(QCodeNode::Name) == tok)) {
            int type = child->type();

            if (type == QCodeNode::Function || type == QCodeNode::Variable)
                continue;

            p = child;
            break;
        }
    }

    while (n && lt.length()) {
        if (extra)
            extra->append(n);

        q = 0;
        tok = lt.takeFirst();
        foreach (QCodeNode *child, n->children) {
            if (child && (child->role(QCodeNode::Name) == tok)) {
                int type = child->type();

                if (type == QCodeNode::Function || type == QCodeNode::Variable)
                    continue;

                q = child;
                break;
            }
        }
        n = q;
    }

    TRACE_IF(p, "success")
    TRACE_IF(!p, "failed")

    return p;
}

QCodeNode *AsCompletion::localLookup(const QList<QCodeNode *> &l,
                                     QByteArray &tt, QByteArray &type,
                                     bool &ptr, QHash<QToken, QToken> &tpl) {
    QList<QCodeNode *> extra;
    QCodeNode *n =
        l.length() ? lookup(l.last(), tt, &extra) : nsAwareLookup(tt);

    if (!n)
        return 0;

    if (n->type() == QCodeNode::Typedef) {
        type = n->role(QCodeNode::Alias);

        QHash<QToken, QToken>::const_iterator tpi = tpl.constBegin();

        while (tpi != tpl.constEnd()) {
            substitute(type, tpi.key(), *tpi);
            ++tpi;
        }

        tt = trimmedType(type, ptr);

        return decrementalLookup(l + extra, tt, type, ptr, tpl);
    }

    updateTemplates(::context(n) << n, type, tpl);

    return n;
}

QCodeNode *AsCompletion::decrementalLookup(const QList<QCodeNode *> &l,
                                           QByteArray &tt, QByteArray &type,
                                           bool &ptr,
                                           QHash<QToken, QToken> &tpl, int k) {
    if (tt.isEmpty())
        return 0;

    TRACE_UP
    TRACE("decremental for %s, %s.", tt.constData(), type.constData())

    if (k < 0)
        k = l.length() + k;

    for (; k >= 0; --k) {
        QList<QCodeNode *> extra;
        QCodeNode *n = lookup(l.at(k), tt, &extra);

        if (!n)
            continue;

        if (n->type() == QCodeNode::Typedef) {
            type = n->role(QCodeNode::Alias);

            QHash<QToken, QToken>::const_iterator tpi = tpl.constBegin();

            while (tpi != tpl.constEnd()) {
                substitute(type, tpi.key(), *tpi);
                ++tpi;
            }

            tt = trimmedType(type, ptr);

            for (int j = 0; j <= k; ++j)
                extra.insert(j, l.at(j));

            return decrementalLookup(extra, tt, type, ptr, tpl);
        }

        updateTemplates(::context(n) << n, type, tpl);

        return n;
    }

    TRACE("global fallback")

    QCodeNode *n = nsAwareLookup(tt);

    if (n)
        updateTemplates(::context(n) << n, type, tpl);

    return n;
}

void AsCompletion::hierarchy(QCodeNode *n, QList<QCodeNode *> &l,
                             QHash<QToken, QToken> &tpl) {
    if (!n)
        return;

#ifdef TRACE_COMPLETION
    qDebug("computing hierarchy for %s", n->qualifiedName().constData());
#endif

    l << n;

    QList<QCodeNode *> cxt;
    QTokenList acst = n->role(QCodeNode::Ancestors).split(',');

    if (acst.isEmpty())
        return;

    updateContext(n, &cxt, 0);

    foreach (QToken a, acst) {
        // TODO : visibility-based filtering

        // bool bPriv = a.contains("private");

        remove(a, "virtual");

        remove(a, "public");
        remove(a, "private");
        remove(a, "protected");

        QHash<QToken, QToken>::const_iterator tpi = tpl.constBegin();

        while (tpi != tpl.constEnd()) {
            substitute(a, tpi.key(), *tpi);
            ++tpi;
        }

        bool ptr = false;
        QToken tt = trimmedType(a, ptr);
        QCodeNode *p = decrementalLookup(cxt, tt, a, ptr, tpl);

        hierarchy(p, l, tpl);
    }
}

void AsCompletion::getMembers(QTokenList::const_iterator beg,
                              QTokenList::const_iterator end,
                              const QHash<QToken, QToken> &variables,
                              QTokenList context, QList<QCodeNode *> &nodes,
                              int *filter) {
    QTokenList scxt;
    QCodeNode *n, *tg = 0;
    QList<QCodeNode *> l, cxt;
    QHash<QToken, QToken> tpl;
    QToken type, op, symbol, tt;
    QTokenList::const_iterator i = beg;
    bool bStat = false, bFirst = true, ptr;
    QHash<QToken, QToken>::const_iterator tpi;

    if (end - beg <= 1) {
        type = variables["this"];
        tt = trimmedType(type, ptr);
        n = lookup(tt);

        if (!n)
            return;

        hierarchy(n, l, tpl);
        updateContext(n, &cxt, &scxt);

        scxt << n->role(QCodeNode::Name);

        // qDebug() << context << " vs " << scxt;

        if (context == scxt)
            *filter |= QCodeCompletionWidget::Private;

        nodes << l;
        return;
    }

    symbol = *i;
    op = *(++i);

    if (op == "::") {
        type = symbol;
        bStat = true;
        symbol = (i + 1) < end ? *(i + 1) : QToken();
    } else if (op == "." || op == "->" || op == "[") {
        QHash<QToken, QToken>::const_iterator vit = variables.constFind(symbol);

        if (vit == variables.constEnd()) {
            vit = variables.constFind("this");

            if (vit == variables.constEnd())
                return;

            i -= 2;
            op = "->";
        } else {
            symbol = (i + 1) < end ? *(i + 1) : QToken();
        }

        type = *vit;
    } else if (op == "(") {
        // functions
        QToken ts = trimmedType(symbol, ptr);
        n = nsAwareLookup(ts);
        int ntype = n ? n->type() : 0;

        if (n && (ntype == QCodeNode::Class || ntype == QCodeNode::Struct)) {
            // ctors
            type = symbol;
            matchForward(i, "(", ")", end);

            if ((i == end) || ((i + 1) == end))
                return;

            op = *(++i);
            symbol = (i + 1) < end ? *(i + 1) : QToken();
            //} else if ( n && ntype == QCodeNode::Function ) {

        } else {
            // TODO : globals/locals

            if (scope_file && m_locals)
                type = functionLookup(m_locals, symbol);

            if (type.isEmpty() && scope_local && pModel)
                type = functionLookup(pModel, symbol);

            if (type.isEmpty() && scope_system && pBackend && pBackend->pModel)
                type = functionLookup(pBackend->pModel, symbol);

            if (type.isEmpty()) {
                // methods
                QHash<QToken, QToken>::const_iterator vit =
                    variables.constFind("this");

                if (vit == variables.constEnd())
                    return;

                i -= 2;
                op = "->";
                type = *vit;
            } else {
                matchForward(i, "(", ")", end);

                if ((i == end) || ((i + 1) == end))
                    return;

                op = *(++i);
                symbol = (i + 1) < end ? *(i + 1) : QToken();

                // qDebug("type %s", type.constData());
            }
        }
    }

    n = nsAwareLookup(join(context, "::"));

    // updateContext() start at first parent...
    if (n && n->children.length())
        n = n->children.first();

    if (n)
        updateContext(n, &cxt, &scxt);

    for (int k = 0; k < m_namespaces.length(); ++k) {
        const QByteArray &ns = m_namespaces.at(k);
        QCodeNode *kn = ns.length() ? lookup(ns) : 0;

        if (kn) {
            cxt.prepend(kn);
            scxt.prepend(ns);
        }
    }

    do {
        n = 0;

        bStat &= (op == "::");

#ifdef TRACE_COMPLETION
        qDebug("\"%s\" \"%s\"", op.constData(), symbol.constData());
#endif

    _overload_branching:
        ptr = false;
        tpi = tpl.constBegin();

        while (tpi != tpl.constEnd()) {
            substitute(type, tpi.key(), *tpi);
            ++tpi;
        }

        tt = trimmedType(type, ptr);

#ifdef TRACE_COMPLETION
        qDebug("looking for %s [%s : %i]", tt.constData(), type.constData(),
               (int)ptr);
#endif

        n = (!bFirst && bStat) ? localLookup(cxt, tt, type, ptr, tpl)
                               : decrementalLookup(cxt, tt, type, ptr, tpl);

        bFirst = false;

        if (!n) {
            qWarning("type not found : %s", tt.constData());
            break;
        }

        updateContext(n, &cxt, &scxt);

        QList<QCodeNode *> h, children;
        hierarchy(n, h, tpl);

        foreach (QCodeNode *r, h)
            children << r->children;

        if (op == "[") {
            if (ptr) {
                type.remove(type.lastIndexOf('*'), 1);
            } else {
                // search for operator overloading
                type.clear();

                foreach (QCodeNode *child, children) {
                    if (child &&
                        (child->role(QCodeNode::Name) == "operator[]")) {
                        type = child->role(QCodeNode::Return);

                        break;
                    }
                }

                if (type.isEmpty()) {
#ifdef TRACE_COMPLETION
                    qWarning("unresolved operator overload.");
#endif
                    break;
                }
            }

            matchForward(i, "[", "]", end);

            if ((i == end) || ((i + 1) == end)) {
#ifdef TRACE_COMPLETION
                qWarning("malformed completion chain.");
#endif
                return;
            }

            op = *(++i);
            symbol = ((i + 1) < end) ? *(i + 1) : QToken();

            continue;
        }

        if (ptr && op != "->") {
#ifdef TRACE_COMPLETION
            qWarning("malformed completion chain : pointer cannot mixup with "
                     "%s operator",
                     op.constData());
#endif
            break;
        }

        if (op == "(") {
// search for operator overloading ??
#ifdef TRACE_COMPLETION
            qWarning("paren overloading is not supported yet.");
#endif
            break;

            matchForward(i, "(", ")", end);

            if ((i == end) || ((i + 1) == end)) {
#ifdef TRACE_COMPLETION
                qWarning("malformed completion chain.");
#endif
                break;
            }
        } else if (op == "->" && !ptr) {
            type.clear();

            foreach (QCodeNode *child, children) {
                if (child && (child->type() == QCodeNode::Function) &&
                    (child->role(QCodeNode::Name) == "operator->")) {
                    type = child->role(QCodeNode::Return);

                    break;
                }
            }

            if (type.isEmpty()) {
#ifdef TRACE_COMPLETION
                qWarning("malformed completion chain. [syntax]");
#endif
                break;
            }

            goto _overload_branching;

        } else if (((op == ".") && !ptr) || (op == "->")) {
            op = (i + 2) < end ? *(i + 2) : QToken();

            if (symbol.isEmpty()) {
                tg = n;
                break;
            }

            type.clear();
            op = (i + 2) < end ? *(i + 2) : QToken();

            if (op == "::") {
                // filtering as in : this->BaseClass::foo()

                i += 2;
                op = ".";
                type = symbol;
                symbol = ((i + 1) < end) ? *(i + 1) : QToken();

                continue;
            }

            QByteArray sig = symbol;
            blockRemove(sig, '<', '>');

            foreach (QCodeNode *child, children) {
                if (child && (child->role(QCodeNode::Name) == sig)) {
                    int nt = child->type();

                    if (nt == QCodeNode::Function && op == "(") {
                        // TODO : support templated functions
                        i += 2;
                        matchForward(i, "(", ")", end);
                        type = child->role(QCodeNode::Return);
                        updateTemplates(child, symbol, tpl);

                        break;
                    } else if (nt == QCodeNode::Variable && op != "(") {
                        ++i;
                        type = child->role(QCodeNode::Type);
                        break;
                    }
                }
            }

            if (type.isEmpty()) {
#ifdef TRACE_COMPLETION
                qWarning("unable to resolve symbol.");
#endif
                break;
            }
        } else if (op == "::") {
            if (!bStat) {
#ifdef TRACE_COMPLETION
                qWarning("filtering broken...");
#endif
                break;
            }

            op = (i + 2) < end ? *(i + 2) : QToken();

            if (symbol.isEmpty()) {
                tg = n;
                break;
            }

            cxt << n;
            scxt << type;

            type.clear();

            QByteArray sig = symbol;
            blockRemove(sig, '<', '>');

            foreach (QCodeNode *child, children) {
                if (child && (child->role(QCodeNode::Name) == sig)) {
                    int nt = child->type();

                    // qDebug("found matching symbol {%s, %s, %c}",
                    // symbol.constData(), op.constData(), (char)nt);

                    if ((nt == QCodeNode::Function) && (op == "(")) {
                        // TODO : support templated functions
                        i += 2;
                        matchForward(i, "(", ")", end);
                        type = child->role(QCodeNode::Return);
                        updateTemplates(child, symbol, tpl);

                        break;
                    } else if ((nt == QCodeNode::Variable) && (op != "(") &&
                               (op != "::")) {
                        ++i;
                        type = child->role(QCodeNode::Type);
                        break;
                    } else if (op == "::") {
                        if ((nt == QCodeNode::Enum) ||
                            (nt == QCodeNode::Union) ||
                            (nt == QCodeNode::Class) ||
                            (nt == QCodeNode::Struct) ||
                            (nt == QCodeNode::Namespace)) {
                            ++i;
                            type = symbol;
                            updateContext(child, &cxt, &scxt);
                            break;
                        }
                    } else if (op == "(") {

                        if ((nt == QCodeNode::Class) ||
                            (nt == QCodeNode::Struct)) {
                            type = symbol;
                            updateContext(child, &cxt, &scxt);
                            i += 2;

                            matchForward(i, "(", ")", end);

                            if ((i == end) || ((i + 1) == end))
                                return;

                            break;
                        }
                    }
                }
            }

            if (type.isEmpty()) {
#ifdef TRACE_COMPLETION
                qDebug("static failure.");
#endif
                break;
            }

        } else {
#ifdef TRACE_COMPLETION
            qWarning("malformed completion chain.");
#endif
            break;
        }

        if ((i + 1) >= end) {
#ifdef TRACE_COMPLETION
            qDebug("THE END ??? %s", type.constData());
#endif
            break;
        }

        op = *(++i);
        symbol = ((i + 1) < end) ? *(i + 1) : QToken();

    } while (i < end);

    if (!tg)
        return;

    hierarchy(tg, l, tpl);
    scxt << tg->role(QCodeNode::Name);

    if (bStat) {
        *filter |= QCodeCompletionWidget::KeepSubTypes |
                   QCodeCompletionWidget::KeepEnums;

        if ((context != scxt) && (tg->type() != QCodeNode::Namespace)) {
            // TODO : take context into account before restricting that much...

            *filter |= QCodeCompletionWidget::IsStatic;
        }
    }

    // qDebug() << context << " vs " << scxt;

    // TODO : take friendship into account
    if (context == scxt)
        *filter |= QCodeCompletionWidget::Private;

    nodes << l;
}

void AsCompletion::complete(QCodeStream *s, const QString &trig) {
    if (!s /*|| !DefaultPlugin::configKey<bool>(
                  "QCodeCompletionEngine/AsCompletion/enabled", true)*/)
        return;

    if (pPopup && pPopup->editor() != editor()) {
        delete pPopup;
        pPopup = 0;
    }

    if (!pPopup) {
        pPopup = new QCodeCompletionWidget(editor());
    }

    pPopup->clear();
    pPopup->setCursor(editor()->cursor());

    // QTime time;
    // time.start();

    QAsLexer lexer;
    lexer.setKeepMacroBlocks(true);
    lexer.setInput(s);

    QTokenList tokens = lexer.tokens();

#ifdef TRACE_COMPLETION
    qDebug("Tokenized...");
#endif

    QString trigger = trig;

    // avoid segfaults by ensuring even CTRL + Space can't trigger completion on
    // invalid input...
    if (tokens.isEmpty() ||
        (!maybeIdentifier(tokens.last()) && (tokens.last() != ".") &&
         (tokens.last() != "->") && (tokens.last() != "::") &&
         (tokens.last() != "(") && trigger.isEmpty())) {
#ifdef TRACE_COMPLETION
        qDebug("Invalid input...");
#endif

        return;
    }

    if (tokens.last() == "(") {
        if (trigger.isEmpty())
            trigger = tokens.takeLast();
        else
            return;
    }

    // TODO
    // scope_file = DefaultPlugin::configKey<bool>(
    //     "QCodeCompletionEngine/AsCompletion/file", true);
    // scope_local = DefaultPlugin::configKey<bool>(
    //     "QCodeCompletionEngine/AsCompletion/projects", true);
    // scope_system = DefaultPlugin::configKey<bool>(
    //     "QCodeCompletionEngine/AsCompletion/sysheaders", true);

    // qDebug("completion scope : {local, global}={%i, %i}", scope_local,
    // scope_system);

#ifdef TRACE_COMPLETION
    qDebug("parsing : elapsed %i ms", time.elapsed());
#endif

    context.clear();
    variables.clear();

    // do context search for in-class implementations...
    QTokenList::const_iterator tmp_it, class_it = tokens.constBegin();

    while ((class_it = std::find(class_it, tokens.constEnd(),
                                 QByteArray("class"))) != tokens.constEnd()) {
        while (++class_it < tokens.constEnd()) {
            if (*class_it == ";") {
                // forward decl
                break;
            } else if (*class_it == ":") {
                // def with ancestors
                tmp_it = class_it;

                while (++tmp_it < tokens.constEnd())
                    if (*tmp_it == "{")
                        break;

                if (!matchForward(tmp_it, "{", "}", tokens.constEnd())) {
                    // cursor within class definition...
                    context << *(class_it - 1);

#ifdef TRACE_COMPLETION
                    qDebug("context pushed : %s", context.last().constData());
#endif

                } else {
                    class_it = tmp_it + 1;
                }

                break;
            } else if (*class_it == "{") {
                // def without ancestors
                tmp_it = class_it;

                if (!matchForward(tmp_it, "{", "}", tokens.constEnd())) {
                    // cursor within class definition...
                    context << *(class_it - 1);

#ifdef TRACE_COMPLETION
                    qDebug("context pushed : %s", context.last().constData());
#endif

                } else {
                    class_it = tmp_it + 1;
                }

                break;
            }
        }
    }

    QToken fn;

    if (trigger == "(") {
        fn = tokens.takeLast();

        // qDebug("calltips for : %s", join(tokens, " ").constData());

        if (tokens.isEmpty() || (tokens.last() != "." && tokens.last() != "->"))
            tokens << fn;

    } else {
        if (trigger.length())
            tokens << trigger.toLocal8Bit();
        else if (maybeIdentifier(tokens.last()))
            pPopup->setPrefix(tokens.takeLast());
    }

    QTokenList::const_iterator tmp, l, i = tokens.constEnd();

    bool abort = false, isId, id = false, op = false;

    while (--i >= tokens.constBegin()) {
        isId = maybeIdentifier(*i);

        if (id) {
            if (!isId) {
                if (*i == ")") {
                    op = true;
                    if (!matchBackward(i, ")", "(", tokens.constBegin())) {
                        // qDebug("invalid paren nesting");
                        return;
                    }
                    continue;
                } else if (*i == "]") {
                    op = true;
                    if (!matchBackward(i, "]", "[", tokens.constBegin())) {
                        // qDebug("invalid paren nesting");
                        return;
                    }
                    continue;
                } else if (*i == ">") {
                    QToken merged;
                    int idx = i - tokens.constBegin();
                    QTokenList::iterator rubber, stopper;
                    rubber = stopper = tokens.begin() + idx;

                    if (!matchBackward(rubber, ">", "<", tokens.begin())) {
                        // qDebug("invalid paren nesting");
                        return;
                    }

                    if (rubber == tokens.begin()) {
                        abort = true;
                        break;
                    }

                    --rubber;

                    if (!isIdentifier(*rubber)) {
                        abort = true;
                        break;
                    }

                    int n = stopper - rubber;

                    do {
                        merged += *rubber;
                        rubber = tokens.erase(rubber);
                    } while (--n);

                    merged += ">";

                    // qDebug("merged : %s", merged.constData());

                    *rubber = merged;
                    i = rubber;
                    id = false;

                    continue;
                }

                abort = true;
                break;
            }

        } else {

            if (*i != "." && *i != "->" && *i != "::") {
                break;
            }

            if (*i != "::")
                op = true;
        }

        id = !id;
    }

    if (!op && (trigger == "(")) {
        tokens << ";";
        i = tokens.constEnd() - 1;
    }

    l = tokens.constEnd();

    if (abort || i >= l) {
#ifdef TRACE_COMPLETION
        qDebug("aborting...");
#endif
        return;
    }

#ifdef TRACE_COMPLETION
    for (tmp = i + 1; tmp != l; ++tmp)
        qDebug("target item : %s", tmp->constData());
#endif

    int filter;
    QList<QCodeNode *> nodes;

    /*
            At this point :
                    ] i ; l [ => tokens to type
                    [ begin() ; i ] => context where to extract typing data

    */

    ++i;

    bool scope_skipped = false;
    QHash<QToken, QToken> parameters;
    QQueue<QHash<QToken, QToken>> frames;

    QTokenList::const_iterator t_beg = i, b_end = i - 1, iter;

    while (--i >= tokens.constBegin()) {
        // qDebug("token : %s", i->constData());

        if (*i == ")") {

            // qDebug("parsing paren");

            tmp = i + 1;

            matchBackward(i, ")", "(", tokens.constBegin());

            iter = i;

            if (i == tokens.constBegin())
                break;

            --i;

            if (isIdentifier(*i) && ((i - 1) >= tokens.constBegin()) &&
                (((context.length() && (*i == context.last())) ||
                  ((context.length() > 2) && (context.last() == "::") &&
                   (*i == context.at(context.length() - 2)))) ||
                 isReserved(*(i - 1)) || (*(i - 1) == "&") ||
                 (*(i - 1) == "*") || (*(i - 1) == ">") || (*(i - 1) == "~") ||
                 (*(i - 1) == "::") ||
                 ((i - 2) >= tokens.constBegin() &&
                  ((*(i - 2) == "static") || (*(i - 2) == "const") ||
                   (*(i - 2) == "::"))) ||
                 (*tmp == "const")) &&
                ((*tmp == "const") || (*tmp == "{") || (*tmp == ":"))) {
                // reached EOF (End Of Function) => stop data collection
                // qDebug("stopping data collection...");

                /*
                        1) determine parameters list
                        2) determine object context (class, namespace, ...)
                */

                if (scope_skipped) {
                    // looks like we're outa function scope after all...
                    // go back to were we belong in case the "remnants" are
                    // parsed into a code tree

                    i = std::find(i, tokens.constEnd(), QByteArray("{"));
                    matchForward(i, "{", "}", tokens.constEnd());

                    break;
                }

#ifdef TRACE_COMPLETION
                qDebug("function context %s", i->constData());
#endif

                if (((i - 2) >= tokens.constBegin()) && (*(i - 1) == "~") &&
                    (*(i - 2) == "::"))
                    --i;

                if ((i - 2) >= tokens.constBegin() && *(i - 1) == "::") {
                    // qDebug("establishing object context...");

                    do {
                        i -= 2;

#ifdef TRACE_COMPLETION
                        qDebug("\tobject context %s::%s", i->constData(),
                               (i + 2)->constData());
#endif
                        context << *i;

                    } while ((i - 2) >= tokens.constBegin() &&
                             *(i - 1) == "::");

                    // qDebug("context is %s", qPrintable(context.join("::")));
                }

                --tmp;
                int tpl_nest = 0;
                QToken t_buf, n_buf;

                while (--tmp > iter) {
                    if (*tmp == ">")
                        ++tpl_nest;
                    else if (*tmp == "<")
                        --tpl_nest;

                    if (!tpl_nest && (*tmp == ",")) {
                        if (t_buf.length() && n_buf.length()) {
#ifdef TRACE_COMPLETION
                            qDebug("\tparameter : %s %s", t_buf.constData(),
                                   n_buf.constData());
#endif

                            parameters[n_buf] = t_buf;
                        }

                        n_buf.clear();
                        t_buf.clear();
                    } else if (*tmp == "=") {
                        n_buf.clear();
                        t_buf.clear();
                    } else if (n_buf.isEmpty()) {
                        n_buf = *tmp;
                    } else {
                        t_buf.prepend(" ").prepend(*tmp);
                    }
                }

                if (t_buf.length() && n_buf.length()) {
#ifdef TRACE_COMPLETION
                    qDebug("\tparameter : %s %s", t_buf.constData(),
                           n_buf.constData());
#endif

                    parameters[n_buf] = t_buf;
                }

                break;
            } else if (*i == "for") {
                // attempt to extract inline decl e.g :
                // for ( int i = 0; ; )
                // foreach ( QString s, )

#ifdef TRACE_COMPLETION
                qDebug("inline extraction from %s to %s", (i + 2)->constData(),
                       tmp->constData());
#endif

                flush(i + 2, tmp, variables,
                      QTokenList() << ";"
                                   << ","
                                   << ")");
            }

        } else if (*i == "}") {

            // qDebug("framing up (skipping)");

            // flush(i + 1, b_end, variables);
            scope_skipped = true;
            matchBackward(i, "}", "{", tokens.constBegin());

            b_end = i;

        } else if (*i == "{") {

            // qDebug("framing down");

            // flush(i+1, b_end, variables);
            scope_skipped = false;
            frames.enqueue(variables);

            variables.clear();

            b_end = i - 1;

        } else if (*i == ";") {
            // qDebug("encountered semicolon {");

            // flush(i + 1, b_end, variables);
            tmp = i;
            bool seenEqual = false, isId = false, isPrevId = false;

            while (--i >= tokens.constBegin()) {
                if (*i == "{" || *i == "}" || *i == ";")
                    break;

                if (seenEqual && *i == ")")
                    break;

                // qDebug("%s", i->constData());

                isId = isIdentifier(*i);

                seenEqual |= (*i == "=") || (*i == "*") || (*i == "&") ||
                             (isId && isPrevId) ||
                             ((*i == ">") && (*(i + 1) != "("));

                if (*i == ")")
                    matchBackward(i, ")", "(", tokens.constBegin());

                if (*i == "]")
                    matchBackward(i, "]", "[", tokens.constBegin());

                if (*i == ">")
                    matchBackward(i, ">", "<", tokens.constBegin());

                isPrevId = isId;
            }

            // qDebug("};");

            flush(++i, tmp, variables);

            b_end = i;
        }
    }

    /*
            At this point :
                    * frames is filled with variables maps ( <QString:id,
       QString:type> )
                    * frames are ordered, i.e calling dequeue returns the frames
       in enclosing order. Thus you can do typing exactly like a C++ compiler
       would do.
                    * parameters and object/function context are set, if any...
    */

    // here we flatten the variable hierarchy
    variables.clear();

    QHash<QToken, QToken>::const_iterator it;

    while (frames.length()) {
        QHash<QToken, QToken> tmp = frames.dequeue();

        for (it = tmp.constBegin(); it != tmp.constEnd(); ++it) {
            if (!variables.contains(it.key()))
                variables[it.key()] = *it;
        }
    }

    // now lets add the parameters

    for (it = parameters.constBegin(); it != parameters.constEnd(); ++it) {
        variables[it.key()] = *it;
    }

    // add "this" keyword
    if (!context.isEmpty()) {
        variables["this"] += ::join(context, "::") + "*";

        // qDebug("this is of type : %s", qPrintable((context.join("::") +
        // "*")));

        // qDebug("\"this\" typed as %s", variables["this"].constData());
    }

    filter = QCodeCompletionWidget::Public | QCodeCompletionWidget::Protected |
             QCodeCompletionWidget::KeepConst |
             QCodeCompletionWidget::KeepStatic;

#ifdef TRACE_COMPLETION
    qDebug("parsing + scoping : elapsed %i ms", time.elapsed());
#endif

    m_locals = 0;
    m_namespaces.clear();
    m_namespaces << QByteArray();
    QList<QCodeNode *> temp;

    QStringList lns /*= DefaultPlugin::configKey<QString>(
                          "QCodeCompletionEngine/AsCompletion/ns", QString())
                          .split('\n')*/
        ;

    foreach (const QString &ns, lns) {
        QByteArray lcns = ns.trimmed().toLocal8Bit();

        if (!m_namespaces.contains(lcns))
            m_namespaces << lcns;
    }

    // qDebug("ns {%s}", join(m_namespaces, ", ").constData());

    if (scope_file) {
        m_locals = new QCodeNode;

        // TODO : parse only the relevant part (but ALL the relevant part)
        // i.e only decl before completion spot, but whole class hierarchy
        // if spot inside one...

        QAsParser localParser;

        bool matchedScope = true;
        int accurateScopeSize = i - tokens.constBegin();
        QTokenList::const_iterator accurateScopeEnd = tokens.constBegin();

        while (accurateScopeEnd < i) {
            if (*accurateScopeEnd == "{") {
                matchedScope =
                    matchForward(accurateScopeEnd, "{", "}", tokens.constEnd());

                if (!matchedScope) {
                    break;
                }
            } else {
                ++accurateScopeEnd;
            }
        }

        if (accurateScopeEnd != i && matchedScope) {
            accurateScopeSize = accurateScopeEnd - tokens.constBegin();
        }

        localParser.update(m_locals, 0, tokens, 0, accurateScopeSize, false,
                           &m_namespaces);

        // foreach ( QCodeNode *loc, m_locals->children )
        //	qDebug("parsed : %s", loc->role(QCodeNode::Name).constData());

        temp << m_locals;
    }

    // qDebug("ns {%s}", join(m_namespaces, ", ").constData());

    // foreach ( const QByteArray& ns, m_namespaces )
    //	qDebug("ns : \"%s\"", ns.constData());
    bool extend = trigger.isEmpty() || trigger == "(";

    if (extend && (tokens.constEnd() == t_beg) && pPopup->prefix().length()) {
        bool ptr = false;
        QCodeNode *cxt_locals = new QCodeNode;
        QHash<QToken, QToken>::const_iterator vit = variables.constBegin();

        while (vit != variables.constEnd()) {
            QCodeNode *var = new QCodeNode;
            var->roles = "v@";
            var->roles += vit.key();
            var->roles += "@";
            var->roles += trimmedType(*vit, ptr);
            var->roles += "@";
            var->roles += QByteArray::number(QCodeNode::VISIBILITY_PUBLIC);
            var->roles += "@";
            var->roles += QByteArray::number(QCodeNode::SPECIFIER_NONE);

            var->attach(cxt_locals);

            ++vit;
        }

        temp << cxt_locals;
        nodes << cxt_locals;

        if (m_locals)
            nodes << m_locals;
    }

    getMembers(t_beg, tokens.constEnd(), variables, context, nodes, &filter);

    if (extend && (tokens.constEnd() == t_beg) && pPopup->prefix().length()) {
        // lets add types and fancy global stuffs
        // BUT keep the "normal" lookup to make sure local/class variables will
        // work as expected

        QList<QCodeNode *> globs;
        globs << pBackend->rootNodes() << pModel->findRootNodes("C++");

        foreach (const QByteArray &ns, m_namespaces)
            if (ns.length())
                globs << pBackend->findNode(ns) << pModel->findNode("C++", ns);

        if (globs.length()) {
            filter |= QCodeCompletionWidget::Public |
                      QCodeCompletionWidget::KeepSubTypes |
                      QCodeCompletionWidget::KeepEnums |
                      QCodeCompletionWidget::KeepStatic;

            nodes << globs;
        }
    }

    if (nodes.isEmpty() && tokens.length() && (trigger == "(")) {
        // qDebug("call tip extension : %s", fn.constData());

        // extend scope for call tips (ctors)
        QHash<QToken, QToken>::const_iterator vit = variables.constFind(fn);

        if (vit != variables.constEnd()) {
            // qDebug("type : %s", vit->constData());

            fn = *vit;

            // stack ctor : Object instance();
            QCodeNode *n = pBackend->findNode(fn);

            if (n)
                nodes << n;

            n = pModel->findNode("C++", fn);

            if (n)
                nodes << n;

        } else {
            // heap ctor : Object *instance = new Object();

            QCodeNode *n = pBackend->findNode(fn);

            if (n)
                nodes << n;

            n = pModel->findNode("C++", fn);

            if (n)
                nodes << n;
        }
    }

#ifdef TRACE_COMPLETION
    qDebug("parsing + scoping + search : elapsed %i ms", time.elapsed());
#endif

    if (nodes.length()) {
        if (trigger == "(") {
            QStringList tips;

            // qDebug("fn %s", fn.constData());

            foreach (QCodeNode *n, nodes) {
                foreach (QCodeNode *f, n->children) {
                    if (f->type() != QCodeNode::Function ||
                        f->role(QCodeNode::Name) != fn)
                        continue;

                    QString tip =
                        QString::fromLocal8Bit(f->role(QCodeNode::Arguments))
                            .prepend('(')
                            .append(')');

                    if (!tips.contains(tip))
                        tips << tip;
                }
            }

            if (tips.length()) {
                QRect r = editor()->cursorRect();
                QDocumentCursor cursor = editor()->cursor();
                QDocumentLine line = cursor.line();

                int hx = editor()->horizontalOffset(),
                    cx = line.cursorToX(cursor.columnNumber());

                QCallTip *ct = new QCallTip(editor()->viewport());
                ct->move(cx - hx, r.y() + r.height());
                ct->setTips(tips);
                ct->show();
                ct->setFocus();

#ifdef TRACE_COMPLETION
                qDebug(
                    "parsing + scoping + search + pre-display : elapsed %i ms",
                    time.elapsed());
#endif
            }
        } else {
            pPopup->setTemporaryNodes(temp);
            pPopup->setFilter(QCodeCompletionWidget::Filter(filter));
            pPopup->setCompletions(nodes);

#ifdef TRACE_COMPLETION
            qDebug("parsing + scoping + search + pre-display : elapsed %i ms",
                   time.elapsed());
#endif

            pPopup->popup();
        }
    } else {
        qDeleteAll(temp);
        qDebug("completion failed");
    }

#ifdef TRACE_COMPLETION
    qDebug("parsing + scoping + search + display : elapsed %i ms",
           time.elapsed());
#endif
}

static void fetchFiles(const QDir &dir, QStringList &fl,
                       const QStringList &exts, bool suffixless) {
    QFileInfoList il = dir.entryInfoList(QDir::Files | QDir::Dirs |
                                         QDir::NoDotAndDotDot | QDir::Readable);

    foreach (QFileInfo info, il) {
        if (info.isDir()) {
            if (info.fileName() != "private")
                fetchFiles(info.absoluteFilePath(), fl, exts, suffixless);

            continue;
        }

        if (exts.contains(info.suffix()) ||
            (suffixless && info.suffix().isEmpty()))
            fl << info.absoluteFilePath();
    }
}

QCodeCompletionBackend::QCodeCompletionBackend() : pModel(0) {
    pModel = new QCodeModel;

    // QString qt_headers = QLibraryInfo::location(QLibraryInfo::HeadersPath) +
    //                      QDir::separator() + "Qt";

    // bool enabled = DefaultPlugin::configKey<bool>(
    //     "QCodeCompletionEngine/AsCompletion/enabled", true);
    // bool suffixless = DefaultPlugin::configKey<bool>(
    //     "QCodeCompletionEngine/AsCompletion/suffixless", true);
    // bool sysheaders = DefaultPlugin::configKey<bool>(
    //     "QCodeCompletionEngine/AsCompletion/sysheaders", true);

    // if (!enabled || !sysheaders)
    //     return;

    // QStringList l =
    //     DefaultPlugin::configKey<QString>(
    //         "QCodeCompletionEngine/AsCompletion/pathlist", QString())
    //         .split("\n");

    QCodeSerializer serializer;
    serializer.setTargetModel(pModel);

    // QDir d(Edyuk::settingsPath());

    // foreach (QFileInfo info, d.entryInfoList(QDir::Files | QDir::Readable)) {
    //     if (info.suffix() != "tag")
    //         continue;

    //     QString src;
    //     bool ok = false;

    //     serializer.deserialize(info.absoluteFilePath(), &ok, &src,
    //                            info.lastModified());

    //     if (ok) {
    //         qDebug("reloaded : %s", qPrintable(src));
    //         for (int i = 0; i < l.length(); ++i) {
    //             QString e = l.at(i);

    //             if (e.replace("\\", "/") == src)
    //                 l.removeAt(i--);
    //         }
    //     } else {
    //         qDebug("loading failed : %s",
    //         qPrintable(info.absoluteFilePath()));
    //     }
    // }

    QAsParser parser;

    QStringList exts = QStringList() << "h";

    // foreach (QString d, l) {
    //     // parser.parseDirectory(d, exts);
    //     d.replace("\\", "/");

    //     QDir dir(d);
    //     QStringList fl;
    //     QString lbl = d;

    //     if (d.endsWith("/include")) {
    //         lbl.chop(8);
    //     }

    //     fetchFiles(dir, fl, exts, suffixless);

    //     if (fl.isEmpty())
    //         continue;

    //     QCodeNode *g = parser.getNode();
    //     g->roles = "g@" + QFileInfo(lbl).fileName().toLocal8Bit() + "@" +
    //                d.toLocal8Bit();

    //     qDebug("status:Building completion database for %s", qPrintable(d));

    //     foreach (const QString &f, fl) {
    //         QCodeDevice stream(f);

    //         QCppLexer lex(&stream);
    //         parser.update(g, &lex, false);
    //     }

    //     d.replace("/", "_").replace(":", "_");
    //     d.prepend(Edyuk::settingsPath() + "/");
    //     d.append(".tag");

    //     QFile tag(d);

    //     if (!tag.open(QFile::WriteOnly | QFile::Text)) {
    //         qWarning("Unable to open %s for writing", qPrintable(d));
    //         continue;
    //     }

    //     QTextStream tagstream(&tag);

    //     serializer.serialize(g, tagstream);
    //     pModel->appendTopLevelNode(g);
    // }
}

QCodeCompletionBackend::~QCodeCompletionBackend() {
    delete pModel;
    pModel = 0;
}

void QCodeCompletionBackend::init() {}

QCodeNode *QCodeCompletionBackend::findNode(const QByteArray &type) const {
    return pModel ? pModel->findNode("", type) : 0;

    // return m_sysheaders.length() ? m_sysheaders.at(0)->findNode(type) : 0;
}

QList<QCodeNode *> QCodeCompletionBackend::rootNodes() const {
    return pModel ? pModel->topLevelNodes() : QList<QCodeNode *>();

    // return m_sysheaders.length() ? m_sysheaders.at(0)->findNode(type) : 0;
}
