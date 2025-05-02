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
#include "class/qascodeparser.h"
#include "class/scriptmachine.h"

#include <QDir>
#include <QFileInfo>
#include <QStack>

Q_GLOBAL_STATIC_WITH_ARGS(
    QByteArrayList, DEFAULT_MARCO,
    ({"__AS_ARRAY__", "__AS_ANY__", "__AS_GRID__", "__AS_HANDLE__",
      "__AS_MATH__", "__AS_WEAKREF__", "__AS_COROUTINE__", "__WING_FILE__",
      "__WING_STRING__", "__WING_COLOR__", "__WING_JSON__", "__WING_REGEX__",
      "__WING_DICTIONARY__"}));

AsPreprocesser::AsPreprocesser(asIScriptEngine *engine) : engine(engine) {
    Q_ASSERT(engine);

    includeCallback = nullptr;
    includeParam = nullptr;

    pragmaCallback = nullptr;
    pragmaParam = nullptr;

    for (auto &m : *DEFAULT_MARCO) {
        definedWords.insert(m, {});
    }
}

AsPreprocesser::~AsPreprocesser() { void ClearAll(); }

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

int AsPreprocesser::loadSectionFromMemory(const QString &section,
                                          const QByteArray &code) {
    int r = processScriptSection(code, section);
    if (r < 0)
        return r;
    else
        return 1;
}

QList<AsPreprocesser::ScriptData> AsPreprocesser::scriptData() const {
    return modifiedScripts;
}

asIScriptEngine *AsPreprocesser::getEngine() { return engine; }

void AsPreprocesser::setIncludeCallback(INCLUDECALLBACK_t callback,
                                        void *userParam) {
    includeCallback = callback;
    includeParam = userParam;
}

void AsPreprocesser::setPragmaCallback(PRAGMACALLBACK_t callback,
                                       void *userParam) {
    pragmaCallback = callback;
    pragmaParam = userParam;
}

bool AsPreprocesser::defineWord(const QString &word, const QByteArray &value) {
    // try to modify system marco is not allowed
    if (DEFAULT_MARCO->contains(word)) {
        return false;
    }
    definedWords.insert(word, value);
    return true;
}

unsigned int AsPreprocesser::sectionCount() const {
    return (unsigned int)(includedScripts.size());
}

QString AsPreprocesser::sectionName(unsigned int idx) const {
    if (qsizetype(idx) >= qsizetype(includedScripts.size()))
        return {};

    return includedScripts.at(idx);
}

void AsPreprocesser::clearAll() { includedScripts.clear(); }

