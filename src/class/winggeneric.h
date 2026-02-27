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

#ifndef WINGGENERIC_H
#define WINGGENERIC_H

#include "WingPlugin/iwingangel.h"

class asIScriptGeneric;

class WingGeneric : public WingHex::IWingGeneric {
public:
    explicit WingGeneric(asIScriptGeneric *gen);

    // IWingGeneric interface
public:
    void *auxiliary() const override;
    void *object() const override;
    int objectTypeId() const override;

    int argCount() const override;
    int argTypeId(uint arg, quint32 *flags) const override;
    QString argTypeName(uint arg, quint32 *flags) override;
    uchar argByte(uint arg) override;
    quint16 argWord(uint arg) override;
    quint32 argDWord(uint arg) override;
    quint64 argQWord(uint arg) override;
    float argFloat(uint arg) override;
    double argDouble(uint arg) override;
    void *argAddress(uint arg) override;
    void *argObject(uint arg) override;
    void *addressOfArg(uint arg) override;
    int returnTypeId(quint32 *flags) const override;
    WingHex::asRetCodes setReturnByte(uchar val) override;
    WingHex::asRetCodes setReturnWord(quint16 val) override;
    WingHex::asRetCodes setReturnDWord(quint32 val) override;
    WingHex::asRetCodes setReturnQWord(quint64 val) override;
    WingHex::asRetCodes setReturnFloat(float val) override;
    WingHex::asRetCodes setReturnDouble(double val) override;
    WingHex::asRetCodes setReturnAddress(void *addr) override;
    WingHex::asRetCodes setReturnObject(void *obj) override;
    void *addressOfReturnLocation() override;

private:
    asIScriptGeneric *_gen;
};

#endif // WINGGENERIC_H
