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

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QList>
#include <QMap>
#include <QRandomGenerator>
#include <QSet>
#include <QStringList>
#include <QTextStream>
#include <QtEndian>

#include "cstructerrorstrategy.h"
#include "cstructvisitorparser.h"
#include "ctypeparser.h"
#include "grammar/c/CStructLexer.h"
#include "grammar/c/CStructParser.h"
#include "structlib/cstructerrorlistener.h"

#include <antlr4-runtime.h>
#include <array>

constexpr auto STURCT_SIZE_LIMIT = 102400; // 100KB

// Struct = QMetaType::User

CTypeParser::~CTypeParser() {}

CTypeParser::CTypeParser(const std::function<void(const MsgInfo &)> &msgcb)
    :
#if WORDSIZE == 8
      _pmode(PointerMode::X64),
#else
      _pmode(PointerMode::X86),
#endif
#ifdef Q_OS_WIN
      _lmode(LongMode::LLP64)
#else
      _lmode(LongMode::LP64)
#endif
      ,
      _msgcb(msgcb) {
    Q_ASSERT(_msgcb);

    initialize();
}

void CTypeParser::initialize() {

#define ADD_TYPE(T, MT)                                                        \
    type_maps_.insert(QStringLiteral(#T), qMakePair(MT, sizeof(T)))

#define ADD_TYPE_S(T, MT, S)                                                   \
    type_maps_.insert(QStringLiteral(#T), qMakePair(MT, S))

    ADD_TYPE(uchar, QMetaType::UChar);
    ADD_TYPE(ushort, QMetaType::UShort);
    ADD_TYPE(uint, QMetaType::UInt);
    ADD_TYPE(ulong, QMetaType::ULong);

    using byte = unsigned char;
    using char8_t = unsigned char;
    using int8 = qint8;
    using int16 = qint16;
    using int32 = qint32;
    using int64 = qint64;
    using uint8 = quint8;
    using uint16 = quint16;
    using uint32 = quint32;
    using uint64 = quint64;
    using longlong = qlonglong;
    using ulonglong = qulonglong;

    using intptr = qintptr;
    using uintptr = quintptr;

    ADD_TYPE(bool, QMetaType::Bool);
    ADD_TYPE(char, QMetaType::Char);
    ADD_TYPE(short, QMetaType::Short);
    ADD_TYPE(int, QMetaType::Int);

    // LLP64: sizeof(long) = 4
    // LP64: sizeof(long) = 8
    ADD_TYPE(long, QMetaType::Long);

    ADD_TYPE(float, QMetaType::Float);
    ADD_TYPE(double, QMetaType::Double);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    ADD_TYPE(wchar_t, QMetaType::Char16);
#else
    ADD_TYPE(wchar_t, QMetaType::QChar);
#endif

    ADD_TYPE(char8_t, QMetaType::Char);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    ADD_TYPE(char16_t, QMetaType::Char16);
    ADD_TYPE(char32_t, QMetaType::Char32);
#else
    ADD_TYPE(char16_t, QMetaType::QChar);
    ADD_TYPE(char32_t, QMetaType::QChar);
#endif
    ADD_TYPE_S(void, QMetaType::Void, 0);

    ADD_TYPE(byte, QMetaType::UChar);

    ADD_TYPE(int8, QMetaType::SChar);
    ADD_TYPE(int16, QMetaType::Short);
    ADD_TYPE(int32, QMetaType::Int);
    ADD_TYPE(int64, QMetaType::LongLong);

    ADD_TYPE(uint8, QMetaType::UChar);
    ADD_TYPE(uint16, QMetaType::UShort);
    ADD_TYPE(uint32, QMetaType::UInt);
    ADD_TYPE(uint64, QMetaType::ULongLong);

    ADD_TYPE(longlong, QMetaType::LongLong);
    ADD_TYPE(ulonglong, QMetaType::ULongLong);

    ADD_TYPE(intptr, QMetaType::LongLong);
    ADD_TYPE(uintptr, QMetaType::ULongLong);

    using BOOL = bool;
    using UCHAR = uchar;
    using BYTE = byte;
    using WORD = uint16;
    using DWORD = uint32;
    using QWORD = uint64;
    using DWORDLONG = QWORD;
    using FLOAT = float;
    using DOUBLE = double;
    using DWORD32 = uint32;
    using DWORD64 = uint64;

    ADD_TYPE(BOOL, QMetaType::Bool);
    ADD_TYPE(UCHAR, QMetaType::UChar);
    ADD_TYPE(BYTE, QMetaType::UChar);
    ADD_TYPE(WORD, QMetaType::UShort);
    ADD_TYPE(DWORD, QMetaType::UInt);
    ADD_TYPE(QWORD, QMetaType::ULongLong);
    ADD_TYPE(DWORDLONG, QMetaType::ULongLong);
    ADD_TYPE(FLOAT, QMetaType::Float);
    ADD_TYPE(DOUBLE, QMetaType::Double);
    ADD_TYPE(DWORD32, QMetaType::UInt);
    ADD_TYPE(DWORD64, QMetaType::UInt);

    using INT8 = qint8;
    using INT16 = qint16;
    using INT32 = qint32;
    using INT64 = qint64;

    ADD_TYPE(INT8, QMetaType::SChar);
    ADD_TYPE(INT16, QMetaType::Short);
    ADD_TYPE(INT32, QMetaType::Int);
    ADD_TYPE(INT64, QMetaType::LongLong);

    using UINT8 = quint8;
    using UINT16 = quint16;
    using UINT32 = quint32;
    using UINT64 = quint64;

    ADD_TYPE(UINT8, QMetaType::UChar);
    ADD_TYPE(UINT16, QMetaType::UShort);
    ADD_TYPE(UINT32, QMetaType::UInt);
    ADD_TYPE(UINT64, QMetaType::ULongLong);

    using SHORT = short;
    using INT = int;
    using LONG = int; // on windows, just keep LLP64
    using WCHAR = wchar_t;

    ADD_TYPE(SHORT, QMetaType::Short);
    ADD_TYPE(INT, QMetaType::Int);

    ADD_TYPE(LONG, QMetaType::Int);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    ADD_TYPE(WCHAR, QMetaType::Char16);
#else
    ADD_TYPE(WCHAR, QMetaType::QChar);
#endif

    using LONGLONG = longlong;
    using ULONGLONG = ulonglong;

    ADD_TYPE(LONGLONG, QMetaType::LongLong);
    ADD_TYPE(ULONGLONG, QMetaType::ULongLong);

    using LONG32 = qint32;
    using LONG64 = qint64;

    ADD_TYPE(LONG32, QMetaType::Long);
    ADD_TYPE(LONG64, QMetaType::LongLong);

    using SIZE_T = size_t;
    if (sizeof(size_t) == sizeof(quint64)) {
        ADD_TYPE(size_t, QMetaType::ULongLong);
        ADD_TYPE(SIZE_T, QMetaType::ULongLong);
    } else {
        ADD_TYPE(size_t, QMetaType::ULong);
        ADD_TYPE(SIZE_T, QMetaType::ULong);
    }

    ADD_TYPE(int8_t, QMetaType::SChar);
    ADD_TYPE(int16_t, QMetaType::Short);
    ADD_TYPE(int32_t, QMetaType::Int);
    ADD_TYPE(int64_t, QMetaType::LongLong);

    ADD_TYPE(uint8_t, QMetaType::UChar);
    ADD_TYPE(uint16_t, QMetaType::UShort);
    ADD_TYPE(uint32_t, QMetaType::UInt);
    ADD_TYPE(uint64_t, QMetaType::ULongLong);
    ADD_TYPE(intptr_t, QMetaType::LongLong);
    ADD_TYPE(uintptr_t, QMetaType::ULongLong);

#undef ADD_TYPE
#undef ADD_TYPE_S

    base_types_ = type_maps_.keys();
}

bool CTypeParser::parse(const QString &file) {
    clear();

    auto fname = file.toStdString();

    std::ifstream fs(fname);
    if (!fs.is_open()) {
        return false;
    }

    antlr4::ANTLRInputStream input(fs);
    input.name = fname;

    CStructLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);

    CStructParser parser(&tokens);
    parser.removeErrorListeners();
    parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());

    CStructErrorListener lis(_msgcb);
    CStructVisitorParser visitor(this, &lis);

    try {
        visitor.visit(parser.compilationUnit());
    } catch (...) {
        clear();
        tokens.reset();
        parser.reset();

        parser.setErrorHandler(std::make_shared<CStructErrorStrategy>());

        parser.addErrorListener(&lis);

        parser.compilationUnit();

        clear();
        return false;
    }

    return true;
}

bool CTypeParser::parseFromSource(const QByteArray &buffer) {
    antlr4::ANTLRInputStream input(buffer.constData(), buffer.length());

    CStructLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);

    CStructParser parser(&tokens);
    parser.removeErrorListeners();
    parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());

    CStructErrorListener lis(_msgcb);
    CStructVisitorParser visitor(this, &lis);

    try {
        visitor.visit(parser.compilationUnit());
    } catch (...) {
        clear();
        tokens.reset();
        parser.reset();

        parser.setErrorHandler(std::make_shared<CStructErrorStrategy>());

        parser.addErrorListener(&lis);

        parser.compilationUnit();

        clear();
        return false;
    }

    return true;
}

