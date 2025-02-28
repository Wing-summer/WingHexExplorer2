#ifndef IWINGPLUGINBASECALLS_H
#define IWINGPLUGINBASECALLS_H

#include "WingPlugin/wingplugincallconvertor.h"
#include "wingplugincalls.h"

#include <QFileDialog>
#include <QIcon>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QObject>

namespace WingHex {

class IWingPluginBase;

class WINGPLUGIN_EXPORT IWingPluginBaseCalls : public WingPluginCalls {
public:
    explicit IWingPluginBaseCalls(IWingPluginBase *const caller);

public:
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

public:
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

public:
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

public:
    Q_REQUIRED_RESULT QColor getColor(const QString &caption,
                                      QWidget *parent = nullptr);
};

} // namespace WingHex

Q_DECLARE_METATYPE(QMessageBox::StandardButtons)
Q_DECLARE_METATYPE(QMessageBox::StandardButton)
Q_DECLARE_METATYPE(bool *)
Q_DECLARE_METATYPE(QString *)

#endif // IWINGPLUGINBASECALLS_H
