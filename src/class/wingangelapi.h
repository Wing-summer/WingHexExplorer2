#ifndef WINGANGELAPI_H
#define WINGANGELAPI_H

#include "AngelScript/sdk/add_on/scriptarray/scriptarray.h"
#include "plugin/iwingplugin.h"

#include <functional>

class asIScriptEngine;

class WingAngelAPI : public WingHex::IWingPlugin {
    Q_OBJECT

public:
    WingAngelAPI();
    virtual ~WingAngelAPI();

    // IWingPlugin interface
public:
    virtual int sdkVersion() const override;
    virtual const QString signature() const override;
    virtual bool
    init(const QList<WingHex::WingPluginInfo> &loadedplugin) override;
    virtual void unload() override;
    virtual const QString pluginName() const override;
    virtual const QString pluginAuthor() const override;
    virtual uint pluginVersion() const override;
    virtual const QString pluginComment() const override;

    void installAPI(asIScriptEngine *engine, asITypeInfo *stringType);

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

private:
    template <class T>
    void registerAPI(asIScriptEngine *engine, const std::function<T> &fn,
                     const char *sig) {
        auto *f = new std::function<T>(fn);
        auto r = engine->RegisterGlobalFunction(
            sig, asMETHOD(std::function<T>, operator()),
            asCALL_THISCALL_ASGLOBAL, f);
        _fnbuffer << f;
        Q_ASSERT(r >= 0);
    }

private:
    QStringList cArray2QStringList(const CScriptArray &array, int stringID,
                                   bool *ok = nullptr);
    QByteArray cArray2ByteArray(const CScriptArray &array, int byteID,
                                bool *ok = nullptr);

    bool read2Ref(qsizetype offset, void *ref, int typeId);

    qsizetype getAsTypeSize(int typeId, void *data);

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

    CScriptArray *_HexReader_selectedBytes();

    CScriptArray *_HexReader_readBytes(qsizetype offset, qsizetype len);

    bool _HexReader_read(qsizetype offset, void *ref, int typeId);

    bool _HexReader_write(qsizetype offset, void *ref, int typeId);

    bool _HexReader_insert(qsizetype offset, void *ref, int typeId);

    bool _HexReader_append(qsizetype offset, void *ref, int typeId);

    qsizetype _HexReader_searchForward(qsizetype begin, const CScriptArray &ba);

    qsizetype _HexReader_searchBackward(qsizetype begin,
                                        const CScriptArray &ba);

    CScriptArray *_HexReader_findAllBytes(qsizetype begin, qsizetype end,
                                          const CScriptArray &ba);

    CScriptArray *_HexReader_getMetadatas(qsizetype offset);

    CScriptArray *_HexReader_getMetaLine(qsizetype line);

    CScriptArray *_HexReader_getsBookmarkPos(qsizetype line);

    CScriptArray *_HexReader_getBookMarks();

    CScriptArray *_HexReader_getSupportedEncodings();

    bool _HexController_writeBytes(qsizetype offset, const CScriptArray &ba);

    bool _HexController_insertBytes(qsizetype offset, const CScriptArray &ba);

    bool _HexController_appendBytes(const CScriptArray &ba);

    bool _DataVisual_updateTextList(int stringID, const CScriptArray &data);

    bool _DataVisual_updateTextTable(int stringID, const QString &json,
                                     const CScriptArray &headers,
                                     const CScriptArray &headerNames);

private:
    QVector<void *> _fnbuffer;
};

#endif // WINGANGELAPI_H