quint64 CTypeParser::generateAnomyID() {
    auto ret = _anomyIndex;
    _anomyIndex++;
    return ret;
}

CTypeParser::StructResult
CTypeParser::addForwardAnyTypeWithoutEnum(const QString &name) {
    Q_ASSERT(!name.isEmpty());
    if (containsType(name))
        return StructResult::NameConflict;
    if (containsEnum(name))
        return StructResult::DuplicateDefinition;

    if (referencedIncomplete_.contains(name)) {
        auto t = referencedIncomplete_.value(name);
        if (t != IncompleteType::Enum) {
            return StructResult::DuplicateDefinition;
        }

        return StructResult::NameConflict;
    }

    if (referencedIncomplete_.size() >= std::numeric_limits<qsizetype>::max()) {
        return StructResult::CountOfLimit;
    }

    referencedIncomplete_.insert(
        name, CTypeParser::IncompleteType::AnyTypeWithoutEnum);
    return StructResult::Ok;
}

CTypeParser::StructResult CTypeParser::addForwardEnum(const QString &name) {
    Q_ASSERT(!name.isEmpty());
    if (containsType(name))
        return StructResult::NameConflict;
    if (containsEnum(name))
        return StructResult::DuplicateDefinition;

    if (referencedIncomplete_.contains(name)) {
        auto t = referencedIncomplete_.value(name);
        if (t != IncompleteType::AnyTypeWithoutEnum) {
            if (t == IncompleteType::Enum) {
                return StructResult::DuplicateDefinition;
            }
            return StructResult::NameConflict;
        }
    } else {
        if (referencedIncomplete_.size() >=
            std::numeric_limits<qsizetype>::max()) {
            return StructResult::CountOfLimit;
        }
    }

    referencedIncomplete_.insert(name, CTypeParser::IncompleteType::Enum);
    return StructResult::Ok;
}

