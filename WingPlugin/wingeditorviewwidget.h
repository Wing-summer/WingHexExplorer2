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

#ifndef WINGEDITORVIEWWIDGET_H
#define WINGEDITORVIEWWIDGET_H

#include "WingPlugin/wingpluginwidget.h"
#include "iwingplugincalls.h"
#include "wingplugin_global.h"
#include <QWidget>

namespace WingHex {

class IWingPlugin;

class WINGPLUGIN_EXPORT WingEditorViewWidget : public WingPluginWidget {

    Q_OBJECT

public:
    class Creator {
    public:
        Creator() = default;

    public:
        virtual QIcon icon() const = 0;

        virtual QString name() const = 0;

        virtual QString id() const = 0;

    public:
        virtual WingEditorViewWidget *create(WingHex::IWingPlugin *plg,
                                             QWidget *parent) const = 0;
    };

public:
    explicit WingEditorViewWidget(WingHex::IWingPlugin *plg,
                                  QWidget *parent = nullptr);

    virtual WingEditorViewWidget *clone() = 0;

signals:
    void savedStateChanged(bool b);

public slots:
    void raiseView();

    virtual void toggled(bool isVisible) = 0;

    virtual void loadState(const QByteArray &state);

    virtual bool hasUnsavedState();

    virtual QByteArray saveState();

    // return false then stop closing and swith the view
    // but you cannot prevent closing the cloned view
    virtual bool onClosing();

    // cloned view will never call it
    virtual void onWorkSpaceNotify(bool isWorkSpace);
};

} // namespace WingHex

#endif // WINGEDITORVIEWWIDGET_H
