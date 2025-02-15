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

#include "ctypeparser.h"
#include "plugin/iwingplugin.h"

class CScriptDictionary;
class asIScriptEngine;
class CScriptArray;

class WingCStruct : public WingHex::IWingPlugin {
    Q_OBJECT

public:
    explicit WingCStruct();
    virtual ~WingCStruct();

    // IWingPluginBase interface
public:
    virtual int sdkVersion() const override;
    virtual const QString signature() const override;
    virtual bool init(const std::unique_ptr<QSettings> &set) override;
    virtual void unload(std::unique_ptr<QSettings> &set) override;
    virtual const QString pluginName() const override;
    virtual const QString pluginComment() const override;
    virtual QIcon pluginIcon() const override;

    virtual QString retranslate(const QString &str) override;

    // IWingPlugin interface
public:
    virtual RegisteredEvents registeredEvents() const override;
    virtual QHash<WingHex::SettingPage *, bool>
    registeredSettingPages() const override;
    virtual QHash<QString, ScriptFnInfo> registeredScriptFns() const override;
    virtual bool eventOnScriptPragma(const QString &script,
                                     const QStringList &comments) override;
    virtual void eventOnScriptPragmaInit() override;
    virtual QHash<QString, UNSAFE_SCFNPTR>
    registeredScriptUnsafeFns() const override;

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

    // metadata
    WING_SERVICE bool metadata(qsizetype offset, const QString &type,
                               const QColor &fg, const QColor &bg,
                               const QString &comment);
    WING_SERVICE bool foreground(qsizetype offset, const QString &type,
                                 const QColor &color);
    WING_SERVICE bool background(qsizetype offset, const QString &type,
                                 const QColor &color);
    WING_SERVICE bool comment(qsizetype offset, const QString &type,
                              const QString &comment);

    WING_SERVICE QVariantHash read(qsizetype offset, const QString &type);
    WING_SERVICE QByteArray readRaw(qsizetype offset, const QString &type);

private:
    MetaType getqsizetypeMetaType() const;

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

    QVariant metadata(const QVariantList &params);
    QVariant foreground(const QVariantList &params);
    QVariant background(const QVariantList &params);
    QVariant comment(const QVariantList &params);

    UNSAFE_RET read(const QList<void *> &params);
    QVariant readRaw(const QVariantList &params);

private:
    CTypeParser _parser;

    QHash<WingHex::SettingPage *, bool> _setpgs;
    QHash<QString, WingCStruct::ScriptFnInfo> _scriptInfo;
    QHash<QString, WingHex::IWingPlugin::UNSAFE_SCFNPTR> _scriptUnsafe;
};

#endif // WINGCSTRUCT_H
