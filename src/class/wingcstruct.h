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

    // IWingPlugin interface
public:
    virtual RegisteredEvents registeredEvents() const override;
    virtual QHash<WingHex::SettingPage *, bool>
    registeredSettingPages() const override;
    virtual QHash<QString, ScriptFnInfo> registeredScriptFns() const override;
    virtual bool eventOnScriptPragma(const QStringList &comments) override;
    virtual bool eventScriptPragmaLineStep(const QString &codes) override;
    virtual void eventScriptPragmaFinished() override;

private:
    // basic
    WING_SERVICE bool addStruct(const QString &header);
    WING_SERVICE bool addStructFromFile(const QString &fileName);
    WING_SERVICE void resetEnv();

    WING_SERVICE bool setColorTable(const QVector<QColor> &table);
    WING_SERVICE QVector<QColor> colorTable();

    WING_SERVICE bool setStructPadding(int padding); // [1,8]
    WING_SERVICE int structPadding();

    // lookup
    WING_SERVICE QStringList structTypes();
    WING_SERVICE qsizetype sizeofStruct(const QString &type);
    WING_SERVICE bool existStruct(const QString &type);

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

    // read / write
    WING_SERVICE QVariantHash read(qsizetype offset, const QString &type);
    WING_SERVICE bool write(qsizetype offset, const QString &type,
                            const QVariantHash &content);
    WING_SERVICE bool insert(qsizetype offset, const QString &type,
                             const QVariantHash &content);
    WING_SERVICE bool append(const QString &type, const QVariantHash &content);

private:
    QByteArray getRawData(const QString &type, const QVariantHash &content);

    QColor getNextColor();

private:
    // wrapper for WingAngelApi
    QVariant addStruct(const QVariantList &params);
    QVariant addStructFromFile(const QVariantList &params);
    QVariant resetEnv(const QVariantList &params);

    QVariant setColorTable(const QVariantList &params);
    QVariant colorTable(const QVariantList &params);

    QVariant setStructPadding(const QVariantList &params);
    QVariant structPadding(const QVariantList &params);

    QVariant structTypes(const QVariantList &params);
    QVariant sizeofStruct(const QVariantList &params);
    QVariant existStruct(const QVariantList &params);

    QVariant metadata(const QVariantList &params);
    QVariant foreground(const QVariantList &params);
    QVariant background(const QVariantList &params);
    QVariant comment(const QVariantList &params);

    QVariant read(const QVariantList &params);
    QVariant write(const QVariantList &params);
    QVariant insert(const QVariantList &params);
    QVariant append(const QVariantList &params);

private:
    CTypeParser _parser;

    QVector<QColor> _colortable;
    QVector<QColor>::const_iterator _curColor;

    QHash<WingHex::SettingPage *, bool> _setpgs;
    QHash<QString, WingCStruct::ScriptFnInfo> _scriptfns;
};

#endif // WINGCSTRUCT_H
