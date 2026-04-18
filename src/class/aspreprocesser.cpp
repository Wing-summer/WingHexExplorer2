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

#include "aspreprocesser.h"
#include "class/asexprevaluator.h"
#include "class/languagemanager.h"
#include "class/skinmanager.h"
#include "grammar/AngelscriptExprParser/AngelscriptExprParserLexer.h"
#include "grammar/AngelscriptExprParser/AngelscriptExprParserParser.h"
#include "structlib/cstructerrorlistener.h"
#include "utilities.h"

#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QMetaEnum>
#include <QRegularExpression>
#include <QStack>
#include <QVersionNumber>

Q_GLOBAL_STATIC_WITH_ARGS(
    QStringList, DEFAULT_MARCO,
    ({"__AS_ARRAY__",      "__AS_ANY__",          "__AS_GRID__",
      "__AS_HANDLE__",     "__AS_MATH__",         "__AS_WEAKREF__",
      "__AS_COROUTINE__",  "__WING_FILE__",       "__WING_FILESYSTEM__",
      "__WING_STRING__",   "__WING_COLOR__",      "__WING_JSON__",
      "__WING_REGEX__",    "__WING_DICTIONARY__", "__WING_PRINT_VAR__",
      "__WING_PRINT_LN__", "__WING_PRINTF__",     "__WING_CLIPBOARD__",
      "__WING_URL__",      "__WING_CRYPTO__",     "__WING_REFLECTION__",
      "__WING_OPTBOX__"}));

AsPreprocesser::AsPreprocesser(asIScriptEngine *engine) : engine(engine) {
    Q_ASSERT(engine);

    registerBuiltin("__LINE__", [](const SourcePos &pos) -> QString {
        return QString::number(pos.line);
    });
    registerBuiltin("__SECTION__", [](const SourcePos &pos) -> QString {
        return QStringLiteral("\"%1\"").arg(pos.file);
    });
    registerBuiltin("__SECTION_BASE__", [](const SourcePos &pos) -> QString {
        QFileInfo finfo(pos.file);
        return QStringLiteral("\"%1\"").arg(finfo.fileName());
    });

    m_runtimeMacros.insert(defaultRuntimeMarcos());
}

AsPreprocesser::~AsPreprocesser() {}

void AsPreprocesser::registerBuiltin(
    const QString &name, std::function<QString(const SourcePos &)> cb) {
    m_builtinMacros[name] = cb;
}

QHash<QString, QString> AsPreprocesser::defaultRuntimeMarcos() {
    static QHash<QString, QString> marcos;
    if (marcos.isEmpty()) {
        for (const auto &m : std::as_const(*DEFAULT_MARCO)) {
            marcos.insert(m, {});
        }

        // software infos
        auto ver = QVersionNumber::fromString(WINGHEX_VERSION);
        marcos.insert(QStringLiteral("__WING_VERSION__"),
                      "\"" WINGHEX_VERSION "\"");
        marcos.insert(QStringLiteral("__WING_VERSION_MAJOR__"),
                      QString::number(ver.majorVersion()));
        marcos.insert(QStringLiteral("__WING_VERSION_MINOR__"),
                      QString::number(ver.minorVersion()));
        marcos.insert(QStringLiteral("__WING_VERSION_PATCH__"),
                      QString::number(ver.microVersion()));
        marcos.insert(QStringLiteral("__ANGELSCRIPT_VERSION__"),
                      "\"" ANGELSCRIPT_VERSION_STRING "\"");
        marcos.insert(QStringLiteral("__ANGELSCRIPT_VERSION_MAJOR__"),
                      QString::number(ANGELSCRIPT_VERSION / 10000));
        marcos.insert(QStringLiteral("__ANGELSCRIPT_VERSION_MINOR__"),
                      QString::number((ANGELSCRIPT_VERSION / 100) % 100));
        marcos.insert(QStringLiteral("__ANGELSCRIPT_VERSION_PATCH__"),
                      QString::number(ANGELSCRIPT_VERSION % 100));
        marcos.insert(QStringLiteral("__WINGHEX_APPNAME__"),
                      "\"" APP_NAME "\"");
        marcos.insert(QStringLiteral("__WINGHEX_AUTHOR__"), "\"wingsummer\"");
        ver = QLibraryInfo::version();
        marcos.insert(QStringLiteral("__QT_VERSION__"),
                      ver.toString().prepend('"').append('"'));
        marcos.insert(QStringLiteral("__QT_VERSION_MAJOR__"),
                      QString::number(ver.majorVersion()));
        marcos.insert(QStringLiteral("__QT_VERSION_MINOR__"),
                      QString::number(ver.minorVersion()));
        marcos.insert(QStringLiteral("__QT_VERSION_PATCH__"),
                      QString::number(ver.microVersion()));

        // UI
        marcos.insert(QStringLiteral("__LANG__"), LanguageManager::instance()
                                                      .defaultLocale()
                                                      .name()
                                                      .prepend('"')
                                                      .append('"'));
        auto theme = SkinManager::instance().currentTheme();
        auto te = QMetaEnum::fromType<SkinManager::Theme>();
        marcos.insert(QStringLiteral("__THEME__"),
                      QString::fromLatin1(te.valueToKey(int(theme)))
                          .prepend('"')
                          .append('"'));
    }
    return marcos;
}

