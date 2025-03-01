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

#ifndef SETTINGPAGE_H
#define SETTINGPAGE_H

#include "wingplugin_global.h"
#include <QWidget>

namespace WingHex {

class WINGPLUGIN_EXPORT PageBase : public QWidget {
public:
    explicit PageBase(QWidget *parent = nullptr);

public:
    virtual QIcon categoryIcon() const = 0;
    virtual QString name() const = 0;
    virtual QString id() const = 0;
};

class WINGPLUGIN_EXPORT SettingPage : public PageBase {
    Q_OBJECT
public:
    explicit SettingPage(QWidget *parent = nullptr);

signals:
    void optionNeedRestartChanged();

public:
    virtual void apply() = 0;
    virtual void reset() = 0;
    virtual void cancel() = 0;
};

class WINGPLUGIN_EXPORT PluginPage : public PageBase {
public:
    explicit PluginPage(QWidget *parent = nullptr);
};

} // namespace WingHex

#endif // SETTINGPAGE_H
