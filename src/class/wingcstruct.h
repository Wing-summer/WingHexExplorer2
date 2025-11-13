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

#ifndef WINGCSTRUCT_H
#define WINGCSTRUCT_H

#include "WingPlugin/iwingplugin.h"
#include "structlib/ctypeparser.h"
#include "utilities.h"

class CScriptDictionary;
class asIScriptEngine;
class CScriptArray;

class WingCStruct : public WingHex::IWingPlugin {
    Q_OBJECT
    Q_INTERFACES(WingHex::IWingPlugin)

private:
    inline quint64 getShiftAndMaskedPtr(const QByteArray &buffer) {
        if (buffer.size() == sizeof(quint32)) {
            return Utilities::processEndian(
                *reinterpret_cast<const quint32 *>(buffer.data()), m_islittle);
        } else {
            return Utilities::processEndian(
                *reinterpret_cast<const quint64 *>(buffer.data()), m_islittle);
        }
    }

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

public:
    explicit WingCStruct();
    virtual ~WingCStruct();

    // IWingPluginBase interface
public:
    virtual bool init(const std::unique_ptr<QSettings> &set) override;
    virtual void unload(std::unique_ptr<QSettings> &set) override;
    virtual const QString pluginName() const override;
    virtual const QString pluginComment() const override;
    virtual QIcon pluginIcon() const override;

    virtual QString retranslate(const QString &str) override;

    // IWingPlugin interface
public:
    virtual RegisteredEvents registeredEvents() const override;
    virtual QList<WingHex::SettingPage *>
    registeredSettingPages() const override;
    virtual std::optional<WingHex::PragmaResult>
    eventOnScriptPragma(const QString &script,
                        const QStringList &comments) override;
    virtual void eventOnScriptPragmaInit() override;

    // IWingPluginCoreBase interface
public:
    virtual void onRegisterScriptObj(WingHex::IWingAngel *o) override;

private:
    struct CINT_TYPE {
    public:
        CINT_TYPE() : d(0) {}
        CINT_TYPE(qint64 v)
            : d(CTypeParser::INT_TYPE(std::in_place_type_t<qint64>(), v)) {}
        CINT_TYPE(quint64 v)
            : d(CTypeParser::INT_TYPE(std::in_place_type_t<quint64>(), v)) {}
        CINT_TYPE(CTypeParser::INT_TYPE v) : d(v) {}
        ~CINT_TYPE() = default;

    public:
        bool isInt() const { return std::holds_alternative<qint64>(d); }

        bool isUInt() const { return std::holds_alternative<quint64>(d); }

        qint64 toInt() const {
            if (isInt()) {
                return std::get<qint64>(d);
            } else {
                return qint64(std::get<quint64>(d));
            }
        }

