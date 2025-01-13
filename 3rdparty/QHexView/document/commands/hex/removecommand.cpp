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
    m_doc->removeBookMarkAdjustRevert(_rmbms, m_offset, m_length);
    m_doc->metadata()->removeAdjustRevert(_rmMetas, m_offset, m_length);
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
    _rmbms = m_doc->removeBookMarkAdjust(m_offset, m_length);
    _rmMetas = m_doc->metadata()->removeAdjust(m_offset, m_length);
}
