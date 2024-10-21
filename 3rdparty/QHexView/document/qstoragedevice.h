#ifndef QSTORAGEDEVICE_H
#define QSTORAGEDEVICE_H

#include <QtGlobal>

#ifdef Q_OS_WIN

#include <QIODevice>
#include <QStorageInfo>
#include <memory>

#include <Windows.h>

class QStorageDevice : public QIODevice {
    Q_OBJECT
public:
    QStorageDevice(QObject *parent = nullptr);

    void setStorage(const QStorageInfo &storage);

    QStorageInfo storage() const;

private:
    struct Buffer {
        qint64 offset = -1;
        // length is usually 20 * 1024 * CHUNK_SIZE = 10 MB
        std::unique_ptr<char[]> buffer;
        DWORD length = 0;

        void clear() {
            offset = -1;
            length = 0;
        }
    } _cache;

    DWORD cacheSize() const;

private:
    QStorageInfo _storage;
    HANDLE hDevice;
    DWORD CHUNK_SIZE;
    qint64 _size;

    // QIODevice interface
public:
    virtual bool isSequential() const override;
    virtual bool open(OpenMode mode) override;
    virtual void close() override;
    virtual qint64 size() const override;
    virtual bool seek(qint64 pos) override;
    virtual bool canReadLine() const override;

protected:
    virtual qint64 readData(char *data, qint64 maxlen) override;
    virtual qint64 writeData(const char *data, qint64 len) override;
};

#endif

#endif // QSTORAGEDEVICE_H