void AsPreprocesser::processBuffer(const QByteArray &buf,
                                   const QString &sourceName,
                                   const QString &currentDir, QString &outText,
                                   Mapping &outMap) {
    const char *data = buf.constData();
    auto n = buf.size();
    qint64 idx = 0;
    qint64 outLineCounter =
        outMap.lines.isEmpty() ? 0 : outMap.lines.last().outLineNumber;
    qint64 srcLine = 1, srcCol = 1;
    bool lineHasSignificantToken = false;

    struct CondState {
        bool parentTaking = true;
        bool branchTaken = false;
        bool currentTaking = false;
        bool seenElse = false;
        bool valid = true;
    };
    QVector<CondState> condStack;

    QString curOutLine;
    QVector<LineSegmentMap> curSegments;

    auto flushCurrLine = [&]() {
        outText += curOutLine;
        outText += '\n';
        OutputLineMap olm;
        olm.outLineNumber = ++outLineCounter;
        qint64 running = 1;
        for (auto s : curSegments) {
            s.outColStart = running;
            running += s.length;
            olm.segments.append(s);
        }
        outMap.addOutputLineMap(olm);
        curOutLine.clear();
        curSegments.clear();
    };
    auto appendCharMapped = [&](QChar ch, const QString &file, qint64 fileLine,
                                qint64 fileCol) {
        if (!curSegments.isEmpty() && curSegments.last().file == file &&
            curSegments.last().originalLine == fileLine &&
            curSegments.last().origColStart + curSegments.last().length ==
                fileCol) {
            curSegments.last().length += 1;
        } else {
            LineSegmentMap seg;
            seg.file = file;
            seg.originalLine = fileLine;
            seg.origColStart = fileCol;
            seg.length = 1;
            curSegments.append(seg);
        }
        curOutLine.append(ch);
    };
    auto appendSourceRange = [&](const QString &text, const QString &file) {
        for (int i = 0; i < text.size(); ++i) {
            QChar ch = text.at(i);
            if (ch == QChar('\n')) {
                flushCurrLine();
                ++srcLine;
                srcCol = 1;
                lineHasSignificantToken = false;
            } else {
                appendCharMapped(ch, file, srcLine, srcCol);
                srcCol += 1;
            }
        }
    };
    auto appendReplacementFromOrigin = [&](const QString &rep,
                                           const QString &file, int fileLine,
                                           int fileCol) {
        for (int i = 0; i < rep.size(); ++i) {
            QChar ch = rep.at(i);
            if (ch == QChar('\n')) {
                flushCurrLine();
                ++fileLine;
                fileCol = 1;
            } else {
                appendCharMapped(ch, file, fileLine, fileCol);
                fileCol += 1;
            }
        }
    };

    auto parseFirstIdentifierInBA = [&](const QByteArray &ba) -> QString {
        unsigned int offset = 0;
        while (offset < (unsigned int)ba.size()) {
            unsigned int tl = 0;
            int cls = asTC_UNKNOWN;
            try {
                cls = engine->ParseToken(ba.constData() + offset,
                                         ba.size() - offset, &tl);
            } catch (...) {
                cls = asTC_UNKNOWN;
                tl = 0;
            }
            if (tl == 0)
                break;
            if (cls == asTC_WHITESPACE || cls == asTC_COMMENT) {
                offset += tl;
                continue;
            }
            if (cls == asTC_IDENTIFIER)
                return QString::fromUtf8(ba.constData() + offset, tl);
            break;
        }

        QString s = QString::fromUtf8(ba);
        static QRegularExpression rx(
            QStringLiteral(R"(^\s*([\p{L}_][\p{L}\p{N}_]*))"));
        auto m = rx.match(s);
        if (m.hasMatch())
            return m.captured(1);
        return QString();
    };

    // handle directive line (a whole line that starts with #)
    auto handleDirective = [&](const QByteArray &dirBytes, qint64 dStartLine,
                               qint64 dStartCol) -> void {
        QString s = QString::fromUtf8(dirBytes);
        while (!s.isEmpty() && (s.endsWith('\n') || s.endsWith('\r')))
            s.chop(1);

        int p = 0;
        while (p < s.size() && s[p].isSpace())
            p++;
        if (p < s.size() && s[p] == '#')
            p++;
        while (p < s.size() && s[p].isSpace())
            p++;

        QString rest = s.mid(p);

        static QRegularExpression re(QStringLiteral(R"(^(\w+)\b(.*)$)"));
        auto m = re.match(rest);

        QString kw = m.hasMatch() ? m.captured(1) : QString();
        QString args = m.hasMatch() ? m.captured(2) : QString();

        auto emitBlankLine = [&]() {
            curOutLine.clear();
            curSegments.clear();
            flushCurrLine();
        };

        auto parentTaking = [&]() -> bool {
            return condStack.isEmpty() ? true : condStack.last().currentTaking;
        };

        // ==================== define / undef ====================
        if (kw == "define" || kw == "undef") {
            errorReport(
                {PreprocErrorCode::ERR_SOURCE_DEFINE_FORBIDDEN, Severity::Error,
                 m_currentSource, dStartLine, dStartCol,
                 QStringLiteral("#%1 in source is forbidden.").arg(kw)});
            emitBlankLine();
            return;
        }

        // ==================== pragma ====================
        if (kw == "pragma") {
            if (pragmaCallback) {
                auto r = pragmaCallback(args, this, m_currentSource);

                if (!r.info.isEmpty())
                    errorReport({PreprocErrorCode::ERR_SUCCESS, Severity::Info,
                                 m_currentSource, dStartLine, dStartCol,
                                 r.info});

                if (!r.warn.isEmpty())
                    errorReport({PreprocErrorCode::ERR_SUCCESS,
                                 Severity::Warning, m_currentSource, dStartLine,
                                 dStartCol, r.warn});

                if (!r.error.isEmpty())
                    errorReport({PreprocErrorCode::ERR_ERROR, Severity::Error,
                                 m_currentSource, dStartLine, dStartCol,
                                 r.error});
            }
            emitBlankLine();
            return;
        }

        // ==================== include ====================
        if (kw == "include") {
            QString incPath;
            if (!parseIncludePathToken(args, incPath)) {
                errorReport({PreprocErrorCode::ERR_IF_PARSE, Severity::Error,
                             m_currentSource, dStartLine, dStartCol,
                             QStringLiteral("Bad #include syntax: %1")
                                 .arg(args.trimmed())});
                emitBlankLine();
                return;
            }

            QString curDir = QFileInfo(m_currentSource).absolutePath();
            auto resolved = resolveIncludeFile(incPath, curDir);

            if (resolved.isEmpty()) {
                errorReport({PreprocErrorCode::ERR_INCLUDE_NOT_FOUND,
                             Severity::Error, m_currentSource, dStartLine,
                             dStartCol,
                             QStringLiteral("Included file not found: %1")
                                 .arg(incPath)});
                emitBlankLine();
                return;
            }

            loadScriptSection(resolved);
            emitBlankLine();
            return;
        }

        // ==================== #if ====================
        if (kw == QStringLiteral("if")) {
            CondState cs;
            cs.parentTaking = parentTaking();

            bool exprOk = true;
            QString expanded = expandExpressionForIf(
                args, m_currentSource, dStartLine, dStartCol, &exprOk);

            if (exprOk) {
                auto er = evalExpression(expanded, m_currentSource, dStartLine,
                                         dStartCol);
                if (er) {
                    cs.currentTaking = cs.parentTaking && er.value();
                    cs.branchTaken = cs.currentTaking;
                } else {
                    cs.currentTaking = false;
                    cs.branchTaken = false;
                    cs.valid = false;
                }
            } else {
                cs.currentTaking = false;
                cs.branchTaken = false;
                cs.valid = false;
            }

            condStack.append(cs);
            emitBlankLine();
            return;
        }

        // ==================== #ifdef ====================
        if (kw == "ifdef" || kw == "ifndef") {
            QByteArray ba = args.toUtf8();
            QString name = parseFirstIdentifierInBA(ba);

            CondState cs;
            cs.parentTaking = parentTaking();

            if (name.isEmpty()) {
                errorReport({PreprocErrorCode::ERR_IF_PARSE, Severity::Error,
                             m_currentSource, dStartLine, dStartCol,
                             QStringLiteral("bad #%1 usage: missing identifier")
                                 .arg(kw)});
                cs.currentTaking = false;
            } else {
                bool defined = m_runtimeMacros.contains(name);
                bool take = (kw == "ifdef") ? defined : !defined;
                cs.currentTaking = cs.parentTaking && take;
                cs.branchTaken = cs.currentTaking;
            }

            condStack.append(cs);
            emitBlankLine();
            return;
        }

        // ==================== #elif ====================
        if (kw == QStringLiteral("elif")) {
            if (condStack.isEmpty()) {
                errorReport({PreprocErrorCode::ERR_ELIF_ELSE_WITHOUT_IF,
                             Severity::Error, m_currentSource, dStartLine,
                             dStartCol, "#elif without matching #if"});
                emitBlankLine();
                return;
            }

            CondState &top = condStack.last();

            if (top.seenElse) {
                errorReport({PreprocErrorCode::ERR_ELIF_ELSE_WITHOUT_IF,
                             Severity::Error, m_currentSource, dStartLine,
                             dStartCol, "#elif after #else is not allowed"});
                top.currentTaking = false;
                emitBlankLine();
                return;
            }

            bool current = false;
            if (top.parentTaking && !top.branchTaken) {
                bool exprOk = true;
                QString expanded = expandExpressionForIf(
                    args, m_currentSource, dStartLine, dStartCol, &exprOk);
                if (exprOk) {
                    auto er = evalExpression(expanded, m_currentSource,
                                             dStartLine, dStartCol);
                    if (er)
                        current = er.value();
                }
            }

            top.currentTaking = top.parentTaking && !top.branchTaken && current;
            top.branchTaken = top.branchTaken || top.currentTaking;

            emitBlankLine();
            return;
        }

        // ==================== #else ====================
        if (kw == "else") {
            if (condStack.isEmpty()) {
                errorReport({PreprocErrorCode::ERR_ELIF_ELSE_WITHOUT_IF,
                             Severity::Error, m_currentSource, dStartLine,
                             dStartCol, "#else without matching #if"});
                emitBlankLine();
                return;
            }

            CondState &top = condStack.last();

            if (top.seenElse) {
                errorReport({PreprocErrorCode::ERR_ELIF_ELSE_WITHOUT_IF,
                             Severity::Error, m_currentSource, dStartLine,
                             dStartCol, "duplicate #else"});
                top.currentTaking = false;
                emitBlankLine();
                return;
            }

            top.seenElse = true;
            top.currentTaking = top.parentTaking && !top.branchTaken;
            top.branchTaken = true;

            emitBlankLine();
            return;
        }

        // ==================== #endif ====================
        if (kw == "endif") {
            if (condStack.isEmpty()) {
                errorReport({PreprocErrorCode::ERR_ELIF_ELSE_WITHOUT_IF,
                             Severity::Error, m_currentSource, dStartLine,
                             dStartCol, "#endif without matching #if"});
                emitBlankLine();
                return;
            }

            condStack.removeLast();
            emitBlankLine();
            return;
        }

        // ==================== unknown ====================
        errorReport({PreprocErrorCode::ERR_MARCO_ERROR, Severity::Warning,
                     m_currentSource, dStartLine, dStartCol,
                     QStringLiteral("Unknown directive #") + kw});

        emitBlankLine();
    }; // end handle directive

    m_currentSource = sourceName;

    while (idx < n) {
        unsigned int tokenLen = 0;
        auto tokClass = engine->ParseToken(data + idx, n - idx, &tokenLen);

        if (tokenLen == 0)
            tokenLen = 1;
        const char *tokStart = data + idx;
        auto tlen = tokenLen;

        bool isDirective = (tokClass == asTC_UNKNOWN && tlen > 0 &&
                            tokStart[0] == '#' && !lineHasSignificantToken);
        bool inCond = !condStack.isEmpty();
        bool taking = !inCond || condStack.last().currentTaking;

        if (isDirective) {
            qint64 j = idx;
            QByteArray dir;
            qint64 dStartLine = srcLine, dStartCol = srcCol;
            // read whole directive line (handle backslash continuation)
            while (j < n) {
                qint64 k = j;
                while (k < n && data[k] != '\n')
                    k++;
                dir.append(data + j, k - j);
                bool cont = false;
                if (k - j > 0) {
                    qint64 m = k - 1;
                    while (m >= j && (data[m] == ' ' || data[m] == '\t' ||
                                      data[m] == '\r'))
                        m--;
                    if (m >= j && data[m] == '\\')
                        cont = true;
                }
                if (k < n && data[k] == '\n')
                    dir.append('\n');
                j = (k < n && data[k] == '\n') ? k + 1 : k;
                if (!cont)
                    break;
            }
            handleDirective(dir, dStartLine, dStartCol);

            for (qsizetype p = 0; p < dir.size(); ++p) {
                char c = dir[p];
                if (c == '\n') {
                    ++srcLine;
                    srcCol = 1;
                    lineHasSignificantToken = false;
                } else
                    ++srcCol;
            }
            idx = j;
            continue;
        }

        if (!taking) {
            // skip tokens in inactive branch, but still update srcLine/srcCol
            for (qsizetype i = 0; i < tlen; ++i) {
                char c = tokStart[i];
                if (c == '\n') {
                    curOutLine.clear();
                    curSegments.clear();
                    flushCurrLine();
                    ++srcLine;
                    srcCol = 1;
                    lineHasSignificantToken = false;
                } else
                    ++srcCol;
            }
            idx += tlen;
            continue;
        }

        auto tokenText = QString::fromUtf8(tokStart, tlen);
        // active branch: emit tokens, expanding macros and builtins
        if (tokClass == asTC_WHITESPACE) {
            auto total = tokenText.size();
            for (qsizetype i = 0; i < total; ++i) {
                QChar ch = tokenText.at(i);
                if (ch == QChar('\n')) {
                    flushCurrLine();
                    ++srcLine;
                    srcCol = 1;
                    lineHasSignificantToken = false;
                } else {
                    appendCharMapped(ch, sourceName, srcLine, srcCol);
                    ++srcCol;
                }
            }
            idx += tlen;
            continue;
        }
        if (tokClass == asTC_COMMENT) {
            appendSourceRange(tokenText, sourceName);
            idx += tlen;
            lineHasSignificantToken = true;
            continue;
        }
        if (tokClass == asTC_VALUE) {
            appendSourceRange(tokenText, sourceName);
            idx += tlen;
            lineHasSignificantToken = true;
            continue;
        }
        if (tokClass == asTC_IDENTIFIER || tokClass == asTC_KEYWORD) {
            // builtin macros first
            if (m_builtinMacros.contains(tokenText)) {
                SourcePos sp{sourceName, srcLine, srcCol};
                QString rep = m_builtinMacros[tokenText](sp);
                appendReplacementFromOrigin(rep, sourceName, srcLine, srcCol);
                for (int k = 0; k < tlen; ++k) {
                    if (tokStart[k] == '\n') {
                        ++srcLine;
                        srcCol = 1;
                        lineHasSignificantToken = false;
                    } else
                        ++srcCol;
                }
                idx += tlen;
                lineHasSignificantToken = true;
                continue;
            }
            // host macro
            if (m_runtimeMacros.contains(tokenText)) {
                QStringList visited;
                expandMacroInCode(tokenText, sourceName, srcLine, srcCol,
                                  visited, appendReplacementFromOrigin);
                for (int k = 0; k < tlen; ++k) {
                    if (tokStart[k] == '\n') {
                        ++srcLine;
                        srcCol = 1;
                        lineHasSignificantToken = false;
                    } else
                        ++srcCol;
                }
                idx += tlen;
                lineHasSignificantToken = true;
                continue;
            }
            // normal identifier
            appendSourceRange(tokenText, sourceName);
            idx += tlen;
            lineHasSignificantToken = true;
            continue;
        }

        // fallback: emit as-is
        appendSourceRange(tokenText, sourceName);
        idx += tlen;
        lineHasSignificantToken = true;
    } // main loop

    if (!condStack.isEmpty()) {
        PreprocError e{
            PreprocErrorCode::ERR_ENDIF_MISSING,
            Severity::Error,
            m_currentSource,
            srcLine,
            srcCol,
            QStringLiteral("Unterminated conditional block: missing #endif")};
        errorReport(e);
    }
    if (!curOutLine.isEmpty() || !curSegments.isEmpty())
        flushCurrLine();
}

