#include "insertcommand.h"

InsertCommand::InsertCommand(QHexDocument *doc, QHexCursor *cursor,
                             qsizetype offset, const QByteArray &data,
                             int nibbleindex, QUndoCommand *parent)
    : HexCommand(doc, cursor, nibbleindex, parent) {
    m_offset = offset;
    m_data = data;
    m_length = data.length();
}

void InsertCommand::undo() {
    m_doc->_remove(m_offset, m_data.length());
    m_cursor->setPos(m_offset, m_nibbleindex);
}
void InsertCommand::redo() {
    m_doc->_insert(m_offset, m_data);
    if (m_data.length() == 1 && m_nibbleindex) {
        m_cursor->setPos(m_offset, 0);
    } else {
        m_cursor->setPos(m_offset + m_length, m_nibbleindex);
    }
}