int AsPreprocesser::processScriptSection(const QByteArray &script,
                                         const QString &sectionname) {

    // #define DBG_CODEP  // debug macro for pre-process

    QVector<QPair<QString, bool>> includes;

    QByteArray modifiedScript = script;

    // First perform the checks for #if directives to exclude code that
    // shouldn't be compiled
    QByteArray::size_type pos = 0;

    QStack<std::optional<bool>> m_condtionStack;

    while (pos < modifiedScript.size()) {
        asUINT len = 0;
        asETokenClass t = engine->ParseToken(modifiedScript.data() + pos,
                                             modifiedScript.size() - pos, &len);
        if (t == asTC_UNKNOWN && modifiedScript[pos] == '#' &&
            (pos + 1 < modifiedScript.size())) {
            int start = pos++;

            // Is this an #if directive?
            t = engine->ParseToken(modifiedScript.data() + pos,
                                   modifiedScript.size() - pos, &len);
            Q_UNUSED(t);

            QByteArray token = modifiedScript.sliced(pos, len);
            pos += len;

            bool isIf = token == QStringLiteral("if");
            bool isIfDef = token == QStringLiteral("ifdef");
            bool isIfnDef = token == QStringLiteral("ifndef");
            bool isDef = token == QStringLiteral("define");
            bool isUnDef = token == QStringLiteral("undef");

            if (isIf || isIfDef || isIfnDef || isDef || isUnDef) {
                t = engine->ParseToken(modifiedScript.data() + pos,
                                       modifiedScript.size() - pos, &len);
                if (t == asTC_WHITESPACE) {
                    if (std::any_of(modifiedScript.data() + pos,
                                    modifiedScript.data() + pos + len,
                                    [](char ch) { return ch == '\n'; })) {
                        if (_isCodeCompleteMode) {
                            pos = modifiedScript.indexOf('\n', pos);
                            continue;
                        }

                        auto str = QObject::tr("IfDefNoWord");
                        engine->WriteMessage(sectionname.toUtf8(),
                                             getLineCount(modifiedScript, pos),
                                             1, asMSGTYPE_ERROR, str.toUtf8());
                        return asERROR;
                    }
                    pos += len;
                    t = engine->ParseToken(modifiedScript.data() + pos,
                                           modifiedScript.size() - pos, &len);
                }

                if (isIfDef || isIfnDef) {
                    if (_isCodeCompleteMode) {
                        auto pos = modifiedScript.indexOf('\n', start);
                        if (pos < 0) {
                            overwriteCode(modifiedScript, start,
                                          modifiedScript.size() - start - 1);
                        } else {
                            overwriteCode(modifiedScript, start, pos - start);
                        }
                        continue;
                    }
                    if (t == asTC_IDENTIFIER) {
                        QByteArray word = modifiedScript.sliced(pos, len);

                        // Overwrite the directive with space characters to
                        // avoid compiler error
                        pos += len;
                        overwriteCode(modifiedScript, start, pos - start);

                        // Has this identifier been defined by the application
                        // or not?
                        if ((isIfDef && !definedWords.contains(word)) ||
                            (isIfnDef && definedWords.contains(word))) {
                            // Exclude all the code until and including the
                            // #endif
                            pos = excludeIfCode(modifiedScript, pos);
                            m_condtionStack.push(false);
                        } else {
                            m_condtionStack.push(true);
                        }
#ifdef DBG_CODEP
                        qDebug().noquote() << modifiedScript;
#endif
                    } else {
                        auto str = QObject::tr("IfDefInvalidWord");
                        engine->WriteMessage(sectionname.toUtf8(),
                                             getLineCount(modifiedScript, pos),
                                             1, asMSGTYPE_ERROR, str.toUtf8());
                        return asERROR;
                    }

                    // ensure end line
                    if (endLinePassFailed(modifiedScript, pos)) {
                        auto str = QObject::tr("UnexceptedToken");
                        engine->WriteMessage(sectionname.toUtf8(),
                                             getLineCount(modifiedScript, pos),
                                             1, asMSGTYPE_ERROR, str.toUtf8());
                        return asERROR;
                    }
                } else if (isIf) {
                    if (_isCodeCompleteMode) {
                        auto pos = modifiedScript.indexOf('\n', start);
                        if (pos < 0) {
                            overwriteCode(modifiedScript, start,
                                          modifiedScript.size() - start - 1);
                        } else {
                            overwriteCode(modifiedScript, start, pos - start);
                        }
                        continue;
                    }
                    // evalutate the string
                    auto npos = modifiedScript.indexOf('\n', pos);
                    QByteArray codes;
                    if (npos >= 0) {
                        codes = modifiedScript.sliced(pos, npos - pos);
                    } else {
                        codes = modifiedScript.sliced(pos);
                    }
                    overwriteCode(modifiedScript, start, npos - start);

                    auto &sm = ScriptMachine::instance();
                    bool ok = false;
                    auto ret = sm.evaluateDefine(codes, ok);

                    if (ret < 0) {
                        auto str = QObject::tr("CalIfFailed");
                        engine->WriteMessage(sectionname.toUtf8(),
                                             getLineCount(modifiedScript, pos),
                                             1, asMSGTYPE_ERROR, str.toUtf8());
                        return asERROR;
                    } else {
                        if (ok) {
                            m_condtionStack.push(true);
                        } else {
                            pos = excludeIfCode(modifiedScript, npos);
                            m_condtionStack.push(false);
                        }
                    }
#ifdef DBG_CODEP
                    qDebug().noquote() << modifiedScript;
#endif
                } else if (isDef) {
                    if (t == asTC_IDENTIFIER) {
                        QByteArray word = modifiedScript.sliced(pos, len);
                        pos += len;

                        if (_isCodeCompleteMode) {
                            defineWord(word, {});
                            auto pos = modifiedScript.indexOf('\n', start);
                            if (pos < 0) {
                                overwriteCode(modifiedScript, start,
                                              modifiedScript.size() - start -
                                                  1);
                            } else {
                                overwriteCode(modifiedScript, start,
                                              pos - start);
                            }
                            continue;
                        }

                        t = engine->ParseToken(modifiedScript.data() + pos,
                                               modifiedScript.size() - pos,
                                               &len);
                        if (t == asTC_WHITESPACE) {
                            // line break?
                            if (std::any_of(
                                    modifiedScript.data() + pos,
                                    modifiedScript.data() + pos + len,
                                    [](char ch) { return ch == '\n'; })) {
                                defineWord(word, {});
                            } else {
                                pos += len;
                                size_t total = 0;
                                auto v = QAsCodeParser::getToken(
                                    engine, modifiedScript.data() + pos,
                                    modifiedScript.size() - pos, &total);
                                // only support these things
                                switch (v) {
                                case ttIdentifier:
                                case ttIntConstant:
                                case ttFloatConstant:
                                case ttDoubleConstant:
                                case ttBitsConstant:
                                case ttStringConstant: {
                                    auto v = modifiedScript.sliced(pos, total);
                                    defineWord(word, v);
                                } break;
                                default:
                                    auto str = QObject::tr("UnexceptedToken");
                                    engine->WriteMessage(
                                        sectionname.toUtf8(),
                                        getLineCount(modifiedScript, pos), 1,
                                        asMSGTYPE_ERROR, str.toUtf8());
                                    return asERROR;
                                }

                                pos += total;
                            }
                            overwriteCode(modifiedScript, start, pos - start);
                        }

                        // ensure end line
                        if (endLinePassFailed(modifiedScript, pos)) {
                            auto str = QObject::tr("UnexceptedToken");
                            engine->WriteMessage(
                                sectionname.toUtf8(),
                                getLineCount(modifiedScript, pos), 1,
                                asMSGTYPE_ERROR, str.toUtf8());
                            return asERROR;
                        }
                    } else {
                        if (_isCodeCompleteMode) {
                            auto pos = modifiedScript.indexOf('\n', start);
                            if (pos < 0) {
                                overwriteCode(modifiedScript, start,
                                              modifiedScript.size() - start -
                                                  1);
                            } else {
                                overwriteCode(modifiedScript, start,
                                              pos - start);
                            }
                            continue;
                        }
                        auto str = QObject::tr("InvalidDef");
                        engine->WriteMessage(sectionname.toUtf8(),
                                             getLineCount(modifiedScript, pos),
                                             1, asMSGTYPE_ERROR, str.toUtf8());
                        return asERROR;
                    }
                } else if (isUnDef) {
                    if (_isCodeCompleteMode) {
                        auto pos = modifiedScript.indexOf('\n', start);
                        if (pos < 0) {
                            overwriteCode(modifiedScript, start,
                                          modifiedScript.size() - start - 1);
                        } else {
                            overwriteCode(modifiedScript, start, pos - start);
                        }
                        continue;
                    }
                    if (t == asTC_IDENTIFIER) {
                        QByteArray word = modifiedScript.sliced(pos, len);

                        // Overwrite the directive with space characters to
                        // avoid compiler error
                        pos += len;
                        overwriteCode(modifiedScript, start, pos - start);
                        constexpr auto PREFIX = "__";
                        if (word.startsWith(PREFIX) && word.endsWith(PREFIX)) {
                            // Warning
                            auto str = QObject::tr("ReservedMarcoType");
                            engine->WriteMessage(
                                sectionname.toUtf8(),
                                getLineCount(modifiedScript, pos), 1,
                                asMSGTYPE_WARNING, str.toUtf8());
                        } else {
                            if (!definedWords.remove(word)) {
                                auto str = QObject::tr("MarcoNotFound:") + word;
                                engine->WriteMessage(
                                    sectionname.toUtf8(),
                                    getLineCount(modifiedScript, pos), 1,
                                    asMSGTYPE_WARNING, str.toUtf8());
                            }
                        }

                        // ensure end line
                        if (endLinePassFailed(modifiedScript, pos)) {
                            auto str = QObject::tr("UnexceptedToken");
                            engine->WriteMessage(
                                sectionname.toUtf8(),
                                getLineCount(modifiedScript, pos), 1,
                                asMSGTYPE_ERROR, str.toUtf8());
                            return asERROR;
                        }
                    } else {
                        auto str = QObject::tr("InvalidDef");
                        engine->WriteMessage(sectionname.toUtf8(),
                                             getLineCount(modifiedScript, pos),
                                             1, asMSGTYPE_ERROR, str.toUtf8());
                        return asERROR;
                    }
                }
            } else if (token == "else") {
                if (_isCodeCompleteMode) {
                    overwriteCode(modifiedScript, start, pos - start);
                    continue;
                }
                if (m_condtionStack.isEmpty()) {
                    auto str = QObject::tr("NoMatchingIf");
                    engine->WriteMessage(sectionname.toUtf8(),
                                         getLineCount(modifiedScript, pos), 1,
                                         asMSGTYPE_ERROR, str.toUtf8());
                    return asERROR;
                } else {
                    overwriteCode(modifiedScript, start, pos - start);
                    auto opBool = m_condtionStack.top();
                    if (opBool) {
                        if (opBool.value()) {
                            pos = excludeElseCode(modifiedScript, pos);
                            m_condtionStack.top().reset();
                        }

                        // ensure end line
                        if (endLinePassFailed(modifiedScript, pos - 1)) {
                            auto str = QObject::tr("UnexceptedToken");
                            engine->WriteMessage(
                                sectionname.toUtf8(),
                                getLineCount(modifiedScript, pos), 1,
                                asMSGTYPE_ERROR, str.toUtf8());
                            return asERROR;
                        }
                    } else {
                        auto str = QObject::tr("DupElseDef");
                        engine->WriteMessage(sectionname.toUtf8(),
                                             getLineCount(modifiedScript, pos),
                                             1, asMSGTYPE_ERROR, str.toUtf8());
                        return asERROR;
                    }
                }
#ifdef DBG_CODEP
                qDebug().noquote() << modifiedScript;
#endif
            } else if (token == "endif") {
                if (_isCodeCompleteMode) {
                    overwriteCode(modifiedScript, start, pos - start);
                    continue;
                }
                // Only remove the #endif if there was a matching #if
                if (m_condtionStack.isEmpty()) {
                    auto str = QObject::tr("NoMatchingIf");
                    engine->WriteMessage(sectionname.toUtf8(),
                                         getLineCount(modifiedScript, pos), 1,
                                         asMSGTYPE_ERROR, str.toUtf8());
                    return asERROR;
                } else {
                    overwriteCode(modifiedScript, start, pos - start);
                    m_condtionStack.pop();
                }

                // ensure end line
                if (endLinePassFailed(modifiedScript, pos)) {
                    auto str = QObject::tr("UnexceptedToken");
                    engine->WriteMessage(sectionname.toUtf8(),
                                         getLineCount(modifiedScript, pos), 1,
                                         asMSGTYPE_ERROR, str.toUtf8());
                    return asERROR;
                }
#ifdef DBG_CODEP
                qDebug().noquote() << modifiedScript;
#endif
            }
        } else {
            if (t == asTC_IDENTIFIER) {
                // define replace
                auto word = modifiedScript.sliced(pos, len);
                auto rword = findReplaceResult(word);
                if (word != rword) {
                    modifiedScript.replace(pos, len, rword);
                    len = rword.length();
                }
            }
            pos += len;
        }
    }

#ifdef DBG_CODEP
    qDebug().noquote() << modifiedScript;
#endif

    // Then check for pre-processor directives
    pos = 0;
    while (pos >= 0 && pos < modifiedScript.size()) {
        asUINT len = 0;
        asETokenClass t = engine->ParseToken(modifiedScript.data() + pos,
                                             modifiedScript.size() - pos, &len);
        if (t == asTC_COMMENT || t == asTC_WHITESPACE) {
            pos += len;
            continue;
        }
        QString token = modifiedScript.sliced(pos, len);

        // Skip possible decorators before class and interface declarations
        if (token == "shared" || token == "abstract" || token == "mixin" ||
            token == "external") {
            pos += len;
            continue;
        }

        // Is this a preprocessor directive?
        if (token == "#" && (pos + 1 < modifiedScript.size())) {
            int start = pos++;

            t = engine->ParseToken(modifiedScript.data() + pos,
                                   modifiedScript.size() - pos, &len);
            if (t == asTC_IDENTIFIER) {
                token = modifiedScript.sliced(pos, len);
                if (token == "include") {
                    pos += len;
                    t = engine->ParseToken(modifiedScript.data() + pos,
                                           modifiedScript.size() - pos, &len);
                    if (t == asTC_WHITESPACE) {
                        pos += len;
                        t = engine->ParseToken(modifiedScript.data() + pos,
                                               modifiedScript.size() - pos,
                                               &len);
                    }

                    if (t == asTC_VALUE && len > 2 &&
                        (modifiedScript[pos] == '"' ||
                         modifiedScript[pos] == '\'')) {
                        // Get the include file
                        QString includefile =
                            modifiedScript.sliced(pos + 1, len - 2);
                        pos += len;

                        // Make sure the includeFile doesn't contain any
                        // line breaks
                        auto p = includefile.indexOf('\n');
                        if (p >= 0) {
                            auto str =
                                QObject::tr("Invalid file name for #include; "
                                            "it contains a line-break: ") +
                                QStringLiteral("'") + includefile.left(p) +
                                QStringLiteral("'");
                            engine->WriteMessage(
                                sectionname.toUtf8(),
                                getLineCount(modifiedScript, pos), 1,
                                asMSGTYPE_ERROR, str.toUtf8());
                        } else {
                            // Store it for later processing
                            includes.append({includefile, true});

                            // Overwrite the include directive with space
                            // characters to avoid compiler error
                            overwriteCode(modifiedScript, start, pos - start);
                        }
                    } else if (t == asTC_KEYWORD &&
                               modifiedScript[pos] == '<') {
                        pos += len;

                        // find the next '>'
                        auto rpos = pos;
                        bool found = false;
                        for (; rpos < modifiedScript.size(); ++rpos) {
                            if (modifiedScript[rpos] == '>') {
                                found = true;
                                break;
                            }
                            if (modifiedScript[rpos] == '\n') {
                                break;
                            }
                        }

                        if (found) {
                            QString includefile =
                                modifiedScript.sliced(pos, rpos - pos)
                                    .trimmed();

                            pos = rpos + 1;

                            // Make sure the includeFile doesn't contain any
                            // line breaks
                            auto p = includefile.indexOf('\n');
                            auto ws = includefile.indexOf(' ');
                            if (!includefile.isEmpty() && p >= 0 && ws >= 0) {
                                // TODO: Show the correct line number for
                                // the error
                                auto str =
                                    QObject::tr(
                                        "Invalid file name for #include; "
                                        "it contains a line-break: ") +
                                    QStringLiteral("'") + includefile.left(p) +
                                    QStringLiteral("'");
                                engine->WriteMessage(sectionname.toUtf8(), 0, 0,
                                                     asMSGTYPE_ERROR,
                                                     str.toUtf8());
                            } else {
                                // Store it for later processing
                                includes.append({includefile, false});

                                // Overwrite the include directive with
                                // space characters to avoid compiler error
                                overwriteCode(modifiedScript, start,
                                              pos - start);
                            }
                        } else {
                            auto str =
                                QObject::tr("Invalid file name for #include; "
                                            "it contains a line-break or "
                                            "unpaired symbol");
                            engine->WriteMessage(sectionname.toUtf8(), 0, 0,
                                                 asMSGTYPE_ERROR, str.toUtf8());
                        }
                    }
                } else if (token == "pragma") {
                    // Read until the end of the line
                    pos += len;
                    for (; pos < modifiedScript.size() &&
                           modifiedScript[pos] != '\n';
                         pos++)
                        ;

                    // Call the pragma callback
                    auto pragmaText =
                        modifiedScript.sliced(start + 7, pos - start - 7);

                    // Overwrite the pragma directive with space characters
                    // to avoid compiler error
                    overwriteCode(modifiedScript, start, pos - start);

                    if (!_isCodeCompleteMode) {
                        int r = pragmaCallback
                                    ? pragmaCallback(pragmaText, this,
                                                     sectionname, pragmaParam)
                                    : -1;
                        if (r < 0) {
                            // TODO: Report the correct line number
                            engine->WriteMessage(
                                sectionname.toUtf8(), 0, 0, asMSGTYPE_ERROR,
                                QObject::tr("Invalid #pragma directive")
                                    .toUtf8());
                            return r;
                        }
                    }
                }
            }
        }
        // Don't search for includes within statement blocks or
        // between tokens in statements
        else {
            pos = skipStatement(modifiedScript, pos);
        }
    }

    // Build the actual script
    engine->SetEngineProperty(asEP_COPY_SCRIPT_SECTIONS, true);

    addScriptSection(sectionname, modifiedScript);

    if (includes.size() > 0) {
        // If the callback has been set, then call it for each included file
        if (includeCallback) {
            for (QVector<QString>::size_type n = 0; n < includes.size(); n++) {
                auto inc = includes[n];
                int r = includeCallback(inc.first, inc.second, sectionname,
                                        this, includeParam);
                if (r < 0)
                    return r;
            }
        } else {
            // By default we try to load the included file from the relative
            // directory of the current file

            // Determine the path of the current script so that we can resolve
            // relative paths for includes
            auto path = QFileInfo(sectionname).filePath();

            // Load the included scripts
            for (QVector<QString>::size_type n = 0; n < includes.size(); n++) {
                // If the include is a relative path, then prepend the path of
                // the originating script

                auto inc = includes.at(n);
                if (!QFileInfo(inc.first).isAbsolute()) {
                    includes[n].first = path + QDir::separator() + inc.first;
                }

                // Include the script section
                int r = loadSectionFromFile(includes[n].first);
                if (r < 0)
                    return r;
            }
        }
    }

    return 0;
}

