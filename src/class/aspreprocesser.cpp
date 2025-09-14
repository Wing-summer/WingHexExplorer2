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
#include "class/languagemanager.h"
#include "class/skinmanager.h"
#include "scriptmachine.h"

#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QMetaEnum>
#include <QRegularExpression>
#include <QStack>
#include <QVersionNumber>

Q_GLOBAL_STATIC_WITH_ARGS(
    QByteArrayList, DEFAULT_MARCO,
    ({"__AS_ARRAY__", "__AS_ANY__", "__AS_GRID__", "__AS_HANDLE__",
      "__AS_MATH__", "__AS_WEAKREF__", "__AS_COROUTINE__", "__WING_FILE__",
      "__WING_STRING__", "__WING_COLOR__", "__WING_JSON__", "__WING_REGEX__",
      "__WING_DICTIONARY__", "__WING_PRINT_VAR__", "__WING_PRINT_LN__",
      "__WING_CLIPBOARD__"}));

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

    for (auto &m : *DEFAULT_MARCO) {
        defineMacroWord(m);
    }

    static QHash<QString, QString> addInfos;
    if (addInfos.isEmpty()) {
        // software infos
        auto ver = QVersionNumber::fromString(WINGHEX_VERSION);
        addInfos.insert(QStringLiteral("__WING_VERSION__"),
                        "\"" WINGHEX_VERSION "\"");
        addInfos.insert(QStringLiteral("__WING_VERSION_MAJOR__"),
                        QString::number(ver.majorVersion()));
        addInfos.insert(QStringLiteral("__WING_VERSION_MINOR__"),
                        QString::number(ver.minorVersion()));
        addInfos.insert(QStringLiteral("__WING_VERSION_PATCH__"),
                        QString::number(ver.microVersion()));
        addInfos.insert(QStringLiteral("__ANGELSCRIPT_VERSION__"),
                        "\"" ANGELSCRIPT_VERSION_STRING "\"");
        addInfos.insert(QStringLiteral("__ANGELSCRIPT_VERSION_MAJOR__"),
                        QString::number(ANGELSCRIPT_VERSION / 10000));
        addInfos.insert(QStringLiteral("__ANGELSCRIPT_VERSION_MINOR__"),
                        QString::number((ANGELSCRIPT_VERSION / 100) % 100));
        addInfos.insert(QStringLiteral("__ANGELSCRIPT_VERSION_PATCH__"),
                        QString::number(ANGELSCRIPT_VERSION % 100));
        addInfos.insert(QStringLiteral("__WINGHEX_APPNAME__"),
                        "\"" APP_NAME "\"");
        addInfos.insert(QString("__WINGHEX_AUTHOR__"), "\"wingsummer\"");
        ver = QLibraryInfo::version();
        addInfos.insert(QStringLiteral("__QT_VERSION__"),
                        ver.toString().prepend('"').append('"'));
        addInfos.insert(QStringLiteral("__QT_VERSION_MAJOR__"),
                        QString::number(ver.majorVersion()));
        addInfos.insert(QStringLiteral("__QT_VERSION_MINOR__"),
                        QString::number(ver.minorVersion()));
        addInfos.insert(QStringLiteral("__QT_VERSION_PATCH__"),
                        QString::number(ver.microVersion()));

        // UI
        addInfos.insert(QStringLiteral("__LANG__"), LanguageManager::instance()
                                                        .defaultLocale()
                                                        .name()
                                                        .prepend('"')
                                                        .append('"'));
        auto theme = SkinManager::instance().currentTheme();
        auto te = QMetaEnum::fromType<SkinManager::Theme>();
        addInfos.insert(
            QStringLiteral("__THEME__"),
            QString(te.valueToKey(int(theme))).prepend('"').append('"'));
    }

    m_runtimeMacros.insert(addInfos);
}

AsPreprocesser::~AsPreprocesser() {}

