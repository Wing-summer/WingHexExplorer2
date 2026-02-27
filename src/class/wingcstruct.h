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
#include "structparser.h"

class CScriptDictionary;
class asIScriptEngine;
class CScriptArray;

class WingCStruct : public WingHex::IWingPlugin {
    Q_OBJECT
    Q_INTERFACES(WingHex::IWingPlugin)

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
    virtual QList<WingHex::SettingPage *>
    registeredSettingPages() const override;

    // IWingPluginCoreBase interface
public:
    virtual void onRegisterScriptObj(WingHex::IWingAngel *o) override;

    // services
private:
    // acquire
    WING_SERVICE bool createParser(const WingHex::SenderInfo &sender);
    WING_SERVICE void destroyParser(const WingHex::SenderInfo &sender);

    // basic
    WING_SERVICE bool parseFromSource(const WingHex::SenderInfo &sender,
                                      const QString &header);
    WING_SERVICE bool parse(const WingHex::SenderInfo &sender,
                            const QString &fileName);
    WING_SERVICE bool reset(const WingHex::SenderInfo &sender);

    WING_SERVICE bool isLittleEndian(const WingHex::SenderInfo &sender) const;
    WING_SERVICE bool setIsLittleEndian(const WingHex::SenderInfo &sender,
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

    WING_SERVICE qint64 sizeOf(const WingHex::SenderInfo &sender,
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
    WING_SERVICE QVariant readMember(const WingHex::SenderInfo &sender,
                                     qsizetype offset, const QString &type,
                                     const QString &member);
    WING_SERVICE QVariantHash readMembers(const WingHex::SenderInfo &sender,
                                          qsizetype offset, const QString &type,
                                          const QStringList &members);
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

    WING_SERVICE QString resolveTypeName(const WingHex::SenderInfo &sender,
                                         const QString &type);

private:
    WingHex::MetaType getqsizetypeMetaType() const;
    QString getqsizeTypeAsString() const;

private:
    QHash<QString, StructParser *> _parsers;

    QList<WingHex::SettingPage *> _setpgs;
};

#endif // WINGCSTRUCT_H
