#include "removecommand.h"

RemoveCommand::RemoveCommand(QHexBuffer *buffer, qsizetype offset,
                             qsizetype length, QHexCursor *cursor,
                             int nibbleindex, QUndoCommand *parent)
    : HexCommand(buffer, cursor, nibbleindex, parent) {
    m_offset = offset;
    m_length = length;
    m_data = m_buffer->read(m_offset, m_length);
}

void RemoveCommand::undo() {
    m_buffer->insert(m_offset, m_data);
    if (m_length > 1) {
        m_cursor->setPos(m_offset + m_length - 1, 1);
    } else {
        if (m_nibbleindex) {
            m_cursor->setPos(m_offset + 1, 1);
        } else {
            m_cursor->setPos(m_offset, 0);
        }
    }
}

void RemoveCommand::redo() {
    m_cursor->setPos(m_offset, m_nibbleindex);
    m_buffer->remove(m_offset, m_length);
}
