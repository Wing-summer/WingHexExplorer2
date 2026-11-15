/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#ifndef PLUGINIODEVICE_H
#define PLUGINIODEVICE_H

#include <memory>

#include <QHash>
#include <QIODevice>
#include <QPointer>

struct PluginIOContext {
    bool active = true;
    bool devicePlugin = false;
};

// Only the device-plugin host factory can construct this wrapper.
class PluginIODevice final : public QObject {
    Q_OBJECT
public:
    static PluginIODevice *
    openFile(const std::shared_ptr<PluginIOContext> &context,
             const QString &path, int mode, QObject *parent);
    static PluginIODevice *wrap(const std::shared_ptr<PluginIOContext> &context,
                                QIODevice *device, bool allowClose,
                                QObject *parent);

public:
    ~PluginIODevice() override;

public:
    QByteArray read(qint64 maximum);
    QByteArray readAll();
    qint64 write(const QByteArray &bytes);
    bool seek(qint64 position);
    qint64 pos() const;
    qint64 size() const;
    bool atEnd() const;
    bool isOpen() const;
    bool isReadable() const;
    bool isWritable() const;
    bool isSequential() const;
    int openMode() const;
    QString errorString() const;
    void close();
    bool flush();
    bool resize(qint64 size);

private:
    PluginIODevice(std::shared_ptr<PluginIOContext> context, QIODevice *device,
                   bool owned, bool allowClose, QObject *parent);
    QIODevice *checked() const;
    std::shared_ptr<PluginIOContext> m_context;
    QPointer<QIODevice> m_device;
    bool m_owned;
    bool m_allowClose;
};

class PluginIOService final : public QObject {
    Q_OBJECT
public:
    PluginIOService(std::shared_ptr<PluginIOContext> context, QObject *parent);

public:
    PluginIODevice *openFile(const QString &path, int mode);
    PluginIODevice *wrap(const QString &handle);
    QString grant(QIODevice *device, bool allowClose);
    void revoke(const QString &handle);
    void invalidate();

private:
    struct Grant {
        QPointer<QIODevice> device;
        bool allowClose;
        std::shared_ptr<PluginIOContext> context;
    };

    std::shared_ptr<PluginIOContext> m_context;
    QHash<QString, Grant> m_grants;
};

#endif // PLUGINIODEVICE_H
