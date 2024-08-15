#ifndef WINGANGELAPI_H
#define WINGANGELAPI_H

#include "AngelScript/add_on/scriptarray/scriptarray.h"
#include "plugin/iwingplugin.h"

class asIScriptEngine;

class WingAngelAPI : public WingHex::IWingPlugin {
    Q_OBJECT

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
    QStringList cArray2QStringList(const CScriptArray &array, int stringID,
                                   bool *ok = nullptr);
    QByteArray cArray2ByteArray(const CScriptArray &array, int byteID,
                                bool *ok = nullptr);

private:
    template <typename Func>
    CScriptArray *retarrayWrapperFunction(Func &&getRet,
                                          const char *angelType) {
        // context, which can be used to obtain a pointer to the
        // engine.
        asIScriptContext *ctx = asGetActiveContext();
        if (ctx) {
            asIScriptEngine *engine = ctx->GetEngine();

            auto ret = getRet();

            // The script array needs to know its type to properly handle the
            // elements. Note that the object type should be cached to avoid
            // performance issues if the function is called frequently.
            asITypeInfo *t = engine->GetTypeInfoByDecl(angelType);
            Q_ASSERT(t);

            auto array = CScriptArray::Create(t, ret.size());
            for (typename decltype(ret)::size_type i = 0; i < ret.size(); ++i) {
                auto v = ret.at(i);
                array->SetValue(i, &v);
            }
            return array;
        } else {
            return nullptr;
        }
    }

    template <typename Func>
    CScriptArray *byteArrayWrapperFunction(Func &&fn) {
        return retarrayWrapperFunction(fn, "array<byte>");
    }

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

    bool _HexReader_read(void *ref, int typeId);

    qsizetype _HexReader_searchForward(qsizetype begin, const CScriptArray &ba);

    qsizetype _HexReader_searchBackward(qsizetype begin,
                                        const CScriptArray &ba);

    CScriptArray *_HexReader_findAllBytes(qsizetype begin, qsizetype end,
                                          const CScriptArray &ba);

    CScriptArray *_HexReader_getMetadatas(qsizetype offset);

    CScriptArray *_HexReader_getMetaLine(qsizetype line);

    CScriptArray *_HexReader_getsBookmarkPos(qsizetype line);

    CScriptArray *_HexReader_getBookMarks();

    CScriptArray *_HexReader_getOpenFiles();

    CScriptArray *_HexReader_getSupportedEncodings();

    bool _HexController_writeBytes(qsizetype offset, const CScriptArray &ba);

    bool _HexController_insertBytes(qsizetype offset, const CScriptArray &ba);

    bool _HexController_appendBytes(const CScriptArray &ba);

    bool _DataVisual_updateTextList(int stringID, const CScriptArray &data);

    bool _DataVisual_updateTextTable(int stringID, const QString &json,
                                     const CScriptArray &headers,
                                     const CScriptArray &headerNames);
};

#endif // WINGANGELAPI_H
