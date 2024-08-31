#ifndef QSTORAGEDEVICE_H
#define QSTORAGEDEVICE_H

#include <QIODevice>
#include <QStorageInfo>

class QStorageDevice : public QIODevice {
    Q_OBJECT
public:
    QStorageDevice();

    void setStorage(const QStorageInfo &storage);

    QStorageInfo storage() const;

private:
    QStorageInfo _storage;

    // QIODevice interface
protected:
    virtual qint64 readData(char *data, qint64 maxlen) override;
    virtual qint64 writeData(const char *data, qint64 len) override;
};

#endif // QSTORAGEDEVICE_H
