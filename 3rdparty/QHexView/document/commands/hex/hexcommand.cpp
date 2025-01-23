#include "hexcommand.h"

HexCommand::HexCommand(QHexDocument *doc, QHexCursor *cursor, int nibbleindex,
                       QUndoCommand *parent)
    : QUndoCommand(parent), m_doc(doc), m_cursor(cursor), m_offset(0),
      m_length(0), m_nibbleindex(nibbleindex) {}

void HexCommand::undo() {
    Q_ASSERT(m_doc->m_bytesModFlag > 0);
    m_doc->m_bytesModFlag--;
}

void HexCommand::redo() { m_doc->m_bytesModFlag++; }
