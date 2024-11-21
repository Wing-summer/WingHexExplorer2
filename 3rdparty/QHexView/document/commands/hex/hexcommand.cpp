#include "hexcommand.h"

HexCommand::HexCommand(QHexDocument *doc, QHexCursor *cursor, int nibbleindex,
                       QUndoCommand *parent)
    : QUndoCommand(parent), m_doc(doc), m_cursor(cursor), m_offset(0),
      m_length(0), m_nibbleindex(nibbleindex) {}
