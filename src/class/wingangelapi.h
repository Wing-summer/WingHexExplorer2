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

public slots:
    virtual void plugin2MessagePipe(WingHex::WingPluginMessage type,
                                    const QVariantList &msg) override;

private:
    void installLogAPI(asIScriptEngine *engine);
    void installExtAPI(asIScriptEngine *engine);
    void installMsgboxAPI(asIScriptEngine *engine);
    void installInputboxAPI(asIScriptEngine *engine, int stringID);
    void installFileDialogAPI(asIScriptEngine *engine);
    void installColorDialogAPI(asIScriptEngine *engine);
    void installHexReaderAPI(asIScriptEngine *engine);
    void installHexControllerAPI(asIScriptEngine *engine);

private:
    QStringList cArray2QStringList(const CScriptArray &array, int stringID,
                                   bool *ok = nullptr);

private:
    QString _InputBox_getItem(int stringID, const QString &title,
                              const QString &label, const CScriptArray &items,
                              int current, bool editable, bool *ok,
                              Qt::InputMethodHints inputMethodHints);

    CScriptArray *_FileDialog_getOpenFileNames(asITypeInfo *stringArrayType,
                                               const QString &caption,
                                               const QString &dir,
                                               const QString &filter,
                                               QString *selectedFilter,
                                               QFileDialog::Options options);
};

#endif // WINGANGELAPI_H