int AsPreprocesser::loadScriptSection(const QString &filename) {
    // Open the script file

    QFile f(filename);

    if (!f.open(QFile::ReadOnly)) {
        // Write a message to the engine's message callback
        auto msg = QObject::tr("Failed to open script file ") +
                   QStringLiteral("'") +
                   QFileInfo(filename).absoluteFilePath() + QStringLiteral("'");
        engine->WriteMessage(filename.toUtf8(), 0, 0, asMSGTYPE_ERROR,
                             msg.toUtf8());

        // TODO: Write the file where this one was included from

        return -1;
    }

    // Read the entire file
    auto code = f.readAll();
    f.close();

    // Process the script section even if it is zero length so that the name is
    // registered
    return processScriptSection(code, filename);
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

int AsPreprocesser::skipStatement(QByteArray &modifiedScript, int pos) {
    asUINT len = 0;

    // Skip until ; or { whichever comes first
    while (pos < (int)modifiedScript.length() && modifiedScript[pos] != ';' &&
           modifiedScript[pos] != '{') {
        engine->ParseToken(modifiedScript.data() + pos,
                           modifiedScript.size() - pos, &len);
        pos += len;
    }

    // Skip entire statement block
    if (pos < (int)modifiedScript.length() && modifiedScript[pos] == '{') {
        pos += 1;

        // Find the end of the statement block
        int level = 1;
        while (level > 0 && pos < (int)modifiedScript.size()) {
            asETokenClass t = engine->ParseToken(
                modifiedScript.data() + pos, modifiedScript.size() - pos, &len);
            if (t == asTC_KEYWORD) {
                if (modifiedScript[pos] == '{')
                    level++;
                else if (modifiedScript[pos] == '}')
                    level--;
            }
            pos += len;
        }
    } else {
        pos += 1;
    }

    return pos;
}

int AsPreprocesser::excludeIfCode(QByteArray &modifiedScript, int pos) {
    asUINT len = 0;
    int nested = 0;
    while (pos < (int)modifiedScript.size()) {
        engine->ParseToken(modifiedScript.data() + pos,
                           modifiedScript.size() - pos, &len);
        if (modifiedScript[pos] == '#') {
            auto sharpPos = pos;
            pos++;
            // Is it an #if or #endif directive?
            engine->ParseToken(modifiedScript.data() + pos,
                               modifiedScript.size() - pos, &len);
            QString token = modifiedScript.sliced(pos, len);

            if (token == "if" || token == "ifdef" || token == "ifndef") {
                nested++;
            } else if (token == "else") {
                if (nested-- == 0) {
                    pos = sharpPos;
                    break;
                }
            }

            modifiedScript[sharpPos] = ' ';
            overwriteCode(modifiedScript, pos, len);
        } else if (modifiedScript[pos] != '\n') {
            overwriteCode(modifiedScript, pos, len);
        }
        pos += len;
    }

    return pos;
}

int AsPreprocesser::excludeElseCode(QByteArray &modifiedScript, int pos) {
    asUINT len = 0;
    int nested = 0;
    while (pos < (int)modifiedScript.size()) {
        engine->ParseToken(modifiedScript.data() + pos,
                           modifiedScript.size() - pos, &len);
        if (modifiedScript[pos] == '#') {
            auto sharpPos = pos;
            pos++;
            // Is it an #if or #endif directive?
            engine->ParseToken(modifiedScript.data() + pos,
                               modifiedScript.size() - pos, &len);
            QString token = modifiedScript.sliced(pos, len);

            if (token == "if" || token == "ifdef" || token == "ifndef") {
                nested++;
            } else if (token == "endif") {
                if (nested-- == 0) {
                    pos = sharpPos;
                    break;
                }
            }

            modifiedScript[sharpPos] = ' ';
            overwriteCode(modifiedScript, pos, len);
        } else if (modifiedScript[pos] != '\n') {
            overwriteCode(modifiedScript, pos, len);
        }
        pos += len;
    }

    return pos;
}

void AsPreprocesser::overwriteCode(QByteArray &modifiedScript, int start,
                                   int len) {
    auto code = modifiedScript.data() + start;
    for (int n = 0; n < len; n++) {
        if (*code != '\n')
            *code = ' ';
        code++;
    }
}

int AsPreprocesser::getLineCount(const QByteArray &modifiedScript,
                                 int pos) const {
    pos = qBound(0, pos, int(modifiedScript.size()));
    return std::count_if(modifiedScript.begin(),
                         std::next(modifiedScript.begin(), pos),
                         [](char ch) -> bool { return ch == '\n'; }) +
           1;
}

bool AsPreprocesser::endLinePassFailed(const QByteArray &modifiedScript,
                                       int pos) { // ensure '\n' end line
    bool endError = false;
    asUINT len = 0;
    auto t = engine->ParseToken(modifiedScript.data() + pos,
                                modifiedScript.size() - pos, &len);
    if (t == asTC_WHITESPACE) {
        if (!std::any_of(modifiedScript.data() + pos,
                         modifiedScript.data() + pos + len,
                         [](char ch) { return ch == '\n'; })) {
            endError = true;
        }
    } else {
        if (len != 0) {
            endError = true;
        }
    }
    return endError;
}

QByteArray AsPreprocesser::findReplaceResult(const QByteArray &v) {
    QByteArray r = v;
    while (definedWords.contains(r)) {
        r = definedWords.value(r);
        if (r.isEmpty()) {
            break;
        }
    }
    return r;
}

bool AsPreprocesser::isCodeCompleteMode() const { return _isCodeCompleteMode; }

void AsPreprocesser::setIsCodeCompleteMode(bool newIsCodeCompleteMode) {
    _isCodeCompleteMode = newIsCodeCompleteMode;
}

QHash<QString, QByteArray> AsPreprocesser::definedMacros() const {
    return definedWords;
}

void AsPreprocesser::addScriptSection(const QString &section,
                                      const QByteArray &code) {
    ScriptData data;
    data.section = section;
    data.script = code;
    modifiedScripts.append(data);
}
