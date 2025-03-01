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

void IWingPluginBaseCalls::logTrace(const QString &message) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::logTrace);
    m.invoke(callReceiver(), Qt::DirectConnection, getSenderObj(),
             WINGAPI_ARG(message));
}

void IWingPluginBaseCalls::logDebug(const QString &message) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::logDebug);
    m.invoke(callReceiver(), Qt::DirectConnection, getSenderObj(),
             WINGAPI_ARG(message));
}

void IWingPluginBaseCalls::logWarn(const QString &message) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::logWarn);
    m.invoke(callReceiver(), Qt::DirectConnection, getSenderObj(),
             WINGAPI_ARG(message));
}

void IWingPluginBaseCalls::logError(const QString &message) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::logError);
    m.invoke(callReceiver(), Qt::DirectConnection, getSenderObj(),
             WINGAPI_ARG(message));
}

void IWingPluginBaseCalls::logInfo(const QString &message) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::logInfo);
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

void IWingPluginBaseCalls::msgAboutQt(QWidget *parent, const QString &title) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::msgAboutQt);
    m.invoke(callReceiver(), Qt::DirectConnection, getSenderObj(),
             WINGAPI_ARG(parent), WINGAPI_ARG(title));
}

QMessageBox::StandardButton IWingPluginBaseCalls::msgInformation(
    QWidget *parent, const QString &title, const QString &text,
    QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::msgInformation);
    QMessageBox::StandardButton ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(text), WINGAPI_ARG(buttons),
             WINGAPI_ARG(defaultButton));
    return ret;
}

QMessageBox::StandardButton
IWingPluginBaseCalls::msgQuestion(QWidget *parent, const QString &title,
                                  const QString &text,
                                  QMessageBox::StandardButtons buttons,
                                  QMessageBox::StandardButton defaultButton) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::msgQuestion);
    QMessageBox::StandardButton ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(text), WINGAPI_ARG(buttons),
             WINGAPI_ARG(defaultButton));
    return ret;
}

QMessageBox::StandardButton
IWingPluginBaseCalls::msgWarning(QWidget *parent, const QString &title,
                                 const QString &text,
                                 QMessageBox::StandardButtons buttons,
                                 QMessageBox::StandardButton defaultButton) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::msgWarning);
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

void IWingPluginBaseCalls::msgAbout(QWidget *parent, const QString &title,
                                    const QString &text) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::msgAbout);
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

QString
IWingPluginBaseCalls::dlgGetText(QWidget *parent, const QString &title,
                                 const QString &label, QLineEdit::EchoMode echo,
                                 const QString &text, bool *ok,
                                 Qt::InputMethodHints inputMethodHints) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::dlgGetText);
    QString ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(label), WINGAPI_ARG(echo), WINGAPI_ARG(text),
             WINGAPI_ARG(ok), WINGAPI_ARG(inputMethodHints));
    return ret;
}

QString IWingPluginBaseCalls::dlgGetMultiLineText(
    QWidget *parent, const QString &title, const QString &label,
    const QString &text, bool *ok, Qt::InputMethodHints inputMethodHints) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::dlgGetMultiLineText);
    QString ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(label), WINGAPI_ARG(text), WINGAPI_ARG(ok),
             WINGAPI_ARG(inputMethodHints));
    return ret;
}

QString
IWingPluginBaseCalls::dlgGetItem(QWidget *parent, const QString &title,
                                 const QString &label, const QStringList &items,
                                 int current, bool editable, bool *ok,
                                 Qt::InputMethodHints inputMethodHints) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::dlgGetItem);
    QString ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(label), WINGAPI_ARG(items), WINGAPI_ARG(current),
             WINGAPI_ARG(editable), WINGAPI_ARG(ok),
             WINGAPI_ARG(inputMethodHints));
    return ret;
}

int IWingPluginBaseCalls::dlgGetInt(QWidget *parent, const QString &title,
                                    const QString &label, int value,
                                    int minValue, int maxValue, int step,
                                    bool *ok) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::dlgGetInt);
    int ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(label), WINGAPI_ARG(value), WINGAPI_ARG(minValue),
             WINGAPI_ARG(maxValue), WINGAPI_ARG(step), WINGAPI_ARG(ok));
    return ret;
}

double IWingPluginBaseCalls::dlgGetDouble(QWidget *parent, const QString &title,
                                          const QString &label, double value,
                                          double minValue, double maxValue,
                                          int decimals, bool *ok, double step) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::dlgGetDouble);
    double ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(title),
             WINGAPI_ARG(label), WINGAPI_ARG(value), WINGAPI_ARG(minValue),
             WINGAPI_ARG(maxValue), WINGAPI_ARG(decimals), WINGAPI_ARG(ok),
             WINGAPI_ARG(step));
    return ret;
}

QString IWingPluginBaseCalls::dlgGetExistingDirectory(
    QWidget *parent, const QString &caption, const QString &dir,
    QFileDialog::Options options) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::dlgGetExistingDirectory);
    QString ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(caption),
             WINGAPI_ARG(dir), WINGAPI_ARG(options));
    return ret;
}

QString IWingPluginBaseCalls::dlgGetOpenFileName(QWidget *parent,
                                                 const QString &caption,
                                                 const QString &dir,
                                                 const QString &filter,
                                                 QString *selectedFilter,
                                                 QFileDialog::Options options) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::dlgGetOpenFileName);
    QString ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(caption),
             WINGAPI_ARG(dir), WINGAPI_ARG(filter), WINGAPI_ARG(selectedFilter),
             WINGAPI_ARG(options));
    return ret;
}

QStringList IWingPluginBaseCalls::dlgGetOpenFileNames(
    QWidget *parent, const QString &caption, const QString &dir,
    const QString &filter, QString *selectedFilter,
    QFileDialog::Options options) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::dlgGetOpenFileNames);
    QStringList ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(caption),
             WINGAPI_ARG(dir), WINGAPI_ARG(filter), WINGAPI_ARG(selectedFilter),
             WINGAPI_ARG(options));
    return ret;
}

QString IWingPluginBaseCalls::dlgGetSaveFileName(QWidget *parent,
                                                 const QString &caption,
                                                 const QString &dir,
                                                 const QString &filter,
                                                 QString *selectedFilter,
                                                 QFileDialog::Options options) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::dlgGetSaveFileName);
    QString ret;
    m.invoke(callReceiver(), Qt::DirectConnection, WINGAPI_RETURN_ARG(ret),
             getSenderObj(), WINGAPI_ARG(parent), WINGAPI_ARG(caption),
             WINGAPI_ARG(dir), WINGAPI_ARG(filter), WINGAPI_ARG(selectedFilter),
             WINGAPI_ARG(options));
    return ret;
}

QColor IWingPluginBaseCalls::dlgGetColor(const QString &caption,
                                         QWidget *parent) {
    SETUP_CALL_CONTEXT(&IWingPluginBaseCalls::dlgGetColor);
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
