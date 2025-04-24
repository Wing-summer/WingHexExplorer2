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

#ifndef CTYPE_PARSER_H
#define CTYPE_PARSER_H

#include <QDir>
#include <QFile>
#include <QHash>
#include <QList>
#include <QPair>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <QString>

/// @beief Struct for variable declaration
///
/// A variable declaration may contain 4 parts
/// (take this statement as example: char* argv[2]):
///    - data_type:     char
///    - var_name:      argv
///    - array_size:    2
///    - is_pointer:    true
/// @note Only one-demension array is supported here, but it's easy to extend
/// with this awareness
///
typedef struct {
    QString data_type;    ///< name of a data type, either basic type or
                          ///< user-defined type
    QString var_name;     ///< variable name
    qsizetype offset;     ///< member offset in struct: 0 for non-struct
    qsizetype array_size; ///< array size: 0 for non-array
    bool is_pointer;      ///< true when it's a pointer
    qsizetype var_size;   ///< size in bytes
} VariableDeclaration;

/// @enum type for token types
enum TokenTypes {
    kUnresolvedToken,

    // keywords
    kStructKeyword,
    kUnionKeyword,
    kEnumKeyword,
    kTypedefKeyword,

    kBasicDataType,
    kAbstractType,
    kComplexType,
    kQualifier,

    // user-defined tokens
    kStructName,
    kUnionName,
    kEnumName,

};

/// @enum type for sigle character tokens
enum SingleToken : char {
    kBlockStart = '{',
    kBlockEnd = '}',
    kPoundSign = '#',

    kComma = ',',
    kSemicolon = ';',
    kEqual = '=',
    kSlash = '/',
    kAsterisk = '*',
    kQuotation = '\"',
};

enum class PointerMode { X86, X64 };

enum class LongMode { LLP64, LP64 };

class CTypeParser {
public:
    explicit CTypeParser();
    virtual ~CTypeParser();

public:
    void parseFiles();
    bool parseFile(const QString &file);
    bool parseSource(const QString &src);

    QStringList includePaths() const;
    void setIncludePaths(const QStringList &newInclude_paths);

public:
    qsizetype padAlignment() const;
    void setPadAlignment(qsizetype newKAlignment);

    PointerMode pointerMode() const;
    void setPointerMode(PointerMode newPmode);

    LongMode longMode() const;
    void setLongmode(LongMode newLmode);

    const QHash<QString, QList<VariableDeclaration>> &structDefs() const;

    const QHash<QString, QList<QPair<QString, int>>> &enumDefs() const;

    const QHash<QString, QPair<QMetaType::Type, qsizetype>> &types() const;

    QPair<QMetaType::Type, qsizetype> type(const QString &t) const;

    const QHash<QString, int> &constDefs() const;

    const QHash<QString, QList<VariableDeclaration>> &unionDefs() const;

    TokenTypes getTokenType(const QString &token) const;

public:
    void dumpTypeDefs() const;

private:
    QString mergeAllLines(const QStringList &lines) const;
    bool getNextToken(QString src, qsizetype &pos, QString &token,
                      bool cross_line = true) const;
    bool getNextLine(QString src, qsizetype &pos, QString &line) const;
    bool getRestLine(const QString &src, qsizetype &pos, QString &line) const;
    void skipCurrentLine(const QString &src, qsizetype &pos,
                         QString &line) const;
    qsizetype splitLineIntoTokens(QString line, QStringList &tokens) const;

    bool parseDeclaration(const QString &line, VariableDeclaration &decl) const;
    bool parseEnumDeclaration(const QString &line, int &last_value,
                              QPair<QString, int> &decl,
                              bool &is_last_member) const;
    bool parseAssignExpression(const QString &line);

    bool parsePreProcDirective(const QString &src, qsizetype &pos);
    bool parseStructUnion(const bool is_struct, const bool is_typedef,
                          const QString &src, qsizetype &pos,
                          VariableDeclaration &var_decl, bool &is_decl);
    bool parseEnum(const bool is_typedef, const QString &src, qsizetype &pos,
                   VariableDeclaration &var_decl, bool &is_decl);

    VariableDeclaration makePadField(const qsizetype size) const;
    qsizetype padStructMembers(QList<VariableDeclaration> &members);
    qsizetype calcUnionSize(const QList<VariableDeclaration> &members) const;

    void storeStructUnionDef(const bool is_struct, const QString &type_name,
                             QList<VariableDeclaration> &members);

private:
    /// read in basic data such as keywords/qualifiers, and basic data type
    /// sizes
    void initialize();

    void findHeaderFiles(const QString &path);
    QString getFile(QString &filename) const;

    // pre-processing
    QString preprocess(QFile &ifs) const;
    void stripComments(QStringList &lines) const;
    void wrapLines(QStringList &lines) const;

    // utility functions
    QString getNextToken(const QString &line, qsizetype &pos) const;
    bool isIgnorable(const QString &token) const;

    bool isNumericToken(const QString &token, long &number) const;
    int getTypeSize(const QString &data_type) const;

    /// class members
private:
    static const char EOL =
        '$'; ///< end of line, used to delimit the source lines within a string

    qsizetype kAlignment_ = 1; ///< alignment

    static const QString kAnonymousTypePrefix;
    static const QRegularExpression kTokenDelimiters;
    static const QString kPaddingFieldName;

private:
    QStringList include_paths_;

    /// basic data that're needed in parsing
    QStringList qualifiers_;
    QHash<QString, TokenTypes> keywords_;

    PointerMode _pmode;
    LongMode _lmode;

    /// header files to parse
    /// key     - filename with relative/absolute path
    /// bool    - whether the file is parsed
    QHash<QString, bool> header_files_;

    /// Size of C data types and also user-defined struct/union types
    /// @note All enum types have fixed size, so they're not stored
    QHash<QString, QPair<QMetaType::Type, qsizetype>> type_maps_;

    /// basic types
    QStringList base_types_;

    /// unsigned types
    QStringList unsigned_types_;

    /// Parsing result - extracted type definitions
    /// for below 3 maps:
    /// key     - type name
    /// value   - type members

    /// struct definitions
    QHash<QString, QList<VariableDeclaration>> struct_defs_;

    /// union definitions
    QHash<QString, QList<VariableDeclaration>> union_defs_;

    /// enum definitions
    QHash<QString, QList<QPair<QString, int>>> enum_defs_;

    /// constants and macros that have integer values
    /// key     - constant/macro name
    /// value   - an integer (all types of number are cast to long type for
    /// convenience)
    QHash<QString, int> const_defs_;
};

#endif // _TYPE_PARSER_H_