bool AsPreprocesser::parseIncludePathToken(const QString &args,
                                           QString &outPath) {
    QString s = args.trimmed();
    if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') ||
                          (s.front() == '\'' && s.back() == '\''))) {
        outPath = s.sliced(1, s.size() - 2);
        return true;
    }
    return false;
}

QString AsPreprocesser::resolveIncludeFile(const QString &filename,
                                           const QString &currentDir) {
    QFileInfo info(filename);
    bool isAbsolute = info.isAbsolute();
    bool hasNoExt = info.suffix().isEmpty();
    QString inc;
    Q_ASSERT(!currentDir.isEmpty());

    if (isAbsolute) {
        inc = filename;
    } else {
        QDir pwd(currentDir);
        inc = pwd.absoluteFilePath(filename);
    }

    if (hasNoExt) {
        inc += QStringLiteral(".as");
    }

    if (QFile::exists(inc)) {
        return inc;
    }

    return {};
}

QString AsPreprocesser::expandExpressionForIf(const QString &expr,
                                              const QString &file, qint64 line,
                                              qint64 col, bool *ok) {
    if (ok)
        *ok = true;

    QString ident;
    if (isBareIdentifierExpr(expr, &ident)) {
        return m_runtimeMacros.contains(ident) ? QStringLiteral("1")
                                               : QStringLiteral("0");
    }

    QByteArray ba = expr.toUtf8();
    unsigned int offset = 0;
    QString out;

    while (offset < (unsigned int)ba.size()) {
        unsigned int tl = 0;
        int cls = asTC_UNKNOWN;
        try {
            cls = engine->ParseToken(ba.constData() + offset,
                                     ba.size() - offset, &tl);
        } catch (...) {
            cls = asTC_UNKNOWN;
            tl = 0;
        }

        if (tl == 0)
            break;

        if (cls == asTC_WHITESPACE || cls == asTC_COMMENT) {
            out += QString::fromUtf8(ba.constData() + offset, tl);
            offset += tl;
            continue;
        }

        if (cls == asTC_IDENTIFIER) {
            QString tok = QString::fromUtf8(ba.constData() + offset, tl);

            if (m_builtinMacros.contains(tok)) {
                SourcePos sp{file, line, col};
                out += m_builtinMacros[tok](sp);
            } else if (m_runtimeMacros.contains(tok)) {
                QString val = m_runtimeMacros.value(tok);

                if (val.isEmpty()) {
                    if (ok)
                        *ok = false;

                    errorReport(
                        {PreprocErrorCode::ERR_IF_PARSE, Severity::Error,
                         m_currentSource, line, col,
                         QStringLiteral("Empty macro '%1' cannot be used in "
                                        "#if/#elif expression with operators.")
                             .arg(tok)});
                    return QString();
                }

                out += val;
            } else {
                out += QStringLiteral("0");
            }

            offset += tl;
            continue;
        }

        out += QString::fromUtf8(ba.constData() + offset, tl);
        offset += tl;
    }

    return out;
}