        quint64 toUInt() const {
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

private:
    // basic
    bool parseFromSource(const QString &header);
    bool parse(const QString &fileName);
    void reset();

    bool isLittleEndian() const;
    void setIsLittleEndian(bool newIslittle);

    bool setPadAlignment(int padding); // 1, 2, 4, 8, 16
    int padAlignment();

    // lookup
    QStringList structTypeDefs();
    QStringList unionTypeDefs();
    QStringList typedefTypeDefs();
    QStringList enumTypeDefs();
    QStringList constVarDefs();

    quint64 sizeOf(const QString &type);
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

    bool isCompletedStruct(const QString &name);
    bool isCompletedUnion(const QString &name);

    QStringList getMissingDependencise(const QString &name);

    QVariantHash read(qsizetype offset, const QString &type);
    QByteArray readRaw(qsizetype offset, const QString &type);

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

    // services
private:
    // basic
    WING_SERVICE bool parseFromSource(const WingHex::SenderInfo &sender,
                                      const QString &header);
    WING_SERVICE bool parse(const WingHex::SenderInfo &sender,
                            const QString &fileName);
    WING_SERVICE void reset(const WingHex::SenderInfo &sender);

    WING_SERVICE bool isLittleEndian(const WingHex::SenderInfo &sender) const;
    WING_SERVICE void setIsLittleEndian(const WingHex::SenderInfo &sender,
                                        bool newIslittle);

    WING_SERVICE bool setPadAlignment(const WingHex::SenderInfo &sender,
                                      int padding); // 1, 2, 4, 8, 16
    WING_SERVICE int padAlignment(const WingHex::SenderInfo &sender);

    // lookup
    WING_SERVICE QStringList structTypeDefs(const WingHex::SenderInfo &sender);
    WING_SERVICE QStringList unionTypeDefs(const WingHex::SenderInfo &sender);
    WING_SERVICE QStringList typedefTypeDefs(const WingHex::SenderInfo &sender);
    WING_SERVICE QStringList enumTypeDefs(const WingHex::SenderInfo &sender);
    WING_SERVICE QStringList constVarDefs(const WingHex::SenderInfo &sender);

    WING_SERVICE quint64 sizeOf(const WingHex::SenderInfo &sender,
                                const QString &type);
    WING_SERVICE bool containsType(const WingHex::SenderInfo &sender,
                                   const QString &name);

    WING_SERVICE bool isBasicType(const WingHex::SenderInfo &sender,
                                  const QString &name);
    WING_SERVICE bool isUnsignedBasicType(const WingHex::SenderInfo &sender,
                                          const QString &name);

    WING_SERVICE bool containsEnum(const WingHex::SenderInfo &sender,
                                   const QString &name);
    WING_SERVICE bool containsStruct(const WingHex::SenderInfo &sender,
                                     const QString &name);
    WING_SERVICE bool containsUnion(const WingHex::SenderInfo &sender,
                                    const QString &name);
    WING_SERVICE bool containsTypeDef(const WingHex::SenderInfo &sender,
                                      const QString &name);
    WING_SERVICE bool containsConstVar(const WingHex::SenderInfo &sender,
                                       const QString &name);
    WING_SERVICE bool isCompletedType(const WingHex::SenderInfo &sender,
                                      const QString &name);

    WING_SERVICE QStringList enumValueNames(const WingHex::SenderInfo &sender,
                                            const QString &name);
    WING_SERVICE qint64 constVarValueInt(const WingHex::SenderInfo &sender,
                                         const QString &name, bool *ok);
    WING_SERVICE quint64 constVarValueUInt(const WingHex::SenderInfo &sender,
                                           const QString &name, bool *ok);

    WING_SERVICE bool isCompletedStruct(const WingHex::SenderInfo &sender,
                                        const QString &name);
    WING_SERVICE bool isCompletedUnion(const WingHex::SenderInfo &sender,
                                       const QString &name);

    WING_SERVICE QStringList getMissingDependencise(
        const WingHex::SenderInfo &sender, const QString &name);

    WING_SERVICE QVariantHash read(const WingHex::SenderInfo &sender,
                                   qsizetype offset, const QString &type);
    WING_SERVICE QByteArray readRaw(const WingHex::SenderInfo &sender,
                                    qsizetype offset, const QString &type);

    WING_SERVICE QString dumpAllTypes(const WingHex::SenderInfo &sender);
    WING_SERVICE QString dumpTypeDefines(const WingHex::SenderInfo &sender);
    WING_SERVICE QString dumpConstants(const WingHex::SenderInfo &sender);
    WING_SERVICE QString dumpStructs(const WingHex::SenderInfo &sender);
    WING_SERVICE QString dumpUnions(const WingHex::SenderInfo &sender);
    WING_SERVICE QString dumpEnums(const WingHex::SenderInfo &sender);

    WING_SERVICE QStringList getParsedErrors(const WingHex::SenderInfo &sender);
    WING_SERVICE QStringList getParsedWarns(const WingHex::SenderInfo &sender);

    // for query struct/union internals
    WING_SERVICE QStringList structOrUnionMemberNames(
        const WingHex::SenderInfo &sender, const QString &type);
    WING_SERVICE QStringList structOrUnionMemberDataTypes(
        const WingHex::SenderInfo &sender, const QString &type);
    WING_SERVICE QStringList structOrUnionMemberDecls(
        const WingHex::SenderInfo &sender, const QString &type);
    WING_SERVICE QStringList structOrUnionMemberDeclWithoutNames(
        const WingHex::SenderInfo &sender, const QString &type);

private:
    WingHex::MetaType getqsizetypeMetaType() const;

    QString getqsizeTypeAsString() const;

    QVariant getData(const char *ptr, const char *end, QMetaType::Type type,
                     size_t shift, size_t mask, qsizetype size);

    QVariantHash __read(qsizetype offset, const QString &type,
                        bool efmtInVariantList);

    QVariantHash readStruct(const char *ptr, const char *end,
                            const QString &type, bool efmtInVariantList);

    QVariant readContent(const char *ptr, const char *end,
                         const VariableDeclaration &m, bool efmtInVariantList);

    bool isValidCStructMetaType(QMetaType::Type type);
    CScriptDictionary *convert2AsDictionary(const QVariantHash &hash);
    CScriptArray *convert2AsArray(const QVariantList &array,
                                  QMetaType::Type type, int id);

private:
    // wrapper for WingAngelApi
    QVariant parseFromSource(const QVariantList &params);
    QVariant parse(const QVariantList &params);
    QVariant reset(const QVariantList &params);

    QVariant isLittleEndian(const QVariantList &params);
    QVariant setIsLittleEndian(const QVariantList &params);

    QVariant setPadAlignment(const QVariantList &params);
    QVariant padAlignment(const QVariantList &params);

    QVariant structTypeDefs(const QVariantList &params);
    QVariant unionTypeDefs(const QVariantList &params);
    QVariant typedefTypeDefs(const QVariantList &params);
    QVariant enumTypeDefs(const QVariantList &params);
    QVariant constVarDefs(const QVariantList &params);

    QVariant sizeOf(const QVariantList &params);
    QVariant containsType(const QVariantList &params);
    QVariant isBasicType(const QVariantList &params);
    QVariant isUnsignedBasicType(const QVariantList &params);
    QVariant containsEnum(const QVariantList &params);
    QVariant containsStruct(const QVariantList &params);
    QVariant containsUnion(const QVariantList &params);
    QVariant containsTypeDef(const QVariantList &params);
    QVariant containsConstVar(const QVariantList &params);
    QVariant isCompletedType(const QVariantList &params);

    QVariant enumValueNames(const QVariantList &params);
    QVariant isCompletedStruct(const QVariantList &params);
    QVariant isCompletedUnion(const QVariantList &params);

    QVariant getMissingDependencise(const QVariantList &params);

    WingHex::UNSAFE_RET constVarValueInt(const QList<void *> &params);
    WingHex::UNSAFE_RET constVarValueUInt(const QList<void *> &params);
    WingHex::UNSAFE_RET constVarValue(const QList<void *> &params);

    WingHex::UNSAFE_RET read(const QList<void *> &params);
    QVariant readRaw(const QVariantList &params);

    QVariant getParsedErrors(const QVariantList &params);
    QVariant getParsedWarns(const QVariantList &params);

    QVariant structOrUnionMemberNames(const QVariantList &params);
    QVariant structOrUnionMemberDataTypes(const QVariantList &params);
    QVariant structOrUnionMemberDecls(const QVariantList &params);
    QVariant structOrUnionMemberDeclWithoutNames(const QVariantList &params);

private:
    CTypeParser *_parser = nullptr;
    QList<WingHex::SettingPage *> _setpgs;
    bool m_islittle = Utilities::checkIsLittleEndian();

    QStringList _errors, _warns;
};

#endif // WINGCSTRUCT_H