CTypeParser::StructResult CTypeParser::defineConstVar(const QString &name,
                                                      const INT_TYPE &var) {
    if (containsType(name)) {
        return StructResult::NameConflict;
    }

    if (containsConstVar(name)) {
        return StructResult::DuplicateDefinition;
    }

    if (const_defs_.size() >= std::numeric_limits<qsizetype>::max()) {
        return StructResult::OutofMemory;
    }

    const_defs_.insert(name, var);
    return StructResult::Ok;
}

CTypeParser::StructResult CTypeParser::defineTypedef(const QString &alias,
                                                     const QString &origin,
                                                     bool isPointer) {
    if (containsType(alias) || containsConstVar(alias))
        return StructResult::NameConflict;

    if (containsTypeDef(alias))
        return StructResult::DuplicateDefinition;

    if (!containsType(origin) && !referencedIncomplete_.contains(origin)) {
        return StructResult::IncompleteReference;
    }

    if (type_defs_.size() >= std::numeric_limits<int>::max() ||
        type_maps_.size() >= std::numeric_limits<qsizetype>::max()) {
        return StructResult::CountOfLimit;
    }

    type_defs_.insert(alias, qMakePair(origin, isPointer));
    if (isPointer) {
        switch (pointerMode()) {
        case PointerMode::X86:
            type_maps_.insert(alias,
                              qMakePair(QMetaType::ULong, sizeof(quint32)));
            break;
        case PointerMode::X64:
            type_maps_.insert(alias,
                              qMakePair(QMetaType::ULongLong, sizeof(quint64)));
            break;
        };
    } else {
        if (referencedIncomplete_.contains(origin)) {
            if (referencedIncomplete_.size() >=
                    std::numeric_limits<qsizetype>::max() ||
                incompleteDeps_.size() >=
                    std::numeric_limits<qsizetype>::max()) {
                type_defs_.remove(alias);
                return StructResult::CountOfLimit;
            }
            referencedIncomplete_.insert(alias,
                                         CTypeParser::IncompleteType::Typedef);
            incompleteDeps_.insert(alias, {origin});
        } else {
            type_maps_.insert(alias, type_maps_[origin]);
        }
    }
    restoreIncompleteType(alias);
    return StructResult::Ok;
}

CTypeParser::StructResult
CTypeParser::defineEnum(const QString &name,
                        const QHash<QString, INT_TYPE> &values) {
    if (containsType(name) || containsConstVar(name))
        return StructResult::NameConflict;

    if (containsEnum(name))
        return StructResult::DuplicateDefinition;

    if (enum_defs_.size() >= std::numeric_limits<int>::max() ||
        const_defs_.size() >= std::numeric_limits<qsizetype>::max() ||
        type_maps_.size() >= std::numeric_limits<qsizetype>::max()) {
        return StructResult::CountOfLimit;
    }

    if (referencedIncomplete_.contains(name)) {
        if (referencedIncomplete_[name] == IncompleteType::Enum) {
            referencedIncomplete_.remove(name);
        } else {
            return StructResult::NameConflict;
        }
    }

    enum_defs_.insert(name, values.keys());
    type_maps_.insert(name, qMakePair(QMetaType::LongLong, sizeof(qint64)));
    for (auto &&v : values.asKeyValueRange()) {
        const_defs_.insert(v.first, v.second);
    }
    restoreIncompleteType(name);
    return StructResult::Ok;
}

CTypeParser::StructResult
CTypeParser::defineUnion(const QString &name,
                         const QVector<VariableDeclaration> &members,
                         int alignment) {
    if (containsType(name) || containsConstVar(name))
        return StructResult::NameConflict;

    if (containsUnion(name))
        return StructResult::DuplicateDefinition;

    if (referencedIncomplete_.contains(name)) {
        auto t = referencedIncomplete_[name];
        if (t == IncompleteType::Union ||
            t == IncompleteType::AnyTypeWithoutEnum) {
            referencedIncomplete_.remove(name);
        } else {
            return StructResult::NameConflict;
        }
    }

    if (union_defs_.size() >= std::numeric_limits<int>::max() ||
        struct_union_alignments_.size() >=
            std::numeric_limits<qsizetype>::max() ||
        referencedIncomplete_.size() >= std::numeric_limits<qsizetype>::max() ||
        incompleteDeps_.size() >= std::numeric_limits<qsizetype>::max()) {
        return StructResult::CountOfLimit;
    }

    union_defs_.insert(name, members);
    switch (alignment) {
    case 1:
    case 2:
    case 4:
    case 8:
        struct_union_alignments_.insert(name, alignment);
        break;
    default:
        break;
    }

    auto missingDeps = getMissingDependencise(members);
    if (missingDeps.isEmpty()) {
        auto ret =
            storeStructUnionDef(false, name, union_defs_[name], alignment);
        if (!ret) {
            union_defs_.remove(name);
            struct_union_alignments_.remove(name);
            addForwardUnion(name);
            return StructResult::OutofMemory;
        }
        restoreIncompleteType(name);
    } else {
        referencedIncomplete_.insert(name, CTypeParser::IncompleteType::Union);
        incompleteDeps_.insert(name, missingDeps);
    }

    return StructResult::Ok;
}

