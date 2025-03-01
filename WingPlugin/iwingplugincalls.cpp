/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** You can redistribute this file and/or modify it under the terms of the
** BSD 3-Clause.
**
** THIS FILE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
** =============================================================================
*/

#include "iwingplugincalls.h"
#include "WingPlugin/wingcore.h"

#include "iwingplugin.h"

using namespace WingHex;

HexPosition::HexPosition() : line(0), column(0), lineWidth(0), nibbleindex(0) {}

qsizetype HexPosition::offset() const {
    return static_cast<qsizetype>(line * lineWidth) + column;
}

qsizetype HexPosition::operator-(const HexPosition &rhs) const {
    return qsizetype(this->offset() - rhs.offset());
}

bool HexPosition::operator==(const HexPosition &rhs) const {
    return (line == rhs.line) && (column == rhs.column) &&
           (nibbleindex == rhs.nibbleindex);
}

bool HexPosition::operator!=(const HexPosition &rhs) const {
    return (line != rhs.line) || (column != rhs.column) ||
           (nibbleindex != rhs.nibbleindex);
}

//=================================

IWingPluginCalls::IWingPluginCalls(IWingPlugin *const caller)
    : WingHex::WingPluginCalls(caller) {}

bool IWingPluginCalls::existsServiceHost(const QString &puid) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::existsServiceHost);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(puid));
    return ret;
}

bool IWingPluginCalls::invokeService(
    const QString &puid, const char *method, Qt::ConnectionType type,
    QGenericReturnArgument ret, QGenericArgument val0, QGenericArgument val1,
    QGenericArgument val2, QGenericArgument val3, QGenericArgument val4) {
    SETUP_CALL_CONTEXT(
        (QOverload<const QString &, const char *, Qt::ConnectionType,
                   QGenericReturnArgument, QGenericArgument, QGenericArgument,
                   QGenericArgument, QGenericArgument,
                   QGenericArgument>::of(&IWingPluginCalls::invokeService)));
    bool r;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(r),
             getSenderObj(), WINGAPI_ARG(puid), WINGAPI_ARG(method),
             WINGAPI_ARG(type), WINGAPI_ARG(ret), WINGAPI_ARG(val0),
             WINGAPI_ARG(val1), WINGAPI_ARG(val2), WINGAPI_ARG(val3),
             WINGAPI_ARG(val4));
    return r;
}

bool IWingPluginCalls::invokeService(
    const QString &puid, const char *member, QGenericReturnArgument ret,
    QGenericArgument val0, QGenericArgument val1, QGenericArgument val2,
    QGenericArgument val3, QGenericArgument val4) {
    return invokeService(puid, member, Qt::DirectConnection, ret, val0, val1,
                         val2, val3, val4);
}

bool IWingPluginCalls::invokeService(
    const QString &puid, const char *member, Qt::ConnectionType type,
    QGenericArgument val0, QGenericArgument val1, QGenericArgument val2,
    QGenericArgument val3, QGenericArgument val4) {
    return invokeService(puid, member, type, QGenericReturnArgument(), val0,
                         val1, val2, val3, val4);
}

bool IWingPluginCalls::invokeService(const QString &puid, const char *member,
                                     QGenericArgument val0,
                                     QGenericArgument val1,
                                     QGenericArgument val2,
                                     QGenericArgument val3,
                                     QGenericArgument val4) {
    return invokeService(puid, member, Qt::DirectConnection,
                         QGenericReturnArgument(), val0, val1, val2, val3,
                         val4);
}

bool IWingPluginCalls::isCurrentDocEditing() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::isCurrentDocEditing);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

QString IWingPluginCalls::currentDocFilename() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::currentDocFilename);
    QString ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

bool IWingPluginCalls::isReadOnly() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::isReadOnly);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

bool IWingPluginCalls::isInsertionMode() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::isInsertionMode);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

bool IWingPluginCalls::isKeepSize() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::isKeepSize);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

bool IWingPluginCalls::isLocked() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::isLocked);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

qsizetype IWingPluginCalls::documentLines() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::documentLines);
    qsizetype ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