void AsPreprocesser::registerBuiltin(
    const QString &name, std::function<QString(const SourcePos &)> cb) {
    m_builtinMacros[name] = cb;
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
        bool parentTaking;
        bool anyBranchTaken;
        bool currentTaking;
    };
    QVector<CondState> condStack;

    QString curOutLine;
    QVector<LineSegmentMap> curSegments;
    auto flushCurrLine = [&]() {
        outText += curOutLine;
        outText += "\n";
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
    auto appendSourceRange = [&](const char *p, qint64 len,
                                 const QString &file) {
        for (qint64 i = 0; i < len; ++i) {
            char c = p[i];
            if (c == '\n') {
                flushCurrLine();
                ++srcLine;
                srcCol = 1;
                lineHasSignificantToken = false;
            } else {
                appendCharMapped(QChar::fromLatin1(c), file, srcLine, srcCol);
                ++srcCol;
            }
        }
    };
    auto appendReplacementFromOrigin = [&](const QString &rep,
                                           const QString &file, int fileLine,
                                           int fileCol) {
        QByteArray ba = rep.toUtf8();
        for (int i = 0; i < ba.size(); ++i) {
            char c = ba[i];
            if (c == '\n') {
                flushCurrLine();
                ++fileLine;
                fileCol = 1;
            } else {
                appendCharMapped(QChar::fromLatin1(c), file, fileLine, fileCol);
                ++fileCol;
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

    // Expand macro (host-provided) in code; builtins use expansion site pos
    std::function<void(const QString &, const QString &, int, int,
                       QStringList &)>
        expandMacroInCode;
    expandMacroInCode = [&](const QString &name, const QString &expFile,
                            int expLine, int expCol, QStringList &visited) {
        if (!m_runtimeMacros.contains(name))
            return;
        if (visited.count(name)) {
            // recursion -> output the token itself mapped to source file (if we
            // have def pos use it)
            appendReplacementFromOrigin(name, expFile, expLine, expCol);
            return;
        }
        visited.append(name);
        QString val = m_runtimeMacros.value(name);
        SourcePos defPos = SourcePos{"<host>", 1, 1};
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
                appendReplacementFromOrigin(QString(c), defPos.file,
                                            defPos.line, defPos.column);
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
                while (p < val.size() &&
                       (val[p].isLetterOrNumber() || val[p] == '_' ||
                        val[p].unicode() > 127))
                    ++p;
                QString id = val.mid(st, p - st);
                if (m_builtinMacros.contains(id)) {
                    SourcePos sp{expFile, expLine, expCol};
                    QString rep = m_builtinMacros[id](sp);
                    appendReplacementFromOrigin(rep, expFile, expLine, expCol);
                } else if (m_runtimeMacros.contains(id)) {
                    expandMacroInCode(id, expFile, expLine, expCol, visited);
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
    };

    // handle directive line (a whole line that starts with #)
    auto handleDirective = [&](const QByteArray &dirBytes, qint64 dStartLine,
                               qint64 dStartCol) {
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
        static QRegularExpression re(R"(^(\w+)\b(.*)$)");
        auto m = re.match(rest);
        QString kw = m.hasMatch() ? m.captured(1) : QString();
        QString args = m.hasMatch() ? m.captured(2) : QString();

        auto emitBlankLine = [&]() {
            curOutLine.clear();
            curSegments.clear();
            flushCurrLine();
        };

        if (kw == "define" || kw == "undef") {
            // Always forbidden in script
            PreprocError e{PreprocErrorCode::ERR_SOURCE_DEFINE_FORBIDDEN,
                           Severity::Error,
                           m_currentSource,
                           dStartLine,
                           dStartCol,
                           QString("#%1 in source is forbidden; macros must be "
                                   "injected by host")
                               .arg(kw)};
            errorReport(e);
            emitBlankLine();
            return;
        }

        if (kw == "pragma") {
            if (pragmaCallback) {
                pragmaCallback(args, this, m_currentSource);
            }
            return;
        }

        if (kw == "include") {
            // parse include token
            QString incPath;
            bool isAngled = false;
            if (!parseIncludePathToken(args, incPath, isAngled)) {
                // bad include syntax -> report error and emit blank line
                PreprocError e{PreprocErrorCode::ERR_IF_PARSE,
                               Severity::Error,
                               m_currentSource,
                               dStartLine,
                               dStartCol,
                               QStringLiteral("Bad #include syntax: %1")
                                   .arg(args.trimmed()),
                               "Use #include \"file\" or #include <file>"};
                errorReport(e);
                emitBlankLine();
                return;
            }

            // Resolve path:
            QString resolved;
            if (!isAngled) {
                // for "": first try current file dir, then includePaths
                QString curDir = QFileInfo(m_currentSource).absolutePath();
                resolved = resolveIncludeFile(incPath, curDir);
            } else {
                // for <>: search only includePaths
                resolved = resolveIncludeFile(incPath, {});
            }

            if (resolved.isEmpty()) {
                PreprocError e{
                    PreprocErrorCode::ERR_INCLUDE_NOT_FOUND,
                    Severity::Error,
                    m_currentSource,
                    dStartLine,
                    dStartCol,
                    QStringLiteral("Included file not found: %1").arg(incPath),
                    "Check include paths or filename"};
                errorReport(e);
                // preserve one blank line so row count moves on (or choose to
                // preserve file lines if you prefer)
                emitBlankLine();
                return;
            }

            loadSectionFromFile(resolved);
            return;
        }

        if (kw == "if") {
            QString expanded = expandExpressionForIf(args, m_currentSource,
                                                     dStartLine, dStartCol);
            auto er = evalExpression(expanded, m_currentSource, dStartLine,
                                     dStartCol);
            bool val = er ? er.value() : false;
            bool parent =
                condStack.isEmpty() ? true : condStack.last().currentTaking;
            CondState cs;
            cs.parentTaking = parent;
            cs.currentTaking = parent && val;
            cs.anyBranchTaken = cs.currentTaking;
            condStack.append(cs);
            emitBlankLine();
            return;
        }

        if (kw == "ifdef") {
            QByteArray ba = args.toUtf8();
            QString name = parseFirstIdentifierInBA(ba);
            if (name.isEmpty()) {
                PreprocError e{PreprocErrorCode::ERR_IF_PARSE,
                               Severity::Error,
                               m_currentSource,
                               dStartLine,
                               dStartCol,
                               "bad #ifdef usage: missing identifier"};
                errorReport(e);
                CondState cs;
                cs.parentTaking =
                    condStack.isEmpty() ? true : condStack.last().currentTaking;
                cs.currentTaking = false;
                cs.anyBranchTaken = false;
                condStack.append(cs);
            } else {
                bool take = m_runtimeMacros.contains(name);
                bool parent =
                    condStack.isEmpty() ? true : condStack.last().currentTaking;
                CondState cs;
                cs.parentTaking = parent;
                cs.currentTaking = parent && take;
                cs.anyBranchTaken = cs.currentTaking;
                condStack.append(cs);
            }
            emitBlankLine();
            return;
        }

        if (kw == "ifndef") {
            QByteArray ba = args.toUtf8();
            QString name = parseFirstIdentifierInBA(ba);
            if (name.isEmpty()) {
                PreprocError e{PreprocErrorCode::ERR_IF_PARSE,
                               Severity::Error,
                               m_currentSource,
                               dStartLine,
                               dStartCol,
                               "bad #ifndef usage: missing identifier",
                               ""};
                errorReport(e);
                CondState cs;
                cs.parentTaking =
                    condStack.isEmpty() ? true : condStack.last().currentTaking;
                cs.currentTaking = false;
                cs.anyBranchTaken = false;
                condStack.append(cs);
            } else {
                bool take = !m_runtimeMacros.contains(name);
                bool parent =
                    condStack.isEmpty() ? true : condStack.last().currentTaking;
                CondState cs;
                cs.parentTaking = parent;
                cs.currentTaking = parent && take;
                cs.anyBranchTaken = cs.currentTaking;
                condStack.append(cs);
            }
            emitBlankLine();
            return;
        }

        if (kw == "elif") {
            if (condStack.isEmpty()) {
                PreprocError e{PreprocErrorCode::ERR_ELIF_ELSE_WITHOUT_IF,
                               Severity::Error,
                               m_currentSource,
                               dStartLine,
                               dStartCol,
                               "#elif without matching #if",
                               ""};
                errorReport(e);
                emitBlankLine();
                return;
            }
            CondState &top = condStack.last();
            bool parent = top.parentTaking;
            bool prevAny = top.anyBranchTaken;
            QString expanded = expandExpressionForIf(args, m_currentSource,
                                                     dStartLine, dStartCol);
            auto er = evalExpression(expanded, m_currentSource, dStartLine,
                                     dStartCol);
            bool val = er ? er.value() : false;
            bool current = parent && (!prevAny) && val;
            top.currentTaking = current;
            top.anyBranchTaken = prevAny || current;
            emitBlankLine();
            return;
        }

        if (kw == "else") {
            if (condStack.isEmpty()) {
                PreprocError e{PreprocErrorCode::ERR_ELIF_ELSE_WITHOUT_IF,
                               Severity::Error,
                               m_currentSource,
                               dStartLine,
                               dStartCol,
                               "#else without matching #if",
                               ""};
                errorReport(e);
                emitBlankLine();
                return;
            }
            CondState &top = condStack.last();
            bool parent = top.parentTaking;
            bool prevAny = top.anyBranchTaken;
            bool current = parent && (!prevAny);
            top.currentTaking = current;
            top.anyBranchTaken = true;
            emitBlankLine();
            return;
        }

        if (kw == "endif") {
            if (condStack.isEmpty()) {
                PreprocError e{PreprocErrorCode::ERR_ELIF_ELSE_WITHOUT_IF,
                               Severity::Error,
                               m_currentSource,
                               dStartLine,
                               dStartCol,
                               "#endif without matching #if",
                               ""};
                errorReport(e);
                emitBlankLine();
                return;
            }
            condStack.removeLast();
            emitBlankLine();
            return;
        }

        // unknown directive: remove line (preserve blank)
        emitBlankLine();
        return;
    }; // end handleDirective

    m_currentSource = sourceName;

    while (idx < n) {
        unsigned int tokenLen = 0;
        auto tokClass = engine->ParseToken(data + idx, n - idx, &tokenLen);

        if (tokenLen == 0)
            tokenLen = 1;
        const char *tokStart = data + idx;
        int tlen = (int)tokenLen;

        bool isDirective = (tokClass == asTC_UNKNOWN && tlen > 0 &&
                            tokStart[0] == '#' && !lineHasSignificantToken);
        bool inCond = !condStack.isEmpty();
        bool taking = !inCond || condStack.last().currentTaking;

        if (isDirective) {
            int j = idx;
            QByteArray dir;
            int dStartLine = srcLine, dStartCol = srcCol;
            // read whole directive line (handle backslash continuation)
            while (j < n) {
                int k = j;
                while (k < n && data[k] != '\n')
                    k++;
                dir.append(data + j, k - j);
                bool cont = false;
                if (k - j > 0) {
                    int m = k - 1;
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
            for (int p = 0; p < dir.size(); ++p) {
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
            for (int i = 0; i < tlen; ++i) {
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

        // active branch: emit tokens, expanding macros and builtins
        if (tokClass == asTC_WHITESPACE) {
            for (int i = 0; i < tlen; ++i) {
                char c = tokStart[i];
                if (c == '\n') {
                    flushCurrLine();
                    ++srcLine;
                    srcCol = 1;
                    lineHasSignificantToken = false;
                } else {
                    appendCharMapped(QChar::fromLatin1(c), sourceName, srcLine,
                                     srcCol);
                    ++srcCol;
                }
            }
            idx += tlen;
            continue;
        }
        if (tokClass == asTC_COMMENT) {
            appendSourceRange(tokStart, tlen, sourceName);
            idx += tlen;
            lineHasSignificantToken = true;
            continue;
        }
        if (tokClass == asTC_VALUE) {
            appendSourceRange(tokStart, tlen, sourceName);
            idx += tlen;
            lineHasSignificantToken = true;
            continue;
        }
        if (tokClass == asTC_IDENTIFIER || tokClass == asTC_KEYWORD) {
            QString ident = QString::fromUtf8(tokStart, tlen);
            // builtin macros first
            if (m_builtinMacros.contains(ident)) {
                SourcePos sp{sourceName, srcLine, srcCol};
                QString rep = m_builtinMacros[ident](sp);
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
            if (m_runtimeMacros.contains(ident)) {
                QStringList visited;
                expandMacroInCode(ident, sourceName, srcLine, srcCol, visited);
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
            appendSourceRange(tokStart, tlen, sourceName);
            idx += tlen;
            lineHasSignificantToken = true;
            continue;
        }

        // fallback: emit as-is
        appendSourceRange(tokStart, tlen, sourceName);
        idx += tlen;
        lineHasSignificantToken = true;
    } // main loop

    if (!condStack.isEmpty()) {
        PreprocError e{PreprocErrorCode::ERR_ENDIF_MISSING,
                       Severity::Error,
                       m_currentSource,
                       srcLine,
                       srcCol,
                       "Unterminated conditional block: missing #endif",
                       ""};
        errorReport(e);
    }
    if (!curOutLine.isEmpty() || !curSegments.isEmpty())
        flushCurrLine();
}

bool AsPreprocesser::parseIncludePathToken(const QString &args,
                                           QString &outPath, bool &isAngled) {
    QString s = args.trimmed();
    if (s.size() >= 2 && s.front() == '\"' && s.back() == '\"') {
        outPath = s.sliced(1, s.size() - 2);
        isAngled = false;
        return true;
    }
    if (s.size() >= 2 && s.front() == '<' && s.back() == '>') {
        outPath = s.sliced(1, s.size() - 2);
        isAngled = true;
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
    if (!currentDir.isEmpty()) {
        if (isAbsolute) {
            inc = filename;
        } else {
            auto pwd = QFileInfo(filename).absoluteDir();
            inc = pwd.absoluteFilePath(filename);
        }
    } else {
        // absolute include is not allowed in #include<>
        if (isAbsolute) {
            return {};
        }

        QDir dir(qApp->applicationDirPath());
        if (!dir.cd(QStringLiteral("aslib"))) {
            // someone crash the software
            return {};
        }
        inc = dir.absoluteFilePath(filename);
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
                                              qint64 col) {
    QString ret;

    qint64 pos = 0;
    auto codes = expr.toUtf8();
    auto total = codes.length();
    while (pos < total) {
        asUINT len = 0;
        auto t =
            engine->ParseToken(codes.data() + pos, codes.size() - pos, &len);
        auto word = codes.sliced(pos, len);
        if (t == asTC_IDENTIFIER) {
            if (m_runtimeMacros.contains(word)) {
                // get macro value (host provided)
                auto val = m_runtimeMacros.value(word);
                ret.append(val);
            }
        } else {
            ret.append(word);
        }
        pos += len;
    }

    return ret;
}

std::optional<bool> AsPreprocesser::evalExpression(const QString &expr,
                                                   const QString &file,
                                                   qint64 line, qint64 col) {
    // TODO check grammar

    // eval
    auto r = ScriptMachine::instance().evaluateDefine(expr);
    if (r.isValid()) {
        return r.toBool();
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

    return 0;
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
    // Open the script file
    QFile f(filename);

    if (!f.open(QFile::ReadOnly)) {
        // Write a message to the engine's message callback
        auto msg = QStringLiteral("Failed to open script file '") +
                   QFileInfo(filename).absoluteFilePath() + QStringLiteral("'");
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
        errorHandler(error);
        errOccurred = true;
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
