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

#ifndef WINGANGEL_H
#define WINGANGEL_H

#include "WingPlugin/iwingangel.h"

#include <QCoreApplication>

class WingAngelAPI;

class WingAngel : public WingHex::IWingAngel {
    Q_DECLARE_TR_FUNCTIONS(WingAngel)

public:
    WingAngel(WingAngelAPI *api, QStringList &marcos);

public:
    bool setCurrentPluginSession(const QByteArray &ns);

    // IWingAngel interface
public:
    virtual WingHex::asRetCodes registerGlobalFunction(
        uint retMetaType, const ScriptFn &fn, const QString &fnName,
        const QVector<QPair<uint, QString>> &params) override;

    virtual WingHex::asRetCodes
    registerGlobalFunction(const QString &decl,
                           const WingHex::UNSAFE_SCFNPTR &fn) override;

    virtual void registerScriptMarco(const QString &marco) override;
    virtual void registerScriptMarcos(const QStringList &marcos) override;
    virtual WingHex::asRetCodes
    setDefaultNamespace(const char *nameSpace) override;
    virtual WingHex::asRetCodes restoreDefaultNamespace() override;
    virtual WingHex::asRetCodes registerEnum(const char *type) override;
    virtual WingHex::asRetCodes
    registerEnumValue(const char *type, const char *name, int value) override;
    virtual WingHex::asRetCodes registerFuncdef(const char *decl) override;
    virtual WingHex::asRetCodes registerTypedef(const char *type,
                                                const char *decl) override;
    virtual WingHex::asRetCodes registerGlobalProperty(const char *declaration,
                                                       void *pointer) override;
    virtual WingHex::asRetCodes
    registerObjectType(const char *obj, int byteSize, quint64 flags) override;

    virtual WingHex::asRetCodes
    registerObjectProperty(const char *obj, const char *declaration,
                           int byteOffset, int compositeOffset,
                           bool isCompositeIndirect) override;
    virtual WingHex::asRetCodes
    registerObjectMethod(const char *obj, const char *declaration,
                         const WingHex::asFuncPtr &funcPointer,
                         asCallConvTypes callConv, void *auxiliary,
                         int compositeOffset,
                         bool isCompositeIndirect) override;

    virtual WingHex::asRetCodes registerObjectBehaviour(
        const char *obj, asBehaviours behaviour, const char *declaration,
        const WingHex::asFuncPtr &funcPointer, asCallConvTypes callConv,
        void *auxiliary, int compositeOffset,
        bool isCompositeIndirect) override;

    virtual WingHex::asRetCodes registerInterface(const char *name) override;

    virtual WingHex::asRetCodes
    registerInterfaceMethod(const char *intf, const char *declaration) override;

    virtual WingHex::asRetCodes
    registerGlobalFunction(const char *declaration,
                           const WingHex::asFuncPtr &funcPointer,
                           asCallConvTypes callConv, void *auxiliary) override;

private:
    static QString getScriptFnSig(uint retMetaType, const ScriptFn &fn,
                                  const QString &fnName,
                                  const QVector<QPair<uint, QString>> &params);

    static QString type2AngelScriptString(uint type, bool isArg,
                                          bool noModifier = false);

private:
    WingHex::asRetCodes returnValue(int ret);

private:
    WingAngelAPI *_api;
    QByteArray _plgsess;

    QStringList &_scriptMarcos;
};

#endif // WINGANGEL_H