qsizetype IWingPluginCalls::documentBytes() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::documentBytes);
    qsizetype ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

qsizetype IWingPluginCalls::currentRow() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::currentRow);
    qsizetype ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

qsizetype IWingPluginCalls::currentColumn() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::currentColumn);
    qsizetype ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

qsizetype IWingPluginCalls::currentOffset() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::currentOffset);
    qsizetype ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

qsizetype IWingPluginCalls::selectedLength() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::selectedLength);
    qsizetype ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

QByteArray IWingPluginCalls::selectedBytes(qsizetype index) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::selectedBytes);
    QByteArray ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(index));
    return ret;
}

QByteArrayList IWingPluginCalls::selectionBytes() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::selectionBytes);
    QByteArrayList ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

qsizetype IWingPluginCalls::selectionLength(qsizetype index) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::selectionLength);
    qsizetype ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(index));
    return ret;
}

qsizetype IWingPluginCalls::selectionCount() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::selectionCount);
    qsizetype ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

bool IWingPluginCalls::stringVisible() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::stringVisible);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

bool IWingPluginCalls::addressVisible() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::addressVisible);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

bool IWingPluginCalls::headerVisible() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::headerVisible);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

quintptr IWingPluginCalls::addressBase() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::addressBase);
    quintptr ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

bool IWingPluginCalls::isModified() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::isModified);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

qint8 IWingPluginCalls::readInt8(qsizetype offset) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::readInt8);
    qint8 ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset));
    return ret;
}

qint16 IWingPluginCalls::readInt16(qsizetype offset) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::readInt16);
    qint16 ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset));
    return ret;
}

qint32 IWingPluginCalls::readInt32(qsizetype offset) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::readInt32);
    qint32 ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset));
    return ret;
}

qint64 IWingPluginCalls::readInt64(qsizetype offset) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::readInt64);
    qint64 ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset));
    return ret;
}

quint8 IWingPluginCalls::readUInt8(qsizetype offset) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::readUInt8);
    quint8 ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset));
    return ret;
}

quint16 IWingPluginCalls::readUInt16(qsizetype offset) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::readUInt16);
    quint16 ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset));
    return ret;
}

quint32 IWingPluginCalls::readUInt32(qsizetype offset) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::readUInt32);
    quint32 ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset));
    return ret;
}

quint64 IWingPluginCalls::readUInt64(qsizetype offset) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::readUInt64);
    quint64 ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset));
    return ret;
}

float IWingPluginCalls::readFloat(qsizetype offset) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::readFloat);
    float ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset));
    return ret;
}

double IWingPluginCalls::readDouble(qsizetype offset) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::readDouble);
    double ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset));
    return ret;
}

QString IWingPluginCalls::readString(qsizetype offset,
                                     const QString &encoding) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::readString);
    QString ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(encoding));
    return ret;
}

QByteArray IWingPluginCalls::readBytes(qsizetype offset, qsizetype count) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::readBytes);
    QByteArray ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(count));
    return ret;
}

qsizetype IWingPluginCalls::findNext(qsizetype begin, const QByteArray &ba) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::findNext);
    qsizetype ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(begin), WINGAPI_ARG(ba));
    return ret;
}

qsizetype IWingPluginCalls::findPrevious(qsizetype begin,
                                         const QByteArray &ba) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::findPrevious);
    qsizetype ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(begin), WINGAPI_ARG(ba));
    return ret;
}

QString IWingPluginCalls::bookMarkComment(qsizetype pos) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::bookMarkComment);
    QString ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(pos));
    return ret;
}

bool IWingPluginCalls::existBookMark(qsizetype pos) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::existBookMark);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(pos));
    return ret;
}

bool IWingPluginCalls::switchDocument(int handle) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::switchDocument);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(handle));
    return ret;
}

bool IWingPluginCalls::raiseDocument(int handle) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::raiseDocument);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(handle));
    return ret;
}

bool IWingPluginCalls::setLockedFile(bool b) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::setLockedFile);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(b));
    return ret;
}

bool IWingPluginCalls::setKeepSize(bool b) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::setKeepSize);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(b));
    return ret;
}

