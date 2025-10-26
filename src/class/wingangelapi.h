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

#ifndef WINGANGELAPI_H
#define WINGANGELAPI_H

#include "AngelScript/sdk/add_on/scriptarray/scriptarray.h"
#include "WingPlugin/iwingplugin.h"

#include <QFileDialog>

#include <functional>

class asIScriptEngine;
class ScriptMachine;
class ScriptingConsole;

namespace WingScriptInternal {

struct ScriptFnInfo {
    uint ret; // MetaType
    QVector<QPair<uint, QString>> params;
    std::function<QVariant(const QVariantList &)> fn;
};

} // namespace WingScriptInternal

class WingAngelAPI : public WingHex::IWingPlugin {
    Q_OBJECT
    Q_INTERFACES(WingHex::IWingPlugin)

    friend class WingAngel;

public:
    WingAngelAPI();
    virtual ~WingAngelAPI();

    // IWingPlugin interface
public:
    virtual bool init(const std::unique_ptr<QSettings> &set) override;
    virtual void unload(std::unique_ptr<QSettings> &set) override;
    virtual const QString pluginName() const override;
    virtual const QString pluginComment() const override;

    virtual RegisteredEvents registeredEvents() const override;

    virtual QString retranslate(const QString &str) override;

private:
    virtual void onRegisterScriptObj(WingHex::IWingAngel *o) override;

    virtual void eventPluginFile(PluginFileEvent e, FileType type,
                                 const QString &newfileName, int handle,
                                 const QString &oldfileName) override;

public:
    void installAPI(ScriptMachine *machine);
    void installBasicTypes(asIScriptEngine *engine);

    static QString qvariantCastASString(const QMetaType::Type &id);

    static int qvariantCastASID(asIScriptEngine *engine,
                                const QMetaType::Type &id);

    static bool isTempBuffered(QMetaType::Type type);

    template <typename T>
    static void assignTmpBuffer(asQWORD &buffer, const T &v) {
        static_assert(std::is_pod<T>());
        static_assert(sizeof(T) <= sizeof(asQWORD));
        *reinterpret_cast<T *>(&buffer) = v;
    }

public:
    static QString type2AngelScriptString(uint type, bool isArg,
                                          bool noModifier);

private:
    void installLogAPI(asIScriptEngine *engine);
    void installExtAPI(asIScriptEngine *engine);
    void installMsgboxAPI(asIScriptEngine *engine);
    void installInputboxAPI(asIScriptEngine *engine);
    void installFileDialogAPI(asIScriptEngine *engine);
    void installColorDialogAPI(asIScriptEngine *engine);
    void installHexBaseType(asIScriptEngine *engine);
    void installHexReaderAPI(asIScriptEngine *engine);
    void installHexControllerAPI(asIScriptEngine *engine);
    void installInvokeServiceAPI(asIScriptEngine *engine);

private:
    void registerAPI(asIScriptEngine *engine, const asSFuncPtr &fn,
                     const char *sig);

private:
    QStringList cArray2QStringList(const CScriptArray &array, int stringID,
                                   bool *ok = nullptr);
    QByteArray cArray2ByteArray(const CScriptArray &array, int byteID,
                                bool *ok = nullptr);

    qsizetype getAsTypeSize(int typeId, void *data);

    template <typename T>
    static T qvariantCastGetValue(void *buffer) {
        static_assert(std::is_pod<T>());
        static_assert(sizeof(T) <= sizeof(asQWORD));
        return *reinterpret_cast<T *>(buffer);
    }

    static void
    qvariantCastOp(asIScriptEngine *engine, const QVariant &var,
                   const std::function<void(void *, QMetaType::Type)> &fn);

    // flag = true: QList or QHash, otherwise QVector or QMap
    static QVariant qvariantGet(asIScriptEngine *engine, const void *raw,
                                int typeID, bool flag);

    static bool getQVariantGetFlag(const WingScriptInternal::ScriptFnInfo &info,
                                   int index);

    template <typename T>
    static const T *getDereferencePointer(const void *value, bool isHandle) {
        if (isHandle) {
            return *reinterpret_cast<const T *const *>(value);
        } else {
            return reinterpret_cast<const T *>(value);
        }
    }

    static void script_call(asIScriptGeneric *gen);

    static void script_unsafe_call(asIScriptGeneric *gen);

private:
    static QString QModelIndex_dataString(const QModelIndex &idx);
    static QString QModelIndex_dataChar(const QModelIndex &idx);
    static uint QModelIndex_dataUInt(const QModelIndex &idx);
    static int QModelIndex_dataInt(const QModelIndex &idx);
    static qlonglong QModelIndex_dataLongLong(const QModelIndex &idx);
    static qlonglong QModelIndex_dataULongLong(const QModelIndex &idx);
    static float QModelIndex_dataFloat(const QModelIndex &idx);
    static double QModelIndex_dataDouble(const QModelIndex &idx);

private:
    // for EXEC_BASE marco
    WING_SERVICE bool execScriptCode(const WingHex::SenderInfo &sender,
                                     const QString &code);
    WING_SERVICE bool execScript(const WingHex::SenderInfo &sender,
                                 const QString &fileName);
    WING_SERVICE bool execCode(const WingHex::SenderInfo &sender,
                               const QString &code);

