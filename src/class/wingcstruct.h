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
#include "ctypeparser.h"

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
    virtual RegisteredEvents registeredEvents() const override;
    virtual QList<WingHex::SettingPage *>
    registeredSettingPages() const override;
    virtual bool eventOnScriptPragma(const QString &script,
                                     const QStringList &comments) override;
    virtual void eventOnScriptPragmaInit() override;

    // IWingPluginCoreBase interface
public:
    virtual void onRegisterScriptObj(WingHex::IWingAngel *o) override;

private:
    // basic
    WING_SERVICE bool addStruct(const QString &header);
    WING_SERVICE bool addStructFromFile(const QString &fileName);
    WING_SERVICE void resetEnv();

    WING_SERVICE bool setStructPadding(int padding); // [1,8]
    WING_SERVICE int structPadding();

    // lookup
    WING_SERVICE QStringList structTypes();
    WING_SERVICE qsizetype sizeofStruct(const QString &type);
    WING_SERVICE bool existStruct(const QString &type);

    // defines
    WING_SERVICE QStringList constDefines();
    WING_SERVICE bool existDefineValue(const QString &type);
    WING_SERVICE int defineValue(const QString &type);

    WING_SERVICE QVariantHash read(qsizetype offset, const QString &type);
    WING_SERVICE QByteArray readRaw(qsizetype offset, const QString &type);

private:
    WingHex::MetaType getqsizetypeMetaType() const;

    QString getqsizeTypeAsString() const;

    QVariant getData(const char *ptr, const char *end, QMetaType::Type type,
                     qsizetype size);

    QVariantHash readStruct(const char *&ptr, const char *end,
                            const QString &type);

    bool isValidCStructMetaType(QMetaType::Type type);
    CScriptDictionary *convert2AsDictionary(const QVariantHash &hash);
    CScriptArray *convert2AsArray(const QVariantList &array,
                                  QMetaType::Type type, int id);

private:
    // wrapper for WingAngelApi
    QVariant addStruct(const QVariantList &params);
    QVariant addStructFromFile(const QVariantList &params);
    QVariant resetEnv(const QVariantList &params);

    QVariant setStructPadding(const QVariantList &params);
    QVariant structPadding(const QVariantList &params);

    QVariant structTypes(const QVariantList &params);
    QVariant sizeofStruct(const QVariantList &params);
    QVariant existStruct(const QVariantList &params);

    // defines
    QVariant constDefines(const QVariantList &params);
    QVariant existDefineValue(const QVariantList &params);
    QVariant defineValue(const QVariantList &params);

    WingHex::UNSAFE_RET read(const QList<void *> &params);
    QVariant readRaw(const QVariantList &params);

private:
    CTypeParser _parser;

    QList<WingHex::SettingPage *> _setpgs;
};

#endif // WINGCSTRUCT_H
