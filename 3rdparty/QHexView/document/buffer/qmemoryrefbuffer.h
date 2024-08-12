#ifndef QMEMORYREFBUFFER_H
#define QMEMORYREFBUFFER_H

#include "qbuffer.h"
#include "qhexbuffer.h"

class QMemoryRefBuffer : public QHexBuffer {
    Q_OBJECT

public:
    explicit QMemoryRefBuffer(QObject *parent = nullptr);
    qsizetype length() const override;
    void insert(qsizetype offset, const QByteArray &data) override;
    void remove(qsizetype offset, qsizetype length) override;
    QByteArray read(qsizetype offset, qsizetype length) override;
    bool read(QIODevice *device) override;
    void write(QIODevice *device) override;

    qsizetype indexOf(const QByteArray &ba, qsizetype from) override;
    qsizetype lastIndexOf(const QByteArray &ba, qsizetype from) override;

private:
    QBuffer *m_buffer;
};

#endif // QMEMORYREFBUFFER_H
