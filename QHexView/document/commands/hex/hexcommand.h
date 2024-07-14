#ifndef HEXCOMMAND_H
#define HEXCOMMAND_H

#include "document/buffer/qhexbuffer.h"
#include "document/qhexcursor.h"
#include <QUndoCommand>

class HexCommand : public QUndoCommand {
public:
    HexCommand(QHexBuffer *buffer, QHexCursor *cursor, int nibbleindex,
               QUndoCommand *parent = nullptr);

protected:
    QHexBuffer *m_buffer;
    qsizetype m_offset;
    qsizetype m_length;
    QByteArray m_data;

    QHexCursor *m_cursor;
    int m_nibbleindex;
};

#endif // HEXCOMMAND_H
