#include "qaslexer.h"
#include "qcodestream.h"

#include <QByteArray>
#include <QChar>
#include <QRegularExpression>
#include <QStack>

//#define _TRACE_FILTERING_

#ifdef _TRACE_FILTERING_
static QByteArray filterIndent;

#define FILTER_IN filterIndent += " ";
#define FILTER_OUT filterIndent.chop(1);

#define FILTER_TRACE(fmt, data) qDebug("%s" fmt, filterIndent.constData(), data)

#else

#define FILTER_IN
#define FILTER_OUT

#define FILTER_TRACE(fmt, data)

#endif

QHash<QToken, QToken> QAsLexer::m_macros;

QAsLexer::QAsLexer() : QCodeLexer(), bTokenized(false), bKeepMacros(false) {
    // initMacros();
}

QAsLexer::QAsLexer(QCodeStream *s)
    : QCodeLexer(s), bTokenized(false), bKeepMacros(false) {
    // initMacros();
    refreshTokens(Normal);
}

void QAsLexer::initMacros(const QTokenList &l) {
    if (m_macros.count())
        return;

    // qDebug("loaded %i macros", l.count());

    for (QToken m : l) {
        m = m.trimmed();
        int idx = m.indexOf('=');

        if (idx == -1) {
            m_macros[m];
            continue;
        }

        m_macros[m.left(idx)] = m.mid(idx + 1);
    }
}

bool QAsLexer::keepMacroBlocks() const { return bKeepMacros; }

void QAsLexer::setKeepMacroBlocks(bool keep) { bKeepMacros = keep; }

void QAsLexer::setInput(QCodeStream *s, LexMode m) {
    QCodeLexer::setInput(s);

    bTokenized = false;

    refreshTokens(m);
}

QToken QAsLexer::nextToken() {
    if (!bTokenized) {
        qWarning("Empty token buffer (no stream passed to lexer)");
    }

    if (tokenPointer < tokenBuffer.constEnd())
        return *(tokenPointer++);

    return QToken();
}

QToken QAsLexer::previousToken() {
    if (!bTokenized) {
        qWarning("Empty token buffer (no stream passed to lexer)");
    }

    if (tokenPointer >= tokenBuffer.constBegin())
        return *(tokenPointer--);

    return QToken();
}

QTokenList QAsLexer::tokens() {
    if (!bTokenized) {
        qWarning("Empty token buffer (no stream passed to lexer)");
    }

    return tokenBuffer;
}

int QAsLexer::lineForToken(int tokenId, int minLine) const {
    // minLine = 0;

    int line = minLine;
    if (minLine)
        minLine = 0;

    minLine *= 2;
    // qDebug("looking for token %i", tokenId);

    while (minLine < m_jumps.count()) {
        if (tokenId < m_jumps.at(minLine))
            break;

        line = m_jumps.at(++minLine);
        // line += m_jumps.at(++minLine);

        // qDebug("gap : %i", line);

        ++minLine;
    }

    return line;
}

template <typename T>
static void s_flush(T &buffer, QList<T> &data, bool a,
                    const QHash<QToken, QToken> &macros,
                    QList<int> &suspicious) {
    if (buffer.length() && a) {
        QHash<QToken, QToken>::const_iterator it = macros.constFind(buffer);

        if (it != macros.constEnd()) {
            suspicious << data.count();
        }

        data << buffer;
    }

    buffer.clear();
}

#define flush(buffer, data, include)                                           \
    s_flush(buffer, data, include | bKeepMacros, m_macros, m_suspicious)

static bool next(QCodeStream *s, char &ascii, QChar &uni) {
    uni = ascii = s->getChar();

    return ascii;
}

static void previous(QCodeStream *s, char c) { s->ungetChar(c); }

