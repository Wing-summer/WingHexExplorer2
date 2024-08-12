#ifndef QFILEREGIONBUFFER_H
#define QFILEREGIONBUFFER_H

#include "qhexbuffer.h"

class QFileRegionBuffer : public QHexBuffer {
    Q_OBJECT
public:
    explicit QFileRegionBuffer(QObject *parent = nullptr);

public:
    qsizetype length() const override;
    void insert(qsizetype offset, const QByteArray &data) override;
    void remove(qsizetype offset, qsizetype length) override;
    QByteArray read(qsizetype offset, qsizetype length) override;
    bool read(QIODevice *iodevice) override;
    void write(QIODevice *iodevice) override;

    qint64 indexOf(const QByteArray &ba, qsizetype from) override;
    qint64 lastIndexOf(const QByteArray &ba, qsizetype from) override;

    void setReadOffset(qsizetype offset);
    qint64 readOffset();

    qint64 readMaxBytes();
    void setReadMaxBytes(qsizetype maxlength);

private:
    qsizetype offset = 0;
    qsizetype maxlength = 1024;
    QByteArray buffer;
};

#endif // QFILEREGIONBUFFER_H