    // =========================================================
    // Some neccessary services for AngelScript's array and dictionary.
    WING_SERVICE QVector<void *>
    retriveAsCArray(const WingHex::SenderInfo &sender, void *array);
    WING_SERVICE QHash<QString, QPair<QString, const void *>>
    retriveAsDictionary(const WingHex::SenderInfo &sender, void *dic);

    WING_SERVICE void *vector2AsArray(const WingHex::SenderInfo &sender,
                                      WingHex::MetaType type,
                                      const QVector<void *> &content);
    WING_SERVICE void *list2AsArray(const WingHex::SenderInfo &sender,
                                    WingHex::MetaType type,
                                    const QList<void *> &content);

    WING_SERVICE void *newAsDictionary(
        const WingHex::SenderInfo &sender,
        const QHash<QString, QPair<WingHex::MetaType, void *>> &content);

    // =========================================================

    void cleanUpHandles(const QVector<int> &handles);

private:
    enum InvokeInternalType { RetWithParam, RetOnly, ParamOnly };

    static const QtPrivate::QMetaTypeInterface *
    _invoke_type_helper(int flags, int strTypeID) {
        constexpr auto idMask = (asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR);
        auto type = flags & idMask;
        switch (type) {
        case asTYPEID_BOOL:
            return QtPrivate::qMetaTypeInterfaceForType<bool>();
        case asTYPEID_INT8:
            return QtPrivate::qMetaTypeInterfaceForType<qint8>();
        case asTYPEID_INT16:
            return QtPrivate::qMetaTypeInterfaceForType<qint16>();
        case asTYPEID_INT32:
            return QtPrivate::qMetaTypeInterfaceForType<qint32>();
        case asTYPEID_INT64:
            return QtPrivate::qMetaTypeInterfaceForType<qint64>();
        case asTYPEID_UINT8:
            return QtPrivate::qMetaTypeInterfaceForType<quint8>();
        case asTYPEID_UINT16:
            return QtPrivate::qMetaTypeInterfaceForType<quint16>();
        case asTYPEID_UINT32:
            return QtPrivate::qMetaTypeInterfaceForType<quint32>();
        case asTYPEID_UINT64:
            return QtPrivate::qMetaTypeInterfaceForType<quint64>();
        case asTYPEID_FLOAT:
            return QtPrivate::qMetaTypeInterfaceForType<float>();
        case asTYPEID_DOUBLE:
            return QtPrivate::qMetaTypeInterfaceForType<double>();
        default:
            if (type == strTypeID) {
                return QtPrivate::qMetaTypeInterfaceForType<QString>();
            }
        }
        return nullptr;
    }

    template <InvokeInternalType T>
    static bool _invoke_internal(int strTypeID,
                                 const QMetaMethodReturnArgument &r,
                                 asIScriptGeneric *generic, const QString &puid,
                                 const char *method) {
        auto total = generic->GetArgCount();
        std::vector<const char *> names;
        std::vector<const void *> data;
        std::vector<const QtPrivate::QMetaTypeInterface *> metas;

        if constexpr (T == RetWithParam || T == ParamOnly) {
            auto paramStart = (T == ParamOnly ? 2 : 3);
            auto tCount = total - paramStart + 1;
            names.resize(tCount, nullptr);
            data.resize(tCount, nullptr);
            metas.resize(tCount, nullptr);

            names[0] = r.name;
            data[0] = r.data;
            metas[0] = r.metaType;

            for (int i = paramStart; i < total; ++i) {
                auto idx = i + 1 - paramStart;
                auto addr = generic->GetArgAddress(i);
                data[idx] = addr;
                auto id = generic->GetArgTypeId(i);
                auto idint = _invoke_type_helper(id, strTypeID);
                if (!idint) {
                    // throw exception
                    auto ctx = asGetActiveContext();
                    if (ctx) {
                        ctx->SetException(
                            "[InvokeService] Only primitive types and "
                            "string are supported");
                    }
                    generic->SetReturnByte(false);
                    return false;
                }
                metas[idx] = idint;
            }
        } else if constexpr (T == RetOnly) {
            names.push_back(r.name);
            data.push_back(r.data);
            metas.push_back(r.metaType);
        }

        auto ret = invoke_service(puid, method, names, data, metas);
        generic->SetReturnByte(ret);
        return ret;
    }

    template <typename T>
    static void _invoke_helper(int strTypeID, void *ptr,
                               asIScriptGeneric *generic, const QString &puid,
                               const char *method) {
        T buffer{};
        auto r = qReturnArg(buffer);
        if (_invoke_internal<RetWithParam>(strTypeID, r, generic, puid,
                                           method)) {
            *static_cast<T *>(ptr) = buffer;
        }
    }

    template <InvokeInternalType T>
    static void _invoke_helper(int strTypeID, asIScriptGeneric *generic,
                               const QString &puid, const char *method) {
        QMetaMethodReturnArgument r{};
        _invoke_internal<T>(strTypeID, r, generic, puid, method);
    }

