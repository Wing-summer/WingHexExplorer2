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

#ifndef IWINGPLUGINBASE_H
#define IWINGPLUGINBASE_H

#include "settingpage.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QMessageBox>
#include <QObject>
#include <QSettings>
#include <QVersionNumber>

namespace WingHex {

using qusizetype = QIntegerForSizeof<std::size_t>::Unsigned;

Q_DECL_UNUSED constexpr auto SDKVERSION = 15;

Q_DECL_UNUSED static QString PLUGINDIR() {
    return QCoreApplication::applicationDirPath() + QStringLiteral("/plugin");
}

Q_DECL_UNUSED static QString
HOSTRESPIMG(const QString &name,
            const QString &suffix = QStringLiteral(".png")) {
    return QStringLiteral(":/com.wingsummer.winghex/images/") + name + suffix;
}

namespace WingPlugin {

class MessageBox : public QObject {
    Q_OBJECT
signals:
    void aboutQt(QWidget *parent = nullptr, const QString &title = QString());

    QMessageBox::StandardButton information(
        QWidget *parent, const QString &title, const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QMessageBox::StandardButton question(
        QWidget *parent, const QString &title, const QString &text,
        QMessageBox::StandardButtons buttons =
            QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QMessageBox::StandardButton
    warning(QWidget *parent, const QString &title, const QString &text,
            QMessageBox::StandardButtons buttons = QMessageBox::Ok,
            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QMessageBox::StandardButton
    critical(QWidget *parent, const QString &title, const QString &text,
             QMessageBox::StandardButtons buttons = QMessageBox::Ok,
             QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    void about(QWidget *parent, const QString &title, const QString &text);

    QMessageBox::StandardButton
    msgbox(QWidget *parent, QMessageBox::Icon icon, const QString &title,
           const QString &text,
           QMessageBox::StandardButtons buttons = QMessageBox::NoButton,
           QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
};

class InputBox : public QObject {
    Q_OBJECT
signals:
    Q_REQUIRED_RESULT QString
    getText(QWidget *parent, const QString &title, const QString &label,
            QLineEdit::EchoMode echo = QLineEdit::Normal,
            const QString &text = QString(), bool *ok = nullptr,
            Qt::InputMethodHints inputMethodHints = Qt::ImhNone);
    Q_REQUIRED_RESULT QString getMultiLineText(
        QWidget *parent, const QString &title, const QString &label,
        const QString &text = QString(), bool *ok = nullptr,
        Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

    Q_REQUIRED_RESULT QString
    getItem(QWidget *parent, const QString &title, const QString &label,
            const QStringList &items, int current = 0, bool editable = true,
            bool *ok = nullptr,
            Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

    Q_REQUIRED_RESULT int getInt(QWidget *parent, const QString &title,
                                 const QString &label, int value = 0,
                                 int minValue = -2147483647,
                                 int maxValue = 2147483647, int step = 1,
                                 bool *ok = nullptr);

    Q_REQUIRED_RESULT double getDouble(QWidget *parent, const QString &title,
                                       const QString &label, double value = 0,
                                       double minValue = -2147483647,
                                       double maxValue = 2147483647,
                                       int decimals = 1, bool *ok = nullptr,
                                       double step = 1);
};

class FileDialog : public QObject {
    Q_OBJECT
signals:
    Q_REQUIRED_RESULT QString getExistingDirectory(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(),
        QFileDialog::Options options = QFileDialog::ShowDirsOnly);

    Q_REQUIRED_RESULT QString getOpenFileName(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(), const QString &filter = QString(),
        QString *selectedFilter = nullptr,
        QFileDialog::Options options = QFileDialog::Options());

    Q_REQUIRED_RESULT QStringList getOpenFileNames(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(), const QString &filter = QString(),
        QString *selectedFilter = nullptr,
        QFileDialog::Options options = QFileDialog::Options());

    Q_REQUIRED_RESULT QString getSaveFileName(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(), const QString &filter = QString(),
        QString *selectedFilter = nullptr,
        QFileDialog::Options options = QFileDialog::Options());
};

class ColorDialog : public QObject {
    Q_OBJECT
signals:
    Q_REQUIRED_RESULT QColor getColor(const QString &caption,
                                      QWidget *parent = nullptr);
};

} // namespace WingPlugin

struct WingDependency {
    QString puid;
    QVersionNumber version;
};

struct WingDockWidgetInfo {
    QString widgetName;
    QString displayName;
    QWidget *widget = nullptr;
    Qt::DockWidgetArea area = Qt::DockWidgetArea::NoDockWidgetArea;
};

#ifdef WING_SERVICE
#undef WING_SERVICE
#endif

#define WING_SERVICE Q_INVOKABLE

const auto WINGSUMMER = QStringLiteral("wingsummer");

enum class AppTheme { Dark, Light };

class IWingPluginBase : public QObject {
    Q_OBJECT

public:
    virtual ~IWingPluginBase() = default;

public:
    virtual int sdkVersion() const = 0;
    virtual const QString signature() const = 0;

    virtual bool init(const std::unique_ptr<QSettings> &set) = 0;
    virtual void unload(std::unique_ptr<QSettings> &set) = 0;

    virtual const QString pluginName() const = 0;
    virtual QIcon pluginIcon() const { return {}; }
    virtual const QString pluginComment() const = 0;

    virtual QString retranslate(const QString &str) { return str; }

public:
    virtual QList<WingDockWidgetInfo> registeredDockWidgets() const {
        return {};
    }

    virtual QList<PluginPage *> registeredPages() const { return {}; }

signals:
    void toast(const QPixmap &icon, const QString &message);
    void trace(const QString &message);
    void debug(const QString &message);
    void warn(const QString &message);
    void error(const QString &message);
    void info(const QString &message);

    bool raiseDockWidget(QWidget *w);

    // theme
    WingHex::AppTheme currentAppTheme();

    // not available for AngelScript
    // only for plugin UI extenstion

    QDialog *createDialog(QWidget *content);

public:
    WingPlugin::MessageBox msgbox;
    WingPlugin::InputBox inputbox;
    WingPlugin::FileDialog filedlg;
    WingPlugin::ColorDialog colordlg;
};

} // namespace WingHex

#endif // IWINGPLUGINBASE_H
