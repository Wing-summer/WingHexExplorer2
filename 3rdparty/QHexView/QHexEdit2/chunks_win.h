#ifndef Chunks_winWIN_H
#define Chunks_winWIN_H

/** \cond docNever */

/*! The Chunks_win class is the storage backend for QHexEdit.
 *
 * When QHexEdit loads data, Chunks_win access them using a QIODevice interface.
 * When the app uses a QByteArray interface, QBuffer is used to provide again a
 * QIODevice like interface. No data will be changed, therefore Chunks_win opens
 * the QIODevice in QIODevice::ReadOnly mode. After every access Chunks_win
 * closes the QIODevice, that's why external applications can overwrite files
 * while QHexEdit shows them.
 *
 * When the the user starts to edit the data, Chunks_win creates a local copy of
 * a chunk of data (4 kilobytes) and notes all changes there. Parallel to that
 * chunk, there is a second chunk, which keep track of which bytes are changed
 * and which not.
 *
 */

#include <QtCore>

// note: this class is only for storage device reading

#ifdef Q_OS_WIN

#include "define.h"
#include <QStorageInfo>
#include <Windows.h>

struct Chunk_win {
    QByteArray data;
    QByteArray dataChanged;
    qsizetype absPos;
};

class Chunks_win : public QObject {
    Q_OBJECT
public:
    // Constructors and file settings
    Chunks_win(QObject *parent = nullptr);
    Chunks_win(const QStorageInfo &ioDevice, QObject *parent);

    ~Chunks_win();

    bool setIODevice(const QStorageInfo &ioDevice);

    // Getting data out of Chunks_win
    QByteArray data(qsizetype pos = 0, qsizetype maxSize = -1);
    bool write(QIODevice *iODevice, qsizetype pos = 0, qsizetype count = -1);

    // Search API
    qsizetype indexOf(const QByteArray &ba, qsizetype from);
    qsizetype lastIndexOf(const QByteArray &ba, qsizetype from);

    // Char manipulations
    bool insert(qsizetype pos, char b);
    bool overwrite(qsizetype pos, char b);
    bool removeAt(qsizetype pos);

    // Utility functions
    char operator[](qsizetype pos);
    qsizetype pos();
    qsizetype size();

private:
    qsizetype getChunkIndex(qsizetype absPos);
    QIODevice *_ioDevice;
    qsizetype _pos;
    qsizetype _size;
    qsizetype CHUNK_SIZE;
    HANDLE _handle = INVALID_HANDLE_VALUE;
    QList<Chunk_win> _chunks;
};

#endif

#endif // Chunks_win_H
