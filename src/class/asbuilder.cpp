#include "asbuilder.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>

BEGIN_AS_NAMESPACE

asBuilder::asBuilder() {
    engine = nullptr;
    module = nullptr;

    includeCallback = nullptr;
    includeParam = nullptr;

    pragmaCallback = nullptr;
    pragmaParam = nullptr;
}

void asBuilder::SetIncludeCallback(INCLUDECALLBACK_t callback,
                                   void *userParam) {
    includeCallback = callback;
    includeParam = userParam;
}

void asBuilder::SetPragmaCallback(PRAGMACALLBACK_t callback, void *userParam) {
    pragmaCallback = callback;
    pragmaParam = userParam;
}

int asBuilder::StartNewModule(asIScriptEngine *inEngine,
                              const char *moduleName) {
    if (inEngine == nullptr)
        return -1;

    engine = inEngine;
    module = inEngine->GetModule(moduleName, asGM_ALWAYS_CREATE);
    if (module == nullptr)
        return -1;

    ClearAll();

    return 0;
}

asIScriptEngine *asBuilder::GetEngine() { return engine; }

asIScriptModule *asBuilder::GetModule() { return module; }

unsigned int asBuilder::GetSectionCount() const {
    return (unsigned int)(includedScripts.size());
}

QString asBuilder::GetSectionName(unsigned int idx) const {
    if (idx >= includedScripts.size())
        return {};

    return includedScripts.at(idx);
}

// Returns 1 if the section was included
// Returns 0 if the section was not included because it had already been
// included before Returns <0 if there was an error
int asBuilder::AddSectionFromFile(const QString &filename) {
    // The file name stored in the set should be the fully resolved name because
    // it is possible to name the same file in multiple ways using relative
    // paths.
    auto fullpath = QFileInfo(filename).absoluteFilePath();

    if (IncludeIfNotAlreadyIncluded(fullpath)) {
        int r = LoadScriptSection(fullpath);
        if (r < 0)
            return r;
        else
            return 1;
    }

    return 0;
}

// Returns 1 if the section was included
// Returns 0 if the section was not included because it had already been
// included before Returns <0 if there was an error
int asBuilder::AddSectionFromMemory(const char *sectionName,
                                    const char *scriptCode,
                                    unsigned int scriptLength, int lineOffset) {
    if (IncludeIfNotAlreadyIncluded(sectionName)) {
        int r = ProcessScriptSection(scriptCode, scriptLength, sectionName,
                                     lineOffset);
        if (r < 0)
            return r;
        else
            return 1;
    }

    return 0;
}

int asBuilder::BuildModule() { return Build(); }

void asBuilder::DefineWord(const QString &word) {
    if (!definedWords.contains(word)) {
        definedWords.append(word);
    }
}

void asBuilder::ClearAll() {
    includedScripts.clear();

#if AS_PROCESS_METADATA == 1
    currentClass.clear();
    currentNamespace.clear();

    foundDeclarations.clear();
    typeMetadataMap.clear();
    funcMetadataMap.clear();
    varMetadataMap.clear();
#endif
}

bool asBuilder::IncludeIfNotAlreadyIncluded(const QString &filename) {
    if (includedScripts.contains(filename)) {
        // Already included
        return false;
    }

    // Add the file to the set of included sections
    includedScripts.append(filename);
    return true;
}

int asBuilder::LoadScriptSection(const QString &filename) {
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
    return ProcessScriptSection(code, code.length(), filename, 0);
}