bool IWingPluginCalls::setStringVisible(bool b) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::setStringVisible);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(b));
    return ret;
}

bool IWingPluginCalls::setAddressVisible(bool b) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::setAddressVisible);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(b));
    return ret;
}

bool IWingPluginCalls::setHeaderVisible(bool b) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::setHeaderVisible);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(b));
    return ret;
}

bool IWingPluginCalls::setAddressBase(quintptr base) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::setAddressBase);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(base));
    return ret;
}

bool IWingPluginCalls::beginMarco(const QString &txt) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::beginMarco);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(txt));
    return ret;
}

bool IWingPluginCalls::endMarco() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::endMarco);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

bool IWingPluginCalls::writeInt8(qsizetype offset, qint8 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::writeInt8);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::writeInt16(qsizetype offset, qint16 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::writeInt16);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::writeInt32(qsizetype offset, qint32 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::writeInt32);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::writeInt64(qsizetype offset, qint64 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::writeInt64);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::writeUInt8(qsizetype offset, quint8 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::writeUInt8);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::writeUInt16(qsizetype offset, quint16 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::writeInt16);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::writeUInt32(qsizetype offset, quint32 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::writeUInt32);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::writeUInt64(qsizetype offset, quint64 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::writeUInt64);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::writeFloat(qsizetype offset, float value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::writeFloat);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::writeDouble(qsizetype offset, double value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::writeDouble);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::writeString(qsizetype offset, const QString &value,
                                   const QString &encoding) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::writeString);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value),
             WINGAPI_ARG(encoding));
    return ret;
}

bool IWingPluginCalls::writeBytes(qsizetype offset, const QByteArray &data) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::writeBytes);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(data));
    return ret;
}

bool IWingPluginCalls::insertInt8(qsizetype offset, qint8 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::insertInt8);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::insertInt16(qsizetype offset, qint16 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::insertInt16);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::insertInt32(qsizetype offset, qint32 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::insertInt32);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::insertInt64(qsizetype offset, qint64 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::insertInt64);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::insertUInt8(qsizetype offset, quint8 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::insertUInt8);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::insertUInt16(qsizetype offset, quint16 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::insertUInt16);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::insertUInt32(qsizetype offset, quint32 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::insertUInt32);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::insertUInt64(qsizetype offset, quint64 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::insertUInt64);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::insertFloat(qsizetype offset, float value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::insertFloat);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::insertDouble(qsizetype offset, double value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::insertDouble);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::insertString(qsizetype offset, const QString &value,
                                    const QString &encoding) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::insertString);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(value),
             WINGAPI_ARG(encoding));
    return ret;
}

bool IWingPluginCalls::insertBytes(qsizetype offset, const QByteArray &data) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::insertBytes);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(data));
    return ret;
}

bool IWingPluginCalls::appendInt8(qint8 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::appendInt8);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::appendInt16(qint16 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::appendInt16);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::appendInt32(qint32 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::appendInt32);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::appendInt64(qint64 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::appendInt64);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::appendUInt8(quint8 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::appendUInt8);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::appendUInt16(quint16 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::appendUInt16);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::appendUInt32(quint32 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::appendUInt32);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::appendUInt64(quint64 value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::appendUInt64);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::appendFloat(float value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::appendFloat);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::appendDouble(double value) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::appendDouble);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(value));
    return ret;
}

bool IWingPluginCalls::appendString(const QString &value,
                                    const QString &encoding) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::appendString);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(value), WINGAPI_ARG(encoding));
    return ret;
}

bool IWingPluginCalls::appendBytes(const QByteArray &data) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::appendBytes);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(data));
    return ret;
}

bool IWingPluginCalls::removeBytes(qsizetype offset, qsizetype len) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::removeBytes);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(len));
    return ret;
}

bool IWingPluginCalls::moveTo(qsizetype line, qsizetype column, int nibbleindex,
                              bool clearSelection) {
    SETUP_CALL_CONTEXT((QOverload<qsizetype, qsizetype, int, bool>::of(
        &IWingPluginCalls::moveTo)));
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(line), WINGAPI_ARG(column),
             WINGAPI_ARG(nibbleindex), WINGAPI_ARG(clearSelection));
    return ret;
}

