#include "replacecommand.h"
#include "document/qhexdocument.h"

ReplaceCommand::ReplaceCommand(QHexDocument *doc, qsizetype offset,
                               const QByteArray &data, QHexCursor *cursor,
                               int nibbleindex, QUndoCommand *parent)
    : HexCommand(doc, cursor, nibbleindex, parent) {
    m_offset = offset;
    m_data = data;
    m_length = data.length();
    m_olddata = doc->read(m_offset, m_length);
}

void ReplaceCommand::undo() {
    m_doc->_replace(m_offset, m_olddata);
    m_cursor->setPos(m_offset, m_nibbleindex);
    HexCommand::undo();
}

void ReplaceCommand::redo() {
    m_doc->_replace(m_offset, m_data);
    if (m_data.length() == 1 && m_nibbleindex) {
        m_cursor->setPos(m_offset, 0);
    } else {
        m_cursor->setPos(m_offset + m_length, !m_nibbleindex);
    }
    HexCommand::redo();
}