int asBuilder::ProcessScriptSection(const QByteArray &script, int length,
                                    const QString &sectionname,
                                    int lineOffset) {
    QVector<QPair<QString, bool>> includes;

    // Perform a superficial parsing of the script first to store the metadata
    if (length)
        modifiedScript = script.left(length);
    else
        modifiedScript = script;

    // First perform the checks for #if directives to exclude code that
    // shouldn't be compiled
    unsigned int pos = 0;
    int nested = 0;
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

            QByteArray token = modifiedScript.mid(pos, len);
            pos += len;

            if (token == "if") {
                t = engine->ParseToken(modifiedScript.data() + pos,
                                       modifiedScript.size() - pos, &len);
                if (t == asTC_WHITESPACE) {
                    pos += len;
                    t = engine->ParseToken(modifiedScript.data() + pos,
                                           modifiedScript.size() - pos, &len);
                }

                if (t == asTC_IDENTIFIER) {
                    QByteArray word = modifiedScript.mid(pos, len);

                    // Overwrite the #if directive with space characters to
                    // avoid compiler error
                    pos += len;
                    OverwriteCode(start, pos - start);

                    // Has this identifier been defined by the application or
                    // not?
                    if (!definedWords.contains(word)) {
                        // Exclude all the code until and including the #endif
                        pos = ExcludeCode(pos);
                    } else {
                        nested++;
                    }
                }
            } else if (token == "endif") {
                // Only remove the #endif if there was a matching #if
                if (nested > 0) {
                    OverwriteCode(start, pos - start);
                    nested--;
                }
            }
        } else
            pos += len;
    }

#if AS_PROCESS_METADATA == 1
    // Preallocate memory
    QString name, declaration;
    QVector<QString> metadata;
    declaration.reserve(100);
