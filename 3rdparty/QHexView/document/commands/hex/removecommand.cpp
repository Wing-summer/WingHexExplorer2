#include "removecommand.h"

RemoveCommand::RemoveCommand(QHexDocument *doc, qsizetype offset,
                             qsizetype length, QHexCursor *cursor,
                             int nibbleindex, QUndoCommand *parent)
    : HexCommand(doc, cursor, nibbleindex, parent) {
    m_offset = offset;
    m_length = length;
    m_data = doc->read(m_offset, m_length);
}

void RemoveCommand::undo() {
    m_doc->_insert(m_offset, m_data);
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
    m_doc->_remove(m_offset, m_length);
}
