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

#ifndef WINGPLUGINWIDGET_H
#define WINGPLUGINWIDGET_H

#include "WingPlugin/iwingplugincalls.h"
#include "wingplugin_global.h"

#include <QDialog>
#include <QMainWindow>
#include <QWidget>
#include <QWindow>

namespace WingHex {

class IWingPlugin;

class WINGPLUGIN_EXPORT WingPluginWidget
    : public QWidget,
      protected WingHex::IWingPluginAPICalls {
    Q_OBJECT

public:
    explicit WingPluginWidget(WingHex::IWingPlugin *plg,
                              QWidget *parent = nullptr);
    virtual ~WingPluginWidget();
};

class WINGPLUGIN_EXPORT WingPluginDialog
    : public QDialog,
      protected WingHex::IWingPluginAPICalls {
    Q_OBJECT

public:
    explicit WingPluginDialog(WingHex::IWingPlugin *plg,
                              QWidget *parent = nullptr);
    virtual ~WingPluginDialog();
};

class WINGPLUGIN_EXPORT WingPluginWindow
    : public QWindow,
      protected WingHex::IWingPluginAPICalls {
    Q_OBJECT

public:
    explicit WingPluginWindow(WingHex::IWingPlugin *plg,
                              QWindow *parent = nullptr);
    explicit WingPluginWindow(WingHex::IWingPlugin *plg,
                              QScreen *parent = nullptr);
    virtual ~WingPluginWindow();
};

class WINGPLUGIN_EXPORT WingPluginMainWindow
    : public QMainWindow,
      protected WingHex::IWingPluginAPICalls {
    Q_OBJECT

public:
    explicit WingPluginMainWindow(WingHex::IWingPlugin *plg,
                                  QWidget *parent = nullptr);
    virtual ~WingPluginMainWindow();
};

} // namespace WingHex

#endif // WINGPLUGINWIDGET_H
