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

#ifndef IWINGPLUGINCALLS_H
#define IWINGPLUGINCALLS_H

#include "WingPlugin/iwingpluginbasecalls.h"

#include <QAbstractItemModel>

namespace WingHex {

struct WINGPLUGIN_EXPORT HexPosition {
    qsizetype line;
    int column;
    quint8 lineWidth;
    int nibbleindex;

    HexPosition();
    Q_REQUIRED_RESULT qsizetype offset() const;
    qsizetype operator-(const HexPosition &rhs) const;
    bool operator==(const HexPosition &rhs) const;
    bool operator!=(const HexPosition &rhs) const;
};

class IWingPlugin;

class WINGPLUGIN_EXPORT IWingPluginCalls : public WingPluginCalls {
public:
    explicit IWingPluginCalls(IWingPlugin *const caller);

public:
    bool existsServiceHost(const QString &puid);

    bool invokeService(const QString &puid, const char *method,
                       Qt::ConnectionType type, QGenericReturnArgument ret,
                       QGenericArgument val0 = QGenericArgument(nullptr),
                       QGenericArgument val1 = QGenericArgument(),
                       QGenericArgument val2 = QGenericArgument(),
                       QGenericArgument val3 = QGenericArgument(),
                       QGenericArgument val4 = QGenericArgument());

    bool invokeService(const QString &puid, const char *member,
                       QGenericReturnArgument ret,
                       QGenericArgument val0 = QGenericArgument(nullptr),
                       QGenericArgument val1 = QGenericArgument(),
                       QGenericArgument val2 = QGenericArgument(),
                       QGenericArgument val3 = QGenericArgument(),
                       QGenericArgument val4 = QGenericArgument());

    bool invokeService(const QString &puid, const char *member,
                       Qt::ConnectionType type, QGenericArgument val0,
                       QGenericArgument val1 = QGenericArgument(),
                       QGenericArgument val2 = QGenericArgument(),
                       QGenericArgument val3 = QGenericArgument(),
                       QGenericArgument val4 = QGenericArgument());

    bool invokeService(const QString &puid, const char *member,
                       QGenericArgument val0,
                       QGenericArgument val1 = QGenericArgument(),
                       QGenericArgument val2 = QGenericArgument(),
                       QGenericArgument val3 = QGenericArgument(),
                       QGenericArgument val4 = QGenericArgument());

public:
    Q_REQUIRED_RESULT bool isCurrentDocEditing();
    Q_REQUIRED_RESULT QString currentDocFilename();

    // document
    Q_REQUIRED_RESULT bool isReadOnly();
    Q_REQUIRED_RESULT bool isInsertionMode();
    Q_REQUIRED_RESULT bool isKeepSize();
    Q_REQUIRED_RESULT bool isLocked();
    Q_REQUIRED_RESULT qsizetype documentLines();
    Q_REQUIRED_RESULT qsizetype documentBytes();
    Q_REQUIRED_RESULT WingHex::HexPosition currentPos();
    Q_REQUIRED_RESULT qsizetype currentRow();
    Q_REQUIRED_RESULT qsizetype currentColumn();
    Q_REQUIRED_RESULT qsizetype currentOffset();
    Q_REQUIRED_RESULT qsizetype selectedLength();

    Q_REQUIRED_RESULT QByteArray selectedBytes(qsizetype index);
    Q_REQUIRED_RESULT QByteArrayList selectionBytes();

    Q_REQUIRED_RESULT WingHex::HexPosition selectionStart(qsizetype index);
    Q_REQUIRED_RESULT WingHex::HexPosition selectionEnd(qsizetype index);
    Q_REQUIRED_RESULT qsizetype selectionLength(qsizetype index);
    Q_REQUIRED_RESULT qsizetype selectionCount();

    Q_REQUIRED_RESULT bool stringVisible();
    Q_REQUIRED_RESULT bool addressVisible();
    Q_REQUIRED_RESULT bool headerVisible();
    Q_REQUIRED_RESULT quintptr addressBase();
    Q_REQUIRED_RESULT bool isModified();

    Q_REQUIRED_RESULT qint8 readInt8(qsizetype offset);
    Q_REQUIRED_RESULT qint16 readInt16(qsizetype offset);
    Q_REQUIRED_RESULT qint32 readInt32(qsizetype offset);
    Q_REQUIRED_RESULT qint64 readInt64(qsizetype offset);
    Q_REQUIRED_RESULT quint8 readUInt8(qsizetype offset);
    Q_REQUIRED_RESULT quint16 readUInt16(qsizetype offset);
    Q_REQUIRED_RESULT quint32 readUInt32(qsizetype offset);
    Q_REQUIRED_RESULT quint64 readUInt64(qsizetype offset);
    Q_REQUIRED_RESULT float readFloat(qsizetype offset);
    Q_REQUIRED_RESULT double readDouble(qsizetype offset);
    Q_REQUIRED_RESULT QString readString(qsizetype offset,
                                         const QString &encoding = {});
    Q_REQUIRED_RESULT QByteArray readBytes(qsizetype offset, qsizetype count);

