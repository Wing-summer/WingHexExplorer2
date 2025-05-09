/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
**
** The original License is LGPL from Andres6936/QHexEdit. I have modified a lot
** so I decide to change the Open Source License. You can use the original
** library under LGPL. Thanks for Andres6936's efforts.
** =============================================================================
*/

#ifndef CHUNKS_H
#define CHUNKS_H

/** \cond docNever */

/*! The Chunks class is the storage backend for QHexEdit.
 *
 * When QHexEdit loads data, Chunks access them using a QIODevice interface.
 * When the app uses a QByteArray interface, QBuffer is used to provide again a
 * QIODevice like interface. No data will be changed, therefore Chunks opens the
 * QIODevice in QIODevice::ReadOnly mode. After every access Chunks closes the
 * QIODevice, that's why external applications can overwrite files while
 * QHexEdit shows them.
 *
 * When the the user starts to edit the data, Chunks creates a local copy of a
 * chunk of data (4 kilobytes) and notes all changes there. Parallel to that
 * chunk, there is a second chunk, which keep track of which bytes are changed
 * and which not.
 *
 */

#include <QtCore>
#include <QtGlobal>

struct Chunk {
    QByteArray data;
    QByteArray dataChanged;
    qsizetype absPos;
};

class Chunks : public QObject {
    Q_OBJECT
public:
    // Constructors and file settings
    Chunks(QObject *parent = nullptr);
    Chunks(QIODevice *ioDevice, QObject *parent);

    ~Chunks();

    bool setIODevice(QIODevice *ioDevice);

    // Getting data out of Chunks
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
    QList<Chunk> _chunks;
};

/** \endcond docNever */

#endif // CHUNKS_H
