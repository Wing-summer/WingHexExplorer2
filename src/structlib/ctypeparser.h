/*==============================================================================
** Copyright (C) 2025-2028 WingSummer
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

#include "structdefine.h"

#include <QDir>
#include <QFile>
#include <QHash>
#include <QList>
#include <QObject>
#include <QPair>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>

class CStructVisitorParser;

enum class PointerMode { X86, X64 };

enum class LongMode { LLP64, LP64 };

class CTypeParser {
    Q_GADGET

public:
    // Result codes for manager operations
    enum class StructResult {
        Ok,
        NameConflict,
        DuplicateDefinition,
        IncompleteReference,
        OutofMemory,
        CountOfLimit,
    };

    enum class CType {
        Unknown,
        ConstVar,
        Enum,
        Struct,
        Union,
        TypeDef,
        BasicType,
        Pointer
    };
    Q_ENUM(CType);

    using INT_TYPE = std::variant<qint64, quint64>;

public:
    explicit CTypeParser(const std::function<void(const MsgInfo &)> &msgcb);
    virtual ~CTypeParser();

public:
    bool parse(const QString &file);
    bool parseFromSource(const QByteArray &buffer);

public:
    quint64 generateAnomyID();

    // Forward declarations
    StructResult addForwardStruct(const QString &name);
    StructResult addForwardUnion(const QString &name);
    StructResult addForwardEnum(const QString &name);
    StructResult addForwardAnyTypeWithoutEnum(const QString &name);

    // Definitions (only register names)
    StructResult defineStruct(const QString &name,
                              const QVector<VariableDeclaration> &members,
                              int alignment);
    StructResult defineUnion(const QString &name,
                             const QVector<VariableDeclaration> &members,
                             int alignment);
    StructResult
    defineStructOrUnion(bool isStruct, const QString &name,
                        const QVector<VariableDeclaration> &members,
                        int alignment);

    StructResult defineEnum(const QString &name,
                            const QHash<QString, INT_TYPE> &values);
    StructResult defineTypedef(const QString &alias, const QString &origin,
                               bool isPointer);

    StructResult defineConstVar(const QString &name, const INT_TYPE &var);

public:
    int padAlignment() const;

    ///
    /// \brief set the struct alignment
    /// \param newKAlignment alignment value (only for 1, 2, 4, 8)
    /// \warning it takes no effect for structs/union that
    ///          already have been added
    ///
    bool setPadAlignment(int newKAlignment);

    PointerMode pointerMode() const;

    ///
    /// \brief set the pointer size mode
    /// \param newPmode
    /// \warning it takes no effect for structs/union that
    ///          already have been added
    ///
    void setPointerMode(PointerMode newPmode);

    LongMode longMode() const;

    ///
    /// \brief set the specification rule for long type
    /// \param newLmode
    /// \warning it takes no effect for structs/union that
    ///          already have been added
    ///
    void setLongmode(LongMode newLmode);

public:
    QStringList structTypeDefs() const;
    QStringList unionTypeDefs() const;
    QStringList typedefTypeDefs() const;
    QStringList enumTypeDefs() const;
    QStringList constVarDefs() const;

public:
    bool containsType(const QString &name) const;

    bool isBasicType(const QString &name) const;
    bool isUnsignedBasicType(const QString &name) const;

    bool containsEnum(const QString &name) const;
    bool containsStruct(const QString &name) const;
    bool containsUnion(const QString &name) const;
    bool containsTypeDef(const QString &name) const;
    bool containsConstVar(const QString &name) const;
    bool isCompletedType(const QString &name) const;

    INT_TYPE constVarValue(const QString &name) const;

    bool isCompletedStruct(const QString &name) const;
    bool isCompletedUnion(const QString &name) const;

    QStringList getMissingDependencise(const QString &name) const;

public:
    QList<VariableDeclaration> structMembers(const QString &name) const;
    QList<VariableDeclaration> unionMembers(const QString &name) const;
    QStringList enumMembers(const QString &name) const;
    QString typedefOriginType(const QString &name) const;

public:
    QMetaType::Type metaType(const QString &name) const;
    QMetaType::Type typeMapValue(const QString &name) const;
    CType type(const QString &name) const;
    CType resolveType(const QString &name) const;
    std::optional<quint64> getTypeSize(const QString &data_type) const;

    QString resolveTypeName(const QString &name) const;

public:
    void dumpAllTypes(QTextStream &output) const;
    void dumpTypeDefines(QTextStream &output) const;
    void dumpConstants(QTextStream &output) const;
    void dumpStructs(QTextStream &output) const;
    void dumpUnions(QTextStream &output) const;
    void dumpEnums(QTextStream &output) const;

    void clear();

private:
    QStringList
    getMissingDependencise(const QVector<VariableDeclaration> &members);

    std::optional<quint64> padStruct(QVector<VariableDeclaration> &members,
                                     int alignment);
    quint64 calcUnionSize(const QVector<VariableDeclaration> &members,
                          int alignment) const;

    Q_REQUIRED_RESULT bool
    storeStructUnionDef(const bool is_struct, const QString &type_name,
                        QVector<VariableDeclaration> &members,
                        qsizetype alignment);

    void restoreIncompleteType(const QString &name);

private:
    enum class IncompleteType {
        Struct,
        Union,
        Typedef,
        Enum,
        AnyTypeWithoutEnum
    };

private:
    /// read in basic data such as keywords/qualifiers, and basic data type
    /// sizes
    void initialize();

    void findHeaderFiles(const QString &path);
    QString getFile(QString &filename) const;

    /// class members
private:
    int kAlignment_ = 1; ///< alignment

private:
    CStructVisitorParser *_visitor = nullptr;

    PointerMode _pmode;
    LongMode _lmode;

    /// Size of C data types and also user-defined struct/union types
    /// @note All enum types have fixed size, so they're not stored
    QHash<QString, QPair<QMetaType::Type, quint64>> type_maps_;

    /// names awaiting definition
    QHash<QString, IncompleteType> referencedIncomplete_;
    QHash<QString, QStringList> incompleteDeps_;

    /// basic types
    QStringList base_types_;

    /// Parsing result - extracted type definitions
    /// for below 3 maps:
    /// key     - type name
    /// value   - type members

    /// struct definitions
    QHash<QString, QList<VariableDeclaration>> struct_defs_;

    /// union definitions
    QHash<QString, QList<VariableDeclaration>> union_defs_;

    QHash<QString, int> struct_union_alignments_;

    /// typedef definitions
    /// <typedefName, <originalName, isPointer>>
    QHash<QString, QPair<QString, bool>> type_defs_;

    /// enum definitions
    QHash<QString, QStringList> enum_defs_;

    /// constants and macros that have integer values
    /// key     - constant/macro name
    /// value   - an integer (all types of number are cast to long type for
    /// convenience)
    QHash<QString, INT_TYPE> const_defs_;

    std::function<void(const MsgInfo &)> _msgcb;

private:
    quint64 _anomyIndex = 0;
};

#endif // _TYPE_PARSER_H_