CTypeParser::StructResult
CTypeParser::defineStructOrUnion(bool isStruct, const QString &name,
                                 const QVector<VariableDeclaration> &members,
                                 int alignment) {
    if (isStruct) {
        return defineStruct(name, members, alignment);
    } else {
        return defineUnion(name, members, alignment);
    }
}

CTypeParser::StructResult
CTypeParser::defineStruct(const QString &name,
                          const QVector<VariableDeclaration> &members,
                          int alignment) {
    if (containsType(name) || containsConstVar(name))
        return StructResult::NameConflict;

    if (containsStruct(name))
        return StructResult::DuplicateDefinition;

    if (referencedIncomplete_.contains(name)) {
        if (referencedIncomplete_.contains(name)) {
            auto t = referencedIncomplete_[name];
            if (t == IncompleteType::Struct ||
                t == IncompleteType::AnyTypeWithoutEnum) {
                referencedIncomplete_.remove(name);
            } else {
                return StructResult::NameConflict;
            }
        }
    }

    if (struct_defs_.size() >= std::numeric_limits<int>::max() ||
        struct_union_alignments_.size() >=
            std::numeric_limits<qsizetype>::max() ||
        referencedIncomplete_.size() >= std::numeric_limits<qsizetype>::max() ||
        incompleteDeps_.size() >= std::numeric_limits<qsizetype>::max()) {
        return StructResult::CountOfLimit;
    }

    struct_defs_.insert(name, members);
    switch (alignment) {
    case 1:
    case 2:
    case 4:
    case 8:
        struct_union_alignments_.insert(name, alignment);
        break;
    default:
        break;
    }

    auto missingDeps = getMissingDependencise(members);
    if (missingDeps.isEmpty()) {
        auto ret =
            storeStructUnionDef(true, name, struct_defs_[name], alignment);
        if (!ret) {
            struct_defs_.remove(name);
            struct_union_alignments_.remove(name);
            addForwardStruct(name);
            return StructResult::OutofMemory;
        }
        restoreIncompleteType(name);
    } else {
        referencedIncomplete_.insert(name, CTypeParser::IncompleteType::Struct);
        incompleteDeps_.insert(name, missingDeps);
    }

    return StructResult::Ok;
}

CTypeParser::StructResult CTypeParser::addForwardStruct(const QString &name) {
    Q_ASSERT(!name.isEmpty());
    if (containsType(name))
        return StructResult::NameConflict;
    if (containsStruct(name))
        return StructResult::DuplicateDefinition;

    if (referencedIncomplete_.contains(name)) {
        auto t = referencedIncomplete_.value(name);
        if (t != IncompleteType::AnyTypeWithoutEnum) {
            if (t == IncompleteType::Struct) {
                return StructResult::DuplicateDefinition;
            }
            return StructResult::NameConflict;
        }
    } else {
        if (referencedIncomplete_.size() >=
            std::numeric_limits<qsizetype>::max()) {
            return StructResult::CountOfLimit;
        }
    }

    referencedIncomplete_.insert(name, CTypeParser::IncompleteType::Struct);
    return StructResult::Ok;
}

CTypeParser::StructResult CTypeParser::addForwardUnion(const QString &name) {
    Q_ASSERT(!name.isEmpty());
    if (containsType(name))
        return StructResult::NameConflict;
    if (containsUnion(name))
        return StructResult::DuplicateDefinition;

    if (referencedIncomplete_.contains(name)) {
        auto t = referencedIncomplete_.value(name);
        if (t != IncompleteType::AnyTypeWithoutEnum) {
            if (t == IncompleteType::Union) {
                return StructResult::DuplicateDefinition;
            }
            return StructResult::NameConflict;
        }
    } else {
        if (referencedIncomplete_.size() >=
            std::numeric_limits<qsizetype>::max()) {
            return StructResult::CountOfLimit;
        }
    }

    referencedIncomplete_.insert(name, CTypeParser::IncompleteType::Union);
    return StructResult::Ok;
}

/// Get size of a type (in bytes)
///
/// @param[in]  data_type    name of a data type, including both C data types
/// and user-defined struct/union/enum types
/// @return -1 if is data type is unknown, else return actual type length
///
/// @note Shouldn't return 0 for unknown data type since "void" type has zero
/// length
///
std::optional<qint64> CTypeParser::getTypeSize(const QString &data_type) const {
    if (data_type.endsWith('*')) {
        return pointerMode() == PointerMode::X86 ? sizeof(quint32)
                                                 : sizeof(quint64);
    } else if (type_maps_.contains(data_type)) {
        return type_maps_.value(data_type).second;
    } else if (enum_defs_.contains(data_type)) {
        return sizeof(qint64);
    } else {
        return std::nullopt;
    }
}

QString CTypeParser::resolveTypeName(const QString &name) const {
    QString ret = name;

    auto type = this->type(name);
    while (type == CType::TypeDef) {
        auto tdef = type_defs_.value(ret);
        if (tdef.second) {
            ret.append('*');
            break;
        }

        ret = tdef.first;
        type = this->type(ret);
    }

    return ret;
}

LongMode CTypeParser::longMode() const { return _lmode; }

