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

class WingAngelAPI : public WingHex::IWingPlugin {
    Q_OBJECT
    Q_INTERFACES(WingHex::IWingPlugin)

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

    virtual QStringList registerScriptMarcos() const override;

private:
    virtual void eventPluginFile(PluginFileEvent e, FileType type,
                                 const QString &newfileName, int handle,
                                 const QString &oldfileName) override;

public:
    void
    registerScriptFns(const QString &ns,
                      const QHash<QString, IWingPlugin::ScriptFnInfo> &rfns);
    void registerUnSafeScriptFns(
        const QString &ns,
        const QHash<QString, IWingPlugin::UNSAFE_SCFNPTR> &rfns);

    void
    registerScriptEnums(const QString &ns,
                        const QHash<QString, QList<QPair<QString, int>>> &objs);

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
    void installScriptFns(asIScriptEngine *engine);
    void installScriptUnSafeFns(asIScriptEngine *engine);
    void installScriptEnums(asIScriptEngine *engine);

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

    static bool
    getQVariantGetFlag(const WingHex::IWingPlugin::ScriptFnInfo &info,
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
                                      MetaType type,
                                      const QVector<void *> &content);
    WING_SERVICE void *list2AsArray(const WingHex::SenderInfo &sender,
                                    MetaType type,
                                    const QList<void *> &content);
    WING_SERVICE void deleteAsArray(const WingHex::SenderInfo &sender,
                                    void *array);

    WING_SERVICE void *
    newAsDictionary(const WingHex::SenderInfo &sender,
                    const QHash<QString, QPair<MetaType, void *>> &content);
    WING_SERVICE void deleteAsDictionary(const WingHex::SenderInfo &sender,
                                         void *dic);

    // =========================================================

    void cleanUpHandles(const QVector<int> &handles);

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
    _MSG_Information(const QString &title, const QString &text,
                     QMessageBox::StandardButtons buttons,
                     QMessageBox::StandardButton defaultButton);

    QMessageBox::StandardButton
    _MSG_Question(const QString &title, const QString &text,
                  QMessageBox::StandardButtons buttons,
                  QMessageBox::StandardButton defaultButton);

    QMessageBox::StandardButton
    _MSG_Warning(const QString &title, const QString &text,
                 QMessageBox::StandardButtons buttons,
                 QMessageBox::StandardButton defaultButton);

    QMessageBox::StandardButton
    _MSG_Critical(const QString &title, const QString &text,
                  QMessageBox::StandardButtons buttons,
                  QMessageBox::StandardButton defaultButton);

    void _MSG_About(const QString &title, const QString &text);

    QMessageBox::StandardButton
    _MSG_msgbox(QMessageBox::Icon icon, const QString &title,
                const QString &text, QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton);

private:
    QString _InputBox_GetText(const QString &title, const QString &label,
                              QLineEdit::EchoMode echo, const QString &text,
                              bool *ok, Qt::InputMethodHints inputMethodHints);

    QString _InputBox_GetMultiLineText(const QString &title,
                                       const QString &label,
                                       const QString &text, bool *ok,
                                       Qt::InputMethodHints inputMethodHints);

    QString _InputBox_getItem(const QString &title, const QString &label,
                              const CScriptArray &items, int current,
                              bool editable, bool *ok,
                              Qt::InputMethodHints inputMethodHints);

    int _InputBox_GetInt(const QString &title, const QString &label, int value,
                         int minValue, int maxValue, int step, bool *ok);

    double _InputBox_GetDouble(const QString &title, const QString &label,
                               double value, double minValue, double maxValue,
                               int decimals, bool *ok, double step);

private:
    QString _FileDialog_GetExistingDirectory(const QString &caption,
                                             const QString &dir,
                                             QFileDialog::Options options);

    QString _FileDialog_GetOpenFileName(const QString &caption,
                                        const QString &dir,
                                        const QString &filter,
                                        QString *selectedFilter,
                                        QFileDialog::Options options);

    CScriptArray *_FileDialog_getOpenFileNames(const QString &caption,
                                               const QString &dir,
                                               const QString &filter,
                                               QString *selectedFilter,
                                               QFileDialog::Options options);

    QString _FileDialog_GetSaveFileName(const QString &caption,
                                        const QString &dir,
                                        const QString &filter,
                                        QString *selectedFilter,
                                        QFileDialog::Options options);

private:
    QVector<IWingPlugin::ScriptFnInfo> _sfns;
    QHash<QString, QHash<QString, qsizetype>> _rfns;

    QVector<IWingPlugin::UNSAFE_SCFNPTR> _usfns;
    QHash<QString, QHash<QString, qsizetype>> _urfns;

    QHash<QString, QHash<QString, QList<QPair<QString, int>>>> _objs;

    QVector<int> _handles;
};

#endif // WINGANGELAPI_H