    Q_REQUIRED_RESULT qsizetype findNext(qsizetype begin, const QByteArray &ba);
    Q_REQUIRED_RESULT qsizetype findPrevious(qsizetype begin,
                                             const QByteArray &ba);

    Q_REQUIRED_RESULT QString bookMarkComment(qsizetype pos);
    Q_REQUIRED_RESULT bool existBookMark(qsizetype pos);

public:
    // document
    Q_REQUIRED_RESULT bool switchDocument(int handle);
    Q_REQUIRED_RESULT bool raiseDocument(int handle);
    Q_REQUIRED_RESULT bool setLockedFile(bool b);
    Q_REQUIRED_RESULT bool setKeepSize(bool b);
    Q_REQUIRED_RESULT bool setStringVisible(bool b);
    Q_REQUIRED_RESULT bool setAddressVisible(bool b);
    Q_REQUIRED_RESULT bool setHeaderVisible(bool b);
    Q_REQUIRED_RESULT bool setAddressBase(quintptr base);

    Q_REQUIRED_RESULT bool beginMarco(const QString &txt = {});
    bool endMarco();

    Q_REQUIRED_RESULT bool writeInt8(qsizetype offset, qint8 value);
    Q_REQUIRED_RESULT bool writeInt16(qsizetype offset, qint16 value);
    Q_REQUIRED_RESULT bool writeInt32(qsizetype offset, qint32 value);
    Q_REQUIRED_RESULT bool writeInt64(qsizetype offset, qint64 value);
    Q_REQUIRED_RESULT bool writeUInt8(qsizetype offset, quint8 value);
    Q_REQUIRED_RESULT bool writeUInt16(qsizetype offset, quint16 value);
    Q_REQUIRED_RESULT bool writeUInt32(qsizetype offset, quint32 value);
    Q_REQUIRED_RESULT bool writeUInt64(qsizetype offset, quint64 value);
    Q_REQUIRED_RESULT bool writeFloat(qsizetype offset, float value);
    Q_REQUIRED_RESULT bool writeDouble(qsizetype offset, double value);
    Q_REQUIRED_RESULT bool writeString(qsizetype offset, const QString &value,
                                       const QString &encoding = QString());
    Q_REQUIRED_RESULT bool writeBytes(qsizetype offset, const QByteArray &data);

    Q_REQUIRED_RESULT bool insertInt8(qsizetype offset, qint8 value);
    Q_REQUIRED_RESULT bool insertInt16(qsizetype offset, qint16 value);
    Q_REQUIRED_RESULT bool insertInt32(qsizetype offset, qint32 value);
    Q_REQUIRED_RESULT bool insertInt64(qsizetype offset, qint64 value);
    Q_REQUIRED_RESULT bool insertUInt8(qsizetype offset, quint8 value);
    Q_REQUIRED_RESULT bool insertUInt16(qsizetype offset, quint16 value);
    Q_REQUIRED_RESULT bool insertUInt32(qsizetype offset, quint32 value);
    Q_REQUIRED_RESULT bool insertUInt64(qsizetype offset, quint64 value);
    Q_REQUIRED_RESULT bool insertFloat(qsizetype offset, float value);
    Q_REQUIRED_RESULT bool insertDouble(qsizetype offset, double value);
    Q_REQUIRED_RESULT bool insertString(qsizetype offset, const QString &value,
                                        const QString &encoding = QString());
    Q_REQUIRED_RESULT bool insertBytes(qsizetype offset,
                                       const QByteArray &data);

    Q_REQUIRED_RESULT bool appendInt8(qint8 value);
    Q_REQUIRED_RESULT bool appendInt16(qint16 value);
    Q_REQUIRED_RESULT bool appendInt32(qint32 value);
    Q_REQUIRED_RESULT bool appendInt64(qint64 value);
    Q_REQUIRED_RESULT bool appendUInt8(quint8 value);
    Q_REQUIRED_RESULT bool appendUInt16(quint16 value);
    Q_REQUIRED_RESULT bool appendUInt32(quint32 value);
    Q_REQUIRED_RESULT bool appendUInt64(quint64 value);
    Q_REQUIRED_RESULT bool appendFloat(float value);
    Q_REQUIRED_RESULT bool appendDouble(double value);
    Q_REQUIRED_RESULT bool appendString(const QString &value,
                                        const QString &encoding = QString());
    Q_REQUIRED_RESULT bool appendBytes(const QByteArray &data);

    Q_REQUIRED_RESULT bool removeBytes(qsizetype offset, qsizetype len);

    // cursor
    Q_REQUIRED_RESULT bool moveTo(qsizetype line, qsizetype column,
                                  int nibbleindex = 1,
                                  bool clearSelection = true);
    Q_REQUIRED_RESULT bool moveTo(qsizetype offset, bool clearSelection = true);
    Q_REQUIRED_RESULT bool select(qsizetype offset, qsizetype length,
                                  SelectionMode mode = SelectionMode::Add);
    Q_REQUIRED_RESULT bool setInsertionMode(bool isinsert);

