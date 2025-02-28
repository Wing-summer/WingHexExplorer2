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

#ifndef WINGPLUGINCALLS_H
#define WINGPLUGINCALLS_H

#include <QMetaMethod>
#include <QObject>

#include "wingplugin_global.h"
#include "wingplugincallconvertor.h"

namespace WingHex {

class WingPluginCallsCorePrivate;

class WingPluginCallsCore : public QObject {
    Q_OBJECT
    friend class WingPluginCalls;

public:
    WingPluginCallsCore();
    virtual ~WingPluginCallsCore();

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    WingPluginCallsCorePrivate *d_ptr;
    Q_DECLARE_PRIVATE(WingPluginCallsCore)
    Q_DISABLE_COPY_MOVE(WingPluginCallsCore)
};

class WINGPLUGIN_EXPORT WingPluginCalls : public WingPluginCallConvertor {
public:
    explicit WingPluginCalls(QObject *coreobj);
    virtual ~WingPluginCalls();

    WingPluginCallsCore *core() const;

protected:
    WingHex::CallTable callTable() const;

    QObject *callReceiver() const;

private:
    WingPluginCallsCore *_core;
    Q_DISABLE_COPY_MOVE(WingPluginCalls)
};

} // namespace WingHex

#endif // WINGPLUGINCALLS_H