#endif

    // Then check for meta data and pre-processor directives
    pos = 0;
    while (pos < modifiedScript.size()) {
        asUINT len = 0;
        asETokenClass t = engine->ParseToken(modifiedScript.data() + pos,
                                             modifiedScript.size() - pos, &len);
        if (t == asTC_COMMENT || t == asTC_WHITESPACE) {
            pos += len;
            continue;
        }
        QString token = modifiedScript.mid(pos, len);

#if AS_PROCESS_METADATA == 1
        // Skip possible decorators before class and interface declarations
        if (token == "shared" || token == "abstract" || token == "mixin" ||
            token == "external") {
            pos += len;
            continue;
        }

        // Check if class or interface so the metadata for members can be
        // gathered
        if (currentClass.isEmpty() &&
            (token == "class" || token == "interface")) {
            // Get the identifier after "class"
            do {
                pos += len;
                if (pos >= modifiedScript.size()) {
                    t = asTC_UNKNOWN;
                    break;
                }
                t = engine->ParseToken(modifiedScript.data() + pos,
                                       modifiedScript.size() - pos, &len);
            } while (t == asTC_COMMENT || t == asTC_WHITESPACE);

            if (t == asTC_IDENTIFIER) {
                currentClass = modifiedScript.mid(pos, len);

                // Search until first { or ; is encountered
                while (pos < modifiedScript.length()) {
                    engine->ParseToken(modifiedScript.data() + pos,
                                       modifiedScript.size() - pos, &len);

                    // If start of class section encountered stop
                    if (modifiedScript[pos] == '{') {
                        pos += len;
                        break;
                    } else if (modifiedScript[pos] == ';') {
                        // The class declaration has ended and there are no
                        // children
                        currentClass.clear();
                        pos += len;
                        break;
                    }

                    // Check next symbol
                    pos += len;
                }
            }

            continue;
        }

        // Check if end of class
        if (currentClass != "" && token == "}") {
            currentClass = "";
            pos += len;
            continue;
        }

        // Check if namespace so the metadata for members can be gathered
        if (token == "namespace") {
            // Get the scope after "namespace". It can be composed of multiple
            // nested namespaces, e.g. A::B::C
            do {
                do {
                    pos += len;
                    t = engine->ParseToken(modifiedScript.data() + pos,
                                           modifiedScript.size() - pos, &len);
                } while (t == asTC_COMMENT || t == asTC_WHITESPACE);

                if (t == asTC_IDENTIFIER) {
                    if (currentNamespace != "")
                        currentNamespace += "::";
                    currentNamespace += modifiedScript.mid(pos, len);
                }
            } while (
                t == asTC_IDENTIFIER ||
                (t == asTC_KEYWORD && modifiedScript.mid(pos, len) == "::"));

            // Search until first { is encountered
            while (pos < modifiedScript.length()) {
                engine->ParseToken(modifiedScript.data() + pos,
                                   modifiedScript.size() - pos, &len);

                // If start of namespace section encountered stop
                if (modifiedScript[pos] == '{') {
                    pos += len;
                    break;
                }

                // Check next symbol
                pos += len;
            }

            continue;
        }

        // Check if end of namespace
        if (currentNamespace != "" && token == "}") {
            size_t found = currentNamespace.lastIndexOf("::");
            if (found >= 0) {
                currentNamespace.remove(found, currentNamespace.size() - found);
            } else {
                currentNamespace = "";
            }
            pos += len;
            continue;
        }

        // Is this the start of metadata?
        if (token == "[") {
            // Get the metadata string
            pos = ExtractMetadata(pos, metadata);

            // Determine what this metadata is for
            int type;
            ExtractDeclaration(pos, name, declaration, type);

            // Store away the declaration in a map for lookup after the build
            // has completed
            if (type > 0) {
                SMetadataDecl decl(metadata, name, declaration, type,
                                   currentClass, currentNamespace);
                foundDeclarations.push_back(decl);
            }
        } else
#endif
            // Is this a preprocessor directive?
            if (token == "#" && (pos + 1 < modifiedScript.size())) {
                int start = pos++;

                t = engine->ParseToken(modifiedScript.data() + pos,
                                       modifiedScript.size() - pos, &len);
                if (t == asTC_IDENTIFIER) {
                    token = modifiedScript.mid(pos, len);
                    if (token == "include") {
                        pos += len;
                        t = engine->ParseToken(modifiedScript.data() + pos,
                                               modifiedScript.size() - pos,
                                               &len);
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
                                modifiedScript.mid(pos + 1, len - 2);
                            pos += len;

                            // Make sure the includeFile doesn't contain any
                            // line breaks
                            auto p = includefile.indexOf('\n');
                            if (p >= 0) {
                                // TODO: Show the correct line number for the
                                // error
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
                                includes.append({includefile, true});

                                // Overwrite the include directive with space
                                // characters to avoid compiler error
                                OverwriteCode(start, pos - start);
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
                                    modifiedScript.mid(pos, rpos - pos)
                                        .trimmed();

                                pos = rpos + 1;

                                // Make sure the includeFile doesn't contain any
                                // line breaks
                                auto p = includefile.indexOf('\n');
                                auto ws = includefile.indexOf(' ');
                                if (!includefile.isEmpty() && p >= 0 &&
                                    ws >= 0) {
                                    // TODO: Show the correct line number for
                                    // the error
                                    auto str =
                                        QObject::tr(
                                            "Invalid file name for #include; "
                                            "it contains a line-break: ") +
                                        QStringLiteral("'") +
                                        includefile.left(p) +
                                        QStringLiteral("'");
                                    engine->WriteMessage(sectionname.toUtf8(),
                                                         0, 0, asMSGTYPE_ERROR,
                                                         str.toUtf8());
                                } else {
                                    // Store it for later processing
                                    includes.append({includefile, false});

                                    // Overwrite the include directive with
                                    // space characters to avoid compiler error
                                    OverwriteCode(start, pos - start);
                                }
                            } else {
                                auto str = QObject::tr(
                                    "Invalid file name for #include; "
                                    "it contains a line-break or "
                                    "unpaired symbol");
                                engine->WriteMessage(sectionname.toUtf8(), 0, 0,
                                                     asMSGTYPE_ERROR,
                                                     str.toUtf8());
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
                            modifiedScript.mid(start + 7, pos - start - 7);
                        int r =
                            pragmaCallback
                                ? pragmaCallback(pragmaText, this, pragmaParam)
                                : -1;
                        if (r < 0) {
                            // TODO: Report the correct line number
                            engine->WriteMessage(
                                sectionname.toUtf8(), 0, 0, asMSGTYPE_ERROR,
                                QObject::tr("Invalid #pragma directive")
                                    .toUtf8());
                            return r;
                        }

                        // Overwrite the pragma directive with space characters
                        // to avoid compiler error
                        OverwriteCode(start, pos - start);
                    }
                } else {
                    // Check for lines starting with #!, e.g. shebang
                    // interpreter directive. These will be treated as comments
                    // and removed by the preprocessor
                    if (modifiedScript[pos] == '!') {
                        // Read until the end of the line
                        pos += len;
                        for (; pos < modifiedScript.size() &&
                               modifiedScript[pos] != '\n';
                             pos++)
                            ;

                        // Overwrite the directive with space characters to
                        // avoid compiler error
                        OverwriteCode(start, pos - start);
                    }
                }
            }
            // Don't search for metadata/includes within statement blocks or
            // between tokens in statements
            else {
                pos = SkipStatement(pos);
            }
    }

    // Build the actual script
    engine->SetEngineProperty(asEP_COPY_SCRIPT_SECTIONS, true);
    module->AddScriptSection(sectionname.toUtf8(), modifiedScript.data(),
                             modifiedScript.size(), lineOffset);

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
                int r = AddSectionFromFile(includes[n].first);
                if (r < 0)
                    return r;
            }
        }
    }

    return 0;
}