void CTypeParser::setLongmode(LongMode newLmode) {
    // correct the size of long
    switch (newLmode) {
    case LongMode::LLP64:
        type_maps_["long"] = qMakePair(QMetaType::Long, sizeof(int));
        type_maps_["ulong"] = qMakePair(QMetaType::ULong, sizeof(uint));
        break;
    case LongMode::LP64:
        type_maps_["long"] = qMakePair(QMetaType::Long, sizeof(qlonglong));
        type_maps_["ulong"] = qMakePair(QMetaType::ULong, sizeof(qulonglong));
        break;
    }
    _lmode = newLmode;
}

QStringList CTypeParser::structTypeDefs() const { return struct_defs_.keys(); }

QStringList CTypeParser::unionTypeDefs() const { return union_defs_.keys(); }

QStringList CTypeParser::typedefTypeDefs() const { return type_defs_.keys(); }

QStringList CTypeParser::enumTypeDefs() const { return enum_defs_.keys(); }

QStringList CTypeParser::constVarDefs() const { return const_defs_.keys(); }

bool CTypeParser::containsType(const QString &name) const {
    return isBasicType(name) || containsTypeDef(name) || containsStruct(name) ||
           containsUnion(name) || containsEnum(name);
}

bool CTypeParser::isBasicType(const QString &name) const {
    return base_types_.contains(name);
}

bool CTypeParser::isUnsignedBasicType(const QString &name) const {
    if (!isBasicType(name)) {
        return false;
    }

    auto t = type_maps_.value(name);
    switch (t.first) {
    case QMetaType::UChar:
    case QMetaType::UInt:
    case QMetaType::UShort:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
        return true;
    default:
        return false;
    }
}

bool CTypeParser::containsEnum(const QString &name) const {
    return enum_defs_.contains(name);
}

bool CTypeParser::containsStruct(const QString &name) const {
    return struct_defs_.contains(name);
}

bool CTypeParser::containsUnion(const QString &name) const {
    return union_defs_.contains(name);
}

bool CTypeParser::containsTypeDef(const QString &name) const {
    return type_defs_.contains(name);
}

bool CTypeParser::containsConstVar(const QString &name) const {
    return const_defs_.contains(name);
}

bool CTypeParser::isCompletedType(const QString &name) const {
    return !referencedIncomplete_.contains(name);
}

bool CTypeParser::isCompletedStruct(const QString &name) const {
    return struct_defs_.contains(name) &&
           referencedIncomplete_.value(name, IncompleteType::Enum) !=
               IncompleteType::Struct;
}

bool CTypeParser::isCompletedUnion(const QString &name) const {
    return union_defs_.contains(name) &&
           referencedIncomplete_.value(name, IncompleteType::Enum) !=
               IncompleteType::Union;
}

QStringList CTypeParser::getMissingDependencise(const QString &name) const {
    return incompleteDeps_.value(name);
}

QList<VariableDeclaration>
CTypeParser::structMembers(const QString &name) const {
    return struct_defs_.value(name);
}

QList<VariableDeclaration>
CTypeParser::unionMembers(const QString &name) const {
    return union_defs_.value(name);
}

QString CTypeParser::typedefOriginType(const QString &name) const {
    auto v = type_defs_.value(name, qMakePair(QString{}, false));
    if (v.first.isEmpty()) {
        return {};
    } else {
        auto n = v.first;
        if (v.second) {
            n.append('*');
        }
        return n;
    }
}

QStringList CTypeParser::enumMembers(const QString &name) const {
    return enum_defs_.value(name);
}

CTypeParser::INT_TYPE CTypeParser::constVarValue(const QString &name) const {
    Q_ASSERT(const_defs_.contains(name));
    return const_defs_.value(name);
}

QMetaType::Type CTypeParser::metaType(const QString &name) const {
    if (name.endsWith('*')) {
        return QMetaType::VoidStar;
    } else if (isBasicType(name)) {
        return type_maps_.value(name).first;
    } else if (containsConstVar(name)) {
        return QMetaType::LongLong;
    } else if (containsStruct(name) || containsUnion(name) ||
               containsEnum(name)) {
        return QMetaType::User;
    } else {
        return QMetaType::UnknownType;
    }
}

QMetaType::Type CTypeParser::typeMapValue(const QString &name) const {
    if (type_maps_.contains(name)) {
        return type_maps_.value(name).first;
    }
    return QMetaType::UnknownType;
}

CTypeParser::CType CTypeParser::resolveType(const QString &name) const {
    if (name.endsWith('*')) {
        return CType::Pointer;
    }

    CTypeParser::CType t;
    QString rname = name;
    while (true) {
        t = type(rname);
        if (t == CTypeParser::CType::TypeDef) {
            auto r = type_defs_.value(rname);
            if (r.second) {
                return CType::Pointer;
            } else {
                rname = r.first;
            }
        } else {
            break;
        }
    }
    return t;
}

CTypeParser::CType CTypeParser::type(const QString &name) const {
    if (name.endsWith('*')) {
        return CType::Pointer;
    }

    if (isBasicType(name)) {
        return CType::BasicType;
    }

    if (containsConstVar(name)) {
        return CType::ConstVar;
    }

    if (containsStruct(name)) {
        return CType::Struct;
    }

    if (containsUnion(name)) {
        return CType::Union;
    }

    if (containsEnum(name)) {
        return CType::Enum;
    }

    if (containsTypeDef(name)) {
        return CType::TypeDef;
    }

    if (referencedIncomplete_.contains(name)) {
        auto v = referencedIncomplete_.value(name);
        switch (v) {
        case IncompleteType::Struct:
            return CType::Struct;
        case IncompleteType::Union:
            return CType::Union;
        case IncompleteType::Typedef:
            return CType::TypeDef;
        case IncompleteType::Enum:
            return CType::Enum;
        default:
            break;
        }
    }

    return CType::Unknown;
}

