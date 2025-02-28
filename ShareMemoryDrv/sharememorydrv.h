/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#ifndef SHAREDMEMORYDRIVER_H
#define SHAREDMEMORYDRIVER_H

#include "WingPlugin/iwingdevice.h"

class SharedMemoryDriver final : public WingHex::IWingDevice {
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "com.wingsummer.iwingdevice" FILE
                          "ShareMemoryDrv.json")
    Q_INTERFACES(WingHex::IWingDevice)

public:
    explicit SharedMemoryDriver();

    virtual ~SharedMemoryDriver();

    // IWingPlugin interface
public:
    virtual int sdkVersion() const override;
    virtual bool init(const std::unique_ptr<QSettings> &set) override;
    virtual void unload(std::unique_ptr<QSettings> &set) override;
    virtual const QString pluginName() const override;
    virtual const QString pluginComment() const override;

public:
    virtual QList<WingHex::PluginPage *> registeredPages() const override;

    // IWingDevice interface
public:
    virtual QString onOpenFileBegin() override;
    virtual QString supportedFileExtDisplayName() const override;
    virtual QIcon supportedFileIcon() const override;
    virtual WingHex::WingIODevice *onOpenFile(const QString &path) override;
    virtual bool onCloseFile(WingHex::WingIODevice *dev) override;

private:
    QList<WingHex::PluginPage *> _plgps;
};

#endif // SHAREDMEMORYDRIVER_H
