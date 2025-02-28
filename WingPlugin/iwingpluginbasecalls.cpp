#include "iwingpluginbasecalls.h"

#include "WingPlugin/wingcore.h"
#include "iwingpluginbase.h"

using namespace WingHex;

IWingPluginBaseCalls::IWingPluginBaseCalls(IWingPluginBase *const caller)
    : WingPluginCalls(caller) {}

void IWingPluginBaseCalls::toast(const QPixmap &icon, const QString &message) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::toast);
    m.invoke(callReceiver(), Qt::DirectConnection, getSenderObj(),
             WINGAPI_ARG(icon), WINGAPI_ARG(message));
}

void IWingPluginBaseCalls::trace(const QString &message) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::trace);
    m.invoke(callReceiver(), Qt::DirectConnection, getSenderObj(),
             WINGAPI_ARG(message));
}

void IWingPluginBaseCalls::debug(const QString &message) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::debug);
    m.invoke(callReceiver(), Qt::DirectConnection, getSenderObj(),
             WINGAPI_ARG(message));
}

void IWingPluginBaseCalls::warn(const QString &message) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::warn);
    m.invoke(callReceiver(), Qt::DirectConnection, getSenderObj(),
             WINGAPI_ARG(message));
}

void IWingPluginBaseCalls::error(const QString &message) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::error);
    m.invoke(callReceiver(), Qt::DirectConnection, getSenderObj(),
             WINGAPI_ARG(message));
}

void IWingPluginBaseCalls::info(const QString &message) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::info);
    m.invoke(callReceiver(), Qt::DirectConnection, getSenderObj(),
             WINGAPI_ARG(message));
}

bool IWingPluginBaseCalls::raiseDockWidget(QWidget *w) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::raiseDockWidget);
    bool ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(w));
    return ret;
}

QDialog *IWingPluginBaseCalls::createDialog(QWidget *content) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::createDialog);
    QDialog *ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(content));
    return ret;
}

void IWingPluginBaseCalls::aboutQt(QWidget *parent, const QString &title) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::aboutQt);
    m.invoke(callReceiver(), Qt::DirectConnection, getSenderObj(),
             WINGAPI_ARG(parent), WINGAPI_ARG(title));
}

QMessageBox::StandardButton
IWingPluginBaseCalls::information(QWidget *parent, const QString &title,
                                  const QString &text,
                                  QMessageBox::StandardButtons buttons,
                                  QMessageBox::StandardButton defaultButton) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::information);
    QMessageBox::StandardButton ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(text), WINGAPI_ARG(buttons),
             WINGAPI_ARG(defaultButton));
    return ret;
}

QMessageBox::StandardButton
IWingPluginBaseCalls::question(QWidget *parent, const QString &title,
                               const QString &text,
                               QMessageBox::StandardButtons buttons,
                               QMessageBox::StandardButton defaultButton) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::question);
    QMessageBox::StandardButton ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(text), WINGAPI_ARG(buttons),
             WINGAPI_ARG(defaultButton));
    return ret;
}

QMessageBox::StandardButton
IWingPluginBaseCalls::warning(QWidget *parent, const QString &title,
                              const QString &text,
                              QMessageBox::StandardButtons buttons,
                              QMessageBox::StandardButton defaultButton) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::warning);
    QMessageBox::StandardButton ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(text), WINGAPI_ARG(buttons),
             WINGAPI_ARG(defaultButton));
    return ret;
}

QMessageBox::StandardButton
IWingPluginBaseCalls::critical(QWidget *parent, const QString &title,
                               const QString &text,
                               QMessageBox::StandardButtons buttons,
                               QMessageBox::StandardButton defaultButton) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::critical);
    QMessageBox::StandardButton ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(text), WINGAPI_ARG(buttons),
             WINGAPI_ARG(defaultButton));
    return ret;
}

void IWingPluginBaseCalls::about(QWidget *parent, const QString &title,
                                 const QString &text) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::about);
    m.invoke(callReceiver(), Qt::DirectConnection, getSenderObj(),
             WINGAPI_ARG(parent), WINGAPI_ARG(title), WINGAPI_ARG(text));
}

QMessageBox::StandardButton
IWingPluginBaseCalls::msgbox(QWidget *parent, QMessageBox::Icon icon,
                             const QString &title, const QString &text,
                             QMessageBox::StandardButtons buttons,
                             QMessageBox::StandardButton defaultButton) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::msgbox);
    QMessageBox::StandardButton ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(icon),
             WINGAPI_ARG(title), WINGAPI_ARG(text), WINGAPI_ARG(buttons),
             WINGAPI_ARG(defaultButton));
    return ret;
}