    // metadata
    Q_REQUIRED_RESULT bool foreground(qsizetype begin, qsizetype length,
                                      const QColor &fgcolor);
    Q_REQUIRED_RESULT bool background(qsizetype begin, qsizetype length,
                                      const QColor &bgcolor);
    Q_REQUIRED_RESULT bool comment(qsizetype begin, qsizetype length,
                                   const QString &comment);

    Q_REQUIRED_RESULT bool metadata(qsizetype begin, qsizetype length,
                                    const QColor &fgcolor,
                                    const QColor &bgcolor,
                                    const QString &comment);

    Q_REQUIRED_RESULT bool removeMetadata(qsizetype offset);
    Q_REQUIRED_RESULT bool clearMetadata();
    Q_REQUIRED_RESULT bool setMetaVisible(bool b);
    Q_REQUIRED_RESULT bool setMetafgVisible(bool b);
    Q_REQUIRED_RESULT bool setMetabgVisible(bool b);
    Q_REQUIRED_RESULT bool setMetaCommentVisible(bool b);

    // mainwindow
    Q_REQUIRED_RESULT WingHex::ErrFile newFile();
    Q_REQUIRED_RESULT WingHex::ErrFile openFile(const QString &filename);
    Q_REQUIRED_RESULT WingHex::ErrFile openExtFile(const QString &ext,
                                                   const QString &file);

    WingHex::ErrFile closeHandle(int handle);
    WingHex::ErrFile closeFile(int handle, bool force = false);

    Q_REQUIRED_RESULT WingHex::ErrFile saveFile(int handle);
    Q_REQUIRED_RESULT WingHex::ErrFile exportFile(int handle,
                                                  const QString &savename);
    Q_REQUIRED_RESULT WingHex::ErrFile saveAsFile(int handle,
                                                  const QString &savename);

    Q_REQUIRED_RESULT WingHex::ErrFile openCurrent();
    Q_REQUIRED_RESULT WingHex::ErrFile closeCurrent(bool force = false);
    Q_REQUIRED_RESULT WingHex::ErrFile saveCurrent();
    Q_REQUIRED_RESULT WingHex::ErrFile exportCurrent(const QString &savename);
    Q_REQUIRED_RESULT WingHex::ErrFile saveAsCurrent(const QString &savename);

    // bookmark
    Q_REQUIRED_RESULT bool addBookMark(qsizetype pos, const QString &comment);
    Q_REQUIRED_RESULT bool modBookMark(qsizetype pos, const QString &comment);
    Q_REQUIRED_RESULT bool removeBookMark(qsizetype pos);
    Q_REQUIRED_RESULT bool clearBookMark();

    // workspace
    Q_REQUIRED_RESULT WingHex::ErrFile openWorkSpace(const QString &filename);

    // extension
    bool closeAllFiles();

public:
    bool dataVisualText(const QString &data, const QString &title = {});
    bool dataVisualTextList(const QStringList &data, const QString &title = {},
                            WingHex::ClickedCallBack clicked = {},
                            WingHex::ClickedCallBack dblClicked = {});

    Q_REQUIRED_RESULT bool
    dataVisualTextTree(const QString &json, const QString &title = {},
                       WingHex::ClickedCallBack clicked = {},
                       WingHex::ClickedCallBack dblClicked = {});
    Q_REQUIRED_RESULT bool
    dataVisualTextTable(const QString &json, const QStringList &headers,
                        const QStringList &headerNames = {},
                        const QString &title = {},
                        WingHex::ClickedCallBack clicked = {},
                        WingHex::ClickedCallBack dblClicked = {});

    // API for Qt Plugin Only
    Q_REQUIRED_RESULT bool
    dataVisualTextListByModel(QAbstractItemModel *model,
                              const QString &title = {},
                              WingHex::ClickedCallBack clicked = {},
                              WingHex::ClickedCallBack dblClicked = {});
    Q_REQUIRED_RESULT bool
    dataVisualTextTableByModel(QAbstractItemModel *model,
                               const QString &title = {},
                               WingHex::ClickedCallBack clicked = {},
                               WingHex::ClickedCallBack dblClicked = {});
    Q_REQUIRED_RESULT bool
    dataVisualTextTreeByModel(QAbstractItemModel *model,
                              const QString &title = {},
                              WingHex::ClickedCallBack clicked = {},
                              WingHex::ClickedCallBack dblClicked = {});
};

class WINGPLUGIN_EXPORT IWingPluginAPICalls : public IWingPluginCalls,
                                              public IWingPluginBaseCalls {
public:
    explicit IWingPluginAPICalls(IWingPlugin *const caller);
};

} // namespace WingHex

Q_DECLARE_METATYPE(const char *)
Q_DECLARE_METATYPE(QGenericArgument)
Q_DECLARE_METATYPE(QGenericReturnArgument)
Q_DECLARE_METATYPE(QModelIndex)
Q_DECLARE_METATYPE(WingHex::ClickedCallBack)
Q_DECLARE_METATYPE(WingHex::HexPosition)

#endif // IWINGPLUGINCALLS_H