void QAsLexer::refreshTokens(LexMode m) {
    if (input->contextFile().contains(QRegularExpression("[\\\\/]arch[\\\\/]")))
        return;

    int seq = 0;
    int line = 0;
    char c, tmp;
    QChar uc, utmp;
    QByteArray buffer, log, sep;

    QStack<bool> branch;
    tokenBuffer.clear();

    branch.push(true);

    QHash<QToken, QToken> macros = m_macros;

    while (next(input, c, uc)) {
        log += c;

        if (isspace(c)) {
            flush(buffer, tokenBuffer, branch.top());

            while (c && isspace(c)) {
                if (c == '\n') {
                    // qDebug("EOL after %i tokens", tokenBuffer.count());
                    ++line;

                    if (m_jumps.count() &&
                        m_jumps.at(m_jumps.count() - 2) == tokenBuffer.count())
                        m_jumps[m_jumps.count() - 1] = line;
                    else
                        m_jumps << tokenBuffer.count() << line; // << 1;

                    if (m == KeepLineSeparators)
                        tokenBuffer << sep;
                }

                next(input, c, uc);
            }
        }

        switch (c) {
        case '!':
        case '=':

            flush(buffer, tokenBuffer, branch.top());

            if (next(input, tmp, utmp)) {
                if (tmp == '=') {
                    buffer += c;
                    buffer += tmp;

                    flush(buffer, tokenBuffer, branch.top());

                    break;
                } else {
                    previous(input, tmp);
                }
            }

            if (branch.top() || bKeepMacros)
                tokenBuffer << QByteArray(1, c);

            break;

        case '.':
            do {
                ++seq;
            } while (next(input, tmp, utmp) && tmp == '.');

            if (seq == 3) {
                if (!(branch.top() || bKeepMacros))
                    break;

                flush(buffer, tokenBuffer, branch.top());
                tokenBuffer << "...";
                previous(input, tmp);
                seq = 0;
                break;
            } else if (tmp) {
                do {
                    previous(input, tmp);
                } while (--seq);
            }

        case '~':

        case '?':

        case '(':
        case '{':
        case '}':
        case '[':
        case ']':
        case ')':

        case ',':
        case ';':

            // "single" characters

            if (!(branch.top() || bKeepMacros))
                break;

            flush(buffer, tokenBuffer, branch.top());
            tokenBuffer << QByteArray(1, c);

            break;

        case '^':
        case '|':
        case '&':
            // bitwises

            flush(buffer, tokenBuffer, branch.top());

            if (next(input, tmp, utmp)) {
                if (tmp == c) {
                    if (branch.top() || bKeepMacros)
                        tokenBuffer << QByteArray(2, c);

                    break;
                } else if (tmp == '=') {
                    buffer += c;
                    buffer += tmp;

                    flush(buffer, tokenBuffer, branch.top());

                    break;
                } else
                    previous(input, tmp);
            }

            if (branch.top() || bKeepMacros)
                tokenBuffer << QByteArray(1, c);

            break;

        case '*':
        case '%':
        case '+':
        case '-':
            // operators

            flush(buffer, tokenBuffer, branch.top());

            if (next(input, tmp, utmp)) {
                if ((tmp == '=') || (c == '-' && tmp == '>') ||
                    (((c == '-') || (c == '+')) && (c == tmp))) {
                    buffer += c;
                    buffer += tmp;

                    flush(buffer, tokenBuffer, branch.top());

                    break;
                } else
                    previous(input, tmp);
            }

            if (branch.top() || bKeepMacros)
                tokenBuffer << QByteArray(1, c);

            break;

        case '<':
        case '>':
            // template or shift

            flush(buffer, tokenBuffer, branch.top());

            if (next(input, tmp, utmp)) {
                if (tmp == c) {
                    if (branch.top() || bKeepMacros)
                        tokenBuffer << QByteArray(2, c);

                    break;
                } else if (tmp == '=') {
                    buffer += c;
                    buffer += tmp;

                    flush(buffer, tokenBuffer, branch.top());

                    break;
                } else
                    previous(input, tmp);
            }

            if (branch.top() || bKeepMacros)
                tokenBuffer << QByteArray(1, c);

            break;

        case ':':
            // label or scope resolution

            flush(buffer, tokenBuffer, branch.top());

            if (next(input, tmp, utmp)) {
                if (tmp == c) {
                    if (branch.top() || bKeepMacros)
                        tokenBuffer << QByteArray(2, c);

                    break;
                } else
                    previous(input, tmp);
            }

            if (branch.top() || bKeepMacros)
                tokenBuffer << QByteArray(1, c);

            break;

        case '#': {
            // macros

            flush(buffer, tokenBuffer, branch.top());

            buffer += c;

            char prev = c;
            bool first = true;

            while (next(input, tmp, utmp))
                if (!utmp.isSpace())
                    break;

            buffer += tmp;

            // skip up to EOL
            while (next(input, tmp, utmp)) {
                if (first && utmp.isSpace()) {
                    first = false;

                    if (buffer == "#ifdef") {
                        // check for __cplusplus ???
                        branch.push(false);
                    } else if (buffer == "#ifndef") {
                        branch.push(branch.top());
                    } else if (buffer == "#else") {

                        if (branch.count() > 1)
                            branch.push(!branch.pop() && branch.top());
                        else
                            qWarning("unpreced #else");

                    } else if (buffer == "#elif") {

                        while (next(input, tmp, utmp))
                            if (!utmp.isSpace())
                                break;

                        if (branch.count() > 1)
                            branch.top() = (tmp == '!') && branch.top();
                        else
                            qWarning("unpreced #elif");

                    } else if (buffer == "#if") {

                        while (next(input, tmp, utmp))
                            if (!utmp.isSpace())
                                break;

                        branch.push((tmp == '!') && branch.top());

                    } else if (buffer == "#endif") {

                        if (branch.count() > 1)
                            branch.pop();
                        else
                            qWarning("too much #endif directives in %s",
                                     qPrintable(input->contextFile()));
                    } else if (buffer == "#include") {

                        if (tokenBuffer.isEmpty()) {
                            while (next(input, tmp, utmp))
                                if (!utmp.isSpace())
                                    break;

                            if (tmp == '\"') {
                                while (next(input, tmp, utmp)) {
                                    if (tmp == '\"')
                                        break;
                                }
                            }
                        }
                    } else if (buffer == "#define") {
                        buffer.clear();

                        while (next(input, tmp, utmp) && !utmp.isSpace()) {
                            buffer += tmp;
                        }

                        // add local macro
                        macros[buffer];
                    }
                }

                if (tmp == '\n' && prev != '\\') {
                    ++line;

                    // qDebug("EOL after %i tokens", tokenBuffer.count());

                    if (m_jumps.count() &&
                        m_jumps.at(m_jumps.count() - 2) == tokenBuffer.count())
                        m_jumps[m_jumps.count() - 1] = line;
                    else
                        m_jumps << tokenBuffer.count() << line; // << 1;

                    buffer.clear();
                    break;
                }

                prev = tmp;
                buffer += tmp;
            }

            buffer.clear();

            break;
        }

        case '/':
            // comments

            flush(buffer, tokenBuffer, branch.top());

            if (next(input, tmp, utmp)) {
                if (tmp == '/') {
                    // buffer += "//";

                    while (next(input, c, uc)) {
                        if (c == '\n') {
                            ++line;
                            // qDebug("EOL after %i tokens",
                            // tokenBuffer.count());

                            if (m_jumps.count() &&
                                m_jumps.at(m_jumps.count() - 2) ==
                                    tokenBuffer.count())
                                m_jumps[m_jumps.count() - 1] = line;
                            else
                                m_jumps << tokenBuffer.count() << line; // << 1;

                            break;
                        }
                        // buffer += c;
                    }

                    // flush(buffer, tokenBuffer);

                } else if (tmp == '*') {

                    // buffer += "/*";

                    while (next(input, c, uc)) {
                        if (c == '\n') {
                            ++line;
                            // qDebug("EOL after %i tokens",
                            // tokenBuffer.count());

                            if (m_jumps.count() &&
                                m_jumps.at(m_jumps.count() - 2) ==
                                    tokenBuffer.count())
                                m_jumps[m_jumps.count() - 1] = line;
                            else
                                m_jumps << tokenBuffer.count() << line; // << 1;

                        } else if ((c == '*') && next(input, tmp, utmp)) {
                            if (tmp == '/')
                                break;

                            previous(input, tmp);
                        }

                        // buffer += c;
                    }

                    // buffer += "* /";

                    // flush(buffer, tokenBuffer);

                } else if (tmp == '=') {
                    buffer = "/=";
                    flush(buffer, tokenBuffer, branch.top());

                } else {
                    buffer = "/";
                    flush(buffer, tokenBuffer, branch.top());

                    previous(input, tmp);
                }
            }

            buffer.clear();

            break;

        case '\'':
        case '\"': {
            flush(buffer, tokenBuffer, branch.top());

            // quotes
            bool escape = false;

            while (next(input, tmp, utmp)) {
                if (tmp == '\\') {
                    buffer += tmp;
                    escape = !escape;
                    continue;
                }

                if ((tmp == c) && !escape)
                    break;

                escape = false;
                buffer += tmp;
            }

            buffer.prepend(c);
            buffer.append(c);

            flush(buffer, tokenBuffer, branch.top());

            break;
        }

        case '\0':
            flush(buffer, tokenBuffer, branch.top());
            break;

        default:

            if (uc.isLetterOrNumber() || c == '_') {
                // valid identifier char

                buffer += c;
            } else {
                // invalid id char not previously parsed

                // qWarning("=> unparsed char : \'%c\' [log=\"%s\"]",
                //		c, log.constData());

                buffer.clear();
                // flush(buffer, tokenBuffer, branch.top());
            }

            break;
        }
    }

    flush(buffer, tokenBuffer, branch.top());

    if (branch.count() > 1)
        qWarning("Unterminated conditionnals in %s",
                 qPrintable(input->contextFile()));

    // lets filter that mess :
    int offset = 0;

    while (m_suspicious.count()) {
        int pos = m_suspicious.takeFirst() + offset;

        QHash<QToken, QToken>::const_iterator it =
            macros.constFind(tokenBuffer.at(pos));

        if (it != macros.constEnd()) {
            if (it->length()) {
                tokenBuffer[pos] = *it;
            } else {
                // remove the macro and its arguments if any
                tokenBuffer.removeAt(pos);
                int prev = offset;
                --offset;

                if (pos < tokenBuffer.count() && tokenBuffer.at(pos) == "(") {
                    int nest = 0;

                    while (pos < tokenBuffer.count()) {
                        QToken tok = tokenBuffer.takeAt(pos);

                        if (tok == "(")
                            ++nest;
                        else if (tok == ")")
                            --nest;

                        --offset;

                        if (!nest)
                            break;
                    }
                }

                // update the gap list used to query line numbers
                prev = offset - prev;

                // qDebug("removed %i suspicious tokens", -prev);

                for (int i = 0; i < m_jumps.count(); i += 2) {
                    if (m_jumps.at(i) <= pos)
                        continue;

                    // qDebug("gap (%i, %i) becomes (%i, %i)", m_jumps[i],
                    // m_jumps[i + 1], m_jumps[i] + prev, m_jumps[i + 1]);
                    m_jumps[i] += prev;
                }
            }
        }
    }

    for (int i = m_jumps.count() - 2; i > 0; i -= 2) {
        int j = i - 2;

        while ((j >= 0) && (m_jumps[j] == m_jumps[i])) {
            // qDebug("merged gap (%i, %i) and (%i, %i)", m_jumps[i], m_jumps[i
            // + 1], m_jumps[j], m_jumps[j + 1]);
            m_jumps.removeAt(i);
            m_jumps.removeAt(j + 1);

            i = j;
            j -= 2;
        }
    }

    bTokenized = true;

    /*
    qDebug("stream %s tokenized :\n-----",
                    qPrintable(input->contextFile()));

    foreach ( QToken k, tokenBuffer )
            qDebug("\t%s", k.constData());

    qDebug("-----\n");
    */
}