PointerMode CTypeParser::pointerMode() const { return _pmode; }

void CTypeParser::setPointerMode(PointerMode newPmode) {
    switch (newPmode) {
    case PointerMode::X86:
        type_maps_[QStringLiteral("intptr_t")] =
            qMakePair(QMetaType::Long, sizeof(quint32));
        type_maps_[QStringLiteral("uintptr_t")] =
            qMakePair(QMetaType::ULong, sizeof(quint32));
        type_maps_[QStringLiteral("intptr")] =
            qMakePair(QMetaType::Long, sizeof(quint32));
        type_maps_[QStringLiteral("uintptr")] =
            qMakePair(QMetaType::ULong, sizeof(quint32));
        break;
    case PointerMode::X64:
        type_maps_[QStringLiteral("intptr_t")] =
            qMakePair(QMetaType::LongLong, sizeof(quint64));
        type_maps_[QStringLiteral("uintptr_t")] =
            qMakePair(QMetaType::ULongLong, sizeof(quint64));
        type_maps_[QStringLiteral("intptr")] =
            qMakePair(QMetaType::LongLong, sizeof(quint64));
        type_maps_[QStringLiteral("uintptr")] =
            qMakePair(QMetaType::ULongLong, sizeof(quint64));
        break;
    }
    _pmode = newPmode;
}

int CTypeParser::padAlignment() const { return kAlignment_; }

bool CTypeParser::setPadAlignment(int newKAlignment) {
    static std::array<int, 5> allowList{1, 2, 4, 8, 16};
    if (std::find(allowList.begin(), allowList.end(), newKAlignment) !=
        allowList.end()) {
        kAlignment_ = newKAlignment;
        return true;
    }
    return false;
}

void CTypeParser::dumpAllTypes(QTextStream &output) const {
    // dump typedef definitions
    output << QStringLiteral("\ntypedef definitions:")
           << QStringLiteral("\n--------------------") << Qt::endl;
    dumpTypeDefines(output);

    // dump numeric const variables or macros
    output << QStringLiteral("\nconstant values:")
           << QStringLiteral("\n--------------------") << Qt::endl;
    dumpConstants(output);

    // dump struct definitions
    output << QStringLiteral("\nstruct definitions:")
           << QStringLiteral("\n--------------------") << Qt::endl;
    dumpStructs(output);

    // dump union definitions
    output << QStringLiteral("\nunion definitions:")
           << QStringLiteral("\n--------------------") << Qt::endl;
    dumpUnions(output);

    // dump enum definitions
    output << QStringLiteral("\nenum definitions:")
           << QStringLiteral("\n--------------------") << Qt::endl;
    dumpEnums(output);
}

void CTypeParser::dumpTypeDefines(QTextStream &output) const {
    static QString padding(4, ' ');
    for (auto it = type_defs_.constKeyValueBegin();
         it != type_defs_.constKeyValueEnd(); ++it) {
        output << padding << it->first << QStringLiteral(" = ")
               << it->second.first;
        if (it->second.second) {
            output << '*';
        }
        if (!isCompletedType(it->first)) {
            output << QStringLiteral(" [?]");
        }
        output << Qt::endl;
    }
}

void CTypeParser::dumpConstants(QTextStream &output) const {
    static QString padding(4, ' ');
    for (auto it = const_defs_.constKeyValueBegin();
         it != const_defs_.constKeyValueEnd(); ++it) {
        auto v = it->second;
        output << padding << it->first << padding << QStringLiteral(" = ");
        if (std::holds_alternative<qint64>(v)) {
            output << std::get<qint64>(v);
        } else if (std::holds_alternative<quint64>(v)) {
            output << std::get<quint64>(v);
        } else {
            output << '?';
        }
        output << Qt::endl;
    }
}

void CTypeParser::dumpStructs(QTextStream &output) const {
    static QString padding(4, ' ');
    for (auto it = struct_defs_.constKeyValueBegin();
         it != struct_defs_.constKeyValueEnd(); ++it) {

        output << QStringLiteral("struct ") << it->first;
        if (!isCompletedType(it->first)) {
            output << QStringLiteral(" [?]");
        }
        output << ':' << Qt::endl;

        auto members = it->second;
        while (!members.empty()) {
            auto var = members.front();
            output << padding << var.data_type;

            if (var.is_pointer)
                output << QStringLiteral("* ");

            output << padding;
            if (var.var_name.isEmpty()) {
                output << '?';
            } else {
                output << var.var_name;
            }

            for (auto &dim : var.array_dims) {
                output << '[' << dim << ']';
            }

            if (var.bit_size) {
                output << QStringLiteral(" : ") << var.bit_size;
            }

            output << padding << QStringLiteral("(off: ") << var.offset
                   << QStringLiteral(", size: ") << var.var_size << ')';

            if (var.bit_size) {
                output << QStringLiteral(" { mask: ")
                       << QString::number(var.op.mask, 16)
                       << QStringLiteral(", shift: ") << var.op.shift
                       << QStringLiteral(" }");
            }

            output << Qt::endl;

            members.pop_front();
        }

        auto type = it->first;
        output << padding << QStringLiteral("(size = ")
               << type_maps_.value(type).second << QStringLiteral(")\n")
               << Qt::endl;
    }
}

