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

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QList>
#include <QMap>
#include <QRandomGenerator>
#include <QSet>
#include <QTextStream>
#include <QtEndian>

#include "ctypeparser.h"

/// underline ('_') shouldn't be included as it can be part of an identifier
const QRegularExpression CTypeParser::kTokenDelimiters(
    R"([ \t\#\{\[\(\<&\|\*\>\)\]\}\?\':\",%!=/;\+\*$])");

/// prefix that is used to make a fake identifier for anonymous
/// struct/union/enum type
const QString CTypeParser::kAnonymousTypePrefix = "_ANONYMOUS_";

/// name of padding field into a struct/union, for the purpose of alignment
const QString CTypeParser::kPaddingFieldName = "_padding_field_";

CTypeParser::~CTypeParser() {}

CTypeParser::CTypeParser() { initialize(); }

void CTypeParser::initialize() {

#define ADD_TYPE(T, MT) type_maps_.insert(#T, qMakePair(MT, sizeof(T)))

#define ADD_TYPE_U(T, MT)                                                      \
    ADD_TYPE(T, MT);                                                           \
    unsigned_types_.append(#T);

#define ADD_TYPE_S(T, MT, S) type_maps_.insert(#T, qMakePair(MT, S))

    // qualifiers to ignore in parsing
    qualifiers_ =
        QStringList{"static", "const",    "far",    "extern",       "volatile",
                    "auto",   "register", "inline", "__attribute__"};

    // keywords that we care
    keywords_["struct"] = kStructKeyword;
    keywords_["union"] = kUnionKeyword;
    keywords_["enum"] = kEnumKeyword;
    keywords_["typedef"] = kTypedefKeyword;
    keywords_["signed"] = kSignedKeyword;
    keywords_["unsigned"] = KUnsignedKeyword;

    using byte = unsigned char;
    using char8_t = unsigned char;
    using int8 = qint8;
    using int16 = qint16;
    using int32 = qint32;
    using int64 = qint64;
    using int16_le = qint16_le;
    using int16_be = qint16_be;
    using int32_le = qint32_le;
    using int32_be = qint32_be;
    using int64_le = qint64_le;
    using int64_be = qint64_be;
    using uint8 = quint8;
    using uint16 = quint16;
    using uint32 = quint32;
    using uint64 = quint64;
    using uint16_le = quint16_le;
    using uint16_be = quint16_be;
    using uint32_le = quint32_le;
    using uint32_be = quint32_be;
    using uint64_le = quint64_le;
    using uint64_be = quint64_be;
    using longlong = qlonglong;
    using ulonglong = qulonglong;

    ADD_TYPE_U(bool, QMetaType::Bool);
    ADD_TYPE(char, QMetaType::Char);
    ADD_TYPE(short, QMetaType::Short);
    ADD_TYPE(int, QMetaType::Int);
    ADD_TYPE(long, QMetaType::Long);
    ADD_TYPE(float, QMetaType::Float);
    ADD_TYPE(double, QMetaType::Double);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    ADD_TYPE_U(wchar_t, QMetaType::Char16);
#else
    ADD_TYPE_U(wchar_t, QMetaType::QChar);
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

    ADD_TYPE_U(byte, QMetaType::UChar);

    ADD_TYPE(int8, QMetaType::SChar);
    ADD_TYPE(int16, QMetaType::Short);
    ADD_TYPE(int16_le, QMetaType::Short);
    ADD_TYPE(int16_be, QMetaType::Short);
    ADD_TYPE(int32, QMetaType::Int);
    ADD_TYPE(int32_le, QMetaType::Int);
    ADD_TYPE(int32_be, QMetaType::Int);
    ADD_TYPE(int64, QMetaType::LongLong);
    ADD_TYPE(int64_le, QMetaType::LongLong);
    ADD_TYPE(int64_be, QMetaType::LongLong);

    ADD_TYPE_U(uint8, QMetaType::UChar);
    ADD_TYPE_U(uint16, QMetaType::UShort);
    ADD_TYPE_U(uint16_le, QMetaType::UShort);
    ADD_TYPE_U(uint16_be, QMetaType::UShort);
    ADD_TYPE_U(uint32, QMetaType::UInt);
    ADD_TYPE_U(uint32_le, QMetaType::UInt);
    ADD_TYPE_U(uint32_be, QMetaType::UInt);
    ADD_TYPE_U(uint64, QMetaType::ULongLong);
    ADD_TYPE_U(uint64_le, QMetaType::ULongLong);
    ADD_TYPE_U(uint64_be, QMetaType::ULongLong);

    ADD_TYPE(longlong, QMetaType::LongLong);
    ADD_TYPE_U(ulonglong, QMetaType::ULongLong);

    using BOOL = bool;
    using BYTE = byte;
    using WORD = uint16;
    using DWORD = uint32;
    using QWORD = uint64;
    using DWORDLONG = QWORD;
    using FLOAT = float;
    using DOUBLE = double;
    using DWORD32 = uint32;
    using DWORD64 = uint64;

    ADD_TYPE_U(BOOL, QMetaType::Bool);
    ADD_TYPE_U(BYTE, QMetaType::UChar);
    ADD_TYPE_U(WORD, QMetaType::UShort);
    ADD_TYPE_U(DWORD, QMetaType::UInt);
    ADD_TYPE_U(QWORD, QMetaType::ULongLong);
    ADD_TYPE_U(DWORDLONG, QMetaType::ULongLong);

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

    ADD_TYPE_U(UINT8, QMetaType::UChar);
    ADD_TYPE_U(UINT16, QMetaType::UShort);
    ADD_TYPE_U(UINT32, QMetaType::UInt);
    ADD_TYPE_U(UINT64, QMetaType::ULongLong);

    using SHORT = short;
    using INT = int;
    using LONG = long;
    using WCHAR = wchar_t;

    ADD_TYPE(SHORT, QMetaType::Short);
    ADD_TYPE(INT, QMetaType::Int);
    ADD_TYPE(LONG, QMetaType::Long);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    ADD_TYPE_U(WCHAR, QMetaType::Char16);
#else
    ADD_TYPE_U(WCHAR, QMetaType::QChar);
#endif

    using LONGLONG = longlong;
    using ULONGLONG = ulonglong;

    ADD_TYPE(LONGLONG, QMetaType::LongLong);
    ADD_TYPE_U(ULONGLONG, QMetaType::ULongLong);

    using LONG32 = qint32;
    using LONG64 = qint64;

    ADD_TYPE(LONG32, QMetaType::Long);
    ADD_TYPE(LONG64, QMetaType::LongLong);

    using SIZE_T = size_t;
    if (sizeof(size_t) == sizeof(quint64)) {
        ADD_TYPE_U(size_t, QMetaType::ULongLong);
        ADD_TYPE_U(SIZE_T, QMetaType::ULongLong);
    } else {
        ADD_TYPE_U(size_t, QMetaType::ULong);
        ADD_TYPE_U(SIZE_T, QMetaType::ULong);
    }

    ADD_TYPE(int8_t, QMetaType::SChar);
    ADD_TYPE(int16_t, QMetaType::Short);
    ADD_TYPE(int32_t, QMetaType::Int);
    ADD_TYPE(int64_t, QMetaType::LongLong);

    ADD_TYPE_U(uint8_t, QMetaType::UChar);
    ADD_TYPE_U(uint16_t, QMetaType::UShort);
    ADD_TYPE_U(uint32_t, QMetaType::UInt);
    ADD_TYPE_U(uint64_t, QMetaType::ULongLong);

#undef ADD_TYPE
#undef ADD_TYPE_S
}

void CTypeParser::setIncludePaths(const QStringList &paths) {
    include_paths_ = paths;
}

void CTypeParser::parseFiles() {
    for (const QString &path : include_paths_) {
        findHeaderFiles(path);

        for (auto it = header_files_.begin(); it != header_files_.end(); ++it) {
            parseFile(it.key());
        }
    }
}

bool CTypeParser::parseFile(const QString &file) {
    // parse a file only when it's not yet parsed
    if (header_files_.contains(file) && header_files_[file]) {
        qDebug() << "File is already processed:" << file;
        return true;
    }

    QFile ifs(file);
    if (!ifs.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file -" << file;
        return false;
    }

    // flag to true before parsing the file so that it won't be parsed
    // duplicately
    header_files_[file] = true;
    qDebug() << "Parsing file -" << file;

    return parseSource(preprocess(ifs));
}

bool CTypeParser::parseSource(const QString &src) {
    qsizetype pos = 0;
    QString token, line;
    bool is_typedef = false;
    TokenTypes type;

    VariableDeclaration decl;
    bool is_decl = false;

    while (getNextToken(src, pos, token)) {
        if (token.length() == 1) {
            switch (token.at(0).toLatin1()) {
            case kPoundSign:
                if (!parsePreProcDirective(src, pos)) {
                    return false;
                }
                break;

            // below characters can be ignored silently
            case kBlockStart:
            case kBlockEnd:
            case kSemicolon:
                decl.force_unsigned = false;
                break;

            default:
                skipCurrentLine(src, pos, line);
                qDebug() << "Character '" + token +
                                "' unexpected, ignore the line";
            }
        } else {
            type = getTokenType(token);
            switch (type) {
            case kStructKeyword:
            case kUnionKeyword:
                if (!parseStructUnion((kStructKeyword == type) ? true : false,
                                      is_typedef, src, pos, decl, is_decl) ||
                    is_decl) {
                    // error occurred
                    qCritical()
                        << "Bad syntax for nested struct variable declaration";
                    return false;
                }

                // reset is_typedef
                is_typedef = false;
                break;

            case kEnumKeyword:
                if (!parseEnum(is_typedef, src, pos, decl, is_decl) ||
                    is_decl) {
                    qCritical()
                        << "Bad syntax for nested enum variable declaration";
                    return false;
                }

                // reset is_typedef
                is_typedef = false;
                break;

            case kTypedefKeyword:
                is_typedef = true;
                break;

            case kBasicDataType:
                // only (const) global variable are supported
                if (!getRestLine(src, pos, line)) {
                    return false;
                }
                if (!parseAssignExpression(line)) {
                    qDebug() << "Expression not supported - " + line;
                }
                break;

            case kSignedKeyword: // ignore
                break;
            case KUnsignedKeyword:
                decl.force_unsigned = true;
                break;
            default:
                break;
            }
        }
    }
    return true;
}

void CTypeParser::findHeaderFiles(const QString &folder) {
    QDir dir(folder);

    if (!dir.exists()) {
        qWarning() << "Failed to open folder:" << folder;
        return;
    }

    QFileInfoList files =
        dir.entryInfoList(QStringList() << "*.h", QDir::Files);
    for (const QFileInfo &fileInfo : files) {
        header_files_[fileInfo.absoluteFilePath()] =
            false; // "false" means not yet parsed
        qDebug() << "Found header file:" << fileInfo.absoluteFilePath();
    }

    QFileInfoList subDirs =
        dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &subDirInfo : subDirs) {
        qDebug() << "Searching folder:" << subDirInfo.absoluteFilePath();
        findHeaderFiles(subDirInfo.absoluteFilePath());
    }
}