bool AsPreprocesser::isBareIdentifierExpr(const QString &expr,
                                          QString *outIdent) {
    QByteArray ba = expr.toUtf8();
    unsigned int offset = 0;
    QString ident;
    bool seenIdent = false;

    while (offset < (unsigned int)ba.size()) {
        unsigned int tl = 0;
        int cls = asTC_UNKNOWN;
        try {
            cls = engine->ParseToken(ba.constData() + offset,
                                     ba.size() - offset, &tl);
        } catch (...) {
            cls = asTC_UNKNOWN;
            tl = 0;
        }

        if (tl == 0)
            break;

        if (cls == asTC_WHITESPACE || cls == asTC_COMMENT) {
            offset += tl;
            continue;
        }

        if (cls == asTC_IDENTIFIER) {
            if (seenIdent)
                return false;
            seenIdent = true;
            ident = QString::fromUtf8(ba.constData() + offset, tl);
            offset += tl;
            continue;
        }

        return false;
    }

    if (!seenIdent)
        return false;

    if (outIdent)
        *outIdent = ident;
    return true;
}

void AsPreprocesser::expandMacroInCode(
    const QString &name, const QString &expFile, int expLine, int expCol,
    QStringList &visited,
    std::function<void(const QString &, const QString &, int, int)>
        appendReplacementFromOrigin) {
    if (!m_runtimeMacros.contains(name))
        return;
    if (visited.count(name)) {
        // recursion -> output the token itself mapped to source file
        // (if we have def pos use it)
        appendReplacementFromOrigin(name, expFile, expLine, expCol);
        return;
    }
    visited.append(name);
    QString val = m_runtimeMacros.value(name);
    SourcePos defPos = SourcePos{{}, 1, 1};
    // empty value => treat as "no-value" macro: remove identifier (i.e.
    // emit nothing)
    if (val.isEmpty()) {
        visited.removeOne(name);
        return;
    }
    // walk the value and expand builtins and nested macros
    qint64 p = 0;
    while (p < val.size()) {
        QChar c = val[p];
        if (c.isSpace()) {
            appendReplacementFromOrigin(QString(c), defPos.file, defPos.line,
                                        defPos.column);
            ++p;
            continue;
        }
        if (c == '"' || c == '\'') {
            QChar q = c;
            QString lit;
            lit.append(c);
            ++p;
            while (p < val.size()) {
                lit.append(val[p]);
                if (val[p] == '\\') {
                    if (p + 1 < val.size()) {
                        lit.append(val[p + 1]);
                        p += 2;
                        continue;
                    }
                }
                if (val[p] == q) {
                    ++p;
                    break;
                }
                ++p;
            }
            appendReplacementFromOrigin(lit, defPos.file, defPos.line,
                                        defPos.column);
            continue;
        }
        if (c.isLetter() || c == '_' || c.unicode() > 127) {
            int st = p;
            ++p;
            while (p < val.size() && (val[p].isLetterOrNumber() ||
                                      val[p] == '_' || val[p].unicode() > 127))
                ++p;
            QString id = val.mid(st, p - st);
            if (m_builtinMacros.contains(id)) {
                SourcePos sp{expFile, expLine, expCol};
                QString rep = m_builtinMacros[id](sp);
                appendReplacementFromOrigin(rep, expFile, expLine, expCol);
            } else if (m_runtimeMacros.contains(id)) {
                expandMacroInCode(id, expFile, expLine, expCol, visited,
                                  appendReplacementFromOrigin);
            } else {
                appendReplacementFromOrigin(id, defPos.file, defPos.line,
                                            defPos.column);
            }
            continue;
        }
        appendReplacementFromOrigin(QString(c), defPos.file, defPos.line,
                                    defPos.column);
        ++p;
    }
    visited.removeOne(name);
}

