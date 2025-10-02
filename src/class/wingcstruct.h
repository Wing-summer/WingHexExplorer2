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
    template <typename T, typename = std::enable_if<std::is_arithmetic_v<T>>>
    inline T getShiftAndMasked(const QByteArray &buffer, size_t shift,
                               size_t mask) {
        auto buf = Utilities::processEndian(buffer, m_islittle);
        if constexpr (std::is_integral_v<T>) {
            auto len = buffer.size();
            Q_ASSERT(len >= sizeof(quint8));

            if (len >= sizeof(quint64)) {
                auto r = *reinterpret_cast<const quint64 *>(buf.constData());
                r >>= shift;
                r &= mask;
                return T(r);
            }

            if (len >= sizeof(quint32)) {
                auto r = *reinterpret_cast<const quint32 *>(buf.constData());
                r >>= shift;
                r &= mask;
                return T(r);
            }

            if (len >= sizeof(quint16)) {
                auto r = *reinterpret_cast<const quint16 *>(buf.constData());
                r >>= shift;
                r &= mask;
                return T(r);
            }

            auto r = *reinterpret_cast<const quint8 *>(buf.constData());
            r >>= shift;
            r &= mask;
            return T(r);
        } else if constexpr (std::is_same_v<T, float>) {
            return *reinterpret_cast<const float *>(buf.constData());
        } else if constexpr (std::is_same_v<T, double>) {
            return *reinterpret_cast<const double *>(buf.constData());
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
    // basic
    WING_SERVICE bool parseFromSource(const QString &header);
    WING_SERVICE bool parse(const QString &fileName);
    WING_SERVICE void reset();

    WING_SERVICE bool isLittleEndian() const;
    WING_SERVICE void setIsLittleEndian(bool newIslittle);

    WING_SERVICE bool setPadAlignment(int padding); // 1, 2, 4, 8, 16
    WING_SERVICE int padAlignment();

    // lookup
    WING_SERVICE QStringList structTypeDefs();
    WING_SERVICE QStringList unionTypeDefs();
    WING_SERVICE QStringList typedefTypeDefs();
    WING_SERVICE QStringList enumTypeDefs();
    WING_SERVICE QStringList constVarDefs();

    WING_SERVICE quint64 sizeOf(const QString &type);
    WING_SERVICE bool containsType(const QString &name);

    WING_SERVICE bool isBasicType(const QString &name);
    WING_SERVICE bool isUnsignedBasicType(const QString &name);

    WING_SERVICE bool containsEnum(const QString &name);
    WING_SERVICE bool containsStruct(const QString &name);
    WING_SERVICE bool containsUnion(const QString &name);
    WING_SERVICE bool containsTypeDef(const QString &name);
    WING_SERVICE bool containsConstVar(const QString &name);
    WING_SERVICE bool isCompletedType(const QString &name);

    WING_SERVICE QStringList enumValueNames(const QString &name);
    WING_SERVICE qint64 constVarValueInt(const QString &name, bool *ok);
    WING_SERVICE quint64 constVarValueUInt(const QString &name, bool *ok);

    WING_SERVICE bool isCompletedStruct(const QString &name);
    WING_SERVICE bool isCompletedUnion(const QString &name);

    WING_SERVICE QStringList getMissingDependencise(const QString &name);

    WING_SERVICE QVariantHash read(qsizetype offset, const QString &type);
    WING_SERVICE QByteArray readRaw(qsizetype offset, const QString &type);

private:
    WingHex::MetaType getqsizetypeMetaType() const;

    QString getqsizeTypeAsString() const;

    QVariant getData(const char *ptr, const char *end, QMetaType::Type type,
                     size_t shift, size_t mask, qsizetype size);

    QVariantHash readStruct(const char *&ptr, const char *end,
                            const QString &type);

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

    WingHex::UNSAFE_RET read(const QList<void *> &params);
    QVariant readRaw(const QVariantList &params);

private:
    CTypeParser *_parser = nullptr;
    QList<WingHex::SettingPage *> _setpgs;
    bool m_islittle = Utilities::checkIsLittleEndian();
};

#endif // WINGCSTRUCT_H