QString IWingPluginBaseCalls::getText(QWidget *parent, const QString &title,
                                      const QString &label,
                                      QLineEdit::EchoMode echo,
                                      const QString &text, bool *ok,
                                      Qt::InputMethodHints inputMethodHints) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::getText);
    QString ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(label), WINGAPI_ARG(echo), WINGAPI_ARG(text),
             WINGAPI_ARG(ok), WINGAPI_ARG(inputMethodHints));
    return ret;
}

QString IWingPluginBaseCalls::getMultiLineText(
    QWidget *parent, const QString &title, const QString &label,
    const QString &text, bool *ok, Qt::InputMethodHints inputMethodHints) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::getMultiLineText);
    QString ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(label), WINGAPI_ARG(text), WINGAPI_ARG(ok),
             WINGAPI_ARG(inputMethodHints));
    return ret;
}

QString IWingPluginBaseCalls::getItem(QWidget *parent, const QString &title,
                                      const QString &label,
                                      const QStringList &items, int current,
                                      bool editable, bool *ok,
                                      Qt::InputMethodHints inputMethodHints) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::getItem);
    QString ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(label), WINGAPI_ARG(items), WINGAPI_ARG(current),
             WINGAPI_ARG(editable), WINGAPI_ARG(ok),
             WINGAPI_ARG(inputMethodHints));
    return ret;
}

int IWingPluginBaseCalls::getInt(QWidget *parent, const QString &title,
                                 const QString &label, int value, int minValue,
                                 int maxValue, int step, bool *ok) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::getInt);
    int ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(label), WINGAPI_ARG(value), WINGAPI_ARG(minValue),
             WINGAPI_ARG(maxValue), WINGAPI_ARG(step), WINGAPI_ARG(ok));
    return ret;
}

double IWingPluginBaseCalls::getDouble(QWidget *parent, const QString &title,
                                       const QString &label, double value,
                                       double minValue, double maxValue,
                                       int decimals, bool *ok, double step) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::getDouble);
    double ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(label), WINGAPI_ARG(value), WINGAPI_ARG(minValue),
             WINGAPI_ARG(maxValue), WINGAPI_ARG(decimals), WINGAPI_ARG(ok),
             WINGAPI_ARG(step));
    return ret;
}

QString IWingPluginBaseCalls::getExistingDirectory(
    QWidget *parent, const QString &caption, const QString &dir,
    QFileDialog::Options options) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::getExistingDirectory);
    QString ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(caption),
             WINGAPI_ARG(dir), WINGAPI_ARG(options));
    return ret;
}

QString IWingPluginBaseCalls::getOpenFileName(QWidget *parent,
                                              const QString &caption,
                                              const QString &dir,
                                              const QString &filter,
                                              QString *selectedFilter,
                                              QFileDialog::Options options) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::getOpenFileName);
    QString ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(caption),
             WINGAPI_ARG(dir), WINGAPI_ARG(filter), WINGAPI_ARG(selectedFilter),
             WINGAPI_ARG(options));
    return ret;
}

QStringList IWingPluginBaseCalls::getOpenFileNames(
    QWidget *parent, const QString &caption, const QString &dir,
    const QString &filter, QString *selectedFilter,
    QFileDialog::Options options) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::getOpenFileNames);
    QStringList ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(caption),
             WINGAPI_ARG(dir), WINGAPI_ARG(filter), WINGAPI_ARG(selectedFilter),
             WINGAPI_ARG(options));
    return ret;
}

QString IWingPluginBaseCalls::getSaveFileName(QWidget *parent,
                                              const QString &caption,
                                              const QString &dir,
                                              const QString &filter,
                                              QString *selectedFilter,
                                              QFileDialog::Options options) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::getSaveFileName);
    QString ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(caption),
             WINGAPI_ARG(dir), WINGAPI_ARG(filter), WINGAPI_ARG(selectedFilter),
             WINGAPI_ARG(options));
    return ret;
}

QColor IWingPluginBaseCalls::getColor(const QString &caption, QWidget *parent) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::getColor);
    QColor ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(caption), WINGAPI_ARG(parent));
    return ret;
}

AppTheme IWingPluginBaseCalls::currentAppTheme() {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::currentAppTheme);
    WingHex::AppTheme theme;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(theme),
             getSenderObj());
    return theme;
}