    static void _invoke_service(asIScriptGeneric *generic);
    static void _invoke_service_p_r(asIScriptGeneric *generic);
    static void _invoke_service_p_p(asIScriptGeneric *generic);

    template <InvokeInternalType T>
    static void _invoke_service_p(asIScriptGeneric *generic) {
        if (generic == nullptr) {
            return;
        }
        // we only support primitive type and string type with AngelScript
        constexpr auto idMask = (asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR);
        auto strTypeID = generic->GetArgTypeId(0) & idMask;

        const QString *puid =
            static_cast<const QString *>(generic->GetArgObject(0));
        const QString *method =
            static_cast<const QString *>(generic->GetArgObject(1));
        if (!puid || !method) {
            // throw exception
            auto ctx = asGetActiveContext();
            if (ctx) {
                ctx->SetException("[InvokeService] null puid or method");
            }
            return;
        }

        if (puid->compare(QStringLiteral("WingAngelAPI"),
                          Qt::CaseInsensitive) == 0) {
            // throw exception
            auto ctx = asGetActiveContext();
            if (ctx) {
                ctx->SetException(
                    "[InvokeService] CANNOT CALL services of WingAngelAPI");
            }
            return;
        }

        auto method_b = method->toUtf8();
        auto method_param = method_b.data();

        _invoke_helper<T>(strTypeID, generic, *puid, method_param);
    }

    static bool invoke_service(
        const QString &puid, const char *method,
        const std::vector<const char *> &names,
        const std::vector<const void *> &data,
        const std::vector<const QtPrivate::QMetaTypeInterface *> &metas);

private:
    CScriptArray *_HexReader_selectedBytes(qsizetype index);

    CScriptArray *_HexReader_selectionBytes();

    CScriptArray *_HexReader_readBytes(qsizetype offset, qsizetype len);

    qsizetype _HexReader_findNext(qsizetype begin, const CScriptArray &ba);

    qsizetype _HexReader_findPrevious(qsizetype begin, const CScriptArray &ba);

    bool _HexController_writeBytes(qsizetype offset, const CScriptArray &ba);

    bool _HexController_insertBytes(qsizetype offset, const CScriptArray &ba);

    bool _HexController_appendBytes(const CScriptArray &ba);

    void _UI_Toast(const QString &message, const QString &icon);

    QColor _Color_get(const QString &caption);

private:
    void _MSG_AboutQt(const QString &title);

    QMessageBox::StandardButton
    _MSG_Information(const QString &title, const QString &text, int buttons,
                     QMessageBox::StandardButton defaultButton);

    QMessageBox::StandardButton
    _MSG_Question(const QString &title, const QString &text, int buttons,
                  QMessageBox::StandardButton defaultButton);

    QMessageBox::StandardButton
    _MSG_Warning(const QString &title, const QString &text, int buttons,
                 QMessageBox::StandardButton defaultButton);

    QMessageBox::StandardButton
    _MSG_Critical(const QString &title, const QString &text, int buttons,
                  QMessageBox::StandardButton defaultButton);

    void _MSG_About(const QString &title, const QString &text);

    QMessageBox::StandardButton
    _MSG_msgbox(QMessageBox::Icon icon, const QString &title,
                const QString &text, int buttons,
                QMessageBox::StandardButton defaultButton);

private:
    QString _InputBox_GetText(const QString &title, const QString &label,
                              QLineEdit::EchoMode echo, const QString &text,
                              bool *ok, int inputMethodHints);

    QString _InputBox_GetMultiLineText(const QString &title,
                                       const QString &label,
                                       const QString &text, bool *ok,
                                       int inputMethodHints);

    QString _InputBox_getItem(const QString &title, const QString &label,
                              const CScriptArray &items, int current,
                              bool editable, bool *ok, int inputMethodHints);

    int _InputBox_GetInt(const QString &title, const QString &label, int value,
                         int minValue, int maxValue, int step, bool *ok);

    double _InputBox_GetDouble(const QString &title, const QString &label,
                               double value, double minValue, double maxValue,
                               int decimals, bool *ok, double step);

private:
    QString _FileDialog_GetExistingDirectory(const QString &caption,
                                             const QString &dir, int options);

    QString _FileDialog_GetOpenFileName(const QString &caption,
                                        const QString &dir,
                                        const QString &filter,
                                        QString *selectedFilter, int options);

    CScriptArray *_FileDialog_getOpenFileNames(const QString &caption,
                                               const QString &dir,
                                               const QString &filter,
                                               QString *selectedFilter,
                                               int options);

    QString _FileDialog_GetSaveFileName(const QString &caption,
                                        const QString &dir,
                                        const QString &filter,
                                        QString *selectedFilter, int options);

private:
    QVector<WingScriptInternal::ScriptFnInfo> _sfns;
    QVector<WingHex::UNSAFE_SCFNPTR> _usfns;

    QVector<int> _handles;
};

#endif // WINGANGELAPI_H
