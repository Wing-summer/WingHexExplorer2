#ifndef QMEMORYBUFFER_H
#define QMEMORYBUFFER_H

#include "qhexbuffer.h"

class QMemoryBuffer : public QHexBuffer {
    Q_OBJECT

public:
    explicit QMemoryBuffer(QObject *parent = nullptr);
    uchar at(qsizetype idx) override;
    qsizetype length() const override;
    void insert(qsizetype offset, const QByteArray &data) override;
    void remove(qsizetype offset, qsizetype length) override;
    QByteArray read(qsizetype offset, qsizetype length) override;
    bool read(QIODevice *device) override;
    void write(QIODevice *device) override;

    qsizetype indexOf(const QByteArray &ba, qsizetype from) override;
    qsizetype lastIndexOf(const QByteArray &ba, qsizetype from) override;

private:
    QByteArray m_buffer;
};

#endif // QMEMORYBUFFER_H
