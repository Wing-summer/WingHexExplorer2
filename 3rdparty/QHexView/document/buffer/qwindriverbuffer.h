#ifndef QWINDRIVERBUFFER_H
#define QWINDRIVERBUFFER_H

#include <qnamespace.h>

#ifdef Q_OS_WIN

#include "QHexEdit2/chunks_win.h"
#include "qhexbuffer.h"
#include <QFile>

class QWinDriverBuffer : public QHexBuffer {

public:
    explicit QWinDriverBuffer(QObject *parent = nullptr);
    ~QWinDriverBuffer() override;
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
    Chunks_win *_chunks;
    uchar *m_memory;
};

#endif

#endif // QWINDRIVERBUFFER_H
