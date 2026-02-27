/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#ifndef STRUCTPARSER_H
#define STRUCTPARSER_H

#include "scriptaddon/scriptqdictionary.h"
#include "structlib/ctypeparser.h"
#include "utilities.h"

struct CINT_TYPE {
public:
    inline CINT_TYPE() : d(0) {}
    inline CINT_TYPE(qint64 v)
        : d(CTypeParser::INT_TYPE(std::in_place_type_t<qint64>(), v)) {}
    inline CINT_TYPE(quint64 v)
        : d(CTypeParser::INT_TYPE(std::in_place_type_t<quint64>(), v)) {}
    inline CINT_TYPE(CTypeParser::INT_TYPE v) : d(v) {}
    inline ~CINT_TYPE() = default;

public:
    inline bool isInt() const { return std::holds_alternative<qint64>(d); }

    inline bool isUInt() const { return std::holds_alternative<quint64>(d); }

    inline qint64 toInt() const {
        if (isInt()) {
            return std::get<qint64>(d);
        } else {
            return qint64(std::get<quint64>(d));
        }
    }

    inline quint64 toUInt() const {
        if (isUInt()) {
            return std::get<quint64>(d);
        } else {
            return quint64(std::get<qint64>(d));
        }
    }

public:
    CTypeParser::INT_TYPE d;
};

using CEnumValue = QPair<CINT_TYPE, QString>;

class WingCStruct;

class StructParser {
public:
    explicit StructParser(WingCStruct *api);
    virtual ~StructParser();

public:
    // basic
    bool parseFromSource(const QString &header);
    bool parse(const QString &fileName);
    void reset();

    bool isLittleEndian() const;
    void setIsLittleEndian(bool newIslittle);

    // 1, 2, 4, 8, 16
    bool setPadAlignment(int padding);
    int padAlignment() const;

    // lookup
    QStringList structTypeDefs();
    QStringList unionTypeDefs();
    QStringList typedefTypeDefs();
    QStringList enumTypeDefs();
    QStringList constVarDefs();

    qint64 sizeOf(const QString &type);
    bool containsType(const QString &name);

    bool isBasicType(const QString &name);
    bool isUnsignedBasicType(const QString &name);

    bool containsEnum(const QString &name);
    bool containsStruct(const QString &name);
    bool containsUnion(const QString &name);
    bool containsTypeDef(const QString &name);
    bool containsConstVar(const QString &name);
    bool isCompletedType(const QString &name);

    QStringList enumValueNames(const QString &name);
    qint64 constVarValueInt(const QString &name, bool *ok);
    quint64 constVarValueUInt(const QString &name, bool *ok);
    CINT_TYPE constVarValue(const QString &name) const;

    bool isCompletedStruct(const QString &name);
    bool isCompletedUnion(const QString &name);

    QStringList getMissingDependencise(const QString &name);

    QVariantHash __read(qsizetype offset, const QString &type,
                        bool efmtInVariantList);
    QVariant __readMember(qsizetype offset, const QString &type,
                          const QString &member, bool efmtInVariantList);
    QVariantHash __readMembers(qsizetype offset, const QString &type,
                               const std::function<QString(uint)> &members,
                               uint total, bool efmtInVariantList);
    QByteArray __readRaw(qsizetype offset, const QString &type);

    QString dumpAllTypes() const;
    QString dumpTypeDefines() const;
    QString dumpConstants() const;
    QString dumpStructs() const;
    QString dumpUnions() const;
    QString dumpEnums() const;

    QStringList getParsedErrors() const;
    QStringList getParsedWarns() const;

    QStringList structOrUnionMemberNames(const QString &type) const;
    QStringList structOrUnionMemberDataTypes(const QString &type) const;
    QStringList structOrUnionMemberDecls(const QString &type) const;
    QStringList structOrUnionMemberDeclWithoutNames(const QString &type) const;

    QString resolveTypeName(const QString &name) const;

private:
    quint64 getShiftAndMaskedPtr(const QByteArray &buffer);

    template <typename T, typename = std::enable_if<std::is_arithmetic_v<T>>>
    inline T getShiftAndMasked(const QByteArray &buffer, size_t shift,
                               size_t mask) {
        if constexpr (std::is_integral_v<T>) {
            auto len = buffer.size();
            if (buffer.isEmpty()) {
                return T();
            }

            if (len >= sizeof(quint64)) {
                auto r = Utilities::processEndian(
                    *reinterpret_cast<const quint64 *>(buffer.constData()),
                    m_islittle);
                r >>= shift;
                r &= mask;
                return T(r);
            }

            if (len >= sizeof(quint32)) {
                auto r = Utilities::processEndian(
                    *reinterpret_cast<const quint32 *>(buffer.constData()),
                    m_islittle);
                r >>= shift;
                r &= mask;
                return T(r);
            }

            if (len >= sizeof(quint16)) {
                auto r = Utilities::processEndian(
                    *reinterpret_cast<const quint16 *>(buffer.constData()),
                    m_islittle);
                r >>= shift;
                r &= mask;
                return T(r);
            }

            auto r = *reinterpret_cast<const quint8 *>(buffer.constData());
            r >>= shift;
            r &= mask;
            return T(r);
        } else if constexpr (std::is_same_v<T, float>) {
            return Utilities::processEndian(
                *reinterpret_cast<const float *>(buffer.constData()),
                m_islittle);
        } else if constexpr (std::is_same_v<T, double>) {
            return Utilities::processEndian(
                *reinterpret_cast<const double *>(buffer.constData()),
                m_islittle);
        } else {
            return T();
        }
    }

private:
    QVariant getData(const char *ptr, const char *end, QMetaType::Type type,
                     qint64 shift, qint64 mask, qsizetype size);

    QVariantHash readStruct(const char *ptr, const char *end,
                            const QString &type, bool efmtInVariantList);

    QVariant readStructMember(const char *ptr, const char *end,
                              const QString &type, const QString &member,
                              bool efmtInVariantList);

    QVariantHash readStructMembers(const char *ptr, const char *end,
                                   const QString &type,
                                   const std::function<QString(uint)> &members,
                                   uint total, bool efmtInVariantList);

    QVariant readContent(const char *ptr, const char *end,
                         const VariableDeclaration &m, bool efmtInVariantList);

protected:
    bool isValidCStructMetaType(QMetaType::Type type);
    CScriptDictionary *convert2AsDictionary(const QVariantHash &hash);
    CScriptArray *convert2AsArray(const QVariantList &array,
                                  QMetaType::Type type, int id);

private:
    WingCStruct *_api = nullptr;
    CTypeParser *_parser = nullptr;
    QStringList _errors, _warns;
    bool m_islittle = Utilities::checkIsLittleEndian();
};

#endif // STRUCTPARSER_H
