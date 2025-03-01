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

#ifndef WINGPLUGIN_GLOBAL_H
#define WINGPLUGIN_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(WINGPLUGIN_LIBRARY)
#define WINGPLUGIN_EXPORT Q_DECL_EXPORT
#else
#define WINGPLUGIN_EXPORT Q_DECL_IMPORT
#endif

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QMetaMethod>
#include <QString>
#include <QVersionNumber>
#include <QWidget>

#ifdef WING_SERVICE
#undef WING_SERVICE
#endif

#define WING_SERVICE Q_INVOKABLE

#define WINGAPI_ARG_SPEC(Type)                                                 \
    inline QArgument<Type> WINGAPI_ARG(const Type &t) {                        \
        return QArgument<Type>(QT_STRINGIFY(Type), t);                         \
    }                                                                          \
    inline QReturnArgument<Type> WINGAPI_RETURN_ARG(Type &t) {                 \
        return QReturnArgument<Type>(QT_STRINGIFY(Type), t);                   \
    }

template <typename T>
inline QArgument<T> WINGAPI_ARG(const T &t) {
    static QByteArray name;
    if (name.isEmpty()) {
        auto m = QMetaType::fromType<std::decay_t<T>>();
        name = m.name();
    }
    return QArgument<std::decay_t<T>>(name, t);
}

template <typename T>
inline QReturnArgument<T> WINGAPI_RETURN_ARG(T &t) {
    static QByteArray name;
    if (name.isEmpty()) {
        auto m = QMetaType::fromType<std::decay_t<T>>();
        name = m.name();
    }
    return QReturnArgument<std::decay_t<T>>(name, t);
}

namespace WingHex {

struct WingDockWidgetInfo {
    QString widgetName;
    QString displayName;
    QWidget *widget = nullptr;
    Qt::DockWidgetArea area = Qt::DockWidgetArea::NoDockWidgetArea;
};

struct WingDependency {
    QString puid;
    QVersionNumber version;
};

enum class AppTheme { Dark, Light };

using CallTable = QHash<QByteArray, QMetaMethod>;

using ClickedCallBack = std::function<void(const QModelIndex &)>;

Q_NAMESPACE_EXPORT(WINGPLUGIN_EXPORT)

enum ErrFile : int {
    Success = 0,
    Error = -1,
    UnSaved = -2,
    Permission = -3,
    NotExist = -4,
    AlreadyOpened = -5,
    IsNewFile = -6,
    WorkSpaceUnSaved = -7,
    ClonedFile = -8,
    InvalidFormat = -9,
    TooManyOpenedFile = -10,
    NotAllowedInNoneGUIThread = -11,
    DevNotFound = -12,
};
Q_ENUM_NS(ErrFile)

enum class SelectionMode : int { Add, Remove, Single };
Q_ENUM_NS(SelectionMode)

} // namespace WingHex

Q_DECLARE_METATYPE(WingHex::AppTheme)
Q_DECLARE_METATYPE(WingHex::CallTable)

// Template specialization for typedefs
WINGAPI_ARG_SPEC(QMessageBox::StandardButtons);
WINGAPI_ARG_SPEC(QInputDialog::InputDialogOptions);
WINGAPI_ARG_SPEC(QFileDialog::Options);
WINGAPI_ARG_SPEC(Qt::InputMethodHints);
WINGAPI_ARG_SPEC(qsizetype);

WINGAPI_ARG_SPEC(WingHex::ClickedCallBack);

#endif // WINGPLUGIN_GLOBAL_H