std::optional<bool> AsPreprocesser::evalExpression(const QString &expr,
                                                   const QString &file,
                                                   qint64 line, qint64 col) {
    // check grammar
    auto ccode = expr.toUtf8();
    antlr4::ANTLRInputStream input(ccode.constData(), ccode.length());
    AngelscriptExprParserLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);

    // reuse the listener, ha!
    CStructErrorListener lis([line, col, this](const MsgInfo &info) {
        PreprocError e{PreprocErrorCode::ERR_IF_PARSE,
                       info.type == MsgType::Error ? Severity::Error
                                                   : Severity::Warning,
                       m_currentSource,
                       line,
                       col,
                       info.info};
        errorReport(e);
    });

    AngelscriptExprParserParser parser(&tokens);
    parser.removeErrorListeners();
    parser.addErrorListener(&lis);

    AsExprEvaluator visitor;
    try {
        return visitor.eval(parser.expr());
    } catch (const AsExprException &ex) {
        PreprocError e{PreprocErrorCode::ERR_IF_PARSE,
                       Severity::Error,
                       m_currentSource,
                       line + ex.line - 1,
                       col + ex.column,
                       QString::fromUtf8(ex.what())};
        errorReport(e);
    } catch (const std::exception &ex) {
        PreprocError e{PreprocErrorCode::ERR_IF_PARSE,
                       Severity::Error,
                       m_currentSource,
                       line,
                       col,
                       QString::fromUtf8(ex.what())};
        errorReport(e);
    }

    return {};
}

