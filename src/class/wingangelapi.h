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
#include "plugin/iwingplugin.h"

#include <any>
#include <functional>
#include <vector>

class asIScriptEngine;
class ScriptMachine;

class WingAngelAPI : public WingHex::IWingPlugin {
    Q_OBJECT

public:
    WingAngelAPI();
    virtual ~WingAngelAPI();

    // IWingPlugin interface
public:
    virtual int sdkVersion() const override;
    virtual const QString signature() const override;
    virtual bool init(const std::unique_ptr<QSettings> &set) override;
    virtual void unload(std::unique_ptr<QSettings> &set) override;
    virtual const QString pluginName() const override;
    virtual const QString pluginAuthor() const override;
    virtual uint pluginVersion() const override;
    virtual const QString pluginComment() const override;

    void
    registerScriptFns(const QString &ns,
                      const QHash<QString, IWingPlugin::ScriptFnInfo> &rfns);

    void installAPI(ScriptMachine *machine);

private:
    void installLogAPI(asIScriptEngine *engine);
    void installExtAPI(asIScriptEngine *engine);
    void installMsgboxAPI(asIScriptEngine *engine);
    void installInputboxAPI(asIScriptEngine *engine, int stringID);
    void installFileDialogAPI(asIScriptEngine *engine);
    void installColorDialogAPI(asIScriptEngine *engine);
    void installHexBaseType(asIScriptEngine *engine);
    void installHexReaderAPI(asIScriptEngine *engine);
    void installHexControllerAPI(asIScriptEngine *engine);
    void installDataVisualAPI(asIScriptEngine *engine, int stringID);
    void installScriptFns(asIScriptEngine *engine);

private:
    template <class T>
    void registerAPI(asIScriptEngine *engine, const std::function<T> &fn,
                     const char *sig) {
        _fnbuffer.push_back(fn);
        auto r = engine->RegisterGlobalFunction(
            sig, asMETHOD(std::function<T>, operator()),
            asCALL_THISCALL_ASGLOBAL,
            std::any_cast<std::function<T>>(&_fnbuffer.back()));
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
    }

    using WrapperFn = std::function<void(asIScriptGeneric *)>;

private:
    QStringList cArray2QStringList(const CScriptArray &array, int stringID,
                                   bool *ok = nullptr);
    QByteArray cArray2ByteArray(const CScriptArray &array, int byteID,
                                bool *ok = nullptr);

    bool read2Ref(qsizetype offset, void *ref, int typeId);

    bool write2Ref(qsizetype offset, void *ref, int typeId);

    bool insert2Ref(qsizetype offset, void *ref, int typeId);

    bool append2Ref(void *ref, int typeId);

    qsizetype getAsTypeSize(int typeId, void *data);

    template <typename T>
    static void assignTmpBuffer(asQWORD &buffer, const T &v) {
        static_assert(std::is_pod<T>());
        static_assert(sizeof(T) <= sizeof(asQWORD));
        *reinterpret_cast<T *>(&buffer) = v;
    }

    template <typename T>
    static T qvariantCastGetValue(void *buffer) {
        static_assert(std::is_pod<T>());
        static_assert(sizeof(T) <= sizeof(asQWORD));
        return *reinterpret_cast<T *>(buffer);
    }

    static void
    qvariantCastOp(asIScriptEngine *engine, const QVariant &var,
                   const std::function<void(void *, QMetaType::Type)> &fn);

    static QVariant qvariantGet(asIScriptEngine *engine, const void *raw,
                                int typeID);

    static int qvariantCastASID(asIScriptEngine *engine,
                                const QMetaType::Type &id);

    static bool isTempBuffered(QMetaType::Type type);

    void script_call(asIScriptEngine *engine, qsizetype id,
                     asIScriptGeneric *gen);

private:
    QString _InputBox_getItem(int stringID, const QString &title,
                              const QString &label, const CScriptArray &items,
                              int current, bool editable, bool *ok,
                              Qt::InputMethodHints inputMethodHints);

    CScriptArray *_FileDialog_getOpenFileNames(const QString &caption,
                                               const QString &dir,
                                               const QString &filter,
                                               QString *selectedFilter,
                                               QFileDialog::Options options);

    CScriptArray *_HexReader_selectedBytes(qsizetype index);

    CScriptArray *_HexReader_selectionBytes();

    CScriptArray *_HexReader_readBytes(qsizetype offset, qsizetype len);

    bool _HexReader_read(qsizetype offset, void *ref, int typeId);

    bool _HexReader_write(qsizetype offset, void *ref, int typeId);

    bool _HexReader_insert(qsizetype offset, void *ref, int typeId);

    bool _HexReader_append(void *ref, int typeId);

    qsizetype _HexReader_searchForward(qsizetype begin, const CScriptArray &ba);

    qsizetype _HexReader_searchBackward(qsizetype begin,
                                        const CScriptArray &ba);

    CScriptArray *_HexReader_findAllBytes(qsizetype begin, qsizetype end,
                                          const CScriptArray &ba);

    CScriptArray *_HexReader_getsBookmarkPos(qsizetype line);

    CScriptArray *_HexReader_getSupportedEncodings();

    CScriptArray *_HexReader_getStorageDrivers();

    bool _HexController_writeBytes(qsizetype offset, const CScriptArray &ba);

    bool _HexController_insertBytes(qsizetype offset, const CScriptArray &ba);

    bool _HexController_appendBytes(const CScriptArray &ba);

    bool _DataVisual_updateTextList(int stringID, const CScriptArray &data);

    bool _DataVisual_updateTextTable(int stringID, const QString &json,
                                     const CScriptArray &headers,
                                     const CScriptArray &headerNames);

private:
    std::vector<std::any> _fnbuffer;
    QVector<IWingPlugin::ScriptFnInfo> _sfns;
    QHash<asIScriptEngine *, std::vector<WrapperFn>> _sfn_wraps;

    QHash<QString, QHash<QString, qsizetype>> _rfns;
};

#endif // WINGANGELAPI_H