void CTypeParser::dumpUnions(QTextStream &output) const {
    static QString padding(4, ' ');
    for (auto itu = union_defs_.constKeyValueBegin();
         itu != union_defs_.constKeyValueEnd(); ++itu) {

        output << QStringLiteral("union ") << itu->first;
        if (!isCompletedType(itu->first)) {
            output << QStringLiteral(" [?]");
        }
        output << ':' << Qt::endl;

        auto members = itu->second;
        while (!members.isEmpty()) {
            auto var = members.front();
            output << padding << var.data_type;

            if (var.is_pointer)
                output << QStringLiteral("* ");

            output << padding;
            if (var.var_name.isEmpty()) {
                output << '?';
            } else {
                output << var.var_name;
            }

            for (auto &dim : var.array_dims) {
                output << '[' << dim << ']';
            }

            output << padding << '(' << var.var_size << ')' << Qt::endl;

            members.pop_front();
        }
        output << padding << QStringLiteral("(size = ")
               << type_maps_.value(itu->first).second << QStringLiteral(")\n")
               << Qt::endl;
    }
}

void CTypeParser::dumpEnums(QTextStream &output) const {
    static QString padding(4, ' ');
    for (auto itv = enum_defs_.constKeyValueBegin();
         itv != enum_defs_.constKeyValueEnd(); ++itv) {

        output << QStringLiteral("enum ") << itv->first << ':' << Qt::endl;

        auto members = itv->second;
        for (auto &n : members) {
            output << padding << n << '(';
            auto value = const_defs_.value(n);
            if (std::holds_alternative<quint64>(value)) {
                output << std::get<quint64>(value) << 'u';
            } else {
                output << std::get<qint64>(value);
            }
            output << ')' << Qt::endl;
        }

        output << '\n' << Qt::endl;
    }
}

void CTypeParser::clear() {
    type_maps_.removeIf([this](decltype(type_maps_)::iterator p) {
        return !base_types_.contains(p.key());
    });
    const_defs_.clear();
    enum_defs_.clear();
    type_defs_.clear();
    struct_defs_.clear();
    union_defs_.clear();
    incompleteDeps_.clear();
    referencedIncomplete_.clear();
    _anomyIndex = 0;
}

QStringList CTypeParser::getMissingDependencise(
    const QVector<VariableDeclaration> &members) {
    QStringList missingDeps;
    for (auto &m : members) {
        if (m.is_pointer) {
            continue;
        } else {
            if (!containsType(m.data_type)) {
                missingDeps.append(m.data_type);
            }
        }
    }
    return missingDeps;
}

qint64 CTypeParser::calcUnionSize(const QList<VariableDeclaration> &members,
                                  int alignment) const {
    qint64 size = 0;
    for (auto it = members.begin(); it != members.end(); ++it) {
        size = qMax(size, it->var_size);
    }

    if (0 != size % alignment) {
        size = static_cast<qint64>(ceil(size * 1.0 / alignment) * alignment);
    }

    return size;
}

/// Store the definition and size of a struct or union
///
/// For structs, the members are padded based on alignment, @see
/// CTypeParser::padStruct
///
bool CTypeParser::storeStructUnionDef(const bool is_struct,
                                      const QString &type_name,
                                      QVector<VariableDeclaration> &members,
                                      qsizetype alignment) {
    qint64 size = 0;
    if (members.isEmpty()) {
        size = 1;
        if (is_struct) {
            struct_defs_[type_name] = members;
        } else {
            union_defs_[type_name] = members;
        }
    } else {
        for (auto &m : members) {
            if (containsEnum(m.data_type)) {
                // warnning
                MsgInfo info;
                info.type = MsgType::Warn;
                info.info =
                    QStringLiteral("sizeof(%1) will be always 8 for enum type")
                        .arg(m.data_type);
                _msgcb(info);
            }
            auto s = getTypeSize(m.data_type);
            Q_ASSERT(s);
            auto total = m.element_count;
            m.var_size = total * s.value();
        }
        if (is_struct) {
            // size limit will be applied into padStruct
            auto os = padStruct(members, alignment);
            if (!os) {
                return false;
            }
            size = os.value();
            struct_defs_[type_name] = members;
        } else {
            size = calcUnionSize(members, alignment);
            if (size > STURCT_SIZE_LIMIT) {
                return false;
            }
            union_defs_[type_name] = members;
        }
    }

    // struct meta type = QMetaType::User;
    type_maps_[type_name] = qMakePair(QMetaType::User, size);
    return true;
}