int asBuilder::Build() {
    int r = module->Build();
    if (r < 0)
        return r;

#if AS_PROCESS_METADATA == 1
    // After the script has been built, the metadata strings should be
    // stored for later lookup by function id, type id, and variable index
    for (int n = 0; n < (int)foundDeclarations.size(); n++) {
        SMetadataDecl &decl = foundDeclarations[n];
        module->SetDefaultNamespace(decl.nameSpace.toUtf8());
        if (decl.type == MDT_TYPE) {
            // Find the type id
            int typeId = module->GetTypeIdByDecl(decl.declaration.toUtf8());
            Q_ASSERT(typeId >= 0);
            if (typeId >= 0)
                typeMetadataMap.insert(typeId, decl.metadata);
        } else if (decl.type == MDT_FUNC) {
            if (decl.parentClass.isEmpty()) {
                // Find the function id
                asIScriptFunction *func =
                    module->GetFunctionByDecl(decl.declaration.toUtf8());
                Q_ASSERT(func);
                if (func)
                    funcMetadataMap.insert(func->GetId(), decl.metadata);
            } else {
                // Find the method id
                int typeId = module->GetTypeIdByDecl(decl.parentClass.toUtf8());
                Q_ASSERT(typeId > 0);

                auto it = classMetadataMap.find(typeId);
                if (it == classMetadataMap.end()) {
                    classMetadataMap.insert(typeId,
                                            SClassMetadata(decl.parentClass));
                    it = classMetadataMap.find(typeId);
                }

                asITypeInfo *type = engine->GetTypeInfoById(typeId);
                asIScriptFunction *func =
                    type->GetMethodByDecl(decl.declaration.toUtf8());
                Q_ASSERT(func);
                if (func)
                    it.value().funcMetadataMap.insert(func->GetId(),
                                                      decl.metadata);
            }
        } else if (decl.type == MDT_VIRTPROP) {
            if (decl.parentClass.isEmpty()) {
                // Find the global virtual property accessors
                asIScriptFunction *func = module->GetFunctionByName(
                    (QStringLiteral("get_") + decl.declaration).toUtf8());
                if (func)
                    funcMetadataMap.insert(func->GetId(), decl.metadata);
                func = module->GetFunctionByName(
                    (QStringLiteral("set_") + decl.declaration).toUtf8());
                if (func)
                    funcMetadataMap.insert(func->GetId(), decl.metadata);
            } else {
                // Find the method virtual property accessors
                int typeId = module->GetTypeIdByDecl(decl.parentClass.toUtf8());
                Q_ASSERT(typeId > 0);
                auto it = classMetadataMap.find(typeId);
                if (it == classMetadataMap.end()) {
                    classMetadataMap.insert(typeId,
                                            SClassMetadata(decl.parentClass));
                    it = classMetadataMap.find(typeId);
                }

                asITypeInfo *type = engine->GetTypeInfoById(typeId);
                asIScriptFunction *func = type->GetMethodByName(
                    (QStringLiteral("get_") + decl.declaration).toUtf8());
                if (func)
                    it.value().funcMetadataMap.insert(func->GetId(),
                                                      decl.metadata);
                func = type->GetMethodByName(
                    (QStringLiteral("set_") + decl.declaration).toUtf8());
                if (func)
                    it.value().funcMetadataMap.insert(func->GetId(),
                                                      decl.metadata);
            }
        } else if (decl.type == MDT_VAR) {
            if (decl.parentClass.isEmpty()) {
                // Find the global variable index
                int varIdx =
                    module->GetGlobalVarIndexByName(decl.declaration.toUtf8());
                Q_ASSERT(varIdx >= 0);
                if (varIdx >= 0)
                    varMetadataMap.insert(varIdx, decl.metadata);
            } else {
                int typeId = module->GetTypeIdByDecl(decl.parentClass.toUtf8());
                Q_ASSERT(typeId > 0);

                // Add the classes if needed
                auto it = classMetadataMap.find(typeId);
                if (it == classMetadataMap.end()) {
                    classMetadataMap.insert(typeId,
                                            SClassMetadata(decl.parentClass));
                    it = classMetadataMap.find(typeId);
                }

                // Add the variable to class
                asITypeInfo *objectType = engine->GetTypeInfoById(typeId);
                int idx = -1;

                // Search through all properties to get proper declaration
                for (asUINT i = 0; i < (asUINT)objectType->GetPropertyCount();
                     ++i) {
                    const char *name;
                    objectType->GetProperty(i, &name);
                    if (decl.declaration == name) {
                        idx = i;
                        break;
                    }
                }

                // If found, add it
                Q_ASSERT(idx >= 0);
                if (idx >= 0)
                    it.value().varMetadataMap.insert(idx, decl.metadata);
            }
        } else if (decl.type == MDT_FUNC_OR_VAR) {
            if (decl.parentClass.isEmpty()) {
                // Find the global variable index
                int varIdx =
                    module->GetGlobalVarIndexByName(decl.name.toUtf8());
                if (varIdx >= 0)
                    varMetadataMap.insert(varIdx, decl.metadata);
                else {
                    asIScriptFunction *func =
                        module->GetFunctionByDecl(decl.declaration.toUtf8());
                    Q_ASSERT(func);
                    if (func)
                        funcMetadataMap.insert(func->GetId(), decl.metadata);
                }
            } else {
                int typeId = module->GetTypeIdByDecl(decl.parentClass.toUtf8());
                Q_ASSERT(typeId > 0);

                // Add the classes if needed
                auto it = classMetadataMap.find(typeId);
                if (it == classMetadataMap.end()) {
                    classMetadataMap.insert(typeId,
                                            SClassMetadata(decl.parentClass));
                    it = classMetadataMap.find(typeId);
                }

                // Add the variable to class
                asITypeInfo *objectType = engine->GetTypeInfoById(typeId);
                int idx = -1;

                // Search through all properties to get proper declaration
                for (asUINT i = 0; i < (asUINT)objectType->GetPropertyCount();
                     ++i) {
                    const char *name;
                    objectType->GetProperty(i, &name);
                    if (decl.name == name) {
                        idx = i;
                        break;
                    }
                }

                // If found, add it
                if (idx >= 0)
                    it.value().varMetadataMap.insert(idx, decl.metadata);
                else {
                    // Look for the matching method instead
                    asITypeInfo *type = engine->GetTypeInfoById(typeId);
                    asIScriptFunction *func =
                        type->GetMethodByDecl(decl.declaration.toUtf8());
                    if (func)
                        it.value().funcMetadataMap.insert(func->GetId(),
                                                          decl.metadata);
                }
            }
        }
    }
    module->SetDefaultNamespace("");
#endif

    return 0;
}

