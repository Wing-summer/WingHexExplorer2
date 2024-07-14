#ifndef QHEXBUFFER_H
#define QHEXBUFFER_H

#include <QIODevice>
#include <QObject>

class QHexBuffer : public QObject {
    Q_OBJECT

public:
    explicit QHexBuffer(QObject *parent = nullptr);
    bool isEmpty() const;

public:
    virtual uchar at(qsizetype idx);
    virtual void replace(qsizetype offset, const QByteArray &data);
    virtual void read(char *data, qsizetype size);
    virtual void read(const QByteArray &ba);

public:
    virtual qsizetype length() const = 0;
    virtual void insert(qsizetype offset, const QByteArray &data) = 0;
    virtual void remove(qsizetype offset, qsizetype length) = 0;
    virtual QByteArray read(qsizetype offset, qsizetype length) = 0;
    virtual bool read(QIODevice *iodevice) = 0;
    virtual void write(QIODevice *iodevice) = 0;

    virtual qsizetype indexOf(const QByteArray &ba, qsizetype from) = 0;
    virtual qsizetype lastIndexOf(const QByteArray &ba, qsizetype from) = 0;
};

#endif // QHEXBUFFER_H
