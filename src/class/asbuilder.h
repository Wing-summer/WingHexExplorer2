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

#ifndef ASBUILDER_H
#define ASBUILDER_H

//---------------------------
// Compilation settings
//

// Set this flag to turn on/off metadata processing
//  0 = off
//  1 = on
#ifndef AS_PROCESS_METADATA
#define AS_PROCESS_METADATA 1
#endif

// TODO: Implement flags for turning on/off include directives and conditional
// programming

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

#include <QMap>
#include <QSet>
#include <QVector>

BEGIN_AS_NAMESPACE

class asBuilder;

// This callback will be called for each #include directive encountered by the
// builder. The callback should call the AddSectionFromFile or
// AddSectionFromMemory to add the included section to the script. If the
// include cannot be resolved then the function should return a negative value
// to abort the compilation.
typedef int (*INCLUDECALLBACK_t)(const QString &include, bool quotedInclude,
                                 const QString &from, asBuilder *builder,
                                 void *userParam);

// This callback will be called for each #pragma directive encountered by the
// builder. The application can interpret the pragmaText and decide what do to
// based on that. If the callback returns a negative value the builder will
// report an error and abort the compilation.
typedef int (*PRAGMACALLBACK_t)(const QByteArray &pragmaText,
                                asBuilder *builder, void *userParam);

// Helper class for loading and pre-processing script files to
// support include directives and metadata declarations

class asBuilder {
public:
    explicit asBuilder();

    // Start a new module
    int StartNewModule(asIScriptEngine *engine, const char *moduleName);

    // Load a script section from a file on disk
    // Returns  1 if the file was included
    //          0 if the file had already been included before
    //         <0 on error
    int AddSectionFromFile(const QString &filename);

    // Load a script section from memory
    // Returns  1 if the section was included
    //          0 if a section with the same name had already been included
    //          before
    //         <0 on error
    int AddSectionFromMemory(const char *sectionName, const char *scriptCode,
                             unsigned int scriptLength = 0, int lineOffset = 0);

    // Build the added script sections
    int BuildModule();

    // Returns the engine
    asIScriptEngine *GetEngine();

    // Returns the current module
    asIScriptModule *GetModule();

    // Register the callback for resolving include directive
    void SetIncludeCallback(INCLUDECALLBACK_t callback, void *userParam);

    // Register the callback for resolving pragma directive
    void SetPragmaCallback(PRAGMACALLBACK_t callback, void *userParam);

    // Add a pre-processor define for conditional compilation
    void DefineWord(const QString &word);

    // Enumerate included script sections
    unsigned int GetSectionCount() const;

    QString GetSectionName(unsigned int idx) const;

#if AS_PROCESS_METADATA == 1
    // Get metadata declared for classes, interfaces, and enums
    QVector<QString> GetMetadataForType(int typeId);

    // Get metadata declared for functions
    QVector<QString> GetMetadataForFunc(asIScriptFunction *func);

    // Get metadata declared for global variables
    QVector<QString> GetMetadataForVar(int varIdx);

    // Get metadata declared for class variables
    QVector<QString> GetMetadataForTypeProperty(int typeId, int varIdx);

    // Get metadata declared for class methods
    QVector<QString> GetMetadataForTypeMethod(int typeId,
                                              asIScriptFunction *method);
#endif

protected:
    void ClearAll();
    int Build();
    int ProcessScriptSection(const QByteArray &script, int length,
                             const QString &sectionname, int lineOffset);
    int LoadScriptSection(const QString &filename);
    bool IncludeIfNotAlreadyIncluded(const QString &filename);

    int SkipStatement(int pos);

    int ExcludeCode(int start);
    void OverwriteCode(int start, int len);

    asIScriptEngine *engine;
    asIScriptModule *module;
    QByteArray modifiedScript;

    INCLUDECALLBACK_t includeCallback;
    void *includeParam;

    PRAGMACALLBACK_t pragmaCallback;
    void *pragmaParam;

#if AS_PROCESS_METADATA == 1
    int ExtractMetadata(int pos, QVector<QString> &outMetadata);
    int ExtractDeclaration(int pos, QString &outName, QString &outDeclaration,
                           int &outType);

    enum METADATATYPE {
        MDT_TYPE = 1,
        MDT_FUNC = 2,
        MDT_VAR = 3,
        MDT_VIRTPROP = 4,
        MDT_FUNC_OR_VAR = 5
    };

    // Temporary structure for storing metadata and declaration
    struct SMetadataDecl {
        SMetadataDecl(const QVector<QString> &m, const QString &n,
                      const QString &d, int t, const QString &c,
                      const QString &ns)
            : metadata(m), name(n), declaration(d), type(t), parentClass(c),
              nameSpace(ns) {}
        QVector<QString> metadata;
        QString name;
        QString declaration;
        int type;
        QString parentClass;
        QString nameSpace;
    };
    QVector<SMetadataDecl> foundDeclarations;
    QString currentClass;
    QString currentNamespace;

    // Storage of metadata for global declarations
    QMap<int, QVector<QString>> typeMetadataMap;
    QMap<int, QVector<QString>> funcMetadataMap;
    QMap<int, QVector<QString>> varMetadataMap;

    // Storage of metadata for class member declarations
    struct SClassMetadata {
        SClassMetadata(const QString &aName) : className(aName) {}
        QString className;
        QMap<int, QVector<QString>> funcMetadataMap;
        QMap<int, QVector<QString>> varMetadataMap;
    };
    QMap<int, SClassMetadata> classMetadataMap;

#endif

    QVector<QString> includedScripts;

    QVector<QString> definedWords;
};

END_AS_NAMESPACE

#endif // ASBUILDER_H
