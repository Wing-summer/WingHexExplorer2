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
#include "class/scriptmachine.h"

#include <QDir>
#include <QFileInfo>
#include <QStack>

Q_GLOBAL_STATIC_WITH_ARGS(
    QStringList, DEFAULT_MARCO,
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

void AsPreprocesser::defineWord(const QString &word,
                                const DefineValueType &value) {
    if (!definedWords.contains(word)) {
        definedWords.insert(word, value);
    }
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

            if (isIf || isIfDef || isIfnDef) {
                t = engine->ParseToken(modifiedScript.data() + pos,
                                       modifiedScript.size() - pos, &len);
                if (t == asTC_WHITESPACE) {
                    pos += len;
                    t = engine->ParseToken(modifiedScript.data() + pos,
                                           modifiedScript.size() - pos, &len);
                }

                if (isIfDef || isIfnDef) {
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
                            pos = excludeCode(modifiedScript, pos);
                            m_condtionStack.push(false);
                        } else {
                            m_condtionStack.push(true);
                        }
                        qDebug().noquote() << modifiedScript;
                    }
                } else {
                    // evalutate the string
                    auto npos = modifiedScript.indexOf('\n', pos);
                    auto codes = modifiedScript.sliced(pos, npos - pos);
                    overwriteCode(modifiedScript, start, npos - start);

                    auto &sm = ScriptMachine::instance();
                    bool ok = false;
                    auto ret = sm.evaluateDefine(codes, ok);

                    if (ret < 0) {
                        return asERROR;
                    } else {
                        if (ok) {
                            m_condtionStack.push(true);
                        } else {
                            pos = excludeCode(modifiedScript, npos);
                            m_condtionStack.push(false);
                        }
                    }

                    qDebug().noquote() << modifiedScript;
                }
            } else if (token == "else") {
                if (m_condtionStack.isEmpty()) {
                    // TODO
                    return asERROR;
                } else {
                    overwriteCode(modifiedScript, start, pos - start);
                    auto opBool = m_condtionStack.top();
                    if (opBool) {
                        if (opBool.value()) {
                            pos = excludeCode(modifiedScript, pos);
                            m_condtionStack.top().reset();
                        }
                    } else {
                        // TODO
                        return asERROR;
                    }
                }
                qDebug().noquote() << modifiedScript;
            } else if (token == "endif") {
                // Only remove the #endif if there was a matching #if
                if (m_condtionStack.isEmpty()) {
                    // TODO
                    return asERROR;
                } else {
                    overwriteCode(modifiedScript, start, pos - start);
                    m_condtionStack.pop();
                }
                qDebug().noquote() << modifiedScript;
            }
        } else {
            pos += len;
        }
    }

    qDebug().noquote() << modifiedScript;

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
                            // TODO: Show the correct line number for the
                            // error
                            auto str =
                                QObject::tr("Invalid file name for #include; "
                                            "it contains a line-break: ") +
                                QStringLiteral("'") + includefile.left(p) +
                                QStringLiteral("'");
                            engine->WriteMessage(sectionname.toUtf8(), 0, 0,
                                                 asMSGTYPE_ERROR, str.toUtf8());
                        } else {
                            // Store it for later processing
                            includes.append({includefile, true});

                            // Overwrite the include directive with space
                            // characters to avoid compiler error
                            overwriteCode(modifiedScript, start, pos - start);
                        }
                    }

                    if (t == asTC_KEYWORD && modifiedScript[pos] == '<') {
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

                    int r = pragmaCallback
                                ? pragmaCallback(pragmaText, this, sectionname,
                                                 pragmaParam)
                                : -1;
                    if (r < 0) {
                        // TODO: Report the correct line number
                        engine->WriteMessage(
                            sectionname.toUtf8(), 0, 0, asMSGTYPE_ERROR,
                            QObject::tr("Invalid #pragma directive").toUtf8());
                        return r;
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

int AsPreprocesser::skipStatement(const QByteArray &modifiedScript, int pos) {
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
    } else
        pos += 1;

    return pos;
}

int AsPreprocesser::excludeCode(QByteArray &modifiedScript, int pos) {
    asUINT len = 0;
    int nested = 1;
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
                modifiedScript[sharpPos] = ' ';
                overwriteCode(modifiedScript, pos, len);
                nested++;
            } else if (token == "endif" || token == "else") {
                if (nested-- == 0) {
                    pos = sharpPos - 1;
                    break;
                }
                modifiedScript[sharpPos] = ' ';
                overwriteCode(modifiedScript, pos, len);
            } else {
                modifiedScript[sharpPos] = ' ';
                overwriteCode(modifiedScript, pos, len);
            }
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

void AsPreprocesser::addScriptSection(const QString &section,
                                      const QByteArray &code) {
    ScriptData data;
    data.section = section;
    data.script = code;
    modifiedScripts.append(data);
}
