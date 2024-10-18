#ifndef QSTORAGEDEVICE_H
#define QSTORAGEDEVICE_H

#include <QtGlobal>

#ifdef Q_OS_WIN

#include <QIODevice>
#include <QStorageInfo>

#include <Windows.h>

class QStorageDevice : public QIODevice {
    Q_OBJECT
public:
    QStorageDevice(QObject *parent = nullptr);

    void setStorage(const QStorageInfo &storage);

    QStorageInfo storage() const;

private:
    QStorageInfo _storage;
    QFile _buffer;
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
