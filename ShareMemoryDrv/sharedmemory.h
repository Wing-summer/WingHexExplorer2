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

#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include "WingPlugin/iwingdevice.h"

#include <QByteArray>
#include <QSharedMemory>

class SharedMemory : public WingHex::WingIODevice {
    Q_OBJECT

public:
    explicit SharedMemory(const QString &key, OpenMode mode,
                          QObject *parent = nullptr);

    virtual ~SharedMemory();

public:
    bool open(OpenMode mode) override;
    virtual bool keepSize() const override;

    void close() override;

    qint64 readData(char *data, qint64 maxSize) override;

    qint64 writeData(const char *data, qint64 maxSize) override;

    virtual qint64 size() const override;

private:
    QString m_key;
    OpenMode m_mode;
    QSharedMemory m_sharedMemory;
};

#endif // SHAREDMEMORY_H