void CTypeParser::restoreIncompleteType(const QString &name) {
    for (auto &deps : incompleteDeps_) {
        if (deps.contains(name)) {
            deps.removeOne(name);
        }
    }

    QStringList ct;
    incompleteDeps_.removeIf(
        [this, &ct](const QPair<QString, QStringList> &item) -> bool {
            auto name = item.first;
            if (item.second.isEmpty()) {
                switch (referencedIncomplete_[name]) {
                case IncompleteType::Struct: {
                    auto ret =
                        storeStructUnionDef(true, name, struct_defs_[name],
                                            struct_union_alignments_[name]);
                    if (!ret) {
                        struct_defs_.remove(name);
                        struct_union_alignments_.remove(name);
                        addForwardStruct(name);
                    }
                } break;
                case IncompleteType::Union: {
                    auto ret =
                        storeStructUnionDef(false, name, union_defs_[name],
                                            struct_union_alignments_[name]);
                    if (!ret) {
                        union_defs_.remove(name);
                        struct_union_alignments_.remove(name);
                        addForwardUnion(name);
                    }
                } break;
                case IncompleteType::Typedef:
                    type_maps_.insert(name, type_maps_[type_defs_[name].first]);
                    break;
                case IncompleteType::Enum:
                case IncompleteType::AnyTypeWithoutEnum:
                    // do nothing
                    break;
                }
                referencedIncomplete_.remove(name);
                ct.append(name);
                return true;
            }
            return false;
        });
    for (auto &c : ct) {
        restoreIncompleteType(c);
    }
}

std::optional<qint64>
CTypeParser::padStruct(QVector<VariableDeclaration> &members, int alignment) {
    // Validate alignment (allowed: 1,2,4,8,16)
    if (!(alignment == 1 || alignment == 2 || alignment == 4 ||
          alignment == 8 || alignment == 16)) {
        return std::nullopt;
    }

    auto align_up = [](qint64 offset, int align) -> std::optional<qint64> {
        qint64 av = align - 1;
        qint64 r;
        if (qAddOverflow(offset, av, &r)) {
            return std::nullopt;
        }
        return (r / align) * align;
    };

    qint64 total = 0;

    // Bitfield tracking
    qint64 bitfield_base_size =
        0; // current unit size in bytes (one of 1,2,4,8,16)
    qint64 bitfield_capacity = 0; // bits in that unit
    qint64 bitfield_used = 0;     // bits used in current unit
    qint64 bitfield_offset = 0;   // byte offset where that unit starts
    QString bitfield_type;

    // helper: choose smallest standard unit >= alignment and >= needed bytes
    auto choose_unit_bytes = [&](qint64 want_bits) -> std::optional<qint64> {
        qint64 needed_bytes = (want_bits + 7) / 8;
        const qint64 choices[] = {1, 2, 4, 8, 16};
        for (qint64 c : choices) {
            if (c >= alignment && c >= needed_bytes) {
                return c;
            }
        }
        return std::nullopt; // too big to represent
    };

    for (auto &member : members) {
        // update element_count from array_dims if present (same as before) ...
        if (!member.array_dims.isEmpty()) {
            qint64 ec = 1;
            for (qint64 &d : member.array_dims) {
                if (d == 0) {
                    ec = 0;
                    break;
                }
                if (ec > std::numeric_limits<qint64>::max() / d) {
                    ec = 0;
                    break;
                }
                ec *= d;
            }
            member.element_count = (ec != 0) ? ec : 1;
        }

        if (member.bit_size > 0) {
            // defensive: ensure bit_size is sane
            if (member.bit_size == 0)
                return std::nullopt;

            qint64 want_bits = member.bit_size;

            // choose storage unit size based on alignment and want_bits
            auto maybe_unit = choose_unit_bytes(want_bits);
            if (!maybe_unit) {
                return std::nullopt;
            }
            qint64 this_base_size = maybe_unit.value();
            qint64 this_capacity = this_base_size * 8;

            // Start a new block if no active block, or base_size differs,
            // or not enough bits remain.
            bool needs_new_block =
                (bitfield_used == 0) ||
                (bitfield_base_size != this_base_size) ||
                (bitfield_used + want_bits > bitfield_capacity);

            if (needs_new_block) {
                auto au = align_up(total, alignment);
                if (!au) {
                    return std::nullopt;
                }
                total = au.value();

                bitfield_offset = total;
                if (qAddOverflow(total, this_base_size, &total)) {
                    return std::nullopt;
                }

                if (total > STURCT_SIZE_LIMIT) {
                    return std::nullopt;
                }

                bitfield_base_size = this_base_size;
                bitfield_capacity = this_capacity;
                bitfield_used = 0;
                bitfield_type = member.data_type;
            }

            member.offset = bitfield_offset;
            member.op.shift = bitfield_used;

            // mask safe compute
            size_t mask64;
            if (want_bits >= 64) {
                mask64 = std::numeric_limits<size_t>::max();
            } else {
                mask64 = ((size_t(1) << want_bits) - 1ULL);
            }
            member.op.mask = mask64;

            bitfield_used += want_bits;

            if (bitfield_used >= bitfield_capacity) {
                bitfield_base_size = 0;
                bitfield_capacity = 0;
                bitfield_used = 0;
                bitfield_type.clear();
            }

        } else {
            // non-bitfield: flush bitfield state and place normal field
            bitfield_base_size = 0;
            bitfield_capacity = 0;
            bitfield_used = 0;
            bitfield_type.clear();

            qint64 block_size = member.var_size;
            if (block_size == 0) {
                return std::nullopt;
            }

            auto au = align_up(total, alignment);
            if (!au) {
                return std::nullopt;
            }
            total = au.value();

            member.offset = total;
            if (qAddOverflow(total, block_size, &total)) {
                return std::nullopt;
            }

            if (total > STURCT_SIZE_LIMIT) {
                return std::nullopt;
            }
        }
    }

    // final struct padding
    auto au = align_up(total, alignment);
    if (!au) {
        return std::nullopt;
    }
    total = au.value();

    if (total > STURCT_SIZE_LIMIT) {
        return std::nullopt;
    }

    return total;
}
