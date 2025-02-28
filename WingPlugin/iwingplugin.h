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

#ifndef IWINGPLUGIN_H
#define IWINGPLUGIN_H

#include "WingPlugin/iwingplugincalls.h"
#include "iwingpluginbase.h"
#include "wingeditorviewwidget.h"
#include "wingplugin_global.h"

#include <variant>

#include <QToolButton>

namespace WingHex {

class IWingPlugin;

struct WINGPLUGIN_EXPORT SenderInfo {
    QString plgcls;
    QString puid;
    QVariant meta;
};

struct WINGPLUGIN_EXPORT WingRibbonToolBoxInfo {
    struct WINGPLUGIN_EXPORT RibbonCatagories {
        const QString FILE = QStringLiteral("File");
        const QString EDIT = QStringLiteral("Edit");
        const QString VIEW = QStringLiteral("View");
        const QString SCRIPT = QStringLiteral("Script");
        const QString PLUGIN = QStringLiteral("Plugin");
        const QString SETTING = QStringLiteral("Setting");
        const QString ABOUT = QStringLiteral("About");
    };

    QString catagory;
    QString displayName;

    struct WINGPLUGIN_EXPORT Toolbox {
        QString name;
        QList<QToolButton *> tools;
    };
    QList<Toolbox> toolboxs;
};

struct WINGPLUGIN_EXPORT ScriptCallError {
    int errorCode;
    QString errmsg;
};

class SettingPage;

class WINGPLUGIN_EXPORT IWingPlugin : public IWingPluginBase,
                                      public IWingPluginCalls {
    Q_OBJECT
public:
    IWingPlugin();

public:
    using ScriptFn = std::function<QVariant(const QVariantList &)>;

    using UNSAFE_RET =
        std::variant<std::monostate, bool, quint8, quint16, quint32, quint64,
                     float, double, void *, ScriptCallError>;
    using UNSAFE_SCFNPTR = std::function<UNSAFE_RET(const QList<void *> &)>;

    enum MetaType : uint {
        Void,

        Bool,
        Int,
        Int32 = Int,
        UInt,
        UInt32 = UInt,
        Int8,
        UInt8,
        Int16,
        UInt16,
        Int64,
        UInt64,

        Float,
        Double,

        String,
        Char,
        Byte,
        Color,

        Map,  // QVariantMap -> dictionary
        Hash, // QVariantHash -> dictionary

        MetaMax, // reserved
        MetaTypeMask = 0xFFFFF,
        Array = 0x100000, // QVector<?> -> array<?>
        List = 0x200000,  // QList<?> -> array<?>
    };

    static_assert(MetaType::MetaMax < MetaType::Array);

    struct ScriptFnInfo {
        MetaType ret;
        QVector<QPair<MetaType, QString>> params;
        ScriptFn fn;
    };

    enum class RegisteredEvent : uint {
        None,
        AppReady = 1u,
        AppClosing = 1u << 1,
        SelectionChanged = 1u << 2,
        CursorPositionChanged = 1u << 3,
        FileOpened = 1u << 4,
        FileSaved = 1u << 5,
        FileSwitched = 1u << 6,
        FileClosed = 1u << 7,
        ScriptPragma = 1u << 8,
        PluginFileOpened = 1u << 9,
        PluginFileClosed = 1u << 10,
        ScriptUnSafeFnRegistering = 1u << 11,
        ScriptPragmaInit = 1u << 12
    };
    Q_DECLARE_FLAGS(RegisteredEvents, RegisteredEvent)

    enum class PluginFileEvent {
        Opened,
        Saved,
        Exported,
        Switched,
        Closed,
        PluginOpened,
        PluginClosed
    };

    enum class FileType { Invalid, File, Extension };
    Q_ENUM(FileType)

public:
    ScriptCallError generateScriptCallError(int errCode, const QString &msg);

    QVariant getScriptCallError(int errCode, const QString &msg);

public:
    virtual ~IWingPlugin() = default;

    virtual RegisteredEvents registeredEvents() const;

public:
    virtual QMenu *registeredHexContextMenu() const;
    virtual QList<WingRibbonToolBoxInfo> registeredRibbonTools() const;
    // QMap<page, whether is visible in setting tab>
    virtual QHash<SettingPage *, bool> registeredSettingPages() const;

    virtual QList<QSharedPointer<WingEditorViewWidget::Creator>>
    registeredEditorViewWidgets() const;

public:
    // QHash< function-name, fn >
    virtual QHash<QString, ScriptFnInfo> registeredScriptFns() const;

    // A hacking way to register script function (Generic_Call)
    // This registering way is not safe. There is no
    // other checking except function's signature.
    // You should handle your all the types and pay yourself.

    // You should set RegisteredEvent::ScriptFnRegistering ON to enable it.

    // QHash< function-name, fn >
    virtual QHash<QString, UNSAFE_SCFNPTR> registeredScriptUnsafeFns() const;

    // QHash< enum , members >
    virtual QHash<QString, QList<QPair<QString, int>>>
    registeredScriptEnums() const;

    // Note: must be valid identifier
    virtual QStringList registerScriptMarcos() const;

public:
    virtual void eventSelectionChanged(const QByteArrayList &selections,
                                       bool isPreview);

    virtual void eventCursorPositionChanged(const WingHex::HexPosition &pos);

    virtual void eventPluginFile(PluginFileEvent e, FileType type,
                                 const QString &newfileName, int handle,
                                 const QString &oldfileName);

    virtual void eventReady();

    virtual bool eventClosing();

public:
    virtual bool eventOnScriptPragma(const QString &script,
                                     const QStringList &comments);

    virtual void eventOnScriptPragmaInit();
};

} // namespace WingHex

Q_DECLARE_METATYPE(WingHex::SenderInfo)
Q_DECLARE_METATYPE(WingHex::ScriptCallError)
Q_DECLARE_INTERFACE(WingHex::IWingPlugin, "com.wingsummer.iwingplugin")

#endif // IWINGPLUGIN_H