std::function<void(const AsPreprocesser::PreprocError &)>
AsPreprocesser::getErrorHandler() const {
    return errorHandler;
}

void AsPreprocesser::setErrorHandler(
    const std::function<void(const PreprocError &)> &newerrorReport) {
    errorHandler = newerrorReport;
}

AsPreprocesser::Result AsPreprocesser::preprocess(const QByteArray &source,
                                                  const QString &sourceName) {
    Result res;
    res.checksum = Utilities::getMd5(source);
    res.source = QString::fromUtf8(source);
    processBuffer(source, sourceName, QFileInfo(sourceName).absolutePath(),
                  res.script, res.mapping);
    return res;
}

int AsPreprocesser::loadSectionFromFile(const QString &filename) {
    // The file name stored in the set should be the fully resolved name because
    // it is possible to name the same file in multiple ways using relative
    // paths.
    auto fullpath = QFileInfo(filename).absoluteFilePath();

    if (includeIfNotAlreadyIncluded(fullpath)) {
        int r = loadScriptSection(fullpath);
        if (r < 0)
            return r;
        else
            return 1;
    }

    return errOccurred ? -1 : 0;
}

QHash<QString, AsPreprocesser::Result> AsPreprocesser::scriptData() const {
    return modifiedScripts;
}

asIScriptEngine *AsPreprocesser::getEngine() const { return engine; }