int asBuilder::SkipStatement(int pos) {
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

// Overwrite all code with blanks until the matching #endif
int asBuilder::ExcludeCode(int pos) {
    asUINT len = 0;
    int nested = 0;
    while (pos < (int)modifiedScript.size()) {
        engine->ParseToken(modifiedScript.data() + pos,
                           modifiedScript.size() - pos, &len);
        if (modifiedScript[pos] == '#') {
            modifiedScript[pos] = ' ';
            pos++;

            // Is it an #if or #endif directive?
            engine->ParseToken(modifiedScript.data() + pos,
                               modifiedScript.size() - pos, &len);
            QString token = modifiedScript.mid(pos, len);
            OverwriteCode(pos, len);

            if (token == "if") {
                nested++;
            } else if (token == "endif") {
                if (nested-- == 0) {
                    pos += len;
                    break;
                }
            }
        } else if (modifiedScript[pos] != '\n') {
            OverwriteCode(pos, len);
        }
        pos += len;
    }

    return pos;
}

// Overwrite all characters except line breaks with blanks
void asBuilder::OverwriteCode(int start, int len) {
    auto code = modifiedScript.data() + start;
    for (int n = 0; n < len; n++) {
        if (*code != '\n')
            *code = ' ';
        code++;
    }
}

#if AS_PROCESS_METADATA == 1
int asBuilder::ExtractMetadata(int pos, QVector<QString> &metadata) {
    metadata.clear();

    // Extract all metadata. They can be separated by whitespace and comments
    for (;;) {
        QString metadataString;

        // Overwrite the metadata with space characters to allow compilation
        modifiedScript[pos] = ' ';

        // Skip opening brackets
        pos += 1;

        int level = 1;
        asUINT len = 0;
        while (level > 0 && pos < (int)modifiedScript.size()) {
            asETokenClass t = engine->ParseToken(
                modifiedScript.data() + pos, modifiedScript.size() - pos, &len);
            if (t == asTC_KEYWORD) {
                if (modifiedScript[pos] == '[')
                    level++;
                else if (modifiedScript[pos] == ']')
                    level--;
            }

            // Copy the metadata to our buffer
            if (level > 0)
                metadataString.append(modifiedScript.mid(pos, len));

            // Overwrite the metadata with space characters to allow compilation
            if (t != asTC_WHITESPACE)
                OverwriteCode(pos, len);

            pos += len;
        }

        metadata.push_back(metadataString);

        // Check for more metadata. Possibly separated by comments
        asETokenClass t = engine->ParseToken(modifiedScript.data() + pos,
                                             modifiedScript.size() - pos, &len);
        while (t == asTC_COMMENT || t == asTC_WHITESPACE) {
            pos += len;
            t = engine->ParseToken(modifiedScript.data() + pos,
                                   modifiedScript.size() - pos, &len);
        }

        if (modifiedScript[pos] != '[')
            break;
    }

    return pos;
}

int asBuilder::ExtractDeclaration(int pos, QString &name, QString &declaration,
                                  int &type) {
    declaration.clear();
    type = 0;

    int start = pos;

    QString token;
    asUINT len = 0;
    asETokenClass t = asTC_WHITESPACE;

    // Skip white spaces, comments, and leading decorators
    do {
        pos += len;
        t = engine->ParseToken(modifiedScript.data() + pos,
                               modifiedScript.size() - pos, &len);
        token = modifiedScript.mid(pos, len);
    } while (t == asTC_WHITESPACE || t == asTC_COMMENT || token == "private" ||
             token == "protected" || token == "shared" || token == "external" ||
             token == "final" || token == "abstract");

    // We're expecting, either a class, interface, function, or variable
    // declaration
    if (t == asTC_KEYWORD || t == asTC_IDENTIFIER) {
        token = modifiedScript.mid(pos, len);
        if (token == "interface" || token == "class" || token == "enum") {
            // Skip white spaces and comments
            do {
                pos += len;
                t = engine->ParseToken(modifiedScript.data() + pos,
                                       modifiedScript.size() - pos, &len);
            } while (t == asTC_WHITESPACE || t == asTC_COMMENT);

            if (t == asTC_IDENTIFIER) {
                type = MDT_TYPE;
                declaration = modifiedScript.mid(pos, len);
                pos += len;
                return pos;
            }
        } else {
            // For function declarations, store everything up to the start of
            // the statement block, except for succeeding decorators (final,
            // override, etc)

            // For variable declaration store just the name as there can only be
            // one

            // We'll only know if the declaration is a variable or function
            // declaration when we see the statement block, or absense of a
            // statement block.
            bool hasParenthesis = false;
            int nestedParenthesis = 0;
            declaration.append(modifiedScript.mid(pos, len));
            pos += len;
            for (; pos < (int)modifiedScript.size();) {
                t = engine->ParseToken(modifiedScript.data() + pos,
                                       modifiedScript.size() - pos, &len);
                token = modifiedScript.mid(pos, len);
                if (t == asTC_KEYWORD) {
                    if (token == "{" && nestedParenthesis == 0) {
                        if (hasParenthesis) {
                            // We've found the end of a function signature
                            type = MDT_FUNC;
                        } else {
                            // We've found a virtual property. Just keep the
                            // name
                            declaration = name;
                            type = MDT_VIRTPROP;
                        }
                        return pos;
                    }
                    if ((token == "=" && !hasParenthesis) || token == ";") {
                        if (hasParenthesis) {
                            // The declaration is ambigous. It can be a variable
                            // with initialization, or a function prototype
                            type = MDT_FUNC_OR_VAR;
                        } else {
                            // Substitute the declaration with just the name
                            declaration = name;
                            type = MDT_VAR;
                        }
                        return pos;
                    } else if (token == "(") {
                        nestedParenthesis++;

                        // This is the first parenthesis we encounter. If the
                        // parenthesis isn't followed by a statement block, then
                        // this is a variable declaration, in which case we
                        // should only store the type and name of the variable,
                        // not the initialization parameters.
                        hasParenthesis = true;
                    } else if (token == ")") {
                        nestedParenthesis--;
                    }
                } else if (t == asTC_IDENTIFIER) {
                    // If a parenthesis is already found then the name is
                    // already known so it must not be overwritten
                    if (!hasParenthesis)
                        name = token;
                }

                // Skip trailing decorators
                if (!hasParenthesis || nestedParenthesis > 0 ||
                    t != asTC_IDENTIFIER ||
                    (token != "final" && token != "override" &&
                     token != "delete" && token != "property"))
                    declaration += token;

                pos += len;
            }
        }
    }

    return start;
}

QVector<QString> asBuilder::GetMetadataForType(int typeId) {
    return typeMetadataMap.value(typeId);
}

QVector<QString> asBuilder::GetMetadataForFunc(asIScriptFunction *func) {
    return funcMetadataMap.value(func->GetId());
}

QVector<QString> asBuilder::GetMetadataForVar(int varIdx) {
    return varMetadataMap.value(varIdx);
}

QVector<QString> asBuilder::GetMetadataForTypeProperty(int typeId, int varIdx) {
    if (classMetadataMap.contains(typeId)) {
        return varMetadataMap.value(varIdx);
    }
    return {};
}

QVector<QString>
asBuilder::GetMetadataForTypeMethod(int typeId, asIScriptFunction *method) {
    if (method) {
        if (classMetadataMap.contains(typeId)) {
            return funcMetadataMap.value(method->GetId());
        }
    }
    return {};
}
#endif

END_AS_NAMESPACE