bool IWingPluginCalls::moveTo(qsizetype offset, bool clearSelection) {
    SETUP_CALL_CONTEXT(
        (QOverload<qsizetype, bool>::of(&IWingPluginCalls::moveTo)));
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(clearSelection));
    return ret;
}

bool IWingPluginCalls::select(qsizetype offset, qsizetype length,
                              SelectionMode mode) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::select);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset), WINGAPI_ARG(length),
             WINGAPI_ARG(mode));
    return ret;
}

bool IWingPluginCalls::setInsertionMode(bool isinsert) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::setInsertionMode);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(isinsert));
    return ret;
}

bool IWingPluginCalls::foreground(qsizetype begin, qsizetype length,
                                  const QColor &fgcolor) {
    return metadata(begin, length, fgcolor, {}, {});
}

bool IWingPluginCalls::background(qsizetype begin, qsizetype length,
                                  const QColor &bgcolor) {
    return metadata(begin, length, {}, bgcolor, {});
}

bool IWingPluginCalls::comment(qsizetype begin, qsizetype length,
                               const QString &comment) {
    return metadata(begin, length, {}, {}, comment);
}

bool IWingPluginCalls::metadata(qsizetype begin, qsizetype length,
                                const QColor &fgcolor, const QColor &bgcolor,
                                const QString &comment) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::metadata);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(begin), WINGAPI_ARG(length),
             WINGAPI_ARG(fgcolor), WINGAPI_ARG(bgcolor), WINGAPI_ARG(comment));
    return ret;
}

bool IWingPluginCalls::removeMetadata(qsizetype offset) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::removeMetadata);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(offset));
    return ret;
}

bool IWingPluginCalls::clearMetadata() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::clearMetadata);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

bool IWingPluginCalls::setMetaVisible(bool b) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::setMetaVisible);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(b));
    return ret;
}

bool IWingPluginCalls::setMetafgVisible(bool b) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::setMetafgVisible);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(b));
    return ret;
}

bool IWingPluginCalls::setMetabgVisible(bool b) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::setMetabgVisible);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(b));
    return ret;
}

bool IWingPluginCalls::setMetaCommentVisible(bool b) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::setMetaCommentVisible);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(b));
    return ret;
}

bool IWingPluginCalls::addBookMark(qsizetype pos, const QString &comment) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::addBookMark);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(pos), WINGAPI_ARG(comment));
    return ret;
}

bool IWingPluginCalls::modBookMark(qsizetype pos, const QString &comment) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::modBookMark);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(pos), WINGAPI_ARG(comment));
    return ret;
}

bool IWingPluginCalls::removeBookMark(qsizetype pos) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::removeBookMark);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(pos));
    return ret;
}

bool IWingPluginCalls::clearBookMark() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::clearBookMark);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

bool IWingPluginCalls::closeAllFiles() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::closeAllFiles);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

bool IWingPluginCalls::dataVisualText(const QString &data,
                                      const QString &title) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::dataVisualText);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(data), WINGAPI_ARG(title));
    return ret;
}

bool IWingPluginCalls::dataVisualTextList(const QStringList &data,
                                          const QString &title,
                                          ClickedCallBack clicked,
                                          ClickedCallBack dblClicked) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::dataVisualTextList);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(data), WINGAPI_ARG(title),
             WINGAPI_ARG(clicked), WINGAPI_ARG(dblClicked));
    return ret;
}

bool IWingPluginCalls::dataVisualTextTree(const QString &json,
                                          const QString &title,
                                          ClickedCallBack clicked,
                                          ClickedCallBack dblClicked) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::dataVisualTextTree);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(json), WINGAPI_ARG(title),
             WINGAPI_ARG(clicked), WINGAPI_ARG(dblClicked));
    return ret;
}

bool IWingPluginCalls::dataVisualTextTable(const QString &json,
                                           const QStringList &headers,
                                           const QStringList &headerNames,
                                           const QString &title,
                                           ClickedCallBack clicked,
                                           ClickedCallBack dblClicked) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::dataVisualTextTable);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(json), WINGAPI_ARG(headers),
             WINGAPI_ARG(headerNames), WINGAPI_ARG(title), WINGAPI_ARG(clicked),
             WINGAPI_ARG(dblClicked));
    return ret;
}