QString CTypeParser::getFile(QString &filename) const {
    for (const QString &path : include_paths_) {
        QDir dir(path);
        QFileInfoList files =
            dir.entryInfoList(QStringList() << filename, QDir::Files);

        if (!files.isEmpty()) {
            filename = files.first().absoluteFilePath();
            return filename;
        }
    }

    return "";
}

QString CTypeParser::preprocess(QFile &ifs) const {
    QString line;
    QStringList lines;

    QTextStream stream(&ifs);
    while (!stream.atEnd()) {
        line = stream.readLine();

        if (!line.trimmed().isEmpty()) {
            lines.append(line);
        }
    }

    stripComments(lines);

    static QTextStream qout(stdout);
    for (auto &line : lines) {
        qout << line << Qt::endl;
    }

    wrapLines(lines);

    return mergeAllLines(lines);
}

// qInfo() << QStringLiteral("parsing line: [") + line +
//                QStringLiteral("]");

void CTypeParser::stripComments(QStringList &lines) const {
    bool is_block = false; // whether a comment block starts
    bool is_changed =
        false; // whether current line is changed after removing comments

    QString line;
    int pos = 0;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QStringList::const_iterator block_start, it = lines.cbegin();
#else
    QStringList::iterator block_start, it = lines.begin();
#endif

    while (it != lines.constEnd()) {
        is_block = is_changed = false;

        line = *it;
        pos = 0;

        qInfo() << QStringLiteral("parsing line: [") + line +
                       QStringLiteral("]");

        // search comment start
        while ((pos = line.indexOf(kSlash, pos)) != -1) {
            if (line.length() <= pos + 1)
                break; // the 1st '/' is at the end of line, so not a comment

            switch (line.at(pos + 1).unicode()) {
            case kSlash: // line comment
                line.remove(pos, line.length() - pos);
                is_changed = true;
                break;

            case kAsterisk: // comment block
                // is_block = true;
                is_changed = true;

                do {
                    int found = line.indexOf("*/", pos + 2);
                    if (found != -1) {
                        line.remove(pos, found - pos + 2);
                        is_block = false;
                    } else {
                        line.remove(pos, line.length() - pos);
                        is_block = true;
                        break;
                    }
                } while (!is_block && (pos = line.indexOf("/*", pos)) != -1);
                // loop for possible multiple comment blocks on one line
                break;

            default:
                pos++; // might be other '/' after this one
            }
        }

        if (!is_changed) {
            ++it;
            continue;
        } else {
            if (!line.isEmpty()) {
                it =
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                    QStringList::const_iterator
#endif
                    (std::next(lines.insert(it, line)));
            }

            it =
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                QStringList::const_iterator
#endif
                (lines.erase(it));
        }

        if (is_block) {
            block_start = it;
            while (it != lines.end()) {
                line = *it;
                if ((pos = line.indexOf("*/")) >= 0) {
                    lines.erase(block_start, ++it);

                    line.remove(0, pos + 2);
                    if (!line.isEmpty()) {
                        // insert rest part for checking of
                        // possible followed comments
                        it =
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                            QStringList::const_iterator
#endif
                            (std::next(lines.insert(it, line)));
                    }

                    is_block = false;
                    break;
                }

                ++it;
            }

            if (is_block) {
                qCritical("Unclosed comment block exists");
            }
        }
    }
}

