#include "appendcommand.h"

AppendCommand::AppendCommand(QHexDocument *doc, QHexCursor *cursor,
                             const QByteArray &data, int nibbleindex,
                             QUndoCommand *parent)
    : HexCommand(doc, cursor, nibbleindex, parent) {
    m_offset = -1;
    m_data = data;
    m_length = data.length();
}

void AppendCommand::undo() {
    auto offset = m_doc->length() - m_length;
    m_doc->_remove(offset, m_length);
    m_doc->insertBookMarkAdjustRevert(offset, m_length);
    m_doc->metadata()->insertAdjustRevert(offset, m_length);
    m_cursor->setPos(offset, m_nibbleindex);
}

void AppendCommand::redo() {
    auto offset = m_doc->length();
    m_doc->_insert(offset, m_data);
    m_doc->insertBookMarkAdjust(offset, m_length);
    m_doc->metadata()->insertAdjust(offset, m_length);
    if (m_data.length() == 1 && m_nibbleindex) {
        m_cursor->setPos(offset, 0);
    } else {
        m_cursor->setPos(offset + m_length, m_nibbleindex);
    }
}
