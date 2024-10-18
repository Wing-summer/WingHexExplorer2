#include "ascompletion.h"

#include "qcalltip.h"
#include "qdocumentcursor.h"
#include "qdocumentline.h"

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

Q_DECL_UNUSED static void remove(QByteArray &b, char c, int i = 0) {
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

AsCompletion::AsCompletion(QObject *p) : QCodeCompletionEngine(p) {

    addTrigger(".");
    addTrigger("->");
    addTrigger("::");

    // unleash the power of call tips
    addTrigger("(");

    // EDYUK_SHORTCUT(triggerAction(), "C++ completion", tr("Ctrl+Space"));
}

AsCompletion::~AsCompletion() {}

QCodeCompletionEngine *AsCompletion::clone() {
    AsCompletion *e = new AsCompletion();

    for (auto &t : triggers())
        e->addTrigger(t);

    emit cloned(e);

    return e;
}

QString AsCompletion::language() const { return "AngelScript"; }

QStringList AsCompletion::extensions() const {
    QStringList l;

    l << "as"
      << "angelscript";

    return l;
}

void AsCompletion::complete(const QDocumentCursor &c, const QString &trigger) {}