// Merge wrapped lines into one line
void CTypeParser::wrapLines(QStringList &lines) const {
    QString line;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto it = lines.cbegin();
    while (it != lines.cend()) {
#else
    auto it = lines.begin();
    while (it != lines.end()) {
#endif
        auto first = it;
        line = *it;
        if (line.isEmpty()) {
            it++;
            continue;
        }

        while ('\\' == line.at(line.length() - 1) && ++it != lines.end()) {
            line = line.left(line.length() - 1);
            line = line.trimmed() + (*it);
        }

        if (it == lines.end()) {
            qWarning() << "Bad syntax: wrap line at last line";
            break;
        }

        if (it != first) {
            lines.insert(first, line); // insert merged string before "first"
                                       // without invalidating the iterators

            ++it; // increase so that current line can be erased from the list
            it =
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                QStringList::const_iterator
#endif
                (lines.erase(first, it));
        } else {
            ++it;
        }
    }
}

// Get next token starting from the specified position of the line
// pos will be moved to after the token if token not at line end
//
// Assuming not all of the rest characters are blanks as all lines are trimed
// before Note: Among the many punctuations that can be used in C language (like
// #[(<&|*>)]} ? \' : \", % != /;)
//   only '_' can be part of a token here
QString CTypeParser::getNextToken(const QString &line, qsizetype &pos) const {
    if (pos >= qsizetype(line.length()))
        return {};

    // skip blanks
    while (line.at(pos).isSpace())
        pos++;
    qsizetype start = pos;

    QChar ch;
    qsizetype len = line.length();
    while (pos < len) {
        ch = line.at(pos);
        if (ch.isSpace() || (ch.isPunct() && '_' != ch))
            break;

        ++pos; // move to the next character
    }

    qDebug() << QStringLiteral("Next token: ") + line.mid(start, pos - start);
    return (start == pos) ? "" : line.mid(start, pos - start);
}

bool CTypeParser::isIgnorable(const QString &token) const {
    if (token.isEmpty()) {
        return true;
    } else {
        return qualifiers_.contains(token);
    }
}

/// Query token type from known keywords/qualifiers or basic/use-defined types
///
/// @param[in]  token   a token
/// @return the corresponding token type, or kUnresolvedToken if not found
///
TokenTypes CTypeParser::getTokenType(const QString &token) const {
    if (keywords_.contains(token)) {
        return keywords_.value(token);
    } else if (qualifiers_.contains(token)) {
        return kQualifier;
    } else if (type_maps_.contains(token)) {
        return kBasicDataType;
    } else if (struct_defs_.contains(token)) {
        return kStructName;
    } else if (union_defs_.contains(token)) {
        return kUnionName;
    } else if (enum_defs_.contains(token)) {
        return kEnumName;
    } else {
        return kUnresolvedToken;
    }
}

/// Check whether the token is a number or can be translated into a number
///
/// @param[in]  token   a token
/// @param[out] number  the token's numeric value
/// @return true if 1) the token is a number, or
///                 2) the token is a macro that have a number as its value, or
///                 3) the token is a const variable that has been assigned to a
///                 number
bool CTypeParser::isNumericToken(const QString &token, long &number) const {
    if (token.isEmpty()) {
        return false;
    }

    bool ret = true;
    number = token.toInt(&ret, 0);

    if (!ret) {
        // the token is not a number, then check whether it can be translated
        // into a number
        if (const_defs_.contains(token)) {
            number = const_defs_.value(token);
            ret = true;
        } else {
            qDebug() << QStringLiteral("Cannot parse token <") + token +
                            QStringLiteral("> into a number");
        }
    }

    return ret;
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
int CTypeParser::getTypeSize(const QString &data_type) const {
    if (type_maps_.contains(data_type)) {
        return type_maps_.value(data_type).second;
    } else if (enum_defs_.contains(data_type)) {
        return sizeof(int);
    } else {
        qCritical() << QStringLiteral("Unknown data type - ") + data_type;
        return -1;
    }
}

PointerMode CTypeParser::pointerMode() const { return _pmode; }

void CTypeParser::setPointerMode(PointerMode newPmode) { _pmode = newPmode; }

const QHash<QString, QPair<QMetaType::Type, qsizetype>> &
CTypeParser::types() const {
    return type_maps_;
}

QPair<QMetaType::Type, qsizetype> CTypeParser::type(const QString &t) const {
    return type_maps_.value(t);
}

const QHash<QString, QList<VariableDeclaration>> &
CTypeParser::unionDefs() const {
    return union_defs_;
}

const QHash<QString, int> &CTypeParser::constDefs() const {
    return const_defs_;
}

const QHash<QString, QList<QPair<QString, int>>> &
CTypeParser::enumDefs() const {
    return enum_defs_;
}

const QHash<QString, QList<VariableDeclaration>> &
CTypeParser::structDefs() const {
    return struct_defs_;
}

QStringList CTypeParser::includePaths() const { return include_paths_; }

qsizetype CTypeParser::padAlignment() const { return kAlignment_; }

void CTypeParser::setPadAlignment(qsizetype newKAlignment) {
    kAlignment_ = newKAlignment;
}

/// Dump the extracted type definitions
void CTypeParser::dumpTypeDefs() const {
    VariableDeclaration var;

    static QTextStream qout(stdout);

    // dump numeric const variables or macros
    qout << "\nconstant values:"
         << "\n--------------------" << Qt::endl;
    for (auto it = const_defs_.constKeyValueBegin();
         it != const_defs_.constKeyValueEnd(); ++it) {
        qout << "\t" << it->first << "\t = " << it->second << Qt::endl;
    }

    // dump struct definitions
    qout << "\nstruct definitions:"
         << "\n--------------------" << Qt::endl;
    for (auto it = struct_defs_.constKeyValueBegin();
         it != struct_defs_.constKeyValueEnd(); ++it) {

        qout << "struct " << it->first << ":" << Qt::endl;

        auto members = it->second;
        while (!members.empty()) {
            var = members.front();
            qout << '\t' << var.data_type;

            if (var.is_pointer)
                qout << "* ";

            qout << "\t" << var.var_name;

            if (0 < var.array_size)
                qout << "[" << var.array_size << "]";

            qout << "\t(" << var.var_size << ")" << Qt::endl;

            members.pop_front();
        }

        auto type = it->first;
        qout << "\t(size = " << type_maps_.value(type).second << ")\n"
             << Qt::endl;
    }

    // dump union definitions
    qout << "\nunion definitions:"
         << "\n--------------------" << Qt::endl;
    for (auto itu = union_defs_.constKeyValueBegin();
         itu != union_defs_.constKeyValueEnd(); ++itu) {

        qout << "union " << itu->first << ":" << Qt::endl;

        auto members = itu->second;
        while (!members.isEmpty()) {
            var = members.front();
            qout << '\t' << var.data_type;

            if (var.is_pointer)
                qout << "* ";

            qout << "\t" << var.var_name;

            if (0 < var.array_size)
                qout << "[" << var.array_size << "]";

            qout << "\t(" << var.var_size << ")" << Qt::endl;

            members.pop_front();
        }
        qout << "\t(size = " << type_maps_.value(itu->first).second << ")\n"
             << Qt::endl;
    }

    // dump enum definitions
    qout << "\nenum definitions:"
         << "\n--------------------" << Qt::endl;
    for (auto itv = enum_defs_.constKeyValueBegin();
         itv != enum_defs_.constKeyValueEnd(); ++itv) {

        qout << "enum " << itv->first << ":" << Qt::endl;

        auto members = itv->second;
        while (!members.isEmpty()) {
            auto var = members.front();
            qout << '\t' << var.first << "(" << var.second << ")" << Qt::endl;
            members.pop_front();
        }
        qout << '\n' << Qt::endl;
    }
}

// Merge all lines into a string for easier parsing
QString CTypeParser::mergeAllLines(const QStringList &lines) const {
    QString src, line, part;
    int pos;

    QStringList::const_iterator it = lines.begin();
    while (it != lines.end()) {
        line = *it;
        if (line.isEmpty()) {
            it++;
            continue;
        }

        if ('#' == line.at(0)) { // don't  split pre-processing line
            src += line + EOL;
        } else {
            while (true) {
                pos = line.indexOf(',');
                if (pos < 0) {
                    pos = line.indexOf(';');
                }
                if (pos < 0) {
                    break;
                }

                if (pos == line.length() - 1) {
                    break;
                } else { // split line
                    part = line.left(pos + 1);
                    src += part.trimmed() + EOL; // trim splited lines

                    line = line.mid(++pos);
                }
            }

            src += line + EOL;
        }

        ++it;
    }

    return src;
}

/// Get next token - it can either be a special character, or a
/// keyword/identifier
///
/// @param[in]      src     source code
/// @param[in,out]  pos     start position to parse the code;
///                         will be updated to the first char after the token
///                         after this method is called
/// @param[out]     token   the next token after @var pos
/// @param[in]      cross_line  true by default, false is only used for parsing
/// pre-processing directives
/// @return         false only when file end is reached
///
/// @note When cross_line is false, only get next token from current line where
/// @var pos resides
/// @note Qualifiers defined in @var qualifiers_ are skipped as they don't
/// matter
///
bool CTypeParser::getNextToken(QString src, qsizetype &pos, QString &token,
                               bool cross_line) const {
    // cross_line=false, only check token from current line
    if (!cross_line) {
        auto p = src.indexOf(EOL, pos);
        if (p >= 0)
            src = src.left(p);
    }

    qsizetype len = src.length();

    // skip leading blanks or EOL
    while (pos < len && (src.at(pos).isSpace() || EOL == src.at(pos)))
        pos++;

    if (pos >= len) {
        token.clear();
        return false;
    }

    qsizetype start = pos;
    int ptk = src.indexOf(kTokenDelimiters, start);
    if (ptk < 0) {
        token = src.mid(start);
        pos = src.length();
    } else if (start == qsizetype(ptk)) {
        pos = ptk + 1;
        token = QString(1, src[ptk]);
    } else {
        pos = ptk;
        token = src.mid(start, ptk - start);
    }

    // skip possible empty or ignorable tokens
    while (!token.isEmpty() && isIgnorable(token))
        getNextToken(src, pos, token);

    return (token.isEmpty() ? false : true);
}

/// Get the next line
/// @param[in,out]  pos     start position to parse the code;
///                         will be updated to end of next line after this
///                         method is called
/// @return false only when current line is the last line
bool CTypeParser::getNextLine(QString src, qsizetype &pos,
                              QString &line) const {
    qsizetype start = src.indexOf(EOL, pos);
    if (start < 0) {
        pos = src.length();
        line.clear();
        return false;
    }

    ++start;
    qsizetype end = src.indexOf(EOL, start);
    if (end < 0) {
        line = src.mid(start);
        pos = src.length();
    } else {
        // assert fail only when it's an empty line
        if (end == start) {
            return false;
        }
        line = src.mid(start, end - start);
        pos = end;
    }

    if (line.trimmed().isEmpty()) {
        return false;
    }
    return true;
}

bool CTypeParser::getRestLine(
    const QString &src, qsizetype &pos,
    QString &line) const { // get rest part of the line

    if (EOL == src.at(pos)) {
        line.clear();
        return false;
    }

    qsizetype p = src.indexOf(EOL, pos);
    if (p >= 0) {
        line = src.mid(pos, p - pos);
        pos = p;
    } else {
        line = src.mid(pos);
        pos = src.length();
    }

    return true;
}

/// skip current line that @var pos resides
///
/// @param[in]      src     source code
/// @param[in,out]  pos     start position to parse the code;
///                         will be updated to the first char of next line after
///                         this method is called
/// store the line content into "line"
void CTypeParser::skipCurrentLine(const QString &src, qsizetype &pos,
                                  QString &line) const {
    if (pos >= qsizetype(src.length())) {
        qCritical("SkipCurrentLine() - string offset larger than its length");
        line.clear();
        return;
    }

    auto start = src.lastIndexOf(EOL, pos);
    auto end = src.indexOf(EOL, pos);
    if (end < 0) {
        line = src.mid(
            start + 1); // it's ok even if start = string::npos as it equals -1
        pos = src.length();
    } else if (qsizetype(end) == pos) {
        start = src.lastIndexOf(EOL, pos - 1);
        line = src.mid(start + 1,
                       end - start - 1); // it's ok even if start = string::npos
        pos++;
    } else {
        line = src.mid(start + 1, end - start - 1);
        pos = end + 1;
    }
}

/// split a line into tokens
///
/// @note: keywords that can be ignored will be removed by getNextToken()
qsizetype CTypeParser::splitLineIntoTokens(QString line,
                                           QStringList &tokens) const {
    QString token;
    qsizetype start = 0;

    while (getNextToken(line, start, token)) {
        tokens.push_back(token);
    }

    return tokens.size();
}

/// Parse a variable declaration
///
/// A declaration can be as complicated as:
///     unsigned char *array[MAX_SIZE]; // qualifiers should be removed from
///     "line" argument struct <complex_type> var;      // the struct/union/enum
///     keyword should be removed from "line" argument
/// @note code lines with multiple variables declared consecutively are ignored,
/// like "int a, b, c = MAX;"
///
/// @param[in]  line    a code line that ends with kSemicolon and is stripped of
/// preceding qualifiers (like unsigned)
///                     and struct/union/enum keywords
/// @param[out] decl    the variable declaration if the line is parsed
/// successfully
/// @return             true if the line can be parsed into a variable
/// declaration successfully
///
/// @note type size are calculated will simple consideration of alignment
/// @note can be improved with consideration of multiple demension array
bool CTypeParser::parseDeclaration(const QString &line,
                                   VariableDeclaration &decl) const {
    if (line.isEmpty()) {
        return false;
    }
    if (line[line.length() - 1] != kSemicolon)
        return false;

    QStringList tokens;
    qsizetype size = splitLineIntoTokens(line, tokens);
    // even the simplest declaration contains 3 tokens: type var ;
    if (size < 3) {
        return false;
    }

    qsizetype index = 0;
    decl.data_type = tokens[index];
    decl.is_pointer = false;

    qsizetype length = getTypeSize(decl.data_type);
    if (0 == length) {
        qCritical() << QStringLiteral("Unknown data type - ") + decl.data_type;
        return false;
    }

    if (tokens[++index].at(0) == kAsterisk) {
        decl.is_pointer = true;
        // size of a pointer is 4 types on a 32-bit system
        length =
            (_pmode == PointerMode::X86 ? sizeof(quint32) : sizeof(quint64));
        decl.var_name = tokens[++index];
    } else {
        decl.var_name = tokens[index];
    }

    if (tokens[++index].at(0) == '[') {
        long number;
        if (isNumericToken(tokens[++index], number)) {
            decl.array_size = number;
            length *= number;
        } else {
            qCritical() << QStringLiteral(
                               "Array size cannot be parsed into a number - ") +
                               tokens[index];
            return false;
        }
    } else {
        decl.array_size = 0;
    }

    decl.var_size = length;

    return true;
}

/// Parsing enum member declaration
///
/// Possible formats:
///		1) Zhejiang             // only for last enum member
///		2) Beijing,
///		3) Shenzhen = <value>   // only for last enum member
///		4) Shanghai = <value>,
///
/// @param[in]		line		the declaration of a enum member
/// @param[in,out]	last_value	[in]the value of last enum member;
/// [out]the value of current enum member
/// @param[out]		decl		enum member declaration
/// @param[out]		is_last_member	true for format 2 & 4, else false
bool CTypeParser::parseEnumDeclaration(const QString &line, int &last_value,
                                       QPair<QString, int> &decl,
                                       bool &is_last_member) const {
    // whether this enum variable is the lastest member of the enum type
    is_last_member = false;
    QStringList tokens;
    long number;

    switch (splitLineIntoTokens(line, tokens)) {
    case 1:
        is_last_member = true;
        decl.second = ++last_value;
        break;

    case 2:
        if (kComma != tokens[1].at(0)) {
            return false;
        }
        decl.second = ++last_value;
        break;

    case 3:
        if (kEqual != tokens[1].at(0)) {
            return false;
        }

        if (!isNumericToken(tokens[2], number)) {
            qCritical() << QStringLiteral(
                               "Cannot convert token into a number - ") +
                               tokens[2];
            return false;
        }

        is_last_member = true;
        decl.second = last_value = static_cast<int>(number);
        break;

    case 4:
        if (!(kEqual == tokens[1].at(0) && kComma == tokens[3].at(0))) {
            return false;
        }

        if (!isNumericToken(tokens[2], number)) {
            qCritical() << QStringLiteral(
                               "Cannot convert token into a number - ") +
                               tokens[2];
            return false;
        }

        decl.second = last_value = static_cast<int>(number);
        break;

    default:
        qCritical() << QStringLiteral(
                           "Bad syntax for enum member declaration - ") +
                           line;
        return false;
    }

    decl.first = tokens[0];
    return true;
}

/// Parse assignment expression
///
/// @param[in]  line    an assignment expression with the format: var = number
/// @return             true if the line can be parsed successfully, and @var
/// const_defs_ will be updated
///
bool CTypeParser::parseAssignExpression(const QString &line) {
    QStringList tokens;
    long number;

    // only 4 tokens for an assignment expression: var = number;
    if (4 == splitLineIntoTokens(line, tokens) &&
        kEqual == tokens[1].at(tokens[1].length() - 1) &&
        kSemicolon == tokens[3].at(tokens[3].length() - 1) &&
        isNumericToken(tokens[2], number)) {
        const_defs_.insert(tokens[0], number);
        return true;
    }

    return false;
}

/// Parse pre-processing directives
///
/// Only below directives with the exact formats are supported:
/// 1) #include "<header filename>"
/// 2) #define <macro name> <number>
/// For others, the whole line will be skipped
///
/// @param src  source code
/// @param pos  start position to parse the code, pointing to the next char
/// after '#';
///             it will be updated to new position when the code is parsed
///
/// @note when a header file inclusion directive is met, the header file is
/// parsed immediately to ensure the correct parsing sequence
///
bool CTypeParser::parsePreProcDirective(const QString &src, qsizetype &pos) {
    QString token, last_token, line;
    long number;

    getNextToken(src, pos, token);
    if (0 == token.compare("include")) {
        if (!getNextToken(src, pos, token, false)) {
            return false;
        }

        // only handle header file included with ""
        if (kQuotation == token[token.length() - 1]) {
            // get included header file name
            if (!getNextToken(src, pos, token, false)) {
                return false;
            }

            // parse the header file immediately
            parseFile(token);

            // ignore the other quotation marks
            skipCurrentLine(src, pos, line);
        } else {
            // ignore angle bracket (<>)
            skipCurrentLine(src, pos, line);
            qInfo() << QStringLiteral("Skip header file included by <> - ") +
                           line;
        }
    } else if (0 == token.compare("define")) {
        if (!getNextToken(src, pos, last_token, false)) {
            return false;
        }

        if (getNextToken(src, pos, token, false) &&
            isNumericToken(token, number)) {
            const_defs_.insert(last_token, number);
        } else {
            skipCurrentLine(src, pos, line);
            qDebug() << QStringLiteral("Ignore define - ") + line;
        }
    } else {
        skipCurrentLine(src, pos, line);
        qInfo() << QStringLiteral("Skip unsupported pre-processing line - ") +
                       line;
    }
    return true;
}

/// Parse struct/union definition or declaration
///
/// @param[in]  is_typedef  whether there's the "typedef" keyword before this
/// statement
/// @param[in]  src         source code to parse
/// @param[in]  pos         position
/// @param[out] var_decl    will be updated only when @var is_decl is true
/// @param[out] is_decl     true if it's a variable declaration (with/without
/// nested type definition);
///                         false if it's a pure type definition
/// return true if it's a struct/union definition or declaration
///
//     starting from the next character after the "struct" or "union" keyword
//     to end of the definition/declaration
//
/// it might be called recursively to parse nested structure of struct/union
//
/// it supports below formats:
///     1) type definition: typedef <type> [<type_name>] {....} <type_alias>;
///     2) type definition:         <type> <type_name> {....};
///     3) var declaration:         <type> <type_name> {....} <var>;
///     4) var declaration:         <type>             {....} <var>;   //
///     anonymous type 5) var declaration:        [<type>] <type_name> <var>; //
///     type_name is defined elsewhere where, <type> refers to either "struct"
///     or "union" keyword
///            <var> here can be as complicated as "*array[MAX_SIZE]"
//
// after calling this function:
//     struct/union definitons will be stored into class member struct_defs_ or
//     union_defs_ pos will point to the next kSemicolon following the block end
//     '}',
//         or equal to src.length() when reaching file end - bad syntax
//     is_decl returns:
//     1) false for definition format 1 and 2;
//     2) true for declaration format 3-5 and "var_decl" argument being updated
///
bool CTypeParser::parseStructUnion(const bool is_struct, const bool is_typedef,
                                   const QString &src, qsizetype &pos,
                                   VariableDeclaration &var_decl,
                                   bool &is_decl) {
    VariableDeclaration member;
    QList<VariableDeclaration> members;

    QString line, token, next_token;
    QString type_name, type_alias;

    if (src.isEmpty() || pos >= qsizetype(src.length())) {
        return false;
    }

    qsizetype start = pos; // store the original position for next guess

    // peek rest of current line starting from "pos"
    if (!getRestLine(src, pos, line)) {
        if (!getNextLine(src, pos, line)) {
            return false;
        }
    }

    // it might be just a simple struct/union variable declaration as format 5
    if (parseDeclaration(line, var_decl)) {
        return (is_decl = true);
    }

    // else, next token should be either '{' or a typename followed by '{'
    pos = start; // reset the position
    if (!getNextToken(src, pos, token)) {
        return false;
    }
    if ('{' != token.at(0)) {
        type_name = token;
        if (!(getNextToken(src, pos, token) && '{' == token.at(0))) {
            return false;
        }
    }

    // the following part should be:
    // 1) struct/union member declarations within the block
    // 2) something out of the block like "} [<type alias|var>];"
    while (getNextToken(src, pos, token)) {
        if ('}' == token.at(0)) { // reach block end
            // process rest part after block end
            // start = 1;
            if (!getNextToken(src, pos, token)) {
                return false;
            }

            if (is_typedef) {
                // format 1
                if (!(getNextToken(src, pos, next_token) &&
                      kSemicolon == next_token.at(0))) {
                    return false;
                }

                is_decl = false;
                type_alias = token; // token is actually type alias
                storeStructUnionDef(is_struct, type_alias, members);

                // when type_name not empty and not the same as type alias,
                // store a copy in case it's used elsewhere
                if (!type_name.isEmpty() &&
                    type_alias.compare(type_name) != 0) {
                    if (is_struct) {
                        struct_defs_[type_name] = members;
                    } else {
                        union_defs_[type_name] = members;
                    }

                    type_maps_[type_name] = type_maps_[type_alias];
                }
            } else { // non-typedef
                if (kSemicolon == token.at(0)) {
                    // format 2
                    is_decl = false;
                    if (type_name.isEmpty()) {
                        return false;
                    }
                    storeStructUnionDef(is_struct, type_name, members);
                } else {
                    // token must be part of a variable declaration
                    // so it must be format 3 or 4
                    if (type_name.isEmpty()) {
                        // format 4: anonymous type
                        // generate a unique random name for this type so that
                        // it can be stored into map
                        do {
                            type_name =
                                kAnonymousTypePrefix +
                                QString::number(
                                    QRandomGenerator::global()->generate());
                        } while (struct_defs_.contains(type_name));
                    }

                    is_decl = true;
                    storeStructUnionDef(is_struct, type_name, members);

                    if (!getRestLine(src, pos, line)) {
                        if (!getNextLine(src, pos, line)) {
                            return false;
                        }
                    }

                    // for easier parsing, make a declaration by adding the
                    // <type_name> before <var>
                    line = type_name + " " + token + " " + line;
                    if (!parseDeclaration(line, var_decl)) {
                        qCritical(
                            "Bad syntax for struct/union type of variable "
                            "declaration after {} block");
                        return false;
                    }
                }
            }

            // break as block ends
            break;
        } else { // parse struct/union member declarations
            TokenTypes type = getTokenType(token);

            if (kStructKeyword == type || kUnionKeyword == type) {
                // a nested struct/union variable declaration
                // bool is_declare;
                // VariableDeclaration var_decl;
                if (!parseStructUnion((kStructKeyword == type) ? true : false,
                                      false, src, pos, member, is_decl)) {
                    qCritical("Bad syntax for nested struct/union variable "
                              "declaration");
                    return false;
                }

                // TODO: also check position here
                if (!is_decl) {
                    return false;
                }
                members.push_back(member);
            } else if (kEnumKeyword == type) {
                if (!parseEnum(false, src, pos, member, is_decl)) {
                    qCritical(
                        "Bad syntax for nested enum variable declaration");
                    return false;
                }

                // TODO: also check position here
                if (!is_decl) {
                    return false;
                }
                members.push_back(member);
            } else {
                // regular struct/union member declaration, including format 5
                if (!getRestLine(src, pos, line)) {
                    if (!getNextLine(src, pos, line)) {
                        return false;
                    }
                }

                line = token + " " + line;
                if (!parseDeclaration(line, member)) {
                    qCritical(
                        "Unresolved struct/union member declaration syntax");
                    return false;
                }

                qInfo() << QStringLiteral("Add member: ") + member.var_name;
                members.push_back(member);
            }
        }
    }

    return true;
}

// Parse enum block
bool CTypeParser::parseEnum(const bool is_typedef, const QString &src,
                            qsizetype &pos, VariableDeclaration &var_decl,
                            bool &is_decl) {
    QPair<QString, int> member;
    QList<QPair<QString, int>> members;
    QString line, token, next_token, type_name;

    int last_value = -1;
    bool is_last_member = false;

    if (src.isEmpty() || pos >= qsizetype(src.length())) {
        return false;
    }

    qsizetype start = pos; // store the original position for next guess

    // peek rest of current line starting from "pos"
    if (!getRestLine(src, pos, line)) {
        if (!getNextLine(src, pos, line)) {
            return false;
        }
    }

    // it might be just a simple enum variable declaration like: enum Home home;
    if (parseDeclaration(line, var_decl)) {
        return (is_decl = true);
    }

    // else, next token should be either '{' or a typename followed by '{'
    pos = start; // reset the position
    if (!getNextToken(src, pos, token)) {
        return false;
    }
    if ('{' != token.at(0)) {
        type_name = token;
        if (!(getNextToken(src, pos, token) && '{' == token.at(0))) {
            return false;
        }
    }

    // the following part should be:
    // 1) enum member declarations within the block
    // 2) something out of the block like "} [<type alias|var>];"
    while (getNextToken(src, pos, token)) {
        if ('}' == token.at(0)) { // reach block end
            // process rest part after block end
            // start = 1;
            if (!getNextToken(src, pos, token)) {
                return false;
            }

            if (is_typedef) {
                // format 1
                if (!(getNextToken(src, pos, next_token) &&
                      kSemicolon == next_token.at(0))) {
                    return false;
                }

                is_decl = false;
                enum_defs_[token] = members; // type alias
                type_maps_[token] = qMakePair(
                    QMetaType::Int,
                    sizeof(int)); // sizeof a enum variable = sizeof(int)

                if (!type_name.isEmpty() && token.compare(type_name) != 0) {
                    enum_defs_[type_name] = members; // type name
                    type_maps_[type_name] =
                        qMakePair(QMetaType::Int, sizeof(int));
                    ;
                }
            } else { // non-typedef
                if (kSemicolon == token.at(0)) {
                    // format 2
                    is_decl = false;
                    if (type_name.isEmpty()) {
                        return false;
                    }
                    enum_defs_[type_name] = members;
                    type_maps_[type_name] =
                        qMakePair(QMetaType::Int, sizeof(int));
                    ;
                } else {
                    // token must be part of a variable declaration
                    // so it must be format 3 or 4

                    if (type_name.isEmpty()) {
                        // format 4: anonymous type
                        // generate a unique random name for this type so that
                        // it can be stored into map
                        do {
                            type_name =
                                kAnonymousTypePrefix +
                                QString::number(
                                    QRandomGenerator::global()->generate());
                        } while (enum_defs_.contains(type_name));
                    }

                    is_decl = true;
                    enum_defs_[type_name] = members;
                    type_maps_[type_name] =
                        qMakePair(QMetaType::Int, sizeof(int));

                    if (!getRestLine(src, pos, line)) {
                        if (!getNextLine(src, pos, line)) {
                            return false;
                        }
                    }

                    // for easier parsing, make a declaration by adding the
                    // <type_name> before <var>
                    line = type_name + " " + token + " " + line;
                    if (!parseDeclaration(line, var_decl)) {
                        qCritical("Bad syntax for enum type of variable "
                                  "declaration after {} block");
                        return false;
                    }
                }
            }

            // break as block ends
            break;
        } else { // parse enum member declarations
            if (is_last_member) {
                qCritical("Bad enum member declaration in last line");
                return false;
            }

            // Note: the last enum memeber can only have one token, so the rest
            // line can be empty here!
            getRestLine(src, pos, line);

            line = token + " " + line;
            if (!parseEnumDeclaration(line, last_value, member,
                                      is_last_member)) {
                qCritical("Unresolved enum member declaration syntax");
                return false;
            }

            qInfo() << QStringLiteral("Add enum member: ") + member.first;
            members.push_back(member);
        }
    }

    return true;
}

VariableDeclaration CTypeParser::makePadField(const qsizetype size) const {
    VariableDeclaration var;

    var.var_name = kPaddingFieldName;
    var.var_size = size;
    var.data_type = "char";
    var.array_size = 0;
    var.is_pointer = false;

    return var;
}

/// Pad a struct with padding fields for memory alignment
///
/// @param[in,out] members  struct members, will be inserted with padding fields
/// when needed
/// @return					struct size after alignment
///
/// @note This method is based on kAlignment_ = 4 on 32-bit system since the
/// padding algorithm can be very complicated considering the different
/// alignment modulus/options of different compiler/OS/CPU About alignment, @see
/// http://c-faq.com/struct/align.esr.html
qsizetype CTypeParser::padStructMembers(QList<VariableDeclaration> &members) {

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto it = members.cbegin();
#else
    auto it = members.begin();
#endif

    qsizetype total = 0;
    qsizetype size;
    qsizetype last_size =
        0; ///< when last_size>0, padding is needed depending on later members;
           ///< else no more padding is needed later
    qsizetype align_size, pad_size;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    while (it != members.cend()) {
#else
    while (it != members.end()) {
#endif
        size = it->var_size;

        if (0 == (size % kAlignment_)) { // current member itself is aligned
            if (last_size > 0) { // need padding previous members to alignment
                align_size = static_cast<qsizetype>(
                    ceil(last_size * 1.0 / kAlignment_) * kAlignment_);
                pad_size = align_size - last_size;
                members.insert(it, makePadField(pad_size));

                total += align_size;
            } else {
                total += size;
            }

            last_size = 0;
            ++it;
        } else { // current member itself cannot align
            // size can only be less than 4 (1 or 2) now unless it's an array
            if (size >= kAlignment_) {
                if (it->array_size > 0) {
                    qDebug("TODO: add array support in PadStructMembers()");
                } else {
                    qCritical() << QStringLiteral("Incorrect type size for ") +
                                       it->var_name;
                }

                return 0;
            }

            if (0 == last_size) { // last member is aligned
                last_size = size;
                ++it;
            } else if (0 == (size + last_size) % kAlignment_) {
                total += size + last_size;
                last_size = 0;
                ++it;
            } else if (1 == last_size) {
                if (1 == size) {
                    last_size += 1;
                    ++it;
                } else if (2 == size) {
                    members.insert(it, makePadField(1));
                    total += kAlignment_;
                    last_size = 0;
                    ++it;
                } else {
                    qCritical() << QStringLiteral("Bad member size - ") +
                                       QString::number(size);
                    return 0;
                }
            } else if (2 == last_size) {
                if (1 != size) {
                    return 0;
                }
                members.insert(++it, makePadField(1));
                total += kAlignment_;
                last_size = 0;
            } else {
                qCritical("Bad alignment");
                return 0;
            }
        }
    }

    return total;
}

qsizetype
CTypeParser::calcUnionSize(const QList<VariableDeclaration> &members) const {
    qsizetype size = 0;
    for (auto it = members.begin(); it != members.end(); ++it) {
        size = qMax(size, it->var_size);
    }

    if (0 != size % kAlignment_) {
        size = static_cast<qsizetype>(ceil(size * 1.0 / kAlignment_) *
                                      kAlignment_);
    }

    return size;
}

/// Store the definition and size of a struct or union
///
/// For structs, the members are padded based on alignment, @see
/// CTypeParser::padStructMembers
///
void CTypeParser::storeStructUnionDef(const bool is_struct,
                                      const QString &type_name,
                                      QList<VariableDeclaration> &members) {
    qsizetype size;
    if (is_struct) {
        size = padStructMembers(members);
        struct_defs_[type_name] = members;
    } else {
        size = calcUnionSize(members);
        union_defs_[type_name] = members;
    }

    type_maps_[type_name] = qMakePair(QMetaType::User, size);
}
