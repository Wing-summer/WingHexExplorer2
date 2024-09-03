#include "hexcommand.h"

HexCommand::HexCommand(QHexBuffer *buffer, QHexCursor *cursor, int nibbleindex,
                       QUndoCommand *parent)
    : QUndoCommand(parent), m_buffer(buffer), m_offset(0), m_length(0),
      m_cursor(cursor), m_nibbleindex(nibbleindex) {}
