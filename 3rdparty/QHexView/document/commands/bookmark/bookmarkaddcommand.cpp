#include "bookmarkaddcommand.h"

BookMarkAddCommand::BookMarkAddCommand(QHexDocument *doc, qsizetype pos,
                                       QString comment, QUndoCommand *parent)
    : BookMarkCommand(doc, pos, comment, parent) {}

void BookMarkAddCommand::redo() { m_doc->addBookMark(m_pos, m_comment); }

void BookMarkAddCommand::undo() { m_doc->removeBookMark(m_pos); }