void AsPreprocesser::setPragmaCallback(const CPragamaCallback &callback) {
    pragmaCallback = callback;
}

void AsPreprocesser::defineMacroWord(const QString &word,
                                     const QString &value) {
    m_runtimeMacros.insert(word, value);
}

void AsPreprocesser::clearAll() {
    includedScripts.clear();
    errOccurred = false;
}

int AsPreprocesser::loadScriptSection(const QString &filename) {
    if (modifiedScripts.size() >= 100) {
        engine->WriteMessage(filename.toUtf8(), 0, 0, asMSGTYPE_ERROR,
                             "Too many included files");
        return -1;
    }

    QFileInfo finfo(filename);
    QFile f(filename);
    if (!finfo.isFile() || f.isSequential()) {
        QString msg = QStringLiteral("Failed to open file '") +
                      finfo.absoluteFilePath() + QStringLiteral("'");
        engine->WriteMessage(filename.toUtf8(), 0, 0, asMSGTYPE_ERROR,
                             msg.toUtf8());
        return -1;
    }

    // Open the script file
    if (f.size() > 1024 * 1024) {
        QString msg = QStringLiteral("Failed to open huge script file '") +
                      finfo.absoluteFilePath() + QStringLiteral("'");
        engine->WriteMessage(filename.toUtf8(), 0, 0, asMSGTYPE_ERROR,
                             msg.toUtf8());
        return -1;
    }

    if (!f.open(QFile::ReadOnly)) {
        // Write a message to the engine's message callback
        QString msg = QStringLiteral("Failed to open script file '") +
                      finfo.absoluteFilePath() + QStringLiteral("'");
        engine->WriteMessage(filename.toUtf8(), 0, 0, asMSGTYPE_ERROR,
                             msg.toUtf8());
        return -1;
    }

    // Read the entire file
    auto code = f.readAll();
    f.close();

    // Process the script section even if it is zero length so that the name
    // is registered
    auto ret = preprocess(code, filename);
    addScriptSection(filename, ret);

    return errOccurred ? -1 : 0;
}

bool AsPreprocesser::includeIfNotAlreadyIncluded(const QString &filename) {
    if (includedScripts.contains(filename)) {
        // Already included
        return false;
    }

    // Add the file to the set of included sections
    includedScripts.append(filename);
    return true;
}

void AsPreprocesser::errorReport(const PreprocError &error) {
    if (errorHandler) {
        if (error.severity == Severity::Error) {
            errOccurred = true;
        }
        errorHandler(error);
    }
}

void AsPreprocesser::addScriptSection(const QString &section,
                                      const Result &result) {
    modifiedScripts.insert(section, result);
}

std::optional<AsPreprocesser::SourcePos>
AsPreprocesser::mapErrPos2Src(const QString &section, qint64 line, qint64 col) {
    if (modifiedScripts.contains(section)) {
        return modifiedScripts[section].mapping.mapOutputPositionToSource(line,
                                                                          col);
    }
    return {};
}
