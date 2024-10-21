#ifndef QFILEBUFFER_H
#define QFILEBUFFER_H

#include "qhexbuffer.h"
#include <QFile>

class Chunks;

class QFileBuffer : public QHexBuffer {
    Q_OBJECT

public:
    explicit QFileBuffer(QObject *parent = nullptr);
    ~QFileBuffer() override;
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
    Chunks *_chunks = nullptr;
    uchar *m_memory = nullptr;
};

#endif // QFILEBUFFER_H