bool IWingPluginCalls::dataVisualTextListByModel(QAbstractItemModel *model,
                                                 const QString &title,
                                                 ClickedCallBack clicked,
                                                 ClickedCallBack dblClicked) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::dataVisualTextListByModel);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(model), WINGAPI_ARG(title),
             WINGAPI_ARG(clicked), WINGAPI_ARG(dblClicked));
    return ret;
}

bool IWingPluginCalls::dataVisualTextTableByModel(QAbstractItemModel *model,
                                                  const QString &title,
                                                  ClickedCallBack clicked,
                                                  ClickedCallBack dblClicked) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::dataVisualTextTableByModel);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(model), WINGAPI_ARG(title),
             WINGAPI_ARG(clicked), WINGAPI_ARG(dblClicked));
    return ret;
}

bool IWingPluginCalls::dataVisualTextTreeByModel(QAbstractItemModel *model,
                                                 const QString &title,
                                                 ClickedCallBack clicked,
                                                 ClickedCallBack dblClicked) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::dataVisualTextTreeByModel);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(model), WINGAPI_ARG(title),
             WINGAPI_ARG(clicked), WINGAPI_ARG(dblClicked));
    return ret;
}

ErrFile IWingPluginCalls::openWorkSpace(const QString &filename) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::openWorkSpace);
    ErrFile ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(filename));
    return ret;
}

ErrFile IWingPluginCalls::saveAsCurrent(const QString &savename) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::saveAsCurrent);
    ErrFile ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(savename));
    return ret;
}

ErrFile IWingPluginCalls::exportCurrent(const QString &savename) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::exportCurrent);
    ErrFile ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(savename));
    return ret;
}

ErrFile IWingPluginCalls::saveCurrent() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::saveCurrent);
    ErrFile ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

ErrFile IWingPluginCalls::closeCurrent(bool force) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::closeCurrent);
    ErrFile ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(force));
    return ret;
}

ErrFile IWingPluginCalls::openCurrent() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::openCurrent);
    ErrFile ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

ErrFile IWingPluginCalls::saveAsFile(int handle, const QString &savename) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::saveAsFile);
    ErrFile ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(handle), WINGAPI_ARG(savename));
    return ret;
}

ErrFile IWingPluginCalls::exportFile(int handle, const QString &savename) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::exportFile);
    ErrFile ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(handle), WINGAPI_ARG(savename));
    return ret;
}

ErrFile IWingPluginCalls::saveFile(int handle) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::saveFile);
    ErrFile ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(handle));
    return ret;
}

ErrFile IWingPluginCalls::closeFile(int handle, bool force) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::closeFile);
    ErrFile ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(handle), WINGAPI_ARG(force));
    return ret;
}

ErrFile IWingPluginCalls::closeHandle(int handle) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::closeHandle);
    ErrFile ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(handle));
    return ret;
}

ErrFile IWingPluginCalls::openExtFile(const QString &ext, const QString &file) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::openExtFile);
    ErrFile ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(ext), WINGAPI_ARG(file));
    return ret;
}

ErrFile IWingPluginCalls::openFile(const QString &filename) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::openFile);
    ErrFile ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(filename));
    return ret;
}

ErrFile IWingPluginCalls::newFile() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::newFile);
    ErrFile ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

HexPosition IWingPluginCalls::selectionEnd(qsizetype index) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::selectionEnd);
    HexPosition ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(index));
    return ret;
}

HexPosition IWingPluginCalls::selectionStart(qsizetype index) {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::selectionStart);
    HexPosition ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(index));
    return ret;
}

HexPosition IWingPluginCalls::currentPos() {
    SETUP_CALL_CONTEXT(&IWingPluginCalls::currentPos);
    HexPosition ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj());
    return ret;
}

IWingPluginAPICalls::IWingPluginAPICalls(IWingPlugin *const caller)
    : IWingPluginCalls(caller), IWingPluginBaseCalls(caller) {}
