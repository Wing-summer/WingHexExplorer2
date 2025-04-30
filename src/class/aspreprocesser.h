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

#ifndef ASPREPROCESSER_H
#define ASPREPROCESSER_H

// DON'T DELETE this
#include "AngelScript/sdk/angelscript/source/as_config.h"

//---------------------------
// Declaration
//

#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif

#if defined(_MSC_VER) && _MSC_VER <= 1200
// disable the annoying warnings on MSVC 6
#pragma warning(disable : 4786)
#endif

#include <QEventLoop>
#include <QMap>
#include <QSet>
#include <QVector>

class AsPreprocesser;
class asCScriptCode;

// This callback will be called for each #include directive encountered by the
// builder. The callback should call the AddSectionFromFile or
// AddSectionFromMemory to add the included section to the script. If the
// include cannot be resolved then the function should return a negative value
// to abort the compilation.
typedef int (*INCLUDECALLBACK_t)(const QString &include, bool quotedInclude,
                                 const QString &from, AsPreprocesser *builder,
                                 void *userParam);

// This callback will be called for each #pragma directive encountered by the
// builder. The application can interpret the pragmaText and decide what do to
// based on that. If the callback returns a negative value the builder will
// report an error and abort the compilation.
typedef int (*PRAGMACALLBACK_t)(const QByteArray &pragmaText,
                                AsPreprocesser *builder,
                                const QString &sectionname, void *userParam);

// Helper class for loading and pre-processing script files to
// support include directives declarations

/** for macros, we support:
 *      * #if <conditions>
 *      * #else
 *      * #endif
 *      * #define <word>
 *      * #define <word> <string|int64|double>
 *      * #undef <word>
 *      * #ifdef <word>
 *      * #ifundef <word>
 */
class AsPreprocesser {
public:
    explicit AsPreprocesser(asIScriptEngine *engine);
    virtual ~AsPreprocesser();

public:
    struct ScriptData {
        QString section;
        QByteArray script;
    };

    using DefineValueType =
        std::variant<std::monostate, QString, qint64, double>;

public:
    // Load a script section from a file on disk
    // Returns  1 if the file was included
    //          0 if the file had already been included before
    //         <0 on error
    int loadSectionFromFile(const QString &filename);
    int loadSectionFromMemory(const QString &section, const QByteArray &code);

    QList<ScriptData> scriptData() const;

    // Returns the engine
    asIScriptEngine *getEngine();

    // Register the callback for resolving include directive
    void setIncludeCallback(INCLUDECALLBACK_t callback, void *userParam);

    // Register the callback for resolving pragma directive
    void setPragmaCallback(PRAGMACALLBACK_t callback, void *userParam);

    // Add a pre-processor define for conditional compilation
    void defineWord(const QString &word, const DefineValueType &value = {});

    // Enumerate included script sections
    unsigned int sectionCount() const;

    QString sectionName(unsigned int idx) const;

protected:
    void clearAll();
    void addScriptSection(const QString &section, const QByteArray &code);
    int processScriptSection(const QByteArray &script,
                             const QString &sectionname);
    int loadScriptSection(const QString &filename);
    bool includeIfNotAlreadyIncluded(const QString &filename);

    int skipStatement(const QByteArray &modifiedScript, int pos);

    int excludeCode(QByteArray &modifiedScript, int pos);
    void overwriteCode(QByteArray &modifiedScript, int start, int len);

    asIScriptEngine *engine;
    QList<ScriptData> modifiedScripts;

    INCLUDECALLBACK_t includeCallback;
    void *includeParam;

    PRAGMACALLBACK_t pragmaCallback;
    void *pragmaParam;

    QStringList includedScripts;

    QEventLoop waitLoop;
    QHash<QString, DefineValueType> definedWords;
};

#endif // ASPREPROCESSER_H
